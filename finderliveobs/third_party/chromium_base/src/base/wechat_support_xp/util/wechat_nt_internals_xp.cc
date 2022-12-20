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

#include "base/wechat_support_xp/util/wechat_nt_internals_xp.h"

#include "base/logging.h"
#include "base/wechat_support_xp/util/wechat_get_function_xp.h"
#include "base/wechat_support_xp/util/wechat_nt_functions_define_xp.h"

// Declarations that the system headers should provide but don’t.


namespace base {
namespace wechat_support_xp {
static const wchar_t kNtDll[] = L"ntdll.dll";

//////////////////////////////////////////////////////////////////////////
// custom define ntdll interface

//////////////////////////////////////////////////////////////////////////

NTSTATUS NtClose(HANDLE handle) {
  static const auto nt_close = GET_FUNCTION_REQUIRED_XP(L"ntdll.dll", ::NtClose);
  return nt_close(handle);
}

//////////////////////////////////////////////////////////////////////////
// process and thread
NTSTATUS NtQueryInformationThreadFunction(
  IN HANDLE          thread_handle,
  IN THREADINFOCLASS thread_information_class,
  OUT PVOID          thread_information,
  IN ULONG           thread_information_length,
  OUT PULONG         return_length) {

  static const auto nt_query_information_thread =
      GET_FUNCTION_REQUIRED_XP(kNtDll, ::NtQueryInformationThread);
  return nt_query_information_thread(thread_handle,
    thread_information_class,
    thread_information,
    thread_information_length,
    return_length);
}

NTSTATUS NtQueryInformationProcessFunction(
  IN HANDLE process_handle,
  IN PROCESSINFOCLASS process_information_class,
  OUT PVOID process_information,
  IN ULONG process_information_length,
  OUT PULONG return_length) {
  static const auto nt_query_information_process =
    GET_FUNCTION_REQUIRED_XP(kNtDll, ::NtQueryInformationProcess);
  return nt_query_information_process(process_handle,
    process_information_class,
    process_information,
    process_information_length,
    return_length);
}

//////////////////////////////////////////////////////////////////////////
// file
NTSTATUS NtQueryInformationFileFunction(
  IN HANDLE                 file_handle,
  OUT PIO_STATUS_BLOCK      io_status_block,
  OUT PVOID                 file_information,
  IN ULONG                  length,
  IN FILE_INFORMATION_CLASS file_information_class) {
  static const auto nt_query_information_file =
    GET_FUNCTION_REQUIRED_XP(kNtDll, NtQueryInformationFile);
  return nt_query_information_file(file_handle, io_status_block,
    file_information, length, file_information_class);
}

NTSTATUS NtSetInformationFileFunction(
  IN HANDLE file_handle,
  OUT PIO_STATUS_BLOCK io_status_block,
  IN PVOID file_information,
  IN ULONG length,
  IN FILE_INFORMATION_CLASS file_information_class) {
  static const auto nt_set_information_file =
    GET_FUNCTION_REQUIRED_XP(kNtDll, NtSetInformationFile);
  return nt_set_information_file(file_handle, io_status_block,
    file_information, length, file_information_class);
}

NTSTATUS NtQueryObjectFunction(
  IN HANDLE handle,
  IN OBJECT_INFORMATION_CLASS object_information_class,
  OUT PVOID object_information OPTIONAL,
  IN ULONG object_information_length,
  OUT PULONG return_length OPTIONAL) {
  static const auto nt_set_information_file =
    GET_FUNCTION_REQUIRED_XP(kNtDll, NtQueryObject);
  return nt_set_information_file(handle, object_information_class,
    object_information, object_information_length, return_length);
}
}  // namespace support_xp
}  // namespace base
