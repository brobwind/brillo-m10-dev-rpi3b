#
# Copyright 2016 http://www.brobwind.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := device/hzak/rpi3b/bsp

# Arm32 device.
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a
TARGET_CPU_VARIANT := generic
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_KERNEL_ARCH := $(TARGET_ARCH)

# Do not build recovery image
TARGET_NO_RECOVERY := true
TARGET_NO_BOOTLOADER := true
TARGET_NO_KERNEL := false

BOARD_KERNEL_CMDLINE := \
	dwc_otg.lpm_enable=0 console=serial0,115200 console=tty1 \
	elevator=deadline rootwait \
	androidboot.hardware=rpi androidboot.selinux=permissive

TARGET_KERNEL_SRC := hardware/bsp/kernel/hzak/rpi-4.1.y
TARGET_KERNEL_DEFCONFIG := bcm2709_defconfig
TARGET_KERNEL_CONFIGS := $(realpath $(LOCAL_PATH)/soc.kconf)
TARGET_KERNEL_DTB := bcm2710-rpi-3-b.dtb

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	ro.logd.size=$(shell echo $$((1024 * 1024))) \
	ro.rfkilldisabled=1 \
	adbd-setup.autostart=1 \
	service.adb.tcp.port=5555

# For recovery boot partition
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/boot/LICENCE.broadcom:rboot/LICENCE.broadcom \
	$(LOCAL_PATH)/boot/COPYING.linux:rboot/COPYING.linux \
	$(LOCAL_PATH)/boot/bootcode.bin:rboot/bootcode.bin \
	$(LOCAL_PATH)/boot/recovery.elf:rboot/recovery.elf \
	$(LOCAL_PATH)/boot/recovery.cmdline:rboot/recovery.cmdline

# For Brillo boot partition
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/boot/LICENCE.broadcom:boot/LICENCE.broadcom \
	$(LOCAL_PATH)/boot/COPYING.linux:boot/COPYING.linux \
	$(LOCAL_PATH)/boot/bootcode.bin:boot/bootcode.bin \
	$(LOCAL_PATH)/boot/config.txt:boot/config.txt \
	$(LOCAL_PATH)/boot/start.elf:boot/start.elf \
	$(LOCAL_PATH)/boot/fixup.dat:boot/fixup.dat \
	$(LOCAL_PATH)/boot/cmdline-rpi-2-b.txt:boot/cmdline-rpi-2-b.txt \
	$(LOCAL_PATH)/boot/cmdline-rpi-3-b.txt:boot/cmdline-rpi-3-b.txt

PRODUCT_COPY_FILES += \
	system/core/rootdir/ueventd.rc:root/ueventd.rc \
	$(LOCAL_PATH)/init.rpi.rc:root/init.rpi.rc \
	$(LOCAL_PATH)/ueventd.rpi.rc:root/ueventd.rpi.rc \
	$(LOCAL_PATH)/fstab.device:root/fstab.rpi

# BCM wifi
WIFI_DRIVER_HAL_MODULE := wifi_driver.rpi
WIFI_DRIVER_HAL_PERIPHERAL := bcm43438

# BCM bluetooth
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_CUSTOM_BT_CONFIG := $(LOCAL_PATH)/bluetooth/vnd_rpi.txt

BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := \
	$(LOCAL_PATH)/bluetooth

# Audio configuration files
PRODUCT_COPY_FILES += \
	frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/audio/media_codecs.xml:system/etc/media_codecs.xml \
	$(LOCAL_PATH)/audio/audio_policy.conf:system/etc/audio_policy.conf

BOARD_SEPOLICY_DIRS += \
	$(LOCAL_PATH)/sepolicy \
