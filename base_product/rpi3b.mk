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

$(call inherit-product, device/generic/brillo/brillo_base.mk)

PRODUCT_NAME	:= rpi3b
PRODUCT_BRAND	:= Brillo
PRODUCT_DEVICE	:= rpi3b

# Install rpi-specific config file for weaved.
PRODUCT_COPY_FILES += \
  device/hzak/rpi3b/base_product/weaved.conf:system/etc/weaved/weaved.conf

# Debug factility
PRODUCT_PACKAGES += \
	debuggerd

PRODUCT_PACKAGES += \
    keystore.default

# Audio utilities. You may not need these for a product.
PRODUCT_PACKAGES += \
    tinyplay tinypcminfo tinymix tinycap

# Wifi hal module
PRODUCT_PACKAGES += \
	wifi_driver.rpi

# Bluetooth hal module
PRODUCT_PACKAGES += \
	bt_bcm_rpi3b

# For the TPM simulator.
PRODUCT_PACKAGES += \
	libtpm2

# Primary audio HAL
PRODUCT_PACKAGES += \
    audio.primary.rpi

# Effect libraries
PRODUCT_PACKAGES += \
	libbundlewrapper \
	libreverbwrapper \
	libvisualizer \
	libdownmix \
	libldnhncr

PRODUCT_COPY_FILES += \
	frameworks/av/media/libeffects/data/audio_effects.conf:system/etc/audio_effects.conf

# CRDA
PRODUCT_PACKAGES += \
	crdad

# Demo project - ledflasher
BOARD_SEPOLICY_DIRS += \
	product/google/example-ledflasher/sepolicy

PRODUCT_PACKAGES += \
	lights.rpi \
	ledflasher \
	ledservice
