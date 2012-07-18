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


#ifndef ANDROID_LEGACY_AUDIO_POLICY_INTERFACE_H
#define ANDROID_LEGACY_AUDIO_POLICY_INTERFACE_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <system/audio_policy.h>

typedef audio_output_flags_t audio_policy_output_flags_t;

struct effect_descriptor_s;

struct default_ap_module {
    struct audio_policy_module module;
};

typedef struct default_ap_device {
    struct audio_policy_device device;
    struct legacy_audio_policy_device* legacy_device;

    struct default_audio_policy* audio_policy;
} default_ap_device_t;

struct default_audio_policy {
    struct audio_policy policy;

    struct audio_policy_service_ops *aps_ops;
    void *service;

    struct legacy_audio_policy* legacy_policy;
};

typedef audio_output_flags_t audio_policy_output_flags_t;

struct legacy_audio_policy {
    int (*set_device_connection_state)(struct legacy_audio_policy *pol,
                                       audio_devices_t device,
                                       audio_policy_dev_state_t state,
                                       const char *device_address);

    audio_policy_dev_state_t (*get_device_connection_state)(
        const struct legacy_audio_policy *pol,
        audio_devices_t device,
        const char *device_address);

    void (*set_phone_state)(struct legacy_audio_policy *pol, int state);

    void (*set_ringer_mode)(struct legacy_audio_policy *pol, uint32_t mode,
                            uint32_t mask);

    void (*set_force_use)(struct legacy_audio_policy *pol,
                          audio_policy_force_use_t usage,
                          audio_policy_forced_cfg_t config);

    audio_policy_forced_cfg_t (*get_force_use)(const struct legacy_audio_policy *pol,
                                               audio_policy_force_use_t usage);

    void (*set_can_mute_enforced_audible)(struct legacy_audio_policy *pol,
                                          bool can_mute);

    int (*init_check)(const struct legacy_audio_policy *pol);

    audio_io_handle_t (*get_output)(struct legacy_audio_policy *pol,
                                    audio_stream_type_t stream,
                                    uint32_t samplingRate,
                                    uint32_t format,
                                    uint32_t channels,
                                    audio_policy_output_flags_t flags);

    int (*start_output)(struct legacy_audio_policy *pol,
                        audio_io_handle_t output,
                        audio_stream_type_t stream,
                        int session);

    int (*stop_output)(struct legacy_audio_policy *pol,
                       audio_io_handle_t output,
                       audio_stream_type_t stream,
                       int session);

    void (*release_output)(struct legacy_audio_policy *pol, audio_io_handle_t output);

    audio_io_handle_t (*get_input)(struct legacy_audio_policy *pol, int inputSource,
                                   uint32_t samplingRate,
                                   uint32_t format,
                                   uint32_t channels,
                                   audio_in_acoustics_t acoustics);

    int (*start_input)(struct legacy_audio_policy *pol, audio_io_handle_t input);

    int (*stop_input)(struct legacy_audio_policy *pol, audio_io_handle_t input);

    void (*release_input)(struct legacy_audio_policy *pol, audio_io_handle_t input);

    void (*init_stream_volume)(struct legacy_audio_policy *pol,
                               audio_stream_type_t stream,
                               int index_min,
                               int index_max);

    int (*set_stream_volume_index)(struct legacy_audio_policy *pol,
                                   audio_stream_type_t stream,
                                   int index);

    int (*get_stream_volume_index)(const struct legacy_audio_policy *pol,
                                   audio_stream_type_t stream,
                                   int *index);

    uint32_t (*get_strategy_for_stream)(const struct legacy_audio_policy *pol,
                                        audio_stream_type_t stream);

    uint32_t (*get_devices_for_stream)(const struct legacy_audio_policy *pol,
                                       audio_stream_type_t stream);

    audio_io_handle_t (*get_output_for_effect)(struct legacy_audio_policy *pol,
                                               struct effect_descriptor_s *desc);

    int (*register_effect)(struct legacy_audio_policy *pol,
                           struct effect_descriptor_s *desc,
                           audio_io_handle_t output,
                           uint32_t strategy,
                           int session,
                           int id);

    int (*unregister_effect)(struct legacy_audio_policy *pol, int id);

    int (*set_effect_enabled)(struct legacy_audio_policy *pol, int id, bool enabled);

    bool (*is_stream_active)(const struct legacy_audio_policy *pol,
                             int stream,
                             uint32_t in_past_ms);

    int (*dump)(const struct legacy_audio_policy *pol, int fd);
};

struct legacy_audio_policy_service_ops {
    audio_io_handle_t (*open_output)(void *service,
                                     uint32_t *pDevices,
                                     uint32_t *pSamplingRate,
                                     uint32_t *pFormat,
                                     uint32_t *pChannels,
                                     uint32_t *pLatencyMs,
                                     audio_policy_output_flags_t flags);

    audio_io_handle_t (*open_duplicate_output)(void *service,
                                               audio_io_handle_t output1,
                                               audio_io_handle_t output2);

    int (*close_output)(void *service, audio_io_handle_t output);

    int (*suspend_output)(void *service, audio_io_handle_t output);

    int (*restore_output)(void *service, audio_io_handle_t output);

    audio_io_handle_t (*open_input)(void *service,
                                    uint32_t *pDevices,
                                    uint32_t *pSamplingRate,
                                    uint32_t *pFormat,
                                    uint32_t *pChannels,
                                    uint32_t acoustics);

    int (*close_input)(void *service, audio_io_handle_t input);

    int (*set_stream_volume)(void *service,
                             audio_stream_type_t stream,
                             float volume,
                             audio_io_handle_t output,
                             int delay_ms);

    int (*set_stream_output)(void *service,
                             audio_stream_type_t stream,
                             audio_io_handle_t output);

    void (*set_parameters)(void *service,
                           audio_io_handle_t io_handle,
                           const char *kv_pairs,
                           int delay_ms);

    char * (*get_parameters)(void *service, audio_io_handle_t io_handle,
                             const char *keys);

    int (*start_tone)(void *service,
                      audio_policy_tone_t tone,
                      audio_stream_type_t stream);

    int (*stop_tone)(void *service);

    int (*set_voice_volume)(void *service,
                            float volume,
                            int delay_ms);

    int (*move_effects)(void *service,
                        int session,
                        audio_io_handle_t src_output,
                        audio_io_handle_t dst_output);
};

typedef struct legacy_audio_policy_module {
    struct hw_module_t common;
} legacy_audio_policy_module_t;



typedef struct legacy_audio_policy_device {
    struct hw_device_t common;
    int (*create_audio_policy)(const struct legacy_audio_policy_device *device,
                               struct legacy_audio_policy_service_ops *aps_ops,
                               void *service,
                               struct legacy_audio_policy **ap);

    int (*destroy_audio_policy)(const struct legacy_audio_policy_device *device,
                                struct legacy_audio_policy *ap);
} legacy_audio_policy_device_t;

static inline int legacy_audio_policy_dev_open(const hw_module_t* module,
                                               struct legacy_audio_policy_device** device)
{
    return module->methods->open(module, AUDIO_POLICY_INTERFACE,
                                 (hw_device_t**)device);
}

static inline int legacy_audio_policy_dev_close(struct legacy_audio_policy_device* device)
{
    return device->common.close(&device->common);
}

#endif  // ANDROID_AUDIO_POLICY_INTERFACE_H

