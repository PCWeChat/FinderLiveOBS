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
#ifndef BASE_WECHAT_SUPPORT_XP_UTIL_NT_INTERNALS_XP_H_
#define BASE_WECHAT_SUPPORT_XP_UTIL_NT_INTERNALS_XP_H_
#include <windows.h>
#include <winternl.h>

#include "base/wechat_support_xp/util/wechat_nt_structs_define_xp.h"

namespace base {
namespace wechat_support_xp {
NTSTATUS NtClose(HANDLE handle);

// http://processhacker.sourceforge.net/doc/ntpsapi_8h_source.html
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH 0x00000002

// Copied from ntstatus.h because um/winnt.h conflicts with general inclusion of
// ntstatus.h.
#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xC0000023L)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

// winternal.h defines THREADINFOCLASS, but not all members.
enum { ThreadBasicInformation = 0 };

// winternal.h defines SYSTEM_INFORMATION_CLASS, but not all members.
enum { SystemExtendedHandleInformation = 64 };

//////////////////////////////////////////////////////////////////////////
NTSTATUS NtQueryInformationThreadFunction(
  IN HANDLE thread_handle,
  IN THREADINFOCLASS thread_information_class,
  OUT PVOID thread_information,
  IN ULONG thread_information_length,
  OUT PULONG return_length);

NTSTATUS NtQueryInformationProcessFunction(
  IN HANDLE process_handle,
  IN PROCESSINFOCLASS process_information_class,
  OUT PVOID process_information,
  IN ULONG process_information_length,
  OUT PULONG return_length);

//////////////////////////////////////////////////////////////////////////
NTSTATUS NtQueryInformationFileFunction(
  IN HANDLE file_handle,
  OUT PIO_STATUS_BLOCK io_status_block,
  OUT PVOID file_information,
  IN ULONG length,
  IN FILE_INFORMATION_CLASS file_information_class);

NTSTATUS NtSetInformationFileFunction(
  IN HANDLE file_handle,
  OUT PIO_STATUS_BLOCK io_status_block,
  IN PVOID file_information,
  IN ULONG length,
  IN FILE_INFORMATION_CLASS file_information_class);

//////////////////////////////////////////////////////////////////////////
NTSTATUS NtQueryObjectFunction(
  IN HANDLE handle,
  IN OBJECT_INFORMATION_CLASS object_information_class,
  OUT PVOID object_information OPTIONAL,
  IN ULONG object_information_length,
  OUT PULONG return_length OPTIONAL);
}  // namespace wechat_support_xp
}  // namespace base
#endif  // BASE_WECHAT_SUPPORT_XP_UTIL_NT_INTERNALS_XP_H_