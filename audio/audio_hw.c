/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_hw_primary"
/*#define LOG_NDEBUG 0*/

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/str_parms.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <system/audio.h>

#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

/** Base path of the hal modules */
#define HAL_LIBRARY_PATH1 "/system/lib/hw"
#define HAL_LIBRARY_PATH2 "/vendor/lib/hw"

#define LEGACY		"legacy"
#define LEGACY_HAL	"/system/lib/hw/android.legacy.endeavoru.so"

/* legacy ( ICS ) audio structure */
struct legacy_audio_hw_device {
    struct hw_device_t common;

    uint32_t (*get_supported_devices)(const struct legacy_audio_hw_device *dev);

    int (*init_check)(const struct legacy_audio_hw_device *dev);

    int (*set_voice_volume)(struct legacy_audio_hw_device *dev, float volume);

    int (*set_master_volume)(struct legacy_audio_hw_device *dev, float volume);

    int (*set_mode)(struct legacy_audio_hw_device *dev, int mode);

    int (*set_mic_mute)(struct legacy_audio_hw_device *dev, bool state);

    int (*get_mic_mute)(const struct legacy_audio_hw_device *dev, bool *state);

    int (*set_parameters)(struct legacy_audio_hw_device *dev, const char *kv_pairs);

    char * (*get_parameters)(const struct legacy_audio_hw_device *dev,
                             const char *keys);

    size_t (*get_input_buffer_size)(const struct legacy_audio_hw_device *dev,
                                    uint32_t sample_rate, int format,
                                    int channel_count);

    int (*open_output_stream)(struct legacy_audio_hw_device *dev, uint32_t devices,
                              int *format, uint32_t *channels,
                              uint32_t *sample_rate,
                              struct audio_stream_out **out);

    void (*close_output_stream)(struct legacy_audio_hw_device *dev,
                                struct audio_stream_out* out);

    int (*open_input_stream)(struct legacy_audio_hw_device *dev, uint32_t devices,
                             int *format, uint32_t *channels,
                             uint32_t *sample_rate,
                             audio_in_acoustics_t acoustics,
                             struct audio_stream_in **stream_in);

    void (*close_input_stream)(struct legacy_audio_hw_device *dev,
                               struct audio_stream_in *in);

    int (*dump)(const struct legacy_audio_hw_device *dev, int fd);
};
typedef struct legacy_audio_hw_device legacy_audio_hw_device_t;

struct audio_device {
    struct audio_hw_device hw_device;
    struct legacy_audio_hw_device* legacy_device;
};

static const char *variant_keys[] = {
    "ro.hardware",  /* This goes first so that it can pick up a different
                       file on the emulator. */
    "ro.product.board",
    "ro.board.platform",
    "ro.arch"
};

static const int HAL_VARIANT_KEYS_COUNT =
    (sizeof(variant_keys)/sizeof(variant_keys[0]));

/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the hmi.
 * @return 0 = success, !0 = failure.
 */
static int load(const char *id,
        const char *path,
        const struct hw_module_t **pHmi)
{
    int status;
    void *handle;
    struct hw_module_t *hmi;

    /*
     * load the symbols resolving undefined symbols before
     * dlopen returns. Since RTLD_GLOBAL is not or'd in with
     * RTLD_NOW the external symbols will not be global
     */
    handle = dlopen(path, RTLD_NOW);
    if (handle == NULL) {
        char const *err_str = dlerror();
        ALOGE("load: module=%s\n%s", path, err_str?err_str:"unknown");
        status = -EINVAL;
        goto done;
    }

    /* Get the address of the struct hal_module_info. */
    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
    hmi = (struct hw_module_t *)dlsym(handle, sym);
    if (hmi == NULL) {
        ALOGE("load: couldn't find symbol %s", sym);
        status = -EINVAL;
        goto done;
    }

    /* Check that the id matches */
    if (strcmp(id, hmi->id) != 0) {
        ALOGE("load: id=%s != hmi->id=%s", id, hmi->id);
        status = -EINVAL;
        goto done;
    }

    hmi->dso = handle;

    /* success */
    status = 0;

    done:
    if (status != 0) {
        hmi = NULL;
        if (handle != NULL) {
            dlclose(handle);
            handle = NULL;
        }
    } else {
        ALOGV("loaded HAL id=%s path=%s hmi=%p handle=%p",
                id, path, *pHmi, handle);
    }

    *pHmi = hmi;

    return status;
}

