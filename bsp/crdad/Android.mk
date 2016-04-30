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

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := regulatory.bin
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := \
	$(TARGET_OUT)/vendor/lib/crda
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := benh@debian.org.key.pub.pem
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := pubkeys/$(LOCAL_MODULE)
LOCAL_MODULE_PATH := \
	$(TARGET_OUT)/vendor/lib/crda/pubkeys
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := linville.key.pub.pem
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := pubkeys/$(LOCAL_MODULE)
LOCAL_MODULE_PATH := \
	$(TARGET_OUT)/vendor/lib/crda/pubkeys
include $(BUILD_PREBUILT)

#commit 805d84ac2e8d193583dedafa4cf885de22a5f92a
#Author: Seth Forshee <seth.forshee@canonical.com>
#Date:   Fri Dec 5 13:30:39 2014 -0600
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := crda

LOCAL_SRC_FILES := \
	crda.c reglib.c

LOCAL_SHARED_LIBRARIES := \
	libnl libcrypto libssl

LOCAL_CFLAGS := \
	-DCONFIG_LIBNL20 -DUSE_OPENSSL \
	-DPUBKEY_DIR="\"/vendor/lib/crda/pubkeys\""

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := crdad
LOCAL_INIT_RC := crdad.rc

LOCAL_SRC_FILES := \
	crdad.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils liblog liblogwrap \
	libbrillo \
	libbrillo-minijail \
	libchrome \
	libminijail

LOCAL_REQUIRED_MODULES := \
	regulatory.bin \
	benh@debian.org.key.pub.pem \
	linville.key.pub.pem \
	crda

include $(BUILD_EXECUTABLE)
