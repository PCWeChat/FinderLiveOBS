

#ifndef FLOBS_FLOBS_DATA_CALLBACK_H_
#define FLOBS_FLOBS_DATA_CALLBACK_H_

#include <stdint.h>

void OnObsOutputVideoFrameFun(char* data,
                              uint32_t length,
                              uint32_t width,
                              uint32_t height,
                              uint64_t timestamp,
                              int rotation);

void OnObsOutputAudioFrameFun(char* data,
                              uint32_t length,
                              uint32_t samplerate,
                              uint32_t channel,
                              uint64_t timestamp);

#endif  // FLOBS_FLOBS_DATA_CALLBACK_H_
