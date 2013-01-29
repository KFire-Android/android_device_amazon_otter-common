# Inherit common product files.
$(call inherit-product, vendor/aokp/configs/common_tablet.mk)

DEVICE_PACKAGE_OVERLAYS += device/amazon/otter-common/overlay/aokp

PRODUCT_COPY_FILES +=  \
    vendor/aokp/prebuilt/bootanimation/bootanimation_600_1024.zip:system/media/bootanimation.zip

