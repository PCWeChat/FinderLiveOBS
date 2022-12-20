#include "base/wechat_support_xp/wechat_support_xp.h"

#include <cstdio>
#include <dbghelp.h>
#include <knownfolders.h>
#include <memory>
#include <shlobj.h>
#include <shellapi.h>
#include <WinBase.h>
#include <winternl.h>

#include "base/files/file_path.h"
#include "base/win/win_util.h"
#include "base/wechat_support_xp/util/wechat_get_function_xp.h"
#include "base/wechat_support_xp/util/wechat_nt_internals_xp.h"
#include "base/wechat_support_xp/util/wechat_nt_structs_define_xp.h"

namespace base {
namespace wechat_support_xp {
static const wchar_t kKernel32Dll[] = L"kernel32.dll";
static const wchar_t kShell32Dll[] = L"shell32.dll";
//////////////////////////////////////////////////////////////////////////
// process and thread
DWORD
WINAPI
GetThreadId(
  _In_ HANDLE thread_handle) {
  
  //higher xp
  static const auto kernel_get_thread_id =
    GET_FUNCTION_XP(kKernel32Dll, ::GetThreadId);
  if (kernel_get_thread_id) {
    return kernel_get_thread_id(thread_handle);
  }

  //xp_compatible
  THREAD_BASIC_INFORMATION thread_basic_information;
  NtQueryInformationThreadFunction(thread_handle, static_cast<THREADINFOCLASS>(ThreadBasicInformation),
    &thread_basic_information, sizeof(THREAD_BASIC_INFORMATION), nullptr);

  return base::win::HandleToUint32(thread_basic_information.ClientId.UniqueThread);
}

BOOL
WINAPI
QueryFullProcessImageNameW(
  _In_ HANDLE process_handle,
  _In_ DWORD flags,
  _Out_writes_to_(*dw_size, *dw_size) LPWSTR exe_name,
  _Inout_ DWORD& dw_size) {

  //higher xp
  static const auto kernel_query_full_process_iamge_name_w =
    GET_FUNCTION_XP(kKernel32Dll, ::QueryFullProcessImageNameW);
  if (kernel_query_full_process_iamge_name_w) {
    return kernel_query_full_process_iamge_name_w(
    process_handle, flags, exe_name, &dw_size);
  }

  //xp_compatible
  ULONG return_size = 0;
  PROCESS_BASIC_INFORMATION process_basic_information = { 0 };
  NTSTATUS result = NtQueryInformationProcessFunction(
    process_handle, ProcessBasicInformation, &process_basic_information, sizeof(PROCESS_BASIC_INFORMATION), &return_size);
  if (!NT_SUCCESS(result) ||
    (sizeof(PROCESS_BASIC_INFORMATION) != return_size)) {
    return FALSE;
  }
  DWORD image_buffer_size = process_basic_information.PebBaseAddress->ProcessParameters->ImagePathName.Length;
  if (dw_size < image_buffer_size) {
    dw_size = image_buffer_size;
    return FALSE;
  }
  if (0 != ::wmemcpy_s(exe_name, dw_size,
    process_basic_information.PebBaseAddress->ProcessParameters->ImagePathName.Buffer,
    image_buffer_size)) {
    return FALSE;
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// file
BOOL
WINAPI
GetFileInformationByHandleEx(
  __in  HANDLE file_handle,
  __in  FILE_INFO_BY_HANDLE_CLASS file_information_class,
  __out_bcount(dwBufferSize) LPVOID file_information,
  __in  DWORD buffer_size,
  __out bool& is_comptible_mode) {
  static const auto kernel_get_file_information_by_handle_ex =
    GET_FUNCTION_XP(kKernel32Dll, ::GetFileInformationByHandleEx);
  if (kernel_get_file_information_by_handle_ex) {
    return kernel_get_file_information_by_handle_ex(
      file_handle, file_information_class, file_information, buffer_size);
  }
  is_comptible_mode = true;
  return TRUE;
}

BOOL
WINAPI
SetFileInformationByHandle(
  __in  HANDLE file_handle,
  __in  FILE_INFO_BY_HANDLE_CLASS file_information_class,
  __in_bcount(buffer_size)  LPVOID file_information,
  __in  DWORD buffer_size) {
    return FALSE;
  // static const auto kernel_set_file_information_by_handle =
  //   GET_FUNCTION_XP(kKernel32Dll, ::SetFileInformationByHandle);
  // if (kernel_set_file_information_by_handle) {
  //   return kernel_set_file_information_by_handle(file_handle, file_information_class, file_information, buffer_size);
  // }
  // if (FileDispositionInfo != file_information_class) {
  //   return FALSE;
  // }
  // IO_STATUS_BLOCK status_block = { 0 };
  // NTSTATUS result = NtSetInformationFileFunction(file_handle, &status_block, file_information, buffer_size,
  //   static_cast<FILE_INFORMATION_CLASS>(CUSTOM_FILE_INFORMATION_CLASS::FileDispositionInformation));
  // if (!NT_SUCCESS(result)) {
  //   return FALSE;
  // }
  // return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//shl
HRESULT
WINAPI
SHGetKnownFolderPath(
  _In_ REFKNOWNFOLDERID rfid,
  _In_ DWORD /* KNOWN_FOLDER_FLAG */ dwFlags,
  _In_opt_ HANDLE hToken,
  _Outptr_ PWSTR *ppszPath) {

  //higher xp
  static const auto shell_shget_known_folder_path =
    GET_FUNCTION_XP(kShell32Dll, ::SHGetKnownFolderPath);
  if (shell_shget_known_folder_path) {
    return shell_shget_known_folder_path(
      rfid, dwFlags, hToken, ppszPath);
  }

  //xp_compatible
  if (FOLDERID_ProgramFilesCommon != rfid) {
    return E_INVALIDARG;
  }
  wchar_t buffer[MAX_PATH] = { 0 };
  HRESULT result = ::SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES_COMMON, nullptr, 0, buffer);
  if (!SUCCEEDED(result)) {
    return result;
  }
  (*ppszPath) = reinterpret_cast<WCHAR*>(::CoTaskMemAlloc(MAX_PATH));
  if (!(*ppszPath)) {
    return E_POINTER;
  }
  if (0 != ::wcscpy_s((*ppszPath), MAX_PATH, buffer)) {
    return E_POINTER;
  }
  return S_OK;
}

HRESULT
WINAPI
WINAPI SHGetPropertyStoreForWindow(
  __in HWND hwnd,
  __in REFIID riid,
  __out void** ppv) {
  static const auto shell_shget_property_store_for_window =
    GET_FUNCTION_XP(kShell32Dll, ::SHGetPropertyStoreForWindow);
  if (shell_shget_property_store_for_window) {
    return shell_shget_property_store_for_window(hwnd, riid, ppv);
  }
  return E_FAIL;
}

HRESULT
WINAPI
SHOpenWithDialog(
  __in_opt HWND hwnd_parent,
  __in const OPENASINFO* poainfo) {
  static const auto shell_shopen_with_dialog =
    GET_FUNCTION_XP(kShell32Dll, ::SHOpenWithDialog);
  if (shell_shopen_with_dialog) {
    return shell_shopen_with_dialog(hwnd_parent, poainfo);
  }
  return E_FAIL;
}
//////////////////////////////////////////////////////////////////////////
BOOL
WINAPI
QueryUnbiasedInterruptTime(
  _Out_ PULONGLONG unbiased_time) {
  
  //higher xp
  static const auto kernel_query_unbiased_interrupt_time =
    GET_FUNCTION_XP(kKernel32Dll, ::QueryUnbiasedInterruptTime);
  if (kernel_query_unbiased_interrupt_time) {
    return kernel_query_unbiased_interrupt_time(unbiased_time);
  }

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL
WINAPI
InitOnceExecuteOnce(
  _Inout_ LONG volatile *once_control,
  _Inout_ PINIT_ONCE init_once,
  _In_ __callback PINIT_ONCE_FN init_fn,
  _Inout_opt_ PVOID parameter,
  _Outptr_opt_result_maybenull_ LPVOID * context) {
  // higher xp
  static const auto kernel_init_once_execute_once =
    GET_FUNCTION_XP(kKernel32Dll, ::InitOnceExecuteOnce);
  if (kernel_init_once_execute_once) {
    return kernel_init_once_execute_once(init_once, init_fn, parameter, context);
  }

  // xp_compatible
  // Try for a fast path first. Note: this should be an acquire semantics read
  // It is on x86 and x64, where Windows runs.
  if (*once_control != 1) {
    while (1) {
      switch (::InterlockedCompareExchange(once_control, 2, 0)) {
      case 0:
        init_fn(init_once, parameter, context);
        ::InterlockedExchange(once_control, 1);
        return 0;
      case 1:
        // The initializer has already been executed
        return 0;
      default:
        // The initializer is being processed by another thread
        ::SwitchToThread();
      }
    }
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL
WINAPI
SymGetSearchPathW(
  __in HANDLE process,
  __out_ecount(search_path_length) PWSTR search_path,
  __in DWORD search_path_length) {
  using SymGetSearchPathWPtr = decltype(::SymGetSearchPathW)*;
  static const auto sym_get_search_path_w = reinterpret_cast<SymGetSearchPathWPtr>(
    ::GetProcAddress(::GetModuleHandleW(L"Dbghelp.dll"), "SymGetSearchPathW"));
  if (!sym_get_search_path_w) {
    return false;
  }
  return sym_get_search_path_w(process, search_path, search_path_length);
}

BOOL
WINAPI
SymSetSearchPathW(
  __in HANDLE process,
  __in_opt PCWSTR search_path) {
  using SymSetSearchPathWPtr = decltype(::SymSetSearchPathW)*;
  static const auto sym_set_search_path_w = reinterpret_cast<SymSetSearchPathWPtr>(
    ::GetProcAddress(::GetModuleHandleW(L"Dbghelp.dll"), "SymSetSearchPathW"));
  if (!sym_set_search_path_w) {
    return false;
  }
  return sym_set_search_path_w(process, search_path);
}

//////////////////////////////////////////////////////////////////////////
BOOL
WINAPI
QueryThreadCycleTime(
  _In_ HANDLE ThreadHandle,
  _Out_ PULONG64 CycleTime
) {
  using QueryThreadCycleTimePtr = decltype(::QueryThreadCycleTime)*;
  return TRUE;
}
}  // support_xp
}  // base