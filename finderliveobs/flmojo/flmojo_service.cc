

#include "flmojo/flmojo_service.h"
#include "flmojo/flmojo_recv.h"

#include "flmojo/flmojo.h"
#include "flmojo/flmojo_readinfo_wrapper.h"
#include "flmojo/flmojo_writeinfo_wrapper.h"

#include "flobs/flobs_manager.h"

#include "flproto/proto/flframe.pb.h"
#include "flproto/proto/flmsgtype.pb.h"
#include "flproto/proto/flobsinfo.pb.h"

#ifdef _WIN32
#include <windows.h>
#endif

enum FLMojoAppendMessagePipeType : uint32_t {
  kFLAppendVideo,
  kFLAppendAudio,
  kFLAppendCamera,
  kFLAppendMaxValue
};

FLMojoService::FLMojoService() {}

FLMojoService::~FLMojoService() {}

void FLMojoService::OnReadPush(uint32_t request_id, const void* request_info) {
  OnMojoPipeMessagePushDispatcher(request_id, request_info);
}

void FLMojoService::OnReadPull(uint32_t request_id, const void* request_info) {
  OnMojoPipeMessagePullDispatcher(request_id, request_info);
}

void FLMojoService::OnReadShared(uint32_t request_id,
                                 const void* request_info) {
  OnMojoSharedMemoryDispatcher(request_id, request_info);
}

void FLMojoService::OnRemoteConnect(bool is_connected) {
  if (is_connected) {
    SendEstablishMessage();
  } else {
    ShutdownProcess();
  }
}

void FLMojoService::OnRemoteDisconnect() {
  FLOBSManager::GetInstance().SaveProject();
  ShutdownProcess();
}

void FLMojoService::OnRemoteProcessLaunched() {}

void FLMojoService::OnRemoteProcessLaunchFailed(int error_code) {}

void FLMojoService::OnRemoteMojoError(const void* errorbuf, int errorsize) {
  ShutdownProcess();
}

void FLMojoService::Start() {
  mmmojo_env_ = CreateFLMojoEnvironment();

  SetFLMojoEnvironmentCallbacks(mmmojo_env_,
                                FLMojoEnvironmentCallbackType::kFLUserData,
                                dynamic_cast<FLMojoDelegate*>(this));
  SetFLMojoEnvironmentCallbacksWrapper(mmmojo_env_);

  SetFLMojoEnvironmentInitParams(
      mmmojo_env_, FLMojoEnvironmentInitParamType::kFLAddNumMessagepipe,
      FLMojoAppendMessagePipeType::kFLAppendMaxValue);

  StartFLMojoEnvironment(mmmojo_env_);
}

