#pragma once

#include <util/dstr.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *get_ppt_file_from_title(const char *src, int64_t length);
const char *get_player_file_from_title(const char *src, int64_t length);

#ifdef __cplusplus
}
#endif