int hw_get_module_by_class(const char *class_id, const char *inst,
                           const struct hw_module_t **module)
{
    int status;
    int i;
    const struct hw_module_t *hmi = NULL;
    char prop[PATH_MAX];
    char path[PATH_MAX];
    char name[PATH_MAX];

    if (inst)
        snprintf(name, PATH_MAX, "%s.%s", class_id, inst);
    else
        strlcpy(name, class_id, PATH_MAX);

    /*
     * Here we rely on the fact that calling dlopen multiple times on
     * the same .so will simply increment a refcount (and not load
     * a new copy of the library).
     * We also assume that dlopen() is thread-safe.
     */

    /* Loop through the configuration variants looking for a module */
    for (i=0 ; i<HAL_VARIANT_KEYS_COUNT+1 ; i++) {
        if (i < HAL_VARIANT_KEYS_COUNT) {
            if (property_get(variant_keys[i], prop, NULL) == 0) {
                continue;
            }
            snprintf(path, sizeof(path), "%s/%s.%s.so",
                     HAL_LIBRARY_PATH2, name, prop);
            if (access(path, R_OK) == 0) break;

            snprintf(path, sizeof(path), "%s/%s.%s.so",
                     HAL_LIBRARY_PATH1, name, prop);
            if (access(path, R_OK) == 0) break;
        } else {
            snprintf(path, sizeof(path), "%s/%s.default.so",
                     HAL_LIBRARY_PATH1, name);
            if (access(path, R_OK) == 0) break;
        }
    }

    status = -ENOENT;
    if (i < HAL_VARIANT_KEYS_COUNT+1) {
        /* load the module, if this fails, we're doomed, and we should not try
         * to load a different variant. */
        status = load(class_id, path, module);
    }

    return status;
}

int hw_get_module(const char *id, const struct hw_module_t **module)
{
    return hw_get_module_by_class(id, NULL, module);
}

