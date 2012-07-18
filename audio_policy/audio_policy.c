/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "audio_policy_endeavoru"
//#define LOG_NDEBUG 0

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/str_parms.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <system/audio_policy.h>
#include <hardware/audio_policy.h>
#include <legacy_audio_policy.h>

#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

/** Base path of the hal modules */
#define HAL_LIBRARY_PATH1 "/system/lib/hw"
#define HAL_LIBRARY_PATH2 "/vendor/lib/hw"

/* file reversed legacy_audio_policy -> audio_policy_legacy for sorting order */
#define LEGACY      "audio_policy_legacy"
#define LEGACY_HAL  "/system/lib/hw/audio_policy_legacy.endeavoru.so"

/* library loading */

static const char *variant_keys[] = {
    "ro.hardware",  /* This goes first so that it can pick up a different
                       file on the emulator. */
    "ro.product.board",
    "ro.board.platform",
    "ro.arch"
};

static const int HAL_VARIANT_KEYS_COUNT =
    (sizeof(variant_keys) / sizeof(variant_keys[0]));

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
        ALOGE("load: module=%s\n%s", path, err_str ? err_str : "unknown");
        status = -EINVAL;
        goto done;
    }

    /* Get the address of the struct hal_module_info. */
    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
    hmi = (struct hw_module_t*)dlsym(handle, sym);
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

    /* TODO: fix lookup

       Loop through the configuration variants looking for a module */

    /*
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

       ALOGD("Found legacy policy: %s", path);

        status = load(class_id, path, module);
       } else {
       ALOGE("Could not find a suitable legacy policy");
       }
     */
    status = load(class_id, LEGACY_HAL, module);

    return status;
}

int hw_get_module(const char *id, const struct hw_module_t **module)
{
    return hw_get_module_by_class(id, NULL, module);
}

static int load_audio_policy_interface(const char *if_name, legacy_audio_policy_device_t **dev)
{
    const hw_module_t *mod;
    int rc;

    ALOGD("loading audio interface: %s", if_name);

    rc = hw_get_module_by_class(AUDIO_POLICY_HARDWARE_MODULE_ID, if_name, &mod);
    ALOGE_IF(rc, "%s couldn't load audio hw module %s.%s (%s)", __func__,
             AUDIO_POLICY_HARDWARE_MODULE_ID, if_name, strerror(-rc));
    if (rc) {
        goto out;
    }

    rc = legacy_audio_policy_dev_open(mod, dev);
    ALOGE_IF(rc, "%s couldn't open audio hw device in %s.%s (%s)", __func__,
             AUDIO_POLICY_HARDWARE_MODULE_ID, if_name, strerror(-rc));
    if (rc) {
        goto out;
    }

    return 0;

 out:
    ALOGE("Error trying to load legacy audio policy..");


    *dev = NULL;
    return rc;
}

/* implementation */

