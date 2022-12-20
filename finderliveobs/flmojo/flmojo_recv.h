

#ifndef FLMOJO_FLMOJO_RECV_H_
#define FLMOJO_FLMOJO_RECV_H_

#include <stdint.h>

void OnMojoPipeMessagePushDispatcher(uint32_t request_id,
                                     const void* request_info);
void OnMojoPipeMessagePullDispatcher(uint32_t request_id,
                                     const void* request_info);
void OnMojoSharedMemoryDispatcher(uint32_t request_id,
                                  const void* request_info);

#endif  // FLMOJO_FLMOJO_RECV_H_