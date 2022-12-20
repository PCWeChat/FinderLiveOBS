

#include "flobs/flobs_filter_info.h"

#include "flproto/proto/flmsgtype.pb.h"
#include "flproto/proto/flobsinfo.pb.h"

FLOBSFilterInfo::FLOBSFilterInfo(OBSSource source, AudioFilterType filter_type)
    : source_(source), filter_type_(filter_type) {}

FLOBSFilterInfo::~FLOBSFilterInfo() {}

double FLOBSFilterInfo::GetDBValue() {
  double result = 0;

  if (!source_) {
    return result;
  }

  auto settings = obs_source_get_settings(source_);
  if (!settings) {
    return result;
  }

  auto properties = obs_source_properties(source_);
  auto property = obs_properties_first(properties);
  while (property) {
    const char* name = obs_property_name(property);
    if (stricmp(name, "db") == 0) {
      obs_property_type type = obs_property_get_type(property);
      obs_number_type float_type = obs_property_float_type(property);
      if (obs_property_visible(property) && (OBS_PROPERTY_FLOAT == type) &&
          (OBS_NUMBER_SLIDER == float_type)) {
        result = obs_data_get_double(settings, name);
      }
      break;
    }
    obs_property_next(&property);
  }

  obs_data_release(settings);

  return result;
}

bool FLOBSFilterInfo::SetDBValue(double _value) {
  bool result = false;
  if (!source_) {
    return result;
  }

  auto settings = obs_source_get_settings(source_);
  if (!settings) {
    return result;
  }

  auto properties = obs_source_properties(source_);
  auto property = obs_properties_first(properties);
  while (property) {
    const char* name = obs_property_name(property);
    if (stricmp(name, "db") == 0) {
      obs_property_type type = obs_property_get_type(property);
      obs_number_type float_type = obs_property_float_type(property);
      if ((OBS_PROPERTY_FLOAT == type) && (OBS_NUMBER_SLIDER == float_type)) {
        obs_data_set_double(settings, name, _value);
        obs_source_update(source_, settings);
      }
      break;
    }
    obs_property_next(&property);
  }

  obs_data_release(settings);

  result = true;
  return result;
}

int FLOBSFilterInfo::GetSuppressLevelValue() {
  int result = 0;

  if (!source_) {
    return result;
  }

  auto settings = obs_source_get_settings(source_);
  if (!settings) {
    return result;
  }

  auto properties = obs_source_properties(source_);
  auto property = obs_properties_first(properties);
  while (property) {
    const char* name = obs_property_name(property);
    if (stricmp(name, "suppress_level") == 0) {
      obs_property_type type = obs_property_get_type(property);
      obs_number_type float_type = obs_property_float_type(property);
      if (obs_property_visible(property) && (OBS_PROPERTY_INT == type) &&
          (OBS_NUMBER_SCROLLER == float_type)) {
        result = static_cast<int>(obs_data_get_int(settings, name));
      }
      break;
    }
    obs_property_next(&property);
  }

  obs_data_release(settings);

  return result;
}

bool FLOBSFilterInfo::SetSuppressLevelValue(int value) {
  bool result = false;
  if (!source_) {
    return result;
  }

  auto settings = obs_source_get_settings(source_);
  if (!settings) {
    return result;
  }

  auto properties = obs_source_properties(source_);
  auto property = obs_properties_first(properties);
  while (property) {
    const char* name = obs_property_name(property);
    if (stricmp(name, "suppress_level") == 0) {
      obs_property_type type = obs_property_get_type(property);
      obs_number_type float_type = obs_property_float_type(property);
      if ((OBS_PROPERTY_INT == type) && (OBS_NUMBER_SCROLLER == float_type)) {
        obs_data_set_int(settings, name, value);
        obs_source_update(source_, settings);
      }
      break;
    }
    obs_property_next(&property);
  }

  obs_data_release(settings);

  result = true;
  return result;
}

AudioFilterType FLOBSFilterInfo::GetType() {
  return filter_type_;
}

OBSSource FLOBSFilterInfo::GetFilter() {
  return source_;
}
