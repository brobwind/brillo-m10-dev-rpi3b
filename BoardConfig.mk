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

# Select the SoC
include device/hzak/rpi3b/bsp/soc.mk

TARGET_USERIMAGES_USE_EXT4				:= true
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE		:= ext4
TARGET_USERIMAGES_SPARSE_EXT_DISABLED	:= true

BOARD_FLASH_BLOCK_SIZE	:= 512
BOARD_NAND_PAGE_SIZE	:= 512
BOARD_NAND_SPARE_SIZE	:= 64

BOARD_SYSTEMIMAGE_PARTITION_SIZE	:= $(shell echo $$((256 * 1024 * 1024)))
BOARD_USERDATAIMAGE_PARTITION_SIZE	:= $(shell echo $$((256 * 1024 * 1024)))
BOARD_CACHEIMAGE_PARTITION_SIZE		:= $(shell echo $$((256 * 1024 * 1024)))

# For android_filesystem_config.h
# This configures filesystem capabilities.
TARGET_ANDROID_FILESYSTEM_CONFIG_H := \
	device/hzak/rpi3b/fs_config/android_filesystem_config.h
