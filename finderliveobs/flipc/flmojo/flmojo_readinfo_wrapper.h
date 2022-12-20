

#ifndef MMMOJO_FLMOJO_FLMOJO_READINFO_WRAPPER_H_
#define MMMOJO_FLMOJO_FLMOJO_READINFO_WRAPPER_H_

#include "flmojo/flmojo.h"
#include "flmojo/flmojo_export.h"

const uint32_t kFLReadInfoWrapperNotUsedRequestType = 0;

class FLMojoReadInfoWrapper {
 public:
  explicit FLMojoReadInfoWrapper(uint32_t request_id, const void* request_info);

  FLMojoReadInfoWrapper(const FLMojoReadInfoWrapper&) = delete;
  FLMojoReadInfoWrapper& operator=(const FLMojoReadInfoWrapper&) = delete;

  ~FLMojoReadInfoWrapper();

  const uint32_t GetRequestId();
  const FLMojoInfoMethod GetRequestMethod();
  const bool GetRequestSync();

  const void* GetRequestInfo();
  const void* GetRequestData();
  const uint32_t GetRequestDataSize();
  const void* GetAttachData();
  const uint32_t GetAttachDataSize();

 private:
  const uint32_t request_id_;
  const void* request_info_;
  const void* request_data_;
  uint32_t request_data_size_ = 0;
  const void* attach_data_;
  uint32_t attach_data_size_ = 0;
};

#endif  // MMMOJO_FLMOJO_FLMOJO_READINFO_WRAPPER_H_
