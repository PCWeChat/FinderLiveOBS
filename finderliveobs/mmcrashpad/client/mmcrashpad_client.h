

#ifndef MM_CRASH_PAD_CLIENT_H_
#define MM_CRASH_PAD_CLIENT_H_

#include "mmcrashpad/client/mmcrashpad_client_export.h"

MM_CRASH_PAD_CLIENT_EXPORT void MMInitializeAnnotations(const char* keybuf,
                                                        int keysize,
                                                        const char* valuebuf,
                                                        int valuesize);
MM_CRASH_PAD_CLIENT_EXPORT void MMInitializeArgument(const char* argumentbuf,
                                                     int argumentsize);
MM_CRASH_PAD_CLIENT_EXPORT bool MMCrashpadInitialization(
    bool initial_client,
    const char* exe_pathbuf,
    int exe_pathsize,
    const char* database_pathbuf,
    int database_pathsize);

#endif  // MM_CRASH_PAD_CLIENT_H_