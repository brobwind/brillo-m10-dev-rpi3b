/*
 * Copyright (C) 2016 http://www.brobwind.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "crdad"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#include <linux/capability.h>

#include <string>
#include <vector>

#include <brillo/minijail/minijail.h>
#include <cutils/log.h>
#include <cutils/uevent.h>
#include <logwrap/logwrap.h>


#define MAX_EPOLL_EVENTS 40
#define UEVENT_MSG_LEN 2048


static int epollfd;
static int eventct;
static int uevent_fd;

static void handle_crda_update(const char *country) {
    setenv("COUNTRY", country, 1);

    brillo::Minijail* m = brillo::Minijail::GetInstance();
    minijail* jail = m->New();
    m->UseCapabilities(jail, CAP_TO_MASK(CAP_NET_ADMIN) | CAP_TO_MASK(CAP_NET_RAW));

    std::vector<char*> args;
    args.push_back(const_cast<char*>("/system/bin/crda"));
    args.push_back(nullptr);

    int status;
    m->RunSyncAndDestroy(jail, args, &status);
}

static int crdad_register_event(int fd, void (*handler)(uint32_t)) {
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLWAKEUP;
    ev.data.ptr = (void *)handler;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        ALOGE("epoll_ctl failed; errno=%d\n", errno);
        return -1;
    }

    eventct++;
    return 0;
}

static void uevent_event(uint32_t /*epevents*/) {
    char msg[UEVENT_MSG_LEN+2];
    char *cp;
    int n;
    const char *devpath = "", *action = "";
    const char *subsystem = "", *country = "";

    n = uevent_kernel_multicast_recv(uevent_fd, msg, UEVENT_MSG_LEN);
    if (n <= 0)
        return;
    if (n >= UEVENT_MSG_LEN)   /* overflow -- discard */
        return;

    msg[n] = '\0';
    msg[n+1] = '\0';
    cp = msg;

    while (*cp) {
        if(!strncmp(cp, "ACTION=", 7)) {
            cp += 7;
            action = cp;
        } else if(!strncmp(cp, "DEVPATH=", 8)) {
            cp += 8;
            devpath = cp;
        } else if(!strncmp(cp, "SUBSYSTEM=", 10)) {
            cp += 10;
            subsystem = cp;
        } else if(!strncmp(cp, "COUNTRY=", 8)) {
            cp += 8;
            country = cp;
        }

        /* advance to after the next \0 */
        while (*cp++)
            ;
    }

#define DEVPATH "/devices/platform/regulatory"
    if (!strncmp(devpath, DEVPATH, strlen(DEVPATH)) && !strcmp(action, "change") &&
            !strcmp(subsystem, "platform")) {
        handle_crda_update(country);
    }
#undef DEVPATH
}

static void uevent_init(void) {
    uevent_fd = uevent_open_socket(64*1024, true);
    if (uevent_fd < 0) {
        ALOGE("uevent_init: uevent_open_socket failed");
        return;
    }

    fcntl(uevent_fd, F_SETFL, O_NONBLOCK);
    if (crdad_register_event(uevent_fd, uevent_event)) {
        ALOGE("register for uevent events failed");
    }
}

static void crdad_mainloop(void) {
    while (1) {
        struct epoll_event events[eventct];
        int nevents;

        nevents = epoll_wait(epollfd, events, eventct, -1);
        if (nevents == -1) {
            if (errno == EINTR)
                continue;
            ALOGE("crdad_mainloop: epoll_wait failed");
            break;
        }

        for (int n = 0; n < nevents; ++n) {
            if (events[n].data.ptr)
                (*(void (*)(int))events[n].data.ptr)(events[n].events);
        }
    }

    return;
}

int main(int argc, char *argv[]) {
    epollfd = epoll_create(MAX_EPOLL_EVENTS);
    if (epollfd == -1) {
        ALOGE("epoll_create failed; errno=%d", errno);
        return -1;
    }

    uevent_init();

    crdad_mainloop();
    ALOGE("Main loop terminated, exiting");

    return 1;
}
