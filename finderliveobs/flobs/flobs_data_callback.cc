

#include "flobs/flobs_data_callback.h"
#include "flmojo/flmojo_service.h"
#include "flobs/flobs_video_decimator.h"

void OnObsOutputVideoFrameFun(char* data,
                              uint32_t length,
                              uint32_t width,
                              uint32_t height,
                              uint64_t timestamp,
                              int rotation) {
  if (FLOBSVideoDecimator::GetInstance().DropFrame()) {
    return;
  }

  FLMojoService::GetInstance().SendVideoFrameMessage(
      data, length, width, height, timestamp, rotation);
}

void OnObsOutputAudioFrameFun(char* data,
                              uint32_t length,
                              uint32_t samplerate,
                              uint32_t channel,
                              uint64_t timestamp) {
  FLMojoService::GetInstance().SendAudioFrameMessage(data, length, samplerate,
                                                     channel, timestamp);
}
