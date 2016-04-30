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

ifeq ($(WIFI_DRIVER_HAL_PERIPHERAL),bcm43438)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := brcmfmac43430-sdio.txt
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_MODULE_PATH := \
	$(TARGET_OUT)/vendor/firmware/brcm

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := brcmfmac43430-sdio.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_MODULE_PATH := \
	$(TARGET_OUT)/vendor/firmware/brcm

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := $(WIFI_DRIVER_HAL_MODULE)

LOCAL_SRC_FILES := \
	bcm43438_hal.cpp

LOCAL_C_INCLUDES := \
	device/generic/brillo/wifi_driver_hal/include

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_REQUIRED_MODULES := \
	brcmfmac43430-sdio.txt \
	brcmfmac43430-sdio.bin

include $(BUILD_SHARED_LIBRARY)

endif
