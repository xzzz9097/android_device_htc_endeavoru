# Copyright 2010 The Android Open Source Project
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
# This file sets variables that control the way modules are built
# thorughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#

# inherit from the proprietary version
-include vendor/htc/endeavoru/BoardConfigVendor.mk

TARGET_BOARD_PLATFORM := tegra

#custom init rc
TARGET_PROVIDES_INIT_RC := true

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_ARCH_VARIANT_CPU := cortex-a9
TARGET_ARCH_VARIANT_FPU := neon
ARCH_ARM_HAVE_32_BYTE_CACHE_LINES := true
ARCH_ARM_HAVE_TLS_REGISTER := true
ARCH_ARM_USE_NON_NEON_MEMCPY := true

TARGET_NO_BOOTLOADER := true
TARGET_BOOTLOADER_BOARD_NAME := 

# Linaro fixes
# USE_OLD_MEMCPY := true
USE_MORE_OPT_FLAGS := yes
DEBUG_NO_STDCXX11 := yes
# DEBUG_NO_STRICT_ALIASING := yes

# Optimization build flags
TARGET_GLOBAL_CFLAGS += -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp
TARGET_GLOBAL_CPPFLAGS += -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp

USE_OPENGL_RENDERER := true
BOARD_EGL_CFG := device/htc/endeavoru/config/egl.cfg
BOARD_EGL_NEEDS_LEGACY_FB := true

BOARD_KERNEL_CMDLINE := 
BOARD_KERNEL_PAGESIZE := 2048

TARGET_USERIMAGES_USE_EXT4 := true

# Partitions Info
#cat /proc/emmc
#dev:        size     erasesize name
#mmcblk0p5: 00800000 00001000 "recovery"
#mmcblk0p4: 00800000 00001000 "boot"
#mmcblk0p12: 50000000 00001000 "system"
#mmcblk0p13: 14000000 00001000 "cache"
#mmcblk0p17: 00200000 00001000 "misc"
#mmcblk0p1: 00600000 00001000 "wlan"
#mmcblk0p2: 00200000 00001000 "WDM"
#mmcblk0p20: 00200000 00001000 "pdata"
#mmcblk0p3: 00600000 00001000 "radiocab"
#mmcblk0p14: 650000000 00001000 "internalsd"
#mmcblk0p15: 89400000 00001000 "userdata"
#mmcblk0p19: 01600000 00001000 "devlog"
#mmcblk0p16: 00200000 00001000 "extra"

BOARD_BOOTIMAGE_PARTITION_SIZE := 8388608
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 8388608
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1342177280
BOARD_USERDATAIMAGE_PARTITION_SIZE := 2302672896
BOARD_FLASH_BLOCK_SIZE := 4096

BOARD_VOLD_MAX_PARTITIONS := 20
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/platform/tegra-udc.0/gadget/lun0/file"

# Wifi related defines
USES_TI_MAC80211 := true
ifdef USES_TI_MAC80211
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
WPA_SUPPLICANT_VERSION           := VER_0_8_X_TI
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_WLAN_DEVICE                := wl12xx_mac80211
BOARD_SOFTAP_DEVICE              := wl12xx_mac80211
WIFI_DRIVER_MODULE_PATH          := "/system/lib/modules/wl12xx_sdio.ko"
WIFI_DRIVER_MODULE_NAME          := "wl12xx_sdio"
WIFI_FIRMWARE_LOADER             := ""
COMMON_GLOBAL_CFLAGS             += -DUSES_TI_MAC80211
endif

# Kernel
# TARGET_KERNEL_SOURCE := kernel/htc/endeavoru
# TARGET_KERNEL_CONFIG := cyanogenmod_endeavoru_defconfig

# Kernel building
TARGET_USE_PREBUILT_KERNEL := true

# Avoid the generation of ldrcc instructions
NEED_WORKAROUND_CORTEX_A9_745320 := true

# Enable WEBGL in WebKit
ENABLE_WEBGL := true

# Audio(prebuilt)
COMMON_GLOBAL_CFLAGS += -DICS_AUDIO_BLOB

# HTC specific
BOARD_USE_NEW_LIBRIL_HTC := true
COMMON_GLOBAL_CFLAGS += -DHTCLOG

# Sensors invensense
BOARD_USES_GENERIC_INVENSENSE := false

# Camera
USE_CAMERA_STUB := false
BOARD_CAMERA_HAVE_ISO := true
COMMON_GLOBAL_CFLAGS += -DHAVE_ISO
COMMON_GLOBAL_CFLAGS += -DMR0_CAMERA_BLOB -DICS_CAMERA_BLOB
COMMON_GLOBAL_CFLAGS += -DDISABLE_HW_ID_MATCH_CHECK
BOARD_NEEDS_MEMORYHEAPPMEM := true

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/htc/endeavoru/bluetooth

# Recovery
# TARGET_PREBUILT_RECOVERY_KERNEL := device/htc/endeavoru/prebuilt/recovery_kernel
# BOARD_USE_CUSTOM_RECOVERY_FONT := \"roboto_15x24.h\"
BOARD_UMS_LUNFILE := "/sys/devices/platform/fsl-tegra-udc/gadget/lun0/file"
BOARD_HAS_NO_SELECT_BUTTON := true

# Releasetools
TARGET_RELEASETOOLS_EXTENSIONS := device/htc/endeavoru
