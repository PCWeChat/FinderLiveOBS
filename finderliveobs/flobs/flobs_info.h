

#ifndef FLOBS_FLOBS_INFO_H_
#define FLOBS_FLOBS_INFO_H_

#include <stdint.h>
#include <windows.h>

#include <string>
#include <vector>

#include "third_party/obs-studio/src/libobs/obs-properties.h"

enum class AudioDeviceType : int {
  KUnknownDevice,
  kInputDevice,
  kOutputDevice
};

enum class AudioFilterType : int { kDeviceGain, kDeviceDenoise };

struct SettingInfo {
  std::string secName;
  std::string proName;
  obs_property_type ptype;
  bool bval = false;
  int64_t ival = 0;
  double dval = 0;
  std::string tval;
};

struct CanvasInfo {
  enum CanvasResType {
    CanvasResType_16_9 = 0,
    CanvasResType_16_10,
    CanvasResType_4_3,
    CanvasResType_1_1,
    CanvasResType_20_9,
    CanvasResType_unknow
  };
  SIZE sz = {0};
  CanvasResType res_type = CanvasResType_16_9;
  // 跟resType_对应的分辨率数字，如横屏{16,9}竖屏{9,16}
  SIZE real_type_num = {0};
  // 竖屏
  bool is_portrait = false;
};

struct ProperyInfo {
  std::string pname;
  obs_property_type ptype = OBS_PROPERTY_INVALID;
  bool bval = false;
  int ival = 0;
  double dval = 0;
  std::string tval;
  std::vector<std::pair<std::string, double>> vecdval;
  std::vector<std::pair<std::string, int64_t>> vecival;
  std::vector<std::pair<std::string, std::string>> vectval;
};

struct OBSSourceBaseInfo {
  int64_t item_id;
  std::string source_type;
  std::string source_name;
  bool visible = false;
  bool lock_status = false;

  uint32_t width;
  uint32_t height;

  OBSSourceBaseInfo() = default;
  OBSSourceBaseInfo(int64_t _id,
                    const std::string& _type,
                    const std::string& _name,
                    bool _visible,
                    bool _lock,
                    uint32_t _w,
                    uint32_t _h) {
    item_id = _id;
    source_type = _type;
    source_name = _name;
    visible = _visible;
    lock_status = _lock;
    width = _w;
    height = _h;
  }
};

#endif  // FLOBS_FLOBS_INFO_H_
