COMMON_FOLDER := device/amazon/omap4-common
OTTER_COMMON_FOLDER := device/amazon/otter-common

$(call inherit-product, $(COMMON_FOLDER)/common.mk)

PRODUCT_AAPT_CONFIG := normal mdpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

# Wifi
PRODUCT_PACKAGES += \
    calibrator \
    crda \
    regulatory.bin \
    lib_driver_cmd_wl12xx \

# Rootfs
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/init.omap4430.usb.rc:/root/init.omap4430.usb.rc \
    $(OTTER_COMMON_FOLDER)/ueventd.omap4430.rc:/root/ueventd.omap4430.rc \

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \

# Prebuilts /system/bin
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/prebuilt/bin/bbx:/system/bin/bbx \
    $(OTTER_COMMON_FOLDER)/prebuilt/bin/battery_log.sh:system/bin/battery_log.sh \
    $(OTTER_COMMON_FOLDER)/prebuilt/bin/klog.sh:system/bin/klog.sh \
    $(OTTER_COMMON_FOLDER)/prebuilt/bin/temperature_log.sh:system/bin/temperature_log.sh

# Prebuilt /system/etc/firmware
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/firmware/ducati-m3.512MB.bin:system/etc/firmware/ducati-m3.512MB.bin \
    $(OTTER_COMMON_FOLDER)/firmware/ducati-m3.bin:system/etc/firmware/ducati-m3.bin.bak \
    $(OTTER_COMMON_FOLDER)/firmware/ti-connectivity/wl127x-fw-4-mr.bin:system/etc/firmware/ti-connectivity/wl127x-fw-4-mr.bin \
    $(OTTER_COMMON_FOLDER)/firmware/ti-connectivity/wl127x-fw-4-plt.bin:system/etc/firmware/ti-connectivity/wl127x-fw-4-plt.bin \
    $(OTTER_COMMON_FOLDER)/firmware/ti-connectivity/wl127x-fw-4-sr.bin:system/etc/firmware/ti-connectivity/wl127x-fw-4-sr.bin \
    $(OTTER_COMMON_FOLDER)/firmware/ti-connectivity/wl1271-nvs_127x.bin:system/etc/firmware/ti-connectivity/wl1271-nvs_127x.bin \
    $(OTTER_COMMON_FOLDER)/firmware/ti-connectivity/TQS_S_2.6.ini:/system/etc/wifi/TQS_S_2.6.ini \

# Prebuilts /system/etc
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/prebuilt/etc/audio_policy.conf:/system/etc/audio_policy.conf \
    $(OTTER_COMMON_FOLDER)/prebuilt/etc/media_codecs.xml:/system/etc/media_codecs.xml \
    $(OTTER_COMMON_FOLDER)/prebuilt/etc/media_profiles.xml:/system/etc/media_profiles.xml \

# Prebuilt /system/usr
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/prebuilt/usr/idc/ilitek_i2c.idc:/system/usr/idc/ilitek_i2c.idc \
    $(OTTER_COMMON_FOLDER)/prebuilt/usr/idc/twl6030_pwrbutton.idc:/system/usr/idc/twl6030_pwrbutton.idc \
    $(OTTER_COMMON_FOLDER)/prebuilt/usr/keylayout/twl6030_pwrbutton.kl:/system/usr/keylayout/twl6030_pwrbutton.kl \

PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory \
    Superuser \
    su

# AOSP specific
ifeq ($(findstring full_otter, $(TARGET_PRODUCT)),full_otter)
# Prebuilt /system/media
PRODUCT_COPY_FILES += \
    $(OTTER_COMMON_FOLDER)/prebuilt/media/bootanimation.zip:/system/media/bootanimation.zip
endif

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=160 \
    persist.lab126.chargeprotect=1 \
    ro.sf.hwrotation=270 \

$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)
$(call inherit-product-if-exists, $(COMMON_FOLDER)/imgtec/sgx-imgtec-bins-540.mk)
