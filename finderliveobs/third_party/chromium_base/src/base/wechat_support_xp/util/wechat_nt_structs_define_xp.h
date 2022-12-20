// Copyright 2015 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BASE_WECHAT_SUPPORT_XP_UTIL_NT_STRUCTS_H_
#define BASE_WECHAT_SUPPORT_XP_UTIL_NT_STRUCTS_H_

#include <windows.h>

namespace base {
namespace wechat_support_xp {

#define STATUS_SUCCESS                ((NTSTATUS)0x00000000L)
#define STATUS_BUFFER_OVERFLOW        ((NTSTATUS)0x80000005L)
#define STATUS_UNSUCCESSFUL           ((NTSTATUS)0xC0000001L)
#define STATUS_NOT_IMPLEMENTED        ((NTSTATUS)0xC0000002L)
#define STATUS_INFO_LENGTH_MISMATCH   ((NTSTATUS)0xC0000004L)
#ifndef STATUS_INVALID_PARAMETER
  // It is now defined in Windows 2008 SDK.
#define STATUS_INVALID_PARAMETER      ((NTSTATUS)0xC000000DL)
#endif
#define STATUS_CONFLICTING_ADDRESSES  ((NTSTATUS)0xC0000018L)
#define STATUS_ACCESS_DENIED          ((NTSTATUS)0xC0000022L)
#define STATUS_BUFFER_TOO_SMALL       ((NTSTATUS)0xC0000023L)
#define STATUS_OBJECT_NAME_NOT_FOUND  ((NTSTATUS)0xC0000034L)
#define STATUS_OBJECT_NAME_COLLISION  ((NTSTATUS)0xC0000035L)
#define STATUS_PROCEDURE_NOT_FOUND    ((NTSTATUS)0xC000007AL)
#define STATUS_INVALID_IMAGE_FORMAT   ((NTSTATUS)0xC000007BL)
#define STATUS_NO_TOKEN               ((NTSTATUS)0xC000007CL)
#define STATUS_NOT_SUPPORTED          ((NTSTATUS)0xC00000BBL)

struct CLIENT_ID {
  HANDLE UniqueProcess;
  HANDLE UniqueThread;
};

struct THREAD_BASIC_INFORMATION {
  NTSTATUS ExitStatus;
  TEB* Teb;
  CLIENT_ID ClientId;
  KAFFINITY AffinityMask;
  LONG Priority;
  LONG BasePriority;
};

//////////////////////////////////////////////////////////////////////////
enum class CUSTOM_FILE_INFORMATION_CLASS { 
 FileNameInformation = 9,
 FileDispositionInformation = 13
};
using FILE_NAME_INFORMATION = struct {
  ULONG FileNameLength;
  WCHAR FileName[1];
};
//////////////////////////////////////////////////////////////////////////
enum class CUSTOM_OBJECT_INFORMATION_CLASS {
  kObjectBasicInformation,
  kObjectNameInformation,
  kObjectTypeInformation,
  kObjectAllInformation,
  kObjectDataInformation
};

using OBJECT_NAME_INFORMATION = struct {
  UNICODE_STRING ObjectName;
};

}  // namespace wechat_support_xp
}  // namespace base

#endif  // BASE_WECHAT_SUPPORT_XP_UTIL_NT_STRUCTS_H_
