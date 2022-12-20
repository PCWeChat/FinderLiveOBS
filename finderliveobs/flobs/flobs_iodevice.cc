

#include "flobs/flobs_iodevice.h"
#include "flobs/flobs_filter_info.h"

#include "flproto/proto/flmsgtype.pb.h"
#include "flproto/proto/flobsinfo.pb.h"

static const std::string kFilterGain = "gain_filter";
static const std::string kFilterNoiseSuppression = "noise_suppress_filter";

FLOBSIODevice::FLOBSIODevice(OBSSource source, AudioDeviceType device_type)
    : source_(source),
      device_type_(device_type),
      add_signal_(obs_source_get_signal_handler(source_),
                  "filter_add",
                  FLOBSIODevice::OBSSourceFilterAdded,
                  this),
      remove_signal_(obs_source_get_signal_handler(source_),
                     "filter_remove",
                     FLOBSIODevice::OBSSourceFilterRemoved,
                     this) {}

FLOBSIODevice::~FLOBSIODevice() {}

bool FLOBSIODevice::SetDeviceId(const std::string& device_id) {
  bool result = false;
  if (source_) {
    obs_data_t* settings = obs_source_get_settings(source_);
    if (settings) {
      obs_data_set_string(settings, "device_id", device_id.c_str());
      obs_source_update(source_, settings);
      obs_data_release(settings);
      result = true;
    }
  }
  return result;
}

std::string FLOBSIODevice::GetDeviceId() {
  std::string device_id;
  if (source_) {
    obs_data_t* settings = obs_source_get_settings(source_);
    if (settings) {
      device_id = obs_data_get_string(settings, "device_id");
      obs_data_release(settings);
    }
  }
  return device_id;
}

bool FLOBSIODevice::SetDBValue(double value) {
  bool result = false;
  if (value == 0.0) {
    RemoveFilterInfo(AudioFilterType::kDeviceGain);
  } else {
    auto it = lst_filterinfos_.find(AudioFilterType::kDeviceGain);
    if (it == lst_filterinfos_.end()) {
      AddFilterInfo(kFilterGain.c_str());
      it = lst_filterinfos_.find(AudioFilterType::kDeviceGain);
    }
    it->second->SetDBValue(value);
  }
  result = true;

  return result;
}

bool FLOBSIODevice::SetSuppressLevelValue(int value) {
  bool result = false;
  if (value == -60) {
    RemoveFilterInfo(AudioFilterType::kDeviceDenoise);
  } else {
    auto it = lst_filterinfos_.find(AudioFilterType::kDeviceDenoise);
    if (it == lst_filterinfos_.end()) {
      AddFilterInfo(kFilterNoiseSuppression.c_str());
      it = lst_filterinfos_.find(AudioFilterType::kDeviceDenoise);
    }
    it->second->SetSuppressLevelValue(value);
  }
  result = true;

  return result;
}

int FLOBSIODevice::GetVolume() {
  if (!source_) {
    return -1;
  }

  int value = static_cast<int>(100 * obs_source_get_volume(source_));
  return value;
}

void FLOBSIODevice::SetVolume(int value) {
  if (!source_) {
    return;
  }
  obs_source_set_volume(source_, static_cast<float>(value) / 100.0);
}

int FLOBSIODevice::GetMonitoringType() {
  if (!source_) {
    return 0;
  }
  return obs_source_get_monitoring_type(source_);
}

bool FLOBSIODevice::SetMonitoringType(int value) {
  if (!source_) {
    return false;
  }
  obs_source_set_monitoring_type(source_,
                                 static_cast<obs_monitoring_type>(value));
  return true;
}

bool FLOBSIODevice::IsSameSource(OBSSource source) {
  bool result = false;
  if (!source_) {
    return result;
  }
  return source_ == source;
}

void FLOBSIODevice::AddFilterInfo(const char* id) {
  auto name = obs_source_get_display_name(id);

  obs_source_t* existing_filter = obs_source_get_filter_by_name(source_, name);
  if (existing_filter) {
    obs_source_release(existing_filter);
  }
  obs_source_t* filter = obs_source_create(id, name, nullptr, nullptr);
  if (filter) {
    obs_source_filter_add(source_, filter);
    obs_source_release(filter);
  }
}

