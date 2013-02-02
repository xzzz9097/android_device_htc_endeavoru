#
# Copyright (C) 2012 The Android Open-Source Project
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

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := $(LOCAL_PATH)/prebuilt/kernel
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

DEVICE_PACKAGE_OVERLAYS := $(LOCAL_PATH)/overlay

# Set secure on user builds
ifeq ($(TARGET_BUILT_VARIANT),user)
ADDITIONAL_DEFAULT_PROPERTIES += \
	ro.secure=1
else
ADDITIONAL_DEFAULT_PROPERTIES += \
	ro.secure=0
endif

# This device is xhdpi.
PRODUCT_AAPT_CONFIG := normal hdpi xhdpi
PRODUCT_AAPT_PREF_CONFIG := xhdpi
PRODUCT_LOCALES += en_US xhdpi

# Init files
PRODUCT_COPY_FILES := \
	$(LOCAL_KERNEL):kernel \
	$(LOCAL_PATH)/ramdisk/init.rc:root/init.rc \
	$(LOCAL_PATH)/ramdisk/init.usb.rc:root/init.usb.rc \
	$(LOCAL_PATH)/ramdisk/init.endeavoru.rc:root/init.endeavoru.rc \
	$(LOCAL_PATH)/ramdisk/ueventd.endeavoru.rc:root/ueventd.endeavoru.rc \
        $(LOCAL_PATH)/ramdisk/fstab.endeavoru:root/fstab.endeavoru \
        $(LOCAL_PATH)/ramdisk/endeavoru_mounthelper.sh:root/endeavoru_mounthelper.sh

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/configs/gps.conf:system/etc/gps.conf \
	$(LOCAL_PATH)/configs/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
	$(LOCAL_PATH)/configs/hostapd.conf:system/etc/wifi/hostapd.conf \
	$(LOCAL_PATH)/configs/nvcamera.conf:system/etc/nvcamera.conf \
	$(LOCAL_PATH)/configs/asound.conf:system/etc/asound.conf \
	$(LOCAL_PATH)/configs/enctune.conf:system/etc/enctune.conf \
	$(LOCAL_PATH)/configs/DSP_number.txt:system/etc/DSP_number.txt \
	$(LOCAL_PATH)/configs/AIC3008_REG_DualMic_XC.csv:system/etc/AIC3008_REG_DualMic_XC.csv \
	$(LOCAL_PATH)/configs/AIC3008_REG_DualMic.csv:system/etc/AIC3008_REG_DualMic.csv \
	$(LOCAL_PATH)/configs/alsa.conf:system/usr/share/alsa/alsa.conf

# GPS Certificate
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/prebuilt/etc/SuplRootCert:system/etc/SuplRootCert

# Wifi
PRODUCT_PACKAGES += \
	TQS_D_1.7.ini \
	calibrator \
        hostapd \
        hostapd_cli \
        iw

# Modules
PRODUCT_COPY_FILES += \
        $(foreach f,$(wildcard $(LOCAL_PATH)/prebuilt/modules/*),$(f):system/lib/modules/$(notdir $(f)))

# Vold
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/vold.fstab:system/etc/vold.fstab

# Lights
PRODUCT_PACKAGES += \
	lights.endeavoru

# Keyboard
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/prebuilt/usr/keylayout/qwerty.kl:system/usr/keylayout/qwerty.kl

# NFC
PRODUCT_PACKAGES += \
	libnfc \
	libnfc_jni \
	Nfc \
	Tag \
	com.android.nfc_extras \
	com.android.future.usb.accessory

# Live Wallpapers
PRODUCT_PACKAGES += \
	LiveWallpapers \
	LiveWallpapersPicker \
	VisualizationWallpapers \
	librs_jni

# Audio
PRODUCT_PACKAGES += \
	audio.a2dp.default \
	libaudioutils \
	libtinyalsa \
        tinymix \
        tinyplay \
        tinycap

# Pollyd
PRODUCT_PACKAGES += \
	pollyd \
	Polly

# hox tools
PRODUCT_PACKAGES += \
	ugexec \
	libbt-vendor \
	hox-uim-sysfs

# Filesystem management tools
PRODUCT_PACKAGES += \
	make_ext4fs \
	setup_fs

# Custom Packages
PRODUCT_PACKAGES += \
        Music \
        CMFileManager \
        CellBroadcastReceiver

# Increase the HWUI font cache
PRODUCT_PROPERTY_OVERRIDES += \
        ro.hwui.text_cache_width=2048 \
        ro.hwui.text_cache_height=256

# Power
PRODUCT_PACKAGES += \
        power.endeavoru
		
# Permissions
PRODUCT_COPY_FILES += \
        frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
        frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
        frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
        frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
        frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
        frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
        frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
        frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
        frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
        frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
        frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
        frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
        frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
        frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
        frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
        frameworks/base/nfc-extras/com.android.nfc_extras.xml:system/etc/permissions/com.android.nfc_extras.xml \
	packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml


PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/prebuilt/usr/idc/atmel-maxtouch.idc:system/usr/idc/atmel-maxtouch.idc \
	$(LOCAL_PATH)/prebuilt/usr/idc/synaptics-rmi-touchscreen.idc:system/usr/idc/synaptics-rmi-touchscreen.idc \
	$(LOCAL_PATH)/prebuilt/usr/idc/tv-touchscreen.idc:system/usr/idc/tv-touchscreen.idc

# Custom media config for HTC camera
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/configs/media_profiles.xml:system/etc/media_profiles.xml \
	$(LOCAL_PATH)/configs/media_codecs.xml:system/etc/media_codecs.xml

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=320 \
	persist.sys.usb.config=mtp,adb \
	ro.telephony.ril_class=QualcommSharedRIL \
        ro.product.manufacturer=HTC

$(call inherit-product-if-exists, hardware/ti/wan/mac80211/Android.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product-if-exists, vendor/htc/endeavoru/endeavoru-vendor.mk)
$(call inherit-product, frameworks/native/build/phone-xhdpi-1024-dalvik-heap.mk)
