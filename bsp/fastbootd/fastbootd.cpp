// Copyright 2016 http://www.brobwind.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define LOG_TAG		"fastbootd"
#include <fcntl.h>
#include <linux/fs.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <base/command_line.h>
#include <base/memory/weak_ptr.h>
#include <base/strings/string_number_conversions.h>
#include <brillo/daemons/dbus_daemon.h>

#include <android-base/file.h>
#include <android-base/parseint.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <cutils/android_reboot.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <logwrap/logwrap.h>
#include <sparse/sparse.h>

#include "firewalld_firewall.h"
#include "socket.h"


static constexpr const char kPort[] = "port";
static constexpr int kDefaultPort = 5554;
static constexpr int kProtocolVersion = 1;
static constexpr size_t kHandshakeLength = 4;
static constexpr int kMaxDownloadSize = 64 * 1024 * 1024; // 64MB

struct {
    const char *name;
    const char *device;
    const char *type;
} part_info[] = {
    { "boot", "/dev/block/mmcblk0p5", "vfat" },
    { "system", "/dev/block/mmcblk0p6", "ext4" },
    { "data", "/dev/block/mmcblk0p7", "ext4" }
};

// =================================================================
// 1. Punch TCP hole
// =================================================================
class FwDaemon final : public brillo::DBusDaemon {
public:
    FwDaemon(std::unique_ptr<FirewallInterface> firewall, uint16_t port)
            : firewall_{std::move(firewall)}, port_{port} {}

protected:
    int OnInit() override;

private:
    void OnFirewallServiceOnline();

    void OnFirewallSuccess(const std::string& itf_name, uint16_t port, bool allowed);
    void IgnoreFirewallDBusMethodError(brillo::Error* /* error */) {}

    // The firewall service handler.
    const std::unique_ptr<FirewallInterface> firewall_;
    uint16_t port_;

    base::WeakPtrFactory<FwDaemon> weak_ptr_factory_{this};
    DISALLOW_COPY_AND_ASSIGN(FwDaemon);
};

int FwDaemon::OnInit() {
    LOG(INFO) << "Waiting for commands...";
    int return_code = brillo::DBusDaemon::OnInit();
    if (return_code != EX_OK) {
        return return_code;
    }

    firewall_->WaitForServiceAsync(bus_.get(), base::Bind(&FwDaemon::OnFirewallServiceOnline,
            weak_ptr_factory_.GetWeakPtr()));

    return EX_OK;
}

void FwDaemon::OnFirewallSuccess(const std::string& itf_name, uint16_t port, bool allowed) {
    if (allowed) {
        LOG(INFO) << "Successfully opened up port " << port << " on interface: "
                << itf_name;
    } else {
        LOG(ERROR) << "Failed to open up port " << port << ", interface: "
                << itf_name;
    }

    Quit();
}

void FwDaemon::OnFirewallServiceOnline() {
    LOG(INFO) << "Firewall service is on-line. ";
    firewall_->PunchTcpHoleAsync(port_, "",
            base::Bind(&FwDaemon::OnFirewallSuccess, weak_ptr_factory_.GetWeakPtr(), "", port_),
            base::Bind(&FwDaemon::IgnoreFirewallDBusMethodError, weak_ptr_factory_.GetWeakPtr()));
}

// =================================================================
// 2. Reboot to the Brillo system
// =================================================================
static void *thread_reboot(void *arg) {
    int *fastbootd_reboot = (int *)arg;

    usleep(8 * 1000 * 1000);

    if (*fastbootd_reboot) {
        ALOGI("fastbootd: rebooting ...");
        android::base::WriteStringToFile("5", "/sys/module/bcm2709/parameters/reboot_part");
        android_reboot(ANDROID_RB_RESTART, 0, NULL);
    }

    return NULL;
}

// =================================================================
// 3. Handle fastboot request
// =================================================================

// Extract the big-endian 8-byte message length into a 64-bit number.
static uint64_t ExtractMessageLength(const void* buffer) {
    uint64_t ret = 0;
    for (int i = 0; i < 8; ++i) {
        ret |= uint64_t{reinterpret_cast<const uint8_t*>(buffer)[i]} << (56 - i * 8);
    }
    return ret;
}

// Encode the 64-bit number into a big-endian 8-byte message length.
static void EncodeMessageLength(uint64_t length, void* buffer) {
    for (int i = 0; i < 8; ++i) {
        reinterpret_cast<uint8_t*>(buffer)[i] = length >> (56 - i * 8);
    }
}

static bool send_reply(Socket *client, const char *id, const char *fmt, ...) {
    va_list ap;
    char buffer[60];
    size_t length = 0;

    if (strlen(fmt) != 0) {
        va_start(ap, fmt);
        length = vsnprintf(buffer, sizeof(buffer), fmt, ap);
        va_end(ap);
    }

    char header[8];
    EncodeMessageLength(length + 4, header);

    if (strlen(fmt) != 0) {
        return client->Send(std::vector<cutils_socket_buffer_t>{{header, 8},
                {id, 4}, {buffer, length}});
    }

    return client->Send(std::vector<cutils_socket_buffer_t>{{header, 8}, {id, 4}});
}

