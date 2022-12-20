

#ifndef FLOBS_FLOBS_AUDIO_H_
#define FLOBS_FLOBS_AUDIO_H_

#include "third_party/obs-studio/src/libobs/util/dstr.h"

static const char kInputAudioSource[] = "wasapi_input_capture";
static const char kOutputAudioSource[] = "wasapi_output_capture";

static inline bool HasAudioDevices(const char* source_id) {
  const char* output_id = source_id;
  obs_properties_t* props = obs_get_source_properties(output_id);
  size_t count = 0;

  if (!props)
    return false;

  obs_property_t* devices = obs_properties_get(props, "device_id");
  if (devices)
    count = obs_property_list_item_count(devices);

  obs_properties_destroy(props);

  return count != 0;
}

static inline char* get_new_source_name(const char* name, const char* format) {
  struct dstr new_name = {0};
  int inc = 0;

  dstr_copy(&new_name, name);

  for (;;) {
    OBSSourceAutoRelease existing_source =
        obs_get_source_by_name(new_name.array);
    if (!existing_source)
      break;

    dstr_printf(&new_name, format, name, ++inc + 1);
  }

  return new_name.array;
}

static inline void ResetAudioDevice(const char* sourceId,
                                    const char* deviceId,
                                    const char* deviceDesc,
                                    int channel) {
  bool disable = deviceId && strcmp(deviceId, "disabled") == 0;
  OBSSourceAutoRelease source;
  OBSDataAutoRelease settings;

  source = obs_get_output_source(channel);
  if (source) {
    if (disable) {
      obs_set_output_source(channel, nullptr);
    } else {
      settings = obs_source_get_settings(source);
      const char* oldId = obs_data_get_string(settings, "device_id");
      if (strcmp(oldId, deviceId) != 0) {
        obs_data_set_string(settings, "device_id", deviceId);
        obs_source_update(source, settings);
      }
    }
  } else if (!disable) {
    BPtr<char> name = get_new_source_name(deviceDesc, "%s (%d)");

    settings = obs_data_create();
    obs_data_set_string(settings, "device_id", deviceId);
    source = obs_source_create(sourceId, name, settings, nullptr);

    obs_set_output_source(channel, source);
  }
}

static inline void SaveAudioDevice(const char* name,
                                   int channel,
                                   obs_data_t* parent,
                                   std::vector<OBSSource>& audioSources) {
  OBSSourceAutoRelease source = obs_get_output_source(channel);
  if (!source)
    return;

  audioSources.push_back(source.Get());

  OBSDataAutoRelease data = obs_save_source(source);

  obs_data_set_obj(parent, name, data);
}

static inline bool SourceMixerHidden(obs_source_t* source) {
  OBSDataAutoRelease priv_settings = obs_source_get_private_settings(source);
  bool hidden = obs_data_get_bool(priv_settings, "mixer_hidden");

  return hidden;
}

#endif  // FLOBS_FLOBS_AUDIO_H_
