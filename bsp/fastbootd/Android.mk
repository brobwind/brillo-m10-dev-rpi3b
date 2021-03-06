# Copyright (C) 2016 http://www.brobwind.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#	  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fastbootd
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	firewalld_firewall.cpp \
	socket.cpp \
	fastbootd.cpp

LOCAL_STATIC_LIBRARIES := \
	libutils liblog \
	libbase libcutils \
	libselinux \
	libsparse_static libz

LOCAL_SHARED_LIBRARIES := \
	libdbus libbrillo libchrome \
	libbrillo-dbus libchrome-dbus \
	libfirewalld-client \
	liblogwrap

LOCAL_STRIP_MODULE := keep_symbols
LOCAL_INIT_RC := fastbootd.rc

include $(BUILD_EXECUTABLE)