bool handle_command_flash_boot_partition(Socket *client, const char *file, const char *devname) {
    const char *mountpoint = "/data/misc/fastbootd/mnt";

    send_reply(client, "INFO", "format boot partition ...");
    {
        const char *argv[] = {
            "/system/bin/newfs_msdos",
            "-F", "32",
            "-L", "boot",
            "-b", "512",
            "-A", devname,
        };
        int status;

        android_fork_execvp(sizeof(argv) / sizeof(argv[0]), (char **)argv, &status, true, true);
    }

    send_reply(client, "INFO", "mount boot partition ...");
    {
        int rc = mount(devname, mountpoint, "vfat", MS_NODEV | MS_NOSUID | MS_DIRSYNC,
                "utf8,uid=0,gid=0,shortname=mixed");
        if (rc != 0) {
            send_reply(client, "FAIL", "failed to mount boot partition");
            return false;
        }
    }

    send_reply(client, "INFO", "extract files to boot partition ...");
    {
        const char *argv[] = {
            "/system/bin/tar",
            "-C", mountpoint,
            "-xvf", file,
        };
        int status;

        android_fork_execvp(sizeof(argv) / sizeof(argv[0]), (char **)argv, &status, true, true);
    }

    send_reply(client, "INFO", "umount boot partition ...");
    umount(mountpoint);

    sync();

    return send_reply(client, "OKAY", "");
}

bool handle_command(Socket *client, std::string cmd, std::vector<std::string> args) {
    const char *trampfile = "/data/misc/fastbootd/mid.bin";

    if (cmd == "getvar") {
        if (args[0] == "max-download-size") {
            return send_reply(client, "OKAY", "%d", kMaxDownloadSize);
        } else if (args[0] == "partition-type") {
            for (size_t i = 0; i < sizeof(part_info) / sizeof(part_info[0]); i++) {
                if (args[1] == part_info[i].name) {
                    return send_reply(client, "OKAY", part_info[i].type);
                }
            }
        } else if (args[0] == "product") {
            char property[PROPERTY_VALUE_MAX];
            property_get("ro.product.board", property, "");
            return send_reply(client, "OKAY", property);
        } else if (args[0] == "serialno") {
            char property[PROPERTY_VALUE_MAX];
            property_get("ro.serialno", property, "");
            return send_reply(client, "OKAY", property);
        } else if (args[0] == "version-bootloader") {
            return send_reply(client, "OKAY", "0.1");
        }
        return send_reply(client, "OKAY", "");
    } else if (cmd == "download") {
        uint32_t size = strtol(args[0].c_str(), 0, 16);
        send_reply(client, "DATA", "%08x", size);

        int fd = open(trampfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0) {
            send_reply(client, "FAIL", "fail to create trampoline file to store data!");
            return false;
        }

        while (size > 0) {
            char buffer[4096];
            ssize_t read = client->Receive(buffer, 8, 0);
            if (read != 8) {
                send_reply(client, "FAIL", "fail to receive data!");
                close(fd);
                return false;
            }
            size_t length = ExtractMessageLength(buffer);
            do {
                read = client->Receive(buffer, std::min(length, sizeof(buffer)), 0);
                if (read < 0) {
                    close(fd);
                    return false;
                }

                write(fd, buffer, read);

                length -= read;
                size -= read;
            } while (length > 0);
        }

        close(fd);

        return send_reply(client, "OKAY", "");
    } else if (cmd == "flash") {
        std::unique_ptr<char, int (*)(const char *)> tmpfile((char *)trampfile, unlink);
        int fd = open(tmpfile.get(), O_RDONLY);
        if (fd < 0) {
            send_reply(client, "FAIL", "please run download command first!");
            return false;
        }

        const char *devname = NULL;
        const char *partname = NULL;
        for (size_t i = 0; i < sizeof(part_info) / sizeof(part_info[0]); i++) {
            if (args[0] == part_info[i].name) {
                devname = part_info[i].device;
                partname = part_info[i].name;
                break;
            }
        }

        if (devname == NULL) {
            close(fd);
            send_reply(client, "FAIL", "partition: %s does not exist!", args[0].c_str());
            return false;
        }

        if (!strcmp("boot", partname)) {
            close(fd);
            return handle_command_flash_boot_partition(client, tmpfile.get(), devname);
        }

        int fddev = open(devname, O_WRONLY | O_CREAT, 0600);
        if (fddev < 0) {
            close(fd);
            send_reply(client, "FAIL", "failed to open partition: %s", args[0].c_str());
            return false;
        }

        struct sparse_file *s = sparse_file_import(fd, true, false);
        if (!s) {
            close(fd);
            close(fddev);

            send_reply(client, "FAIL", "failed to read sparse file!");
            return false;
        }

        sparse_file_write(s, fddev, false, false, false);
        sparse_file_destroy(s);

        close(fd);
        close(fddev);

        sync();

        return send_reply(client, "OKAY", "");
    } else if (cmd == "erase") {
        const char *devname = NULL;
        for (size_t i = 0; i < sizeof(part_info) / sizeof(part_info[0]); i++) {
            if (args[0] == part_info[i].name) {
                devname = part_info[i].device;
                break;
            }
        }

        if (devname == NULL) {
            send_reply(client, "FAIL", "partition: %s does not exist!", args[0].c_str());
            return false;
        }

        uint64_t devsize = 0;
        int fd = open(devname, O_RDONLY);
        ioctl(fd, BLKGETSIZE64, &devsize);

        const uint64_t blksize = 64 * 1024;
        const uint64_t numblk = (devsize + blksize - 1) / blksize;
        const uint64_t updsize = (numblk / 10) * blksize;
        for (uint64_t offset = 0; offset < devsize; offset += updsize) {
            uint64_t realsize = std::min(updsize, devsize - offset);
            const char *argv[] = {
                "/system/bin/dd",
                "if=/dev/zero",
                android::base::StringPrintf("of=%s", devname).c_str(),
                android::base::StringPrintf("seek=%lld", offset).c_str(),
                android::base::StringPrintf("bs=%lld", realsize).c_str(),
                "count=1",
            };
            int status;

            android_fork_execvp(sizeof(argv) / sizeof(argv[0]), (char **)argv, &status, true, true);
            send_reply(client, "INFO", android::base::StringPrintf("erase %s: %3lld/100",
                    devname, (offset + realsize) * 100 / devsize).c_str());
        }

        return send_reply(client, "OKAY", "");
    } else if (cmd == "continue") {
        android::base::WriteStringToFile("5", "/sys/module/bcm2709/parameters/reboot_part");
        android_reboot(ANDROID_RB_RESTART, 0, NULL);
//        while (true) { pause(); }
        return send_reply(client, "OKAY", "");
    } else if (cmd == "reboot" || cmd == "reboot-bootloader") {
        android::base::WriteStringToFile("0", "/sys/module/bcm2709/parameters/reboot_part");
        android_reboot(ANDROID_RB_RESTART, 0, NULL);
//        while (true) { pause(); }
        return send_reply(client, "OKAY", "");
    }

    return send_reply(client, "FAIL", "unknown command: %s", cmd.c_str());
}