bool FLOBSIODevice::RemoveFilterInfo(AudioFilterType type) {
  bool result = false;
  auto it = lst_filterinfos_.find(type);
  if (it != lst_filterinfos_.end()) {
    auto filter = it->second->GetFilter();
    obs_source_filter_remove(source_, filter);
    result = true;
  }
  return result;
}

void FLOBSIODevice::UpdateFilters() {
  if (!source_) {
    return;
  }

  obs_source_enum_filters(
      source_,
      [](obs_source_t*, obs_source_t* filter, void* p) {
        FLOBSIODevice* thisObject = reinterpret_cast<FLOBSIODevice*>(p);
        thisObject->AddFilter(filter, false);
      },
      this);
}

void FLOBSIODevice::SetFiltersValue(void* device_infos) {
  if (!device_infos) {
    return;
  }
  auto device_infos_ptr =
      static_cast<flremoting::protocol::ObsDeviceMessage*>(device_infos);
  for (auto& info : lst_filterinfos_) {
    auto type = info.first;
    if (type == AudioFilterType::kDeviceGain) {
      auto value = info.second->GetDBValue();
      device_infos_ptr->set_gain_value(value);
    } else if (type == AudioFilterType::kDeviceDenoise) {
      auto value = info.second->GetSuppressLevelValue();
      device_infos_ptr->set_noise_value(value);
    }
  }
}

void FLOBSIODevice::OBSSourceFilterAdded(void* param, calldata_t* data) {
  FLOBSIODevice* obj = reinterpret_cast<FLOBSIODevice*>(param);
  obs_source_t* filter = (obs_source_t*)calldata_ptr(data, "filter");
  obj->AddFilter(filter);
}

void FLOBSIODevice::OBSSourceFilterRemoved(void* param, calldata_t* data) {
  FLOBSIODevice* obj = reinterpret_cast<FLOBSIODevice*>(param);
  obs_source_t* filter = (obs_source_t*)calldata_ptr(data, "filter");
  obj->RemoveFilter(filter);
}

void FLOBSIODevice::AddFilter(OBSSource filter, bool focus) {
  const char* name = obs_source_get_id(filter);
  auto type = AudioFilterType::kDeviceGain;
  if (kFilterGain == name) {
    type = AudioFilterType::kDeviceGain;
  } else if (kFilterNoiseSuppression == name) {
    type = AudioFilterType::kDeviceDenoise;
  }

  std::unique_ptr<FLOBSFilterInfo> ptr_info =
      std::make_unique<FLOBSFilterInfo>(filter, type);
  lst_filterinfos_[type] = std::move(ptr_info);
}

void FLOBSIODevice::RemoveFilter(OBSSource filter) {
  const char* name = obs_source_get_id(filter);
  auto type = AudioFilterType::kDeviceGain;
  if (kFilterGain == name) {
    type = AudioFilterType::kDeviceGain;
  } else if (kFilterNoiseSuppression == name) {
    type = AudioFilterType::kDeviceDenoise;
  }
  auto it = lst_filterinfos_.find(type);
  if (it != lst_filterinfos_.end()) {
    lst_filterinfos_.erase(it);
  }
}

void FLOBSIODevice::ParseDeviceProperty(obs_property_t* source_property,
                                        void* device_infos) {
  if (!obs_property_visible(source_property)) {
    return;
  }
  obs_property_type source_property_type =
      obs_property_get_type(source_property);
  if (OBS_PROPERTY_LIST != source_property_type) {
    return;
  }
  obs_combo_format format = obs_property_list_format(source_property);
  if (format != OBS_COMBO_FORMAT_STRING) {
    return;
  }
  size_t count = obs_property_list_item_count(source_property);
  auto device_infos_ptr =
      static_cast<flremoting::protocol::ObsDeviceMessage*>(device_infos);
  for (size_t index = 0; index < count; index++) {
    const char* name = obs_property_list_item_name(source_property, index);
    const char* data = obs_property_list_item_string(source_property, index);
    auto device_info = device_infos_ptr->add_device_infos();
    device_info->set_device_name(name);
    device_info->set_device_data(data);
  }
}

void FLOBSIODevice::SetDeviceSource(void* device_infos) {
  if (!source_ || !device_infos) {
    return;
  }
  obs_properties_t* source_properties = obs_source_properties(source_);
  if (!source_properties) {
    return;
  }
  obs_property_t* source_property = obs_properties_first(source_properties);
  while (source_property) {
    ParseDeviceProperty(source_property, device_infos);
    obs_property_next(&source_property);
  }
}
