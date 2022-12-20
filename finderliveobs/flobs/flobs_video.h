

#ifndef FLOBS_FLOBS_VIDEO_H_
#define FLOBS_FLOBS_VIDEO_H_

#include "third_party/obs-studio/src/libobs/media-io/video-io.h"
#include "third_party/obs-studio/src/libobs/util/dstr.h"
#include "third_party/obs-studio/src/libobs/util/util.hpp"

static inline enum video_format GetVideoFormatFromName(const char* name) {
  if (astrcmpi(name, "I420") == 0)
    return VIDEO_FORMAT_I420;
  else if (astrcmpi(name, "NV12") == 0)
    return VIDEO_FORMAT_NV12;
  else if (astrcmpi(name, "I444") == 0)
    return VIDEO_FORMAT_I444;
#if 0  // currently unsupported
	else if (astrcmpi(name, "YVYU") == 0)
		return VIDEO_FORMAT_YVYU;
	else if (astrcmpi(name, "YUY2") == 0)
		return VIDEO_FORMAT_YUY2;
	else if (astrcmpi(name, "UYVY") == 0)
		return VIDEO_FORMAT_UYVY;
#endif
  else
    return VIDEO_FORMAT_RGBA;
}

static inline enum obs_scale_type GetScaleType(ConfigFile& basicConfig) {
  const char* scaleTypeStr =
      config_get_string(basicConfig, "Video", "ScaleType");

  if (astrcmpi(scaleTypeStr, "bilinear") == 0)
    return OBS_SCALE_BILINEAR;
  else if (astrcmpi(scaleTypeStr, "lanczos") == 0)
    return OBS_SCALE_LANCZOS;
  else if (astrcmpi(scaleTypeStr, "area") == 0)
    return OBS_SCALE_AREA;
  else
    return OBS_SCALE_BICUBIC;
}

#endif  // FLOBS_FLOBS_VIDEO_H_