void command_loop(Socket *client) {
    char buffer[64];

    while (true) {
        if (client->Receive(buffer, 8, 0) != 8) {
            return;
        }

        uint64_t length = ExtractMessageLength(buffer);
        if (length > 64) {
            ALOGE("Wrong message length: %" PRId64, length);
            return;
        }

        ssize_t read = client->Receive(buffer, length, 0);
        if (read != (ssize_t)length) {
            ALOGE("Failed to retrive command.");
            return;
        }

        std::vector<std::string> cmd = android::base::Split(std::string(buffer, read), ":");
        if (handle_command(client, cmd[0], std::vector<std::string>(cmd.begin() + 1, cmd.end())) != true) {
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    base::CommandLine::Init(argc, argv);
    base::CommandLine* cl = base::CommandLine::ForCurrentProcess();

    int32_t port = kDefaultPort;
    if (cl->HasSwitch(kPort)) {
        std::string value = cl->GetSwitchValueASCII(kPort);
        base::StringToInt(value, &port);
    }

    // Puch a TCP hole to accept connection
    FwDaemon daemon(std::unique_ptr<FirewallInterface>{new FirewalldFirewall()}, (uint16_t)port);
    daemon.Run();

    std::unique_ptr<Socket> server = Socket::NewServer(Socket::Protocol::kTcp, port);
    if (server == nullptr) {
        ALOGE("Failed to create fastbootd service.");
        return 1;
    }

    int fastbootd_reboot = 0;
    if (property_get_bool("persist.sys.fastbootd.reboot", 1)) {
        pthread_t hreboot;
        pthread_create(&hreboot, NULL, thread_reboot, &fastbootd_reboot);
        fastbootd_reboot = 1;
    }

    // Check if in demo mode
    if (fastbootd_reboot == 1) {
        struct stat st;
        int result = stat("/boot/DEMO", &st);
        if (result == 0 && S_ISREG(st.st_mode)) {
            fastbootd_reboot = 0;
        }
    }

    std::string handshake_message(android::base::StringPrintf("FB%02d", kProtocolVersion));
    while (true) {
        std::unique_ptr<Socket> client = server->Accept();
        if (client == nullptr) {
            ALOGE("Failed to accept client connection.");
            continue;
        }

        char buffer[4];
        ssize_t bytes = client->Receive(buffer, sizeof(buffer), 500);
        if (bytes != 4) {
            ALOGE("Failed to get client version.");
            continue;
        }

        if (memcmp(buffer, "FB01", 4) != 0) {
            ALOGE("Unsupported client: %c%c%c%c", buffer[0], buffer[1], buffer[2], buffer[3]);
            continue;
        }

        if (fastbootd_reboot) {
            fastbootd_reboot = 0;
            property_set("persist.sys.fastbootd.reboot", "0");
        }

        if (!client->Send(handshake_message.c_str(), kHandshakeLength)) {
            ALOGE("Failed to send handshake.");
            continue;
        }

        command_loop(client.get());
    }

    return 0;
}