void FLMojoService::SendPushStringMessage(uint32_t request_id,
                                          const std::string& request_msg) {
  FLMojoWriteInfoWrapper writeinfo_wrapper(FLMojoInfoMethod::kFLPush, false,
                                           request_id, request_msg.size());
  memcpy(writeinfo_wrapper.GetRequestData(), request_msg.c_str(),
         request_msg.size());
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

void FLMojoService::SendPullReqStringMessage(uint32_t request_id,
                                             const std::string& request_msg) {
  FLMojoWriteInfoWrapper writeinfo_wrapper(FLMojoInfoMethod::kFLPullReq, false,
                                           request_id, request_msg.size());
  memcpy(writeinfo_wrapper.GetRequestData(), request_msg.c_str(),
         request_msg.size());
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

void FLMojoService::SendPushProtobufMessage(uint32_t request_id,
                                            const void* protobuf_message) {
  const google::protobuf::MessageLite* cast_protobuf_message =
      reinterpret_cast<const google::protobuf::MessageLite*>(protobuf_message);
  FLMojoWriteInfoWrapper writeinfo_wrapper(FLMojoInfoMethod::kFLPush, false,
                                           request_id,
                                           cast_protobuf_message->ByteSize());
  cast_protobuf_message->SerializeToArray(
      writeinfo_wrapper.GetRequestData(),
      writeinfo_wrapper.GetRequestDataSize());
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

void FLMojoService::SendPullRespStringMessage(const std::string& message,
                                              void* readinfo_wrapper,
                                              uint32_t response_type) {
  auto* readinfo_wrapper_ptr =
      static_cast<FLMojoReadInfoWrapper*>(readinfo_wrapper);
  auto response_msgsize = message.size();
  FLMojoWriteInfoWrapper writeinfo_wrapper(FLMojoInfoMethod::kFLPullResp, false,
                                           response_type, response_msgsize);
  writeinfo_wrapper.SwapFLMojoInfoCallback(
      const_cast<void*>(readinfo_wrapper_ptr->GetRequestInfo()));
  memcpy(writeinfo_wrapper.GetRequestData(), message.c_str(), response_msgsize);
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

void FLMojoService::SendPullRespProtobufMessage(const void* message,
                                                void* readinfo_wrapper,
                                                uint32_t response_type) {
  auto* readinfo_wrapper_ptr =
      static_cast<FLMojoReadInfoWrapper*>(readinfo_wrapper);
  const auto* message_ptr =
      static_cast<const google::protobuf::MessageLite*>(message);
  auto response_msgsize = message_ptr->ByteSizeLong();
  FLMojoWriteInfoWrapper writeinfo_wrapper(FLMojoInfoMethod::kFLPullResp, false,
                                           response_type, response_msgsize);
  writeinfo_wrapper.SwapFLMojoInfoCallback(
      const_cast<void*>(readinfo_wrapper_ptr->GetRequestInfo()));
  message_ptr->SerializeToArray(writeinfo_wrapper.GetRequestData(),
                                response_msgsize);
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

void FLMojoService::SendVideoFrameMessage(const void* frame_data,
                                          uint32_t frame_length,
                                          uint32_t frame_width,
                                          uint32_t frame_height,
                                          uint64_t frame_timestamp,
                                          int frame_rotation) {
  flremoting::protocol::FrameVideoMessage frame_video_msg;
  frame_video_msg.set_frame_id(0);
  frame_video_msg.set_frame_width(frame_width);
  frame_video_msg.set_frame_height(frame_height);
  frame_video_msg.set_frame_timestamp(frame_timestamp);
  frame_video_msg.set_frame_rotation(frame_rotation);

  auto attach_msgsize = frame_video_msg.ByteSizeLong();

  FLMojoWriteInfoWrapper writeinfo_wrapper(
      FLMojoInfoMethod::kFLShared, true,
      flremoting::protocol::MsgType::LIVEOUTPUT_FRAME_VIDEO, frame_length,
      attach_msgsize);

  auto request_data = writeinfo_wrapper.GetRequestData();
  if (!request_data) {
    return;
  }
  memcpy(request_data, frame_data, frame_length);
  frame_video_msg.SerializeToArray(writeinfo_wrapper.GetAttachData(),
                                   attach_msgsize);
  writeinfo_wrapper.SetUseAppendMessagePipe(
      FLMojoAppendMessagePipeType::kFLAppendVideo);
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

void FLMojoService::SendAudioFrameMessage(const void* frame_data,
                                          uint32_t frame_length,
                                          uint32_t frame_samplerate,
                                          uint32_t frame_channel,
                                          uint64_t frame_timestamp) {
  flremoting::protocol::FrameAudioMessage frame_audio_msg;
  frame_audio_msg.set_frame_id(0);
  frame_audio_msg.set_frame_rate(frame_samplerate);
  frame_audio_msg.set_frame_channel(frame_channel);
  frame_audio_msg.set_frame_timestamp(frame_timestamp);

  auto attach_msgsize = frame_audio_msg.ByteSizeLong();

  FLMojoWriteInfoWrapper writeinfo_wrapper(
      FLMojoInfoMethod::kFLShared, true,
      flremoting::protocol::MsgType::LIVEOUTPUT_FRAME_AUDIO, frame_length,
      attach_msgsize);

  auto request_data = writeinfo_wrapper.GetRequestData();
  if (!request_data) {
    return;
  }
  memcpy(request_data, frame_data, frame_length);
  frame_audio_msg.SerializeToArray(writeinfo_wrapper.GetAttachData(),
                                   attach_msgsize);
  writeinfo_wrapper.SetUseAppendMessagePipe(
      FLMojoAppendMessagePipeType::kFLAppendAudio);
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

bool FLMojoService::SendCameraFrameReqMessage(int64_t frame_id,
                                              uint32_t frame_format,
                                              uint32_t frame_width,
                                              uint32_t frame_height,
                                              const void* frame_data,
                                              size_t frame_size) {
  flremoting::protocol::ObsSourceFrame2Message frame2_msg;

  frame2_msg.set_frame_id(frame_id);
  frame2_msg.set_frame_format(frame_format);
  frame2_msg.set_frame_width(frame_width);
  frame2_msg.set_frame_height(frame_height);

  auto attach_msgsize = frame2_msg.ByteSizeLong();

  FLMojoWriteInfoWrapper writeinfo_wrapper(
      FLMojoInfoMethod::kFLShared, false,
      flremoting::protocol::MsgType::OBSCAMERAVIDEOBEAUTIFYREQ, frame_size,
      attach_msgsize);

  auto request_data = writeinfo_wrapper.GetRequestData();
  if (!request_data) {
    return false;
  }
  memcpy(request_data, frame_data, frame_size);
  frame2_msg.SerializeToArray(writeinfo_wrapper.GetAttachData(),
                              attach_msgsize);
  writeinfo_wrapper.SetUseAppendMessagePipe(
      FLMojoAppendMessagePipeType::kFLAppendCamera);
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
  return true;
}

void FLMojoService::SendEstablishMessage() {
  FLMojoWriteInfoWrapper writeinfo_wrapper(
      FLMojoInfoMethod::kFLPush, false,
      flremoting::protocol::MsgType::REMOTEESTABLISH);
  writeinfo_wrapper.SendRequestMessage(mmmojo_env_);
}

void FLMojoService::ShutdownProcess() {
#ifdef _WIN32
  ::TerminateProcess(GetCurrentProcess(), static_cast<UINT>(0x33));
#endif
}