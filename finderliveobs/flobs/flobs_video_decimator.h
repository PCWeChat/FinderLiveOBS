

#ifndef FLOBS_FLOBS_VIDEO_DECIMATOR_H_
#define FLOBS_FLOBS_VIDEO_DECIMATOR_H_

#include <stdint.h>
#include <atomic>

class FLOBSVideoDecimator {
 public:
  static FLOBSVideoDecimator& GetInstance() {
    static FLOBSVideoDecimator instance;
    return instance;
  }

  FLOBSVideoDecimator();

  FLOBSVideoDecimator(const FLOBSVideoDecimator&) = delete;
  FLOBSVideoDecimator& operator=(const FLOBSVideoDecimator&) = delete;

  ~FLOBSVideoDecimator();

  void Reset();
  bool ResetIfNeed(int target_frame_rate, float incoming_frame_rate);
  void EnableTemporalDecimation(bool enable);
  void SetTargetFramerate(int frame_rate);
  void SetIncomingFramerate(float frame_rate);
  bool DropFrame();

 private:
  std::atomic<int> overshoot_modifier_ = 0;
  std::atomic<uint32_t> drop_count_ = 0;
  std::atomic<uint32_t> keep_count_ = 1;
  std::atomic<uint32_t> target_frame_rate_ = 0;
  std::atomic<float> incoming_frame_rate_ = 0.0f;
  std::atomic<bool> enable_temporal_decimation_ = false;
};

#endif  // FLOBS_FLOBS_VIDEO_DECIMATOR_H_
