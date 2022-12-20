

#include "flobs/flobs_inforeport_service.h"

#include "flmojo/flmojo_send.h"

#include "flproto/proto/flframe.pb.h"
#include "flproto/proto/flmsgtype.pb.h"
#include "flproto/proto/flobsinfo.pb.h"

#include "third_party/obs-studio/src/libobs/obs.h"
#include "third_party/obs-studio/src/libobs/obs.hpp"

#include "flobs/flobs_manager.h"

FLOBSInfoReportService::FLOBSInfoReportService() {}

FLOBSInfoReportService::~FLOBSInfoReportService() {}

void FLOBSInfoReportService::__Init() {
  is_working_ = true;
  thread_ = std::make_shared<std::thread>([this]() { this->__RunLoop(); });
}

void FLOBSInfoReportService::__Cleanup() {
  is_working_ = false;
  condition_.notify_all();
  if (thread_ && thread_->joinable()) {
    thread_->join();
    thread_ = nullptr;
  }
}

void FLOBSInfoReportService::__RunLoop() {
  while (is_working_) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      // has spurious wake up
      condition_.wait_for(lock, std::chrono::seconds(1));
    }

    if (!is_working_) {
      break;
    }

    flremoting::protocol::ObsSceneItemInfoMessage infos_msg;
    infos_msg.set_fps(obs_get_active_fps());
    infos_msg.set_peak_volume(FLOBSManager::GetInstance().GetNowPeakVolume());

    OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();
    if (scene) {
      obs_scene_enum_items(
          scene,
          [](obs_scene_t* scene, obs_sceneitem_t* item, void* param) -> bool {
            flremoting::protocol::ObsSceneItemInfoMessage* infos_msg =
                reinterpret_cast<
                    flremoting::protocol::ObsSceneItemInfoMessage*>(param);
            auto* info_msg = infos_msg->add_infos();

            obs_transform_info info;
            obs_source_t* source = obs_sceneitem_get_source(item);
            info_msg->set_itemid(obs_sceneitem_get_id(item));
            obs_sceneitem_get_info(item, &info);
            info_msg->set_name(obs_source_get_unversioned_id(source));
            info_msg->set_posx(info.pos.x);
            info_msg->set_posy(info.pos.y);

            {
              obs_sceneitem_crop crop;
              vec2 scale;
              obs_sceneitem_get_scale(item, &scale);
              obs_sceneitem_get_crop(item, &crop);
              info_msg->set_width(
                  float(obs_source_get_width(source) - crop.left - crop.right) *
                  scale.x);
              info_msg->set_height(float(obs_source_get_height(source) -
                                         crop.top - crop.bottom) *
                                   scale.y);
            }
            info_msg->set_visible(obs_sceneitem_visible(item));
            return true;
          },
          &infos_msg);
    }

    std::string send_msg;
    auto result = infos_msg.SerializeToString(&send_msg);
    if (result) {
      SendObsSceneItemInfoMessage(send_msg);
    }
  }
}