static int ap_set_device_connection_state(struct audio_policy *pol,
                                          audio_devices_t device,
                                          audio_policy_dev_state_t state,
                                          const char *device_address)
{
    ALOGD("ap_set_device_connection_state -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->set_device_connection_state(
        apol->legacy_policy,
        device,
        state,
        device_address
        );

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static audio_policy_dev_state_t ap_get_device_connection_state(
    const struct audio_policy *pol,
    audio_devices_t device,
    const char *device_address)
{
    ALOGD("ap_get_device_connection_state -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    audio_policy_dev_state_t retVal = apol->legacy_policy->get_device_connection_state(
        apol->legacy_policy,
        device,
        device_address
        );

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static void ap_set_phone_state(struct audio_policy *pol, audio_mode_t state)
{
    ALOGD("ap_set_phone_state -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    apol->legacy_policy->set_phone_state(apol->legacy_policy, state);
}

// deprecated, never called
static void ap_set_ringer_mode(struct audio_policy *pol, uint32_t mode,
                               uint32_t mask)
{
    ALOGD("ap_set_ringer_mode -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    apol->legacy_policy->set_ringer_mode(apol->legacy_policy, mode, mask);
}

static void ap_set_force_use(struct audio_policy *pol,
                             audio_policy_force_use_t usage,
                             audio_policy_forced_cfg_t config)
{
    ALOGD("ap_set_force_use -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    apol->legacy_policy->set_force_use(apol->legacy_policy, usage, config);
}

/* retreive current device category forced for a given usage */
static audio_policy_forced_cfg_t ap_get_force_use(
    const struct audio_policy *pol,
    audio_policy_force_use_t usage)
{
    ALOGD("ap_get_force_use -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    audio_policy_forced_cfg_t retVal = apol->legacy_policy->get_force_use(apol->legacy_policy, usage);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

/* if can_mute is true, then audio streams that are marked ENFORCED_AUDIBLE
 * can still be muted. */
static void ap_set_can_mute_enforced_audible(struct audio_policy *pol,
                                             bool can_mute)
{
    ALOGD("ap_set_can_mute_enforced_audible -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    apol->legacy_policy->set_can_mute_enforced_audible(apol->legacy_policy, can_mute);
}

static int ap_init_check(const struct audio_policy *pol)
{
    ALOGD("ap_init_check -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->init_check(apol->legacy_policy);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static audio_io_handle_t ap_get_output(struct audio_policy *pol,
                                       audio_stream_type_t stream,
                                       uint32_t sampling_rate,
                                       audio_format_t format,
                                       uint32_t channels,
                                       audio_output_flags_t flags)
{
    ALOGD("ap_get_output -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    audio_io_handle_t retVal = apol->legacy_policy->get_output(
        apol->legacy_policy,
        stream,
        sampling_rate,
        format,
        channels,
        flags
        );

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_start_output(struct audio_policy *pol, audio_io_handle_t output,
                           audio_stream_type_t stream, int session)
{
    ALOGD("ap_start_output -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->start_output(apol->legacy_policy, output, stream, session);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_stop_output(struct audio_policy *pol, audio_io_handle_t output,
                          audio_stream_type_t stream, int session)
{
    ALOGD("ap_stop_output -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->stop_output(apol->legacy_policy, output, stream, session);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static void ap_release_output(struct audio_policy *pol,
                              audio_io_handle_t output)
{
    ALOGD("ap_release_output -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    apol->legacy_policy->release_output(apol->legacy_policy, output);
}

static audio_io_handle_t ap_get_input(struct audio_policy *pol, audio_source_t inputSource,
                                      uint32_t sampling_rate,
                                      audio_format_t format,
                                      uint32_t channels,
                                      audio_in_acoustics_t acoustics)
{
    ALOGD("ap_get_input -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    audio_io_handle_t retVal = apol->legacy_policy->get_input(
        apol->legacy_policy,
        inputSource,
        sampling_rate,
        format,
        channels,
        acoustics
        );

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_start_input(struct audio_policy *pol, audio_io_handle_t input)
{
    ALOGD("ap_start_input -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->start_input(apol->legacy_policy, input);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_stop_input(struct audio_policy *pol, audio_io_handle_t input)
{
    ALOGD("ap_stop_input -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->stop_input(apol->legacy_policy, input);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static void ap_release_input(struct audio_policy *pol, audio_io_handle_t input)
{
    ALOGD("ap_release_input -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    apol->legacy_policy->release_input(apol->legacy_policy, input);
}

static void ap_init_stream_volume(struct audio_policy *pol,
                                  audio_stream_type_t stream, int index_min,
                                  int index_max)
{
    ALOGD("ap_init_stream_volume -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    apol->legacy_policy->init_stream_volume(apol->legacy_policy, stream, index_min, index_max);
}

static int ap_set_stream_volume_index(struct audio_policy *pol,
                                      audio_stream_type_t stream,
                                      int index)
{
    ALOGD("ap_set_stream_volume -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->set_stream_volume_index(apol->legacy_policy, stream, index);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_get_stream_volume_index(const struct audio_policy *pol,
                                      audio_stream_type_t stream,
                                      int *index)
{
    ALOGD("ap_get_stream_volume -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->get_stream_volume_index(apol->legacy_policy, stream, index);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_set_stream_volume_index_for_device(struct audio_policy *pol,
                                                 audio_stream_type_t stream,
                                                 int index,
                                                 audio_devices_t device)
{
    ALOGD("ap_set_stream_volume_index_for_device -> %s", LEGACY_HAL);
    ALOGD("# stream: %d index: %d device: %d", stream, index, device);

    /*
       TODO: check what we can do here as this function did not exist.

       struct default_audio_policy *apol = (struct default_audio_policy *)pol;
       uint32_t retVal = apol->legacy_policy->set_stream_volume_index_for_device(
                    apol->legacy_policy,
                    stream,
                    index,
                    device);
     */
    int retVal = 0;

//	ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_get_stream_volume_index_for_device(const struct audio_policy *pol,
                                                 audio_stream_type_t stream,
                                                 int *index,
                                                 audio_devices_t device)
{
    ALOGD("ap_get_stream_volume_index_for_device -> %s", LEGACY_HAL);
    ALOGD("# stream: %d index: %d device: %d", stream, *index, device);

    /*
       TODO: check what we can do here as this function did not exist.

       struct default_audio_policy *apol = (struct default_audio_policy *)pol;
       uint32_t retVal = apol->legacy_policy->get_stream_volume_index_for_device(
                    apol->legacy_policy,
                    stream,
                    index,
                    device);
     */
    int retVal = 0;

//	ALOGD("retVal = %d", retVal);
    return retVal;
}

static uint32_t ap_get_strategy_for_stream(const struct audio_policy *pol,
                                           audio_stream_type_t stream)
{
    ALOGD("ap_get_strategy_for_stream -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    uint32_t retVal = apol->legacy_policy->get_strategy_for_stream(apol->legacy_policy, stream);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static audio_devices_t ap_get_devices_for_stream(const struct audio_policy *pol,
                                                 audio_stream_type_t stream)
{
    ALOGD("ap_get_devices_for_stream -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    audio_devices_t retVal = apol->legacy_policy->get_devices_for_stream(apol->legacy_policy, stream);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static audio_io_handle_t ap_get_output_for_effect(struct audio_policy *pol,
                                                  struct effect_descriptor_s *desc)
{
    ALOGD("ap_get_output_for_effect -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    audio_io_handle_t retVal = apol->legacy_policy->get_output_for_effect(apol->legacy_policy, desc);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_register_effect(struct audio_policy *pol,
                              struct effect_descriptor_s *desc,
                              audio_io_handle_t output,
                              uint32_t strategy,
                              int session,
                              int id)
{
    ALOGD("ap_register_effect -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->register_effect(
        apol->legacy_policy,
        desc,
        output,
        strategy,
        session,
        id
        );

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_unregister_effect(struct audio_policy *pol, int id)
{
    ALOGD("ap_unregister_effect -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->unregister_effect(apol->legacy_policy, id);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_set_effect_enabled(struct audio_policy *pol, int id, bool enabled)
{
    ALOGD("ap_set_effect_enabled -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->set_effect_enabled(apol->legacy_policy, id, enabled);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static bool ap_is_stream_active(const struct audio_policy *pol, audio_stream_type_t stream,
                                uint32_t in_past_ms)
{
    ALOGD("ap_is_stream_active -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    bool retVal = apol->legacy_policy->is_stream_active(apol->legacy_policy, stream, in_past_ms);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int ap_dump(const struct audio_policy *pol, int fd)
{
    ALOGD("ap_dump -> %s", LEGACY_HAL);

    struct default_audio_policy *apol = (struct default_audio_policy*)pol;
    int retVal = apol->legacy_policy->dump(apol->legacy_policy, fd);

    ALOGD("retVal = %d", retVal);
    return retVal;
}

static int create_default_ap(const struct audio_policy_device *device,
                             struct audio_policy_service_ops *aps_ops,
                             void *service,
                             struct audio_policy **ap)
{
    ALOGD("create_default_ap -> %s", LEGACY_HAL);

    struct default_audio_policy *dap;
    int ret;

    *ap = NULL;

    if (!service || !aps_ops)
        return -EINVAL;

    dap = (struct default_audio_policy*)calloc(1, sizeof(*dap));
    if (!dap)
        return -ENOMEM;

    dap->policy.set_device_connection_state = ap_set_device_connection_state;
    dap->policy.get_device_connection_state = ap_get_device_connection_state;
    dap->policy.set_phone_state = ap_set_phone_state;
    dap->policy.set_ringer_mode = ap_set_ringer_mode;
    dap->policy.set_force_use = ap_set_force_use;
    dap->policy.get_force_use = ap_get_force_use;
    dap->policy.set_can_mute_enforced_audible =
        ap_set_can_mute_enforced_audible;
    dap->policy.init_check = ap_init_check;
    dap->policy.get_output = ap_get_output;
    dap->policy.start_output = ap_start_output;
    dap->policy.stop_output = ap_stop_output;
    dap->policy.release_output = ap_release_output;
    dap->policy.get_input = ap_get_input;
    dap->policy.start_input = ap_start_input;
    dap->policy.stop_input = ap_stop_input;
    dap->policy.release_input = ap_release_input;
    dap->policy.init_stream_volume = ap_init_stream_volume;
    dap->policy.set_stream_volume_index = ap_set_stream_volume_index;
    dap->policy.get_stream_volume_index = ap_get_stream_volume_index;

    /*

       AudioPolicyService::setStreamVolumeIndex (AudioPolicyService.cpp)

       the index_for_device methods are new in JB and not taken care off by the ICS driver.
       setting them to 0 should make AudioFlinger choose the old path.

     */
    dap->policy.set_stream_volume_index_for_device = 0; //ap_set_stream_volume_index_for_device;
    dap->policy.get_stream_volume_index_for_device = 0; //ap_get_stream_volume_index_for_device;

    dap->policy.get_strategy_for_stream = ap_get_strategy_for_stream;
    dap->policy.get_devices_for_stream = ap_get_devices_for_stream;
    dap->policy.get_output_for_effect = ap_get_output_for_effect;
    dap->policy.register_effect = ap_register_effect;
    dap->policy.unregister_effect = ap_unregister_effect;
    dap->policy.set_effect_enabled = ap_set_effect_enabled;
    dap->policy.is_stream_active = ap_is_stream_active;
    dap->policy.dump = ap_dump;

    dap->service = service;
    dap->aps_ops = aps_ops;

    ALOGD("create_legacy_default_ap -> %s", LEGACY_HAL);

    default_ap_device_t* default_device = ( default_ap_device_t*)device;
    default_device->legacy_device->create_audio_policy(default_device->legacy_device,
                                                       aps_ops, service,
                                                       &dap->legacy_policy);

    ALOGD("/create_legacy_default_ap -> %s", LEGACY_HAL);


    default_device->audio_policy = dap;

    *ap = &dap->policy;
    return 0;
}

static int destroy_default_ap(const struct audio_policy_device *ap_dev,
                              struct audio_policy *ap)
{
    ALOGD("destroy_default_ap -> %s", LEGACY_HAL);

    default_ap_device_t* default_device = ( default_ap_device_t*)ap_dev;
    struct default_audio_policy* apol = (struct default_audio_policy*)ap;

    int retVal = default_device->legacy_device->destroy_audio_policy(default_device->legacy_device,
                                                                     apol->legacy_policy);

    ALOGD("retVal = %d", retVal);

    free(ap);
    return 0;
}

static int default_ap_dev_close(hw_device_t* device)
{
    ALOGD("default_ap_dev_close -> %s", LEGACY_HAL);

    default_ap_device_t* default_device = ( default_ap_device_t*)device;
    int retVal = default_device->legacy_device->common.close(default_device->legacy_device);

    ALOGD("retVal = %d", retVal);

    free(device);
    return 0;
}

static int default_ap_dev_open(const hw_module_t* module, const char* name,
                               hw_device_t** device)
{
    ALOGD("default_ap_dev_open -> %s", LEGACY_HAL);

    struct default_ap_device *dev;

    *device = NULL;

    if (strcmp(name, AUDIO_POLICY_INTERFACE) != 0)
        return -EINVAL;

    dev = (struct default_ap_device*)calloc(1, sizeof(*dev));
    if (!dev)
        return -ENOMEM;

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version = 0;
    dev->device.common.module = (hw_module_t*)module;
    dev->device.common.close = default_ap_dev_close;
    dev->device.create_audio_policy = create_default_ap;
    dev->device.destroy_audio_policy = destroy_default_ap;
    dev->audio_policy = 0;

    /*
       load_audio_interface based on AudioFlinger
       responsible for load audio.legacy.endeavoru which used to be audio.primary.tegra
     */

    ALOGD("Trying to load legacy policy..");

    int rc = load_audio_policy_interface(LEGACY, &dev->legacy_device);
    if (rc) {
        ALOGD("loadHwModule() error %d loading module %s ", rc, LEGACY);
        return 0;
    } else {
        ALOGD("Succesfully loaded legacy module");
    }

    *device = &dev->device.common;

    return 0;
}

static struct hw_module_methods_t default_ap_module_methods = {
    .open = default_ap_dev_open,
};

struct default_ap_module HAL_MODULE_INFO_SYM = {
    .module = {
        .common = {
            .tag = HARDWARE_MODULE_TAG,
            .version_major = 1,
            .version_minor = 0,
            .id = AUDIO_POLICY_HARDWARE_MODULE_ID,
            .name = "Legacy tegra audio policy wrapper HAL",
            .author = "rogro82@xda",
            .methods = &default_ap_module_methods,
        },
    },
};
