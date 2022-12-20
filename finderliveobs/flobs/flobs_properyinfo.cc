

#include "flobs/flobs_properyinfo.h"

#include <memory>

bool GetInputSourcePropety(obs_source_t* source,
                           std::string property_name,
                           ProperyInfo& info) {
  if (source == nullptr) {
    return false;
  }

  std::unique_ptr<obs_properties_t, decltype(&obs_properties_destroy)> props(
      obs_source_properties(source), obs_properties_destroy);
  OBSDataAutoRelease settings = obs_source_get_settings(source);

  obs_property_t* property = obs_properties_first(props.get());

  while (property) {
    const char* name = obs_property_name(property);
    if (property_name == name) {
      info.pname = name;
      info.ptype = obs_property_get_type(property);
      switch (info.ptype) {
        case OBS_PROPERTY_INVALID: {
        } break;
        case OBS_PROPERTY_BOOL: {
          info.bval = obs_data_get_bool(settings, name);
        } break;
        case OBS_PROPERTY_INT:
        case OBS_PROPERTY_COLOR: {
          info.ival = obs_data_get_int(settings, name);
        } break;
        case OBS_PROPERTY_FLOAT: {
          info.dval = obs_data_get_double(settings, name);
        } break;
        case OBS_PROPERTY_TEXT: {
          info.tval = obs_data_get_string(settings, name);
        } break;
        case OBS_PROPERTY_PATH: {
          info.tval = obs_data_get_string(settings, name);
        } break;
        case OBS_PROPERTY_LIST: {
          obs_combo_type type = obs_property_list_type(property);
          obs_combo_format format = obs_property_list_format(property);
          size_t count = obs_property_list_item_count(property);

          if (format == OBS_COMBO_FORMAT_INT) {
            info.ival = obs_data_get_int(settings, name);
          } else if (format == OBS_COMBO_FORMAT_FLOAT) {
            info.dval = obs_data_get_double(settings, name);
          } else if (format == OBS_COMBO_FORMAT_STRING) {
            info.tval = obs_data_get_string(settings, name);
          }

          for (size_t idx = 0; idx < count; idx++) {
            const char* name = obs_property_list_item_name(property, idx);
            if (format == OBS_COMBO_FORMAT_INT) {
              int64_t val = obs_property_list_item_int(property, idx);
              info.vecival.push_back(std::make_pair(std::string(name), val));

            } else if (format == OBS_COMBO_FORMAT_FLOAT) {
              double val = obs_property_list_item_float(property, idx);
              info.vecdval.push_back(std::make_pair(std::string(name), val));

            } else if (format == OBS_COMBO_FORMAT_STRING) {
              std::string val = obs_property_list_item_string(property, idx);
              info.vectval.push_back(std::make_pair(std::string(name), val));
            }
          }
        } break;
        case OBS_PROPERTY_FONT:
        case OBS_PROPERTY_BUTTON:
        case OBS_PROPERTY_EDITABLE_LIST:
        case OBS_PROPERTY_FRAME_RATE:
        case OBS_PROPERTY_GROUP:
        default:
          break;
      }
      break;
    }
    obs_property_next(&property);
  }

  return true;
}

void SetInputSourcePropety(obs_source_t* source,
                           const std::vector<ProperyInfo>& infoVec) {
  if (!infoVec.size()) {
    return;
  }

  OBSDataAutoRelease settings = obs_source_get_settings(source);
  std::unique_ptr<obs_properties_t, decltype(&obs_properties_destroy)> props(
      obs_source_properties(source), obs_properties_destroy);

  for (auto info : infoVec) {
    if (info.ptype == OBS_PROPERTY_INT) {
      obs_data_set_int(settings, info.pname.c_str(), info.ival);
    }
    if (info.ptype == OBS_PROPERTY_BOOL) {
      obs_data_set_bool(settings, info.pname.c_str(), info.bval);
    } else if (info.ptype == OBS_PROPERTY_FLOAT) {
      obs_data_set_double(settings, info.pname.c_str(), info.dval);
    } else if (info.ptype == OBS_PROPERTY_PATH ||
               info.ptype == OBS_PROPERTY_TEXT ||
               info.ptype == OBS_PROPERTY_LIST) {
      obs_data_set_string(settings, info.pname.c_str(), info.tval.c_str());
    }

    obs_property_t* property =
        obs_properties_get(props.get(), info.pname.c_str());
    obs_property_modified(property, settings);
  }

  obs_source_update(source, settings);
}
