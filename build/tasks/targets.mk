# Copyright (C) 2016 http://www.brobwind.com
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

LOCAL_PATH := $(call my-dir)

RPI_KERNEL_DTBS := \
	bcm2709-rpi-2-b.dtb \
	bcm2710-rpi-3-b.dtb

RPI_RECOVERY_OUT		:= $(PRODUCT_OUT)/rboot
RPI_RECOVERY_KERNEL		:= $(RPI_RECOVERY_OUT)/recovery7.img
RPI_RECOVERY_RAMDISK	:= $(RPI_RECOVERY_OUT)/recovery.rfs
RPI_RECOVERY_FILES		:= \
	$(filter $(RPI_RECOVERY_OUT)/%,$(ALL_PREBUILT) $(ALL_DEFAULT_INSTALLED_MODULES)) \
	$(addprefix $(RPI_RECOVERY_OUT)/,$(RPI_KERNEL_DTBS)) \
	$(RPI_RECOVERY_KERNEL) $(RPI_RECOVERY_RAMDISK)

RPI_BOOT_OUT			:= $(PRODUCT_OUT)/boot
RPI_BOOT_KERNEL			:= $(RPI_BOOT_OUT)/kernel7.img
RPI_BOOT_RAMDISK		:= $(RPI_BOOT_OUT)/ramdisk7.img
RPI_BOOT_FILES			:= \
	$(filter $(RPI_BOOT_OUT)/%,$(ALL_PREBUILT) $(ALL_DEFAULT_INSTALLED_MODULES)) \
	$(addprefix $(RPI_BOOT_OUT)/,$(RPI_KERNEL_DTBS)) \
	$(RPI_BOOT_KERNEL) $(RPI_BOOT_RAMDISK)

define build-rpi3b-kernel-dtb
$(RPI_RECOVERY_OUT)/$(1): $(KERNEL_BIN) | $(ACP)
	$(hide)$(ACP) -fp $(KERNEL_OUT)/arch/arm/boot/dts/$$(notdir $$@) $$@

$(RPI_BOOT_OUT)/$(1): $(KERNEL_BIN) | $(ACP)
	$(hide)$(ACP) -fp $(KERNEL_OUT)/arch/arm/boot/dts/$$(notdir $$@) $$@
endef

$(foreach item,$(RPI_KERNEL_DTBS),$(eval $(call build-rpi3b-kernel-dtb,$(item))))

$(RPI_RECOVERY_KERNEL) $(RPI_BOOT_KERNEL): $(INSTALLED_KERNEL_TARGET) | $(ACP)
	$(hide)$(ACP) -fp $< $@

$(RPI_RECOVERY_RAMDISK) $(RPI_BOOT_RAMDISK): $(INSTALLED_RAMDISK_TARGET) | $(ACP)
	$(hide)$(ACP) -fp $< $@

$(INSTALLED_BOOTIMAGE_TARGET) bootimage-nodeps: $(RPI_RECOVERY_FILES) $(RPI_BOOT_FILES)
