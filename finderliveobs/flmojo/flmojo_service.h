

#ifndef FLMOJO_FLMOJO_SERVICE_H_
#define FLMOJO_FLMOJO_SERVICE_H_

#include <stdint.h>
#include <string>

#include "flmojo/flmojo.h"
#include "flmojo/flmojo_delegate.h"

class FLMojoService : public FLMojoDelegate {
 public:
  static FLMojoService& GetInstance() {
    static FLMojoService instance;
    return instance;
  }

  FLMojoService();

  FLMojoService(const FLMojoService&) = delete;
  FLMojoService& operator=(const FLMojoService&) = delete;

  ~FLMojoService();

  void Start();

  void SendEstablishMessage();

  // string
  void SendPushStringMessage(uint32_t request_id,
                             const std::string& request_msg);
  void SendPullReqStringMessage(uint32_t request_id,
                                const std::string& request_msg);
  void SendPullRespStringMessage(const std::string& message,
                                 void* readinfo_wrapper,
                                 uint32_t response_type);

  // protobuf
  void SendPushProtobufMessage(uint32_t request_id,
                               const void* protobuf_message);
  void SendPullRespProtobufMessage(const void* message,
                                   void* readinfo_wrapper,
                                   uint32_t response_type);

  // using shared memory
  void SendVideoFrameMessage(const void* frame_data,
                             uint32_t frame_length,
                             uint32_t frame_width,
                             uint32_t frame_height,
                             uint64_t frame_timestamp,
                             int frame_rotation);
  void SendAudioFrameMessage(const void* frame_data,
                             uint32_t frame_length,
                             uint32_t frame_samplerate,
                             uint32_t frame_channel,
                             uint64_t frame_timestamp);
  bool SendCameraFrameReqMessage(int64_t frame_id,
                                 uint32_t frame_format,
                                 uint32_t frame_width,
                                 uint32_t frame_height,
                                 const void* frame_data,
                                 size_t frame_size);

 private:
  void OnReadPush(uint32_t request_id, const void* request_info) override;
  void OnReadPull(uint32_t request_id, const void* request_info) override;
  void OnReadShared(uint32_t request_id, const void* request_info) override;
  void OnRemoteConnect(bool is_connected) override;
  void OnRemoteDisconnect() override;
  void OnRemoteProcessLaunched() override;
  void OnRemoteProcessLaunchFailed(int error_code) override;
  void OnRemoteMojoError(const void* errorbuf, int errorsize) override;

  void ShutdownProcess();

  void* mmmojo_env_ = nullptr;
};

#endif  // FLMOJO_FLMOJO_SERVICE_H_
