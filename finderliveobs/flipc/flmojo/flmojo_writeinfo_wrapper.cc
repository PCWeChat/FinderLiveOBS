

#include "flmojo/flmojo_writeinfo_wrapper.h"
#include "flmojo/flmojo.h"
#include "flmojo/flmojo_export.h"

FLMojoWriteInfoWrapper::FLMojoWriteInfoWrapper(FLMojoInfoMethod method,
                                               bool sync,
                                               uint32_t request_id,
                                               uint32_t request_data_size,
                                               uint32_t attach_data_size)
    : method_(method),
      sync_(sync),
      request_id_(request_id),
      request_data_size_(request_data_size),
      attach_data_size_(attach_data_size) {
  request_info_ = CreateFLMojoWriteInfo(method_, sync_, request_id_);
}

void* FLMojoWriteInfoWrapper::GetRequestData() {
  if (!request_data_ && request_data_size_) {
    request_data_ =
        GetFLMojoWriteInfoRequest(request_info_, request_data_size_);
    if (!request_data_) {
      RemoveFLMojoWriteInfo(request_info_);
    }
  }
  return request_data_;
}

const unsigned int FLMojoWriteInfoWrapper::GetRequestDataSize() {
  return request_data_size_;
}

void* FLMojoWriteInfoWrapper::GetAttachData() {
  if (!attach_data_ && attach_data_size_) {
    attach_data_ = GetFLMojoWriteInfoAttach(request_info_, attach_data_size_);
  }
  return attach_data_;
}

const unsigned int FLMojoWriteInfoWrapper::GetAttachDataSize() {
  return attach_data_size_;
}

void FLMojoWriteInfoWrapper::SetUseAppendMessagePipe(int num_message_pipe) {
  SetFLMojoWriteInfoMessagePipe(request_info_, num_message_pipe);
}

void FLMojoWriteInfoWrapper::SetSavedResponseInfoSync(void** readinfo) {
  SetFLMojoWriteInfoResponseSync(request_info_, readinfo);
}

bool FLMojoWriteInfoWrapper::SendRequestMessage(void* mmmojo_env) {
  return SendFLMojoWriteInfo(mmmojo_env, request_info_);
}

void FLMojoWriteInfoWrapper::SwapFLMojoInfoCallback(void* readinfo) {
  SwapFLMojoWriteInfoCallback(request_info_, readinfo);
}

void FLMojoWriteInfoWrapper::SwapFLMojoInfoData(void* readinfo) {
  SwapFLMojoWriteInfoMessage(request_info_, readinfo);
}
