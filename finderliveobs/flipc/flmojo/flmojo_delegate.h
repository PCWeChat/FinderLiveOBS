

#ifndef MMMOJO_FLMOJO_DELEGATE_H_
#define MMMOJO_FLMOJO_DELEGATE_H_

#include "flmojo/flmojo.h"

class FLMojoDelegate {
 public:
  FLMojoDelegate() = default;

  FLMojoDelegate(const FLMojoDelegate&) = delete;
  FLMojoDelegate& operator=(const FLMojoDelegate&) = delete;

  virtual ~FLMojoDelegate() = default;

  virtual void OnReadPush(uint32_t request_id, const void* request_info) = 0;
  virtual void OnReadPull(uint32_t request_id, const void* request_info) = 0;
  virtual void OnReadShared(uint32_t request_id, const void* request_info) = 0;
  virtual void OnRemoteConnect(bool is_connected) = 0;
  virtual void OnRemoteDisconnect() = 0;
  virtual void OnRemoteProcessLaunched() = 0;
  virtual void OnRemoteProcessLaunchFailed(int error_code) = 0;
  virtual void OnRemoteMojoError(const void* errorbuf, int errorsize) = 0;
};

// wrapper
static void OnFLPushWrapper(uint32_t request_id,
                            const void* request_info,
                            void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnReadPush(request_id, request_info);
}

static void OnFLPullWrapper(uint32_t request_id,
                            const void* request_info,
                            void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnReadPull(request_id, request_info);
}

static void OnFLSharedWrapper(uint32_t request_id,
                              const void* request_info,
                              void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnReadShared(request_id,
                                                        request_info);
}

static void OnFLRemoteConnectWrapper(bool is_connected, void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnRemoteConnect(is_connected);
}

static void OnFLRemoteDisconnectWrapper(void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnRemoteDisconnect();
}

static void OnFLRemoteProcessLaunchedWrapper(void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnRemoteProcessLaunched();
}

static void OnFLRemoteProcessLaunchFailedWrapper(int error_code,
                                                 void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnRemoteProcessLaunchFailed(
      error_code);
}

static void OnFLRemoteMojoErrorWrapper(const void* errorbuf,
                                       int errorsize,
                                       void* user_data) {
  static_cast<FLMojoDelegate*>(user_data)->OnRemoteMojoError(errorbuf,
                                                             errorsize);
}

struct FLMojoEnvironmentCallbacks {
  FLMojoEnvironmentCallbackType type;
  void* func;
};

static FLMojoEnvironmentCallbacks g_flmojo_env_callbacks[] = {
    {FLMojoEnvironmentCallbackType::kFLReadPush, &OnFLPushWrapper},
    {FLMojoEnvironmentCallbackType::kFLReadPull, &OnFLPullWrapper},
    {FLMojoEnvironmentCallbackType::kFLReadShared, &OnFLSharedWrapper},
    {FLMojoEnvironmentCallbackType::kFLRemoteConnect,
     &OnFLRemoteConnectWrapper},
    {FLMojoEnvironmentCallbackType::kFLRemoteDisconnect,
     &OnFLRemoteDisconnectWrapper},
    {FLMojoEnvironmentCallbackType::kFLRemoteProcessLaunched,
     &OnFLRemoteProcessLaunchedWrapper},
    {FLMojoEnvironmentCallbackType::kFLRemoteProcessLaunchFailed,
     &OnFLRemoteProcessLaunchFailedWrapper},
    {FLMojoEnvironmentCallbackType::kFLRemoteMojoError,
     &OnFLRemoteMojoErrorWrapper},
};

static void SetFLMojoEnvironmentCallbacksWrapper(void* mmmojo_env) {
  for (auto& callback : g_flmojo_env_callbacks) {
    SetFLMojoEnvironmentCallbacks(mmmojo_env, callback.type, callback.func);
  }
}

#endif  // MMMOJO_FLMOJO_DELEGATE_H_
