

#include "flmojo/flmojo_readinfo_wrapper.h"
#include "flmojo/flmojo.h"
#include "flmojo/flmojo_writeinfo_wrapper.h"

FLMojoReadInfoWrapper::FLMojoReadInfoWrapper(uint32_t request_id,
                                             const void* request_info)
    : request_id_(request_id), request_info_(request_info) {
  request_data_ = GetFLMojoReadInfoRequest(request_info_, &request_data_size_);
  attach_data_ = GetFLMojoReadInfoAttach(request_info_, &attach_data_size_);
}

FLMojoReadInfoWrapper::~FLMojoReadInfoWrapper() {
  RemoveFLMojoReadInfo(const_cast<void*>(request_info_));
}

const uint32_t FLMojoReadInfoWrapper::GetRequestId() {
  return request_id_;
}

const FLMojoInfoMethod FLMojoReadInfoWrapper::GetRequestMethod() {
  return static_cast<FLMojoInfoMethod>(GetFLMojoReadInfoMethod(request_info_));
}

const bool FLMojoReadInfoWrapper::GetRequestSync() {
  return GetFLMojoReadInfoSync(request_info_);
}

const void* FLMojoReadInfoWrapper::GetRequestInfo() {
  return request_info_;
}

const void* FLMojoReadInfoWrapper::GetRequestData() {
  return request_data_;
}

const uint32_t FLMojoReadInfoWrapper::GetRequestDataSize() {
  return request_data_size_;
}

const void* FLMojoReadInfoWrapper::GetAttachData() {
  return attach_data_;
}

const uint32_t FLMojoReadInfoWrapper::GetAttachDataSize() {
  return attach_data_size_;
}
