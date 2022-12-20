

#ifndef FLOBS_FLOBS_IODEVICE_H_
#define FLOBS_FLOBS_IODEVICE_H_

#include <stdint.h>
#include <map>
#include <memory>
#include <string>

#include "third_party/obs-studio/src/libobs/obs.h"
#include "third_party/obs-studio/src/libobs/obs.hpp"
#include "third_party/obs-studio/src/libobs/util/util.hpp"

#include "flobs/flobs_info.h"

class FLOBSFilterInfo;
class FLOBSIODevice {
 public:
  explicit FLOBSIODevice(OBSSource source, AudioDeviceType device_type);

  FLOBSIODevice(const FLOBSIODevice&) = delete;
  FLOBSIODevice& operator=(const FLOBSIODevice&) = delete;

  ~FLOBSIODevice();

  void UpdateFilters();
  void SetFiltersValue(void* device_infos);

  void SetDeviceSource(void* device_infos);

  void AddFilterInfo(const char* id);
  bool RemoveFilterInfo(AudioFilterType type);

  //
  bool SetDeviceId(const std::string& device_id);
  std::string GetDeviceId();

  bool SetDBValue(double value);
  bool SetSuppressLevelValue(int value);

  int GetVolume();
  void SetVolume(int value);

  int GetMonitoringType();
  bool SetMonitoringType(int value);

  bool IsSameSource(OBSSource source);

 private:
  static void OBSSourceFilterAdded(void* param, calldata_t* data);
  static void OBSSourceFilterRemoved(void* param, calldata_t* data);

  void AddFilter(OBSSource filter, bool focus = true);
  void RemoveFilter(OBSSource filter);

  void ParseDeviceProperty(obs_property_t* property, void* device_infos);

  OBSSource source_;
  AudioDeviceType device_type_;
  std::map<AudioFilterType, std::unique_ptr<FLOBSFilterInfo>> lst_filterinfos_;

  OBSSignal add_signal_;
  OBSSignal remove_signal_;
};

#endif  // FLOBS_FLOBS_IODEVICE_H_
