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
#

# Skip droiddoc build to save build time
BOARD_SKIP_ANDROID_DOC_BUILD := true

# cpu info
BOARD_HAS_LOCKED_BOOTLOADER := true
TARGET_NO_BOOTLOADER := true
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_ARCH_VARIANT_CPU := cortex-a9
ARCH_ARM_HAVE_NEON := true
ARCH_ARM_HAVE_TLS_REGISTER := true
ARCH_ARM_HAVE_32_BYTE_CACHE_LINES := true

# use endeavor init script
TARGET_PROVIDES_INIT_TARGET_RC := true

# vold
BOARD_VOLD_MAX_PARTITIONS := 20
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true

# USB
TARGET_USE_CUSTOM_LUN_FILE_PATH := /sys/devices/platform/fsl-tegra-udc/gadget/lun

# Lights
TARGET_PROVIDES_LIBLIGHTS := true

# partitions
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 8388608
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1342177280
BOARD_USERDATAIMAGE_PARTITION_SIZE := 2302672896
BOARD_FLASH_BLOCK_SIZE := 4096

# Board nameing
TARGET_NO_RADIOIMAGE := true
TARGET_BOOTLOADER_BOARD_NAME := endeavoru
TARGET_BOARD_PLATFORM := tegra
TARGET_TEGRA_VERSION := t30
BASE_CFLAGS := -mfpu=neon -mfloat-abi=softfp
TARGET_GLOBAL_CFLAGS += $(BASE_CFLAGS)
TARGET_GLOBAL_CPPFLAGS += $(BASE_CFLAGS)

# Assert
TARGET_OTA_ASSERT_DEVICE := endeavoru

# Suspend while charging
BOARD_ALLOW_SUSPEND_IN_CHARGER := true

# Avoid the generation of ldrcc instructions
NEED_WORKAROUND_CORTEX_A9_745320 := true

# Media
BOARD_USES_HW_MEDIARECORDER := true
BOARD_USES_HW_MEDIAPLUGINS := true

# Enable WEBGL in WebKit
ENABLE_WEBGL := true
WEBCORE_ACCELERATED_SCROLLING := true

# Graphics
BOARD_EGL_CFG := device/htc/endeavoru/configs/egl.cfg
USE_OPENGL_RENDERER := true
BOARD_USES_OVERLAY := true
BOARD_USES_HWCOMPOSER := true
BOARD_NO_ALLOW_DEQUEUE_CURRENT_BUFFER := true
TARGET_HAS_WAITFORVSYNC := true
TARGET_HAVE_HDMI_OUT := true
TARGET_USES_GL_VENDOR_EXTENSIONS := true
USE_OPENGL_RENDERER := true
BOARD_EGL_NEEDS_LEGACY_FB := true

# Graphics - Skia
BOARD_USE_SKIA_LCDTEXT := true
BOARD_USES_SKIAHWJPEG := true

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/htc/endeavoru/bluetooth

# HTC ril compatability
BOARD_FORCE_RILD_AS_ROOT := true
TARGET_PROVIDES_LIBRIL := vendor/htc/endeavoru/proprietary/lib/libhtc-ril.so
BOARD_USE_NEW_LIBRIL_HTC := true

# HTCLOG
COMMON_GLOBAL_CFLAGS += -DHTCLOG

# Camera
USE_CAMERA_STUB := false
BOARD_NEEDS_MEMORYHEAPPMEM := true
COMMON_GLOBAL_CFLAGS += -DICS_CAMERA_BLOB

# camera wrapper needs this
COMMON_GLOBAL_CFLAGS += -DDISABLE_HW_ID_MATCH_CHECK -D__ARM_CACHE_LINE_SIZE=32
COMMON_GLOBAL_CPPFLAGS += -DDISABLE_HW_ID_MATCH_CHECK


# Audio
BOARD_USES_GENERIC_AUDIO := false
BOARD_USES_ALSA_AUDIO := false
COMMON_GLOBAL_CFLAGS += -DICS_AUDIO_BLOB

# Connectivity - Wi-Fi
USES_TI_MAC80211 := true
WIFI_BAND := 802_11_ABGN
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_wl12xx
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_wl12xx
BOARD_WLAN_DEVICE := wl12xx_mac80211
BOARD_SOFTAP_DEVICE := wl12xx_mac80211
WIFI_DRIVER_MODULE_PATH := "/system/lib/modules/wl12xx_sdio.ko"
WIFI_DRIVER_MODULE_NAME := "wl12xx_sdio"
WIFI_FIRMWARE_LOADER := ""
COMMON_GLOBAL_CFLAGS += -DUSES_TI_MAC80211

# Assert
TARGET_OTA_ASSERT_DEVICE := endeavoru

# Kernel building
TARGET_USE_PREBUILT_KERNEL := true
