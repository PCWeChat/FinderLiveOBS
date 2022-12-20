

#ifndef MMMOJO_FLMOJO_FLMOJO_WRITEINFO_WRAPPER_H_
#define MMMOJO_FLMOJO_FLMOJO_WRITEINFO_WRAPPER_H_

#include "flmojo/flmojo.h"
#include "flmojo/flmojo_export.h"

class FLMojoWriteInfoWrapper {
 public:
  explicit FLMojoWriteInfoWrapper(FLMojoInfoMethod method,
                                  bool sync,
                                  uint32_t request_id,
                                  uint32_t request_data_size = 0,
                                  uint32_t attach_data_size = 0);

  FLMojoWriteInfoWrapper(const FLMojoWriteInfoWrapper&) = delete;
  FLMojoWriteInfoWrapper& operator=(const FLMojoWriteInfoWrapper&) = delete;

  ~FLMojoWriteInfoWrapper() = default;

  void* GetRequestData();
  const uint32_t GetRequestDataSize();
  void* GetAttachData();
  const uint32_t GetAttachDataSize();

  void SwapFLMojoInfoCallback(void* readinfo);
  void SwapFLMojoInfoData(void* readinfo);

  void SetUseAppendMessagePipe(int num_message_pipe);
  void SetSavedResponseInfoSync(void** readinfo);
  bool SendRequestMessage(void* mmmojo_env);

 private:
  const FLMojoInfoMethod method_;
  const bool sync_;
  const uint32_t request_id_;

  void* request_info_ = nullptr;
  void* request_data_ = nullptr;
  uint32_t request_data_size_;
  void* attach_data_ = nullptr;
  uint32_t attach_data_size_;
};

#endif  // MMMOJO_FLMOJO_FLMOJO_WRITEINFO_WRAPPER_H_
