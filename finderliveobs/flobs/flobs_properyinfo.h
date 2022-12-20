

#ifndef FLOBS_FLOBS_PROPERYINFO_H_
#define FLOBS_FLOBS_PROPERYINFO_H_

#include <stdint.h>
#include <windows.h>

#include <string>
#include <vector>

#include "third_party/obs-studio/src/libobs/obs-properties.h"
#include "third_party/obs-studio/src/libobs/obs.h"
#include "third_party/obs-studio/src/libobs/obs.hpp"

#include "flobs/flobs_info.h"

bool GetInputSourcePropety(obs_source_t* source,
                           std::string property_name,
                           ProperyInfo& info);

void SetInputSourcePropety(obs_source_t* source,
                           const std::vector<ProperyInfo>& infoVec);

#endif  // FLOBS_FLOBS_PROPERYINFO_H_
