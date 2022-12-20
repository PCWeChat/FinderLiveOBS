

#ifndef FLOBS_FLOBS_CAMERA_BEAUTIFY_H_
#define FLOBS_FLOBS_CAMERA_BEAUTIFY_H_

#include <stdint.h>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <utility>

#include "flmojo/flmojo_readinfo_wrapper.h"
#include "flmojo/flmojo_writeinfo_wrapper.h"

struct obs_source_frame2;
struct obs_source;

struct CameraBeautifyInfo {
  obs_source_frame2* frame = nullptr;
  obs_source* source = nullptr;
  std::unique_ptr<FLMojoReadInfoWrapper> task;
};

void OnCameraVideoBeautifyCallback(void* source,
                                   void* frame,
                                   size_t size,
                                   uint32_t format);

class FLOBSCameraBeautify {
 public:
  static FLOBSCameraBeautify& GetInstance() {
    static FLOBSCameraBeautify instance;
    return instance;
  }

  FLOBSCameraBeautify();

  FLOBSCameraBeautify(const FLOBSCameraBeautify&) = delete;
  FLOBSCameraBeautify& operator=(const FLOBSCameraBeautify&) = delete;

  ~FLOBSCameraBeautify();

  void CameraVideoBeautifyCallback(void* source,
                                   void* frame,
                                   size_t size,
                                   uint32_t format);

  void WakeCameraVideoBeautify(std::unique_ptr<FLMojoReadInfoWrapper> task);
  void CameraVideoBeautifyIsWorking(bool status);
  void CameraVideoBeautifyDestory(int64_t frame_id);
  void CameraVideoBeautifyDestoryAll();

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  obs_source_frame2* frame_ = nullptr;
  obs_source* source_ = nullptr;
  std::atomic<bool> is_working_ = true;
  std::map<std::int64_t, std::unique_ptr<CameraBeautifyInfo>> lst_infos_;
};

#endif  // FLOBS_FLOBS_CAMERA_BEAUTIFY_H_
