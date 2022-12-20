

#ifndef MM_CRASH_PAD_CLIENT_EXPORT_H_
#define MM_CRASH_PAD_CLIENT_EXPORT_H_

#if defined(MM_CRASH_PAD_CLIENT_SHARED_LIBRARY)
#if defined(WIN32)

#if defined(MM_CRASH_PAD_CLIENT_IMPLEMENTATION)
#define MM_CRASH_PAD_CLIENT_EXPORT __declspec(dllexport)
#else
#define MM_CRASH_PAD_CLIENT_EXPORT __declspec(dllimport)
#endif  // defined(MM_CRASH_PAD_CLIENT_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(MM_CRASH_PAD_CLIENT_IMPLEMENTATION)
#define MM_CRASH_PAD_CLIENT_EXPORT __attribute__((visibility("default")))
#else
#define MM_CRASH_PAD_CLIENT_EXPORT
#endif  // defined(MM_CRASH_PAD_CLIENT_IMPLEMENTATION)
#endif

#else  // defined(MM_CRASH_PAD_CLIENT_SHARED_LIBRARY)
#define MM_CRASH_PAD_CLIENT_EXPORT
#endif

#endif  // MM_CRASH_PAD_CLIENT_EXPORT_H_
