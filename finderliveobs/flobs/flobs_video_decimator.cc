

// modidy from
// https://android.googlesource.com/platform/external/webrtc/+/0f59a88/webrtc/modules/video_processing/video_decimator.cc
// https://chromium.googlesource.com/external/webrtc/stable/src/+/master/modules/video_processing/main/source/video_decimator.cc
#include "flobs/flobs_video_decimator.h"

FLOBSVideoDecimator::FLOBSVideoDecimator() {}

FLOBSVideoDecimator::~FLOBSVideoDecimator() {}

void FLOBSVideoDecimator::Reset() {
  overshoot_modifier_ = 0;
  drop_count_ = 0;
  // here we set keep count is 1
  keep_count_ = 1;
  target_frame_rate_ = 0;
  incoming_frame_rate_ = 0.0f;
  enable_temporal_decimation_ = false;
}

void FLOBSVideoDecimator::EnableTemporalDecimation(bool enable) {
  enable_temporal_decimation_ = enable;
}

void FLOBSVideoDecimator::SetTargetFramerate(int frame_rate) {
  target_frame_rate_ = frame_rate;
}

void FLOBSVideoDecimator::SetIncomingFramerate(float frame_rate) {
  incoming_frame_rate_ = frame_rate;
}

bool FLOBSVideoDecimator::ResetIfNeed(int target_frame_rate,
                                      float incoming_frame_rate) {
  return target_frame_rate != target_frame_rate_ ||
         incoming_frame_rate != incoming_frame_rate_;
}

bool FLOBSVideoDecimator::DropFrame() {
  if (!enable_temporal_decimation_)
    return false;
  if (incoming_frame_rate_ <= 0)
    return false;
  const uint32_t incomingframe_rate =
      static_cast<uint32_t>(incoming_frame_rate_ + 0.5f);
  if (target_frame_rate_ == 0)
    return true;
  bool drop = false;
  if (incomingframe_rate > target_frame_rate_) {
    int overshoot =
        overshoot_modifier_ + (incomingframe_rate - target_frame_rate_);
    if (overshoot < 0) {
      overshoot = 0;
      overshoot_modifier_ = 0;
    }
    if (overshoot && 2 * overshoot < (int)incomingframe_rate) {
      if (drop_count_) {  // Just got here so drop to be sure.
        drop_count_ = 0;
        return true;
      }
      const uint32_t dropVar = incomingframe_rate / overshoot;
      if (keep_count_ >= dropVar) {
        drop = true;
        overshoot_modifier_ = -((int)incomingframe_rate % overshoot) / 3;
        keep_count_ = 1;
      } else {
        keep_count_++;
      }
    } else {
      keep_count_ = 0;
      const uint32_t dropVar = overshoot / target_frame_rate_;
      if (drop_count_ < dropVar) {
        drop = true;
        drop_count_++;
      } else {
        overshoot_modifier_ = overshoot % target_frame_rate_;
        drop = false;
        drop_count_ = 0;
      }
    }
  }
  return drop;
}
