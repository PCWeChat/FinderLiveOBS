

#include "flmojo/flmojo_recv.h"
#include "flmojo/flmojo_readinfo_wrapper.h"
#include "flmojo/flmojo_service.h"
#include "flmojo/flmojo_writeinfo_wrapper.h"

#include "flbase/flbase_config.h"
#include "flobs/flobs_camera_beautify.h"
#include "flobs/flobs_manager.h"
#include "flobs/flobs_video_decimator.h"

#include "flproto/proto/flcomm.pb.h"
#include "flproto/proto/flconfig.pb.h"
#include "flproto/proto/flmsgtype.pb.h"
#include "flproto/proto/flobsinfo.pb.h"
#include "flproto/proto/flpreview.pb.h"

#include "flobs/flobs_private_def.h"
#include "flobs/flobs_public_def.h"

#include "base/bind.h"
#include "base/task_scheduler/post_task.h"
#include "flobs/flobs_task_executor.h"

void OnMojoPipeMessagePushDispatcher(uint32_t request_id,
                                     const void* request_info) {
  std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper =
      std::make_unique<FLMojoReadInfoWrapper>(request_id, request_info);
  switch (request_id) {
    case flremoting::protocol::MsgType::LIVEOUTPUT_START: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE, base::BindRepeating([]() {
            FLOBSManager::GetInstance().LiveOutputStart();
          }));
    } break;
    case flremoting::protocol::MsgType::LIVEOUTPUT_STOP: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE, base::BindRepeating([]() {
            FLOBSManager::GetInstance().LiveOutputStop();
          }));
    } break;
    case flremoting::protocol::MsgType::PREVIEWCANVASMODECHANGE: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                flremoting::protocol::PreviewSizeMessage preview_size_msg;
                auto result = preview_size_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  FLOBSManager::GetInstance().UpdatePreviewCanvas(
                      preview_size_msg.cx(), preview_size_msg.cy());
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::OBSITEMINFO: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                std::unique_ptr<flremoting::protocol::ObsItemMessage>
                    ptr_obs_item_msg = std::make_unique<
                        flremoting::protocol::ObsItemMessage>();
                auto result = ptr_obs_item_msg->ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  FLOBSManager::GetInstance().AddSourceData(
                      ptr_obs_item_msg.get());
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::SETINPUTVOLUME: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                flremoting::protocol::InputVolumeData input_volume_data_msg;
                auto result = input_volume_data_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  auto value = input_volume_data_msg.value();
                  FLOBSManager::GetInstance().SetInputVolume(value);
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::SETOUTPUTVOLUME: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                flremoting::protocol::OutputVolumeData output_volume_data_msg;
                auto result = output_volume_data_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  auto value = output_volume_data_msg.value();
                  FLOBSManager::GetInstance().SetOutputVolume(value);
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::SETALLITEMUNSELECT: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE, base::BindRepeating([]() {
            FLOBSManager::GetInstance().SetAllSourceDataUnselect();
          }));
    } break;
    case flremoting::protocol::MsgType::PREVIEWSZCHANGE: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE, base::BindRepeating([]() {
            FLOBSManager::GetInstance().OnPreviewChange();
          }));
    } break;
    case flremoting::protocol::MsgType::OBSCURSORINFO: {
      base::PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                flremoting::protocol::ObsCursorMessage cursor_msg;
                auto result = cursor_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  FLOBSManager::GetInstance().UpdateCurrentSourceCursorSetting(
                      cursor_msg.cursor_status());
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::OBSFPSINFO: {
      base::PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                flremoting::protocol::ObsFPSInfoMessage fps_msg;
                auto result = fps_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  FLOBSManager::GetInstance().SetFPSNum(fps_msg.fpsnum());
                  FLOBSManager::GetInstance().SetFPSDen(fps_msg.fpsden());
                  FLOBSManager::GetInstance().SetVideoFPS();
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::MEANRATEINFO: {
      base::PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                flremoting::protocol::ObsMeanRateInfoMessage mean_rate_msg;
                auto result = mean_rate_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  FLOBSManager::GetInstance().DropVideoFrame(
                      mean_rate_msg.enable(), mean_rate_msg.fpsnum());
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    default:
      break;
  }
}

//
namespace flmojo_resp_util {

void SendMsgByMojoPullResp(
    const google::protobuf::MessageLite& message,
    std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper,
    uint32_t response_type) {
  FLMojoService::GetInstance().SendPullRespProtobufMessage(
      &message, readinfo_wrapper.get(), response_type);
}

void SendMsgByMojoPullResp(
    const std::string& message,
    std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper,
    uint32_t response_type) {
  FLMojoService::GetInstance().SendPullRespStringMessage(
      message, readinfo_wrapper.get(), response_type);
}

}  // namespace flmojo_resp_util

void OnMojoPipeMessagePullDispatcher(uint32_t request_id,
                                     const void* request_info) {
  std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper =
      std::make_unique<FLMojoReadInfoWrapper>(request_id, request_info);
  switch (request_id) {
    case flremoting::protocol::MsgType::STARTREMOTEINFOREQ: {
      FLOBSTaskExecutor::GetUIThreadTaskRunner()->PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                flremoting::protocol::StartRemoteMessage start_remote_msg;
                auto result = start_remote_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  int ret_code = 1;

                  FLOBSManager::GetInstance().InitObsEnv(
                      start_remote_msg.userpath(),
                      reinterpret_cast<HWND>(start_remote_msg.previewhwnd()));

                  auto result = FLOBSManager::GetInstance().OBSInit();
                  if (result) {
                    ret_code = 0;
                  }

                  flremoting::protocol::BaseResponse resp_msg;
                  resp_msg.set_ret_code(ret_code);

                  flmojo_resp_util::SendMsgByMojoPullResp(
                      resp_msg, std::move(readinfo_wrapper),
                      flremoting::protocol::MsgType::STARTREMOTEINFORESP);

                  if (!ret_code) {
                    FLOBSManager::GetInstance().CreatePreviewDisplay();
                  }
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::OBSITEMEDITINFOREQ: {
      base::PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                // construct resp
                flremoting::protocol::ObsItemEditMessageResp item_edit_resp_msg;
                flremoting::protocol::BaseResponse* base_msg =
                    item_edit_resp_msg.mutable_base_msg();
                flremoting::protocol::ObsItemEditMessage* edit_msg =
                    item_edit_resp_msg.mutable_edit_msg();
                int ret_code = 1;

                // process this logic in local thread
                flremoting::protocol::ObsItemEditMessageReq item_edit_req_msg;
                auto result = item_edit_req_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  auto& req_edit_msg = item_edit_req_msg.edit_msg();
                  result = FLOBSManager::GetInstance().EditSourceItem(
                      req_edit_msg.item_id(), req_edit_msg.action_type(),
                      item_edit_req_msg.edit_info());
                  if (result) {
                    edit_msg->CopyFrom(req_edit_msg);
                    ret_code = 0;
                  }
                }
                base_msg->set_ret_code(ret_code);

                flmojo_resp_util::SendMsgByMojoPullResp(
                    item_edit_resp_msg, std::move(readinfo_wrapper),
                    flremoting::protocol::MsgType::OBSITEMEDITINFORESP);
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::SETDEVICEINFOREQ: {
      base::PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                std::unique_ptr<
                    flremoting::protocol::ObsDeviceEditMessageRequest>
                    req_msg = std::make_unique<
                        flremoting::protocol::ObsDeviceEditMessageRequest>();
                auto result = req_msg->ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());
                if (result) {
                  std::string msg =
                      FLOBSManager::GetInstance().SetDeviceValue(req_msg.get());

                  flmojo_resp_util::SendMsgByMojoPullResp(
                      msg, std::move(readinfo_wrapper),
                      flremoting::protocol::MsgType::SETDEVICEINFORESP);
                }
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    case flremoting::protocol::MsgType::UPDATEINPUTINFOREQ:
    case flremoting::protocol::MsgType::UPDATEOUTPUTINFOREQ: {
      base::PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper,
                 uint32_t request_id) {
                flremoting::protocol::ObsDeviceMessage device_msg;
                auto result = device_msg.ParseFromArray(
                    readinfo_wrapper->GetRequestData(),
                    readinfo_wrapper->GetRequestDataSize());

                if (result) {
                  auto is_activate = device_msg.is_activate();

                  std::string msg;
                  uint32_t response_type = 0;
                  if (request_id ==
                      flremoting::protocol::MsgType::UPDATEINPUTINFOREQ) {
                    response_type =
                        flremoting::protocol::MsgType::UPDATEINPUTINFORESP;
                    FLOBSManager::GetInstance().UpdateInputInfo(is_activate,
                                                                &msg);
                  } else if (request_id == flremoting::protocol::MsgType::
                                               UPDATEOUTPUTINFOREQ) {
                    response_type =
                        flremoting::protocol::MsgType::UPDATEOUTPUTINFORESP;
                    FLOBSManager::GetInstance().UpdateOutputInfo(is_activate,
                                                                 &msg);
                  }

                  flmojo_resp_util::SendMsgByMojoPullResp(
                      msg, std::move(readinfo_wrapper), response_type);
                }
              },
              base::Passed(std::move(readinfo_wrapper)), request_id));
    } break;
    default:
      break;
  }
}

void OnMojoSharedMemoryDispatcher(uint32_t request_id,
                                  const void* request_info) {
  std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper =
      std::make_unique<FLMojoReadInfoWrapper>(request_id, request_info);
  switch (request_id) {
    case flremoting::protocol::MsgType::OBSCAMERAVIDEOBEAUTIFYRESP: {
      base::PostTask(
          FROM_HERE,
          base::BindRepeating(
              [](std::unique_ptr<FLMojoReadInfoWrapper> readinfo_wrapper) {
                FLOBSCameraBeautify::GetInstance().WakeCameraVideoBeautify(
                    std::move(readinfo_wrapper));
              },
              base::Passed(std::move(readinfo_wrapper))));
    } break;
    default:
      break;
  }
}