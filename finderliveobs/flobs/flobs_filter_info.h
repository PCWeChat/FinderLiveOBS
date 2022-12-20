

#ifndef FLOBS_FLOBS_FILTER_INFO_H_
#define FLOBS_FLOBS_FILTER_INFO_H_

#include <stdint.h>
#include <map>
#include <memory>
#include <string>

#include "third_party/obs-studio/src/libobs/obs.h"
#include "third_party/obs-studio/src/libobs/obs.hpp"
#include "third_party/obs-studio/src/libobs/util/util.hpp"

#include "flobs/flobs_info.h"

class FLOBSFilterInfo {
 public:
  explicit FLOBSFilterInfo(OBSSource source, AudioFilterType filter_type);

  FLOBSFilterInfo(const FLOBSFilterInfo&) = delete;
  FLOBSFilterInfo& operator=(const FLOBSFilterInfo&) = delete;

  ~FLOBSFilterInfo();

  double GetDBValue();
  bool SetDBValue(double value);

  int GetSuppressLevelValue();
  bool SetSuppressLevelValue(int value);

  AudioFilterType GetType();

  OBSSource GetFilter();

 private:
  OBSSource source_;
  AudioFilterType filter_type_;
};

#endif  // FLOBS_FLOBS_FILTER_INFO_H_
