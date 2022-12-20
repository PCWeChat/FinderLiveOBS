

#include "flobs/flobs_camera_beautify.h"

#include "flmojo/flmojo_service.h"
#include "flobs/flobs_manager.h"

#include "third_party/obs-studio/src/libobs/obs.h"
#include "third_party/obs-studio/src/libobs/obs.hpp"

#include "third_party/obs-studio/src/plugins/win-dshow/libdshowcapture/dshowcapture.hpp"

#include "flproto/proto/flobsinfo.pb.h"

void OnCameraVideoBeautifyCallback(void* _source,
                                   void* _frame,
                                   size_t _size,
                                   uint32_t _format) {
  FLOBSCameraBeautify::GetInstance().CameraVideoBeautifyCallback(
      _source, _frame, _size, _format);
}

FLOBSCameraBeautify::FLOBSCameraBeautify() {}

FLOBSCameraBeautify::~FLOBSCameraBeautify() {}

void FLOBSCameraBeautify::CameraVideoBeautifyCallback(void* source,
                                                      void* frame,
                                                      size_t size,
                                                      uint32_t format) {
  OBSSceneItemAutoRelease item = obs_scene_sceneitem_from_source(
      FLOBSManager::GetInstance().GetCurrentScene(), (obs_source*)source);
  if (!is_working_ || !item) {
    obs_source_output_video2((obs_source*)source, (obs_source_frame2*)frame);
    return;
  }

  obs_source* source_ptr = nullptr;
  obs_source_frame2* frame_ptr = nullptr;
  auto frame_id = obs_sceneitem_get_id(item);
  DShow::VideoFormat frame_format = static_cast<DShow::VideoFormat>(format);

  {
    std::unique_lock<std::mutex> lock(mutex_);
    std::unique_ptr<CameraBeautifyInfo> camera_beautify_info =
        std::make_unique<CameraBeautifyInfo>();
    camera_beautify_info->source = static_cast<obs_source*>(source);
    camera_beautify_info->frame = static_cast<obs_source_frame2*>(frame);

    auto result = FLMojoService::GetInstance().SendCameraFrameReqMessage(
        frame_id, format, camera_beautify_info->frame->width,
        camera_beautify_info->frame->height,
        camera_beautify_info->frame->data[0], size);

    if (result) {
      lst_infos_[frame_id] = std::move(camera_beautify_info);

      condition_.wait(lock);

      auto it = lst_infos_.find(frame_id);
      if (it != lst_infos_.end()) {
        source_ptr = it->second->source;
        frame_ptr = it->second->frame;
        if (it->second->task) {
          auto const_cast_data =
              const_cast<void*>(it->second->task->GetRequestData());
          frame_ptr->data[0] = reinterpret_cast<uint8_t*>(const_cast_data);
          if (frame_format == DShow::VideoFormat::I420) {
            frame_ptr->data[1] =
                frame_ptr->data[0] + (frame_ptr->width * frame_ptr->height);
            frame_ptr->data[2] =
                frame_ptr->data[1] + (frame_ptr->width * frame_ptr->height / 4);
          } else if (frame_format == DShow::VideoFormat::YV12) {
            frame_ptr->data[2] =
                frame_ptr->data[0] + (frame_ptr->width * frame_ptr->height);
            frame_ptr->data[1] =
                frame_ptr->data[2] + (frame_ptr->width * frame_ptr->height / 4);
          } else if (frame_format == DShow::VideoFormat::NV12) {
            frame_ptr->data[1] =
                frame_ptr->data[0] + (frame_ptr->width * frame_ptr->height);
          }
        }
      }
    } else {
      source_ptr = static_cast<obs_source*>(source);
      frame_ptr = static_cast<obs_source_frame2*>(frame);
    }
  }

  if (source_ptr && frame_ptr) {
    obs_source_output_video2(source_ptr, frame_ptr);
  }
}

void FLOBSCameraBeautify::WakeCameraVideoBeautify(
    std::unique_ptr<FLMojoReadInfoWrapper> task) {
  flremoting::protocol::ObsSourceFrame2Message resp_msg;
  auto result =
      resp_msg.ParseFromArray(task->GetAttachData(), task->GetAttachDataSize());
  std::unique_lock<std::mutex> lock(mutex_);
  if (result) {
    auto it = lst_infos_.find(resp_msg.frame_id());
    if (it != lst_infos_.end()) {
      it->second->task = std::move(task);
    }
  }
  condition_.notify_one();
}

void FLOBSCameraBeautify::CameraVideoBeautifyIsWorking(bool status) {
  is_working_ = status;
}

void FLOBSCameraBeautify::CameraVideoBeautifyDestory(int64_t frame_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = lst_infos_.find(frame_id);
  if (it != lst_infos_.end()) {
    lst_infos_.erase(it);
  }
  condition_.notify_one();
}

void FLOBSCameraBeautify::CameraVideoBeautifyDestoryAll() {
  std::unique_lock<std::mutex> lock(mutex_);
  lst_infos_.clear();
  condition_.notify_all();
}
