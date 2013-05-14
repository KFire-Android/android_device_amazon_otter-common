# Copyright (C) 2013 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

COMMON_FOLDER := device/amazon/omap4-common
OTTER_COMMON_FOLDER := device/amazon/otter-common

$(call inherit-product, $(COMMON_FOLDER)/common.mk)

DEVICE_PACKAGE_OVERLAYS += $(OTTER_COMMON_FOLDER)/overlay/aosp

PRODUCT_AAPT_CONFIG := normal mdpi hdpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

# Wifi
PRODUCT_PACKAGES += \
    calibrator \
    crda \
    regulatory.bin \
    lib_driver_cmd_wl12xx

# Rootfs
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/ueventd.omap4430.rc:/root/ueventd.omap4430.rc

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml


# Prebuilts /system/etc
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/prebuilt/etc/audio_policy.conf:/system/etc/audio_policy.conf \
    $(OTTER_COMMON_FOLDER)/prebuilt/etc/media_codecs.xml:/system/etc/media_codecs.xml \
    $(OTTER_COMMON_FOLDER)/prebuilt/etc/media_profiles.xml:/system/etc/media_profiles.xml \
    $(OTTER_COMMON_FOLDER)/prebuilt/etc/mixer_paths.xml:/system/etc/mixer_paths.xml

# Prebuilt /system/usr
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/prebuilt/usr/idc/ilitek_i2c.idc:/system/usr/idc/ilitek_i2c.idc \
    $(OTTER_COMMON_FOLDER)/prebuilt/usr/idc/twl6030_pwrbutton.idc:/system/usr/idc/twl6030_pwrbutton.idc \
    $(OTTER_COMMON_FOLDER)/prebuilt/usr/keylayout/twl6030_pwrbutton.kl:/system/usr/keylayout/twl6030_pwrbutton.kl

# AOSP specific
ifeq ($(findstring full_otter, $(TARGET_PRODUCT)),full_otter)
# Prebuilt /system/media
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/prebuilt/media/bootanimation.zip:/system/media/bootanimation.zip
endif

PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory \
    Superuser \
    su

# TI OMAP4
PRODUCT_PACKAGES += \
    libdomx \
    libOMX_Core \
    libOMX.TI.DUCATI1.VIDEO.H264E \
    libOMX.TI.DUCATI1.VIDEO.MPEG4E \
    libOMX.TI.DUCATI1.VIDEO.DECODER \
    libOMX.TI.DUCATI1.VIDEO.DECODER.secure \
    libOMX.TI.DUCATI1.VIDEO.CAMERA \
    libOMX.TI.DUCATI1.MISC.SAMPLE \
    libstagefrighthw \
    libI420colorconvert \
    libtiutils \
    libcamera \
    libion_ti \
    libomxcameraadapter \
    smc_pa_ctrl \
    tf_daemon \
    libtf_crypto_sst

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=160 \
    persist.lab126.chargeprotect=1 \
    ro.sf.hwrotation=270

$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)
$(call inherit-product-if-exists, vendor/imgtec/amazon-common/device-partial-540.mk)
$(call inherit-product-if-exists, vendor/amazon/otter-common/otter-common-vendor.mk)