static int load_audio_interface(const char *if_name, audio_hw_device_t **dev)
{
    const hw_module_t *mod;
    int rc;

    ALOGD("loading audio interface: %s", if_name);

    rc = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID, if_name, &mod);
    ALOGE_IF(rc, "%s couldn't load audio hw module %s.%s (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, if_name, strerror(-rc));
    if (rc) {
        goto out;
    }

    rc = audio_hw_device_open(mod, dev);
    ALOGE_IF(rc, "%s couldn't open audio hw device in %s.%s (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, if_name, strerror(-rc));
    if (rc) {
        goto out;
    }

    return 0;

out:
    *dev = NULL;
    return rc;
}

static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out)
{
	ALOGD("adev_open_output_stream -> %s", LEGACY_HAL);

	/*
	audio_stream_out will have to get modified because of missing
	get_next_write_timestamp in ICS.

	problem is that we dont know the size of stream_out as its allocated
	in the legacy HAL and probably stores more data.

	So we cant just send the new audio_stream_out and add the method afterwards
	as it will probably have memory allocated in that area.

	Possible fix:
	
	change the definition in audio.h to have enough free space before following method:

	int (*get_next_write_timestamp)(const struct audio_stream_out *stream,
                                 int64_t *timestamp);

	then we can set get_next_write_timestamp our selves after open_output_stream without
	corrupting its structure.

	currently get_next_write_timestamp is removed from audio.h and functions relying on it
	have been modified:

	* Audioflinger.cpp ( AudioFlinger::MixerThread::threadLoop_mix() )
	* libhardware/modules/audio/audio_hw.c

	*/

	struct audio_device *adev = (struct audio_device *)dev;
	int retVal = adev->legacy_device->open_output_stream(
				adev->legacy_device, 
				devices, 
			        (int*)&config->format,
              			(uint32_t*)&config->channel_mask,
              			(uint32_t*)&config->sample_rate,
				stream_out);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream)
{
	ALOGD("adev_close_output_stream -> %s", LEGACY_HAL);

	struct audio_device *adev = (struct audio_device *)dev;
	adev->legacy_device->close_output_stream(adev->legacy_device, stream);
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
	ALOGD("adev_set_parameters -> %s", LEGACY_HAL);

	struct audio_device *adev = (struct audio_device *)dev;
	int retVal = adev->legacy_device->get_parameters(adev->legacy_device, kvpairs);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static char * adev_get_parameters(const struct audio_hw_device *dev,
                                  const char *keys)
{
	ALOGD("adev_get_parameters -> %s", LEGACY_HAL);

	struct audio_device *adev = (struct audio_device *)dev;
	char* retVal = adev->legacy_device->get_parameters(adev->legacy_device, keys);

	ALOGD("retVal = %s", retVal);
	return retVal;
}

static int adev_init_check(const struct audio_hw_device *dev)
{
	ALOGD("adev_init_check -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)dev;
    	int retVal = adev->legacy_device->init_check(adev->legacy_device);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
	ALOGD("adev_set_voice_volume -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)dev;
    	int retVal = adev->legacy_device->set_voice_volume(adev->legacy_device, volume);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
	ALOGD("adev_set_master_volume -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)dev;
    	int retVal = adev->legacy_device->set_master_volume(adev->legacy_device, volume);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
{
	ALOGD("adev_set_mode -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)dev;
    	int retVal = adev->legacy_device->set_mode(adev->legacy_device, mode);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
	ALOGD("adev_set_mic_mute -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)dev;
    	int retVal = adev->legacy_device->set_mic_mute(adev->legacy_device, state);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
	ALOGD("adev_get_mic_mute -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)dev;
    	int retVal = adev->legacy_device->get_mic_mute(adev->legacy_device, state);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config)
{
	ALOGD("adev_get_input_buffer_size -> %s", LEGACY_HAL);

	/* use popcount to get nr of channels from the mask */
	uint16_t channel_count = (uint16_t)popcount(config->channel_mask);
	ALOGD("channel count: %d", channel_count);

	struct audio_device *adev = (struct audio_device *)dev;
	size_t retVal = adev->legacy_device->get_input_buffer_size(
				adev->legacy_device, 
              			config->sample_rate,
			        config->format,
              			channel_count);

	return retVal;
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in)
{
	ALOGD("adev_open_input_stream -> %s", LEGACY_HAL);

	struct audio_device *adev = (struct audio_device *)dev;
	int retVal = adev->legacy_device->open_input_stream(
				adev->legacy_device, 
				devices, 
			        (int*)&config->format,
              			(uint32_t*)&config->channel_mask,
              			(uint32_t*)&config->sample_rate,
				0, /* JB no longer has acoustics */
				stream_in);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                   struct audio_stream_in *stream)
{
	ALOGD("adev_close_input_stream -> %s", LEGACY_HAL);

	struct audio_device *adev = (struct audio_device *)dev;
	adev->legacy_device->close_input_stream(
				adev->legacy_device, 
				stream);
}

static int adev_dump(const audio_hw_device_t *device, int fd)
{
	ALOGD("adev_dump -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)device;
    	int retVal = adev->legacy_device->dump(adev->legacy_device, fd);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static int adev_close(hw_device_t *device)
{
	ALOGD("adev_close -> %s", LEGACY_HAL);
	
    	struct audio_device *adev = (struct audio_device *)device;
	
        free(adev->legacy_device);
        free(device);
        return 0;
}

static uint32_t adev_get_supported_devices(const struct audio_hw_device *dev)
{
	ALOGD("adev_get_supported_devices -> %s", LEGACY_HAL);

    	struct audio_device *adev = (struct audio_device *)dev;
    	uint32_t retVal = adev->legacy_device->get_supported_devices(adev->legacy_device);

	ALOGD("retVal = %d", retVal);
	return retVal;
}

static int adev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
	ALOGD("adev_open (start) -> %s", LEGACY_HAL);

    struct audio_device *adev;
    int ret;

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = calloc(1, sizeof(struct audio_device));
    if (!adev)
        return -ENOMEM;

    adev->hw_device.common.tag = HARDWARE_DEVICE_TAG;
    adev->hw_device.common.version = AUDIO_DEVICE_API_VERSION_1_0;
    adev->hw_device.common.module = (struct hw_module_t *) module;
    adev->hw_device.common.close = adev_close;

    adev->hw_device.get_supported_devices = adev_get_supported_devices;
    adev->hw_device.init_check = adev_init_check;
    adev->hw_device.set_voice_volume = adev_set_voice_volume;
    adev->hw_device.set_master_volume = adev_set_master_volume;
    adev->hw_device.set_mode = adev_set_mode;
    adev->hw_device.set_mic_mute = adev_set_mic_mute;
    adev->hw_device.get_mic_mute = adev_get_mic_mute;
    adev->hw_device.set_parameters = adev_set_parameters;
    adev->hw_device.get_parameters = adev_get_parameters;
    adev->hw_device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->hw_device.open_output_stream = adev_open_output_stream;
    adev->hw_device.close_output_stream = adev_close_output_stream;
    adev->hw_device.open_input_stream = adev_open_input_stream;
    adev->hw_device.close_input_stream = adev_close_input_stream;
    adev->hw_device.dump = adev_dump;

    ALOGD("adev_open (end) -> %s", LEGACY_HAL);

    /* 
	load_audio_interface based on AudioFlinger 
	responsible for load audio.legacy.endeavoru which used to be audio.primary.tegra
    */

    int rc = load_audio_interface(LEGACY, &adev->legacy_device);
    if (rc) {
        ALOGI("loadHwModule() error %d loading module %s ", rc, LEGACY);
        return 0;
    }

    *device = &adev->hw_device.common;

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Legacy tegra audio wrapper HW HAL",
        .author = "rogro82@xda",
        .methods = &hal_module_methods,
    },
};
