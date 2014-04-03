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

# Inherit some common cyanogenmod stuff.
$(call inherit-product-if-exists, vendor/cm/config/common_full_tablet_wifionly.mk)

#TARGET_SCREEN_WIDTH := 600
TARGET_SCREEN_WIDTH := 480
TARGET_SCREEN_HEIGHT := 1024
TARGET_BOOTANIMATION_USE_RGB565 := true
TARGET_CONTINUOUS_SPLASH_ENABLED := true

DEVICE_PACKAGE_OVERLAYS += device/amazon/otter-common/overlay/cm

