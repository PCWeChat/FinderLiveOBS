#ifndef BASE_WECHAT_SUPPORT_XP_SUPPORT_XP_H_
#define BASE_WECHAT_SUPPORT_XP_SUPPORT_XP_H_

#include <wtypes.h>
#include <shtypes.h>
#include <shlobj.h>
#include "base/base_export.h"

namespace base {
namespace wechat_support_xp {

//////////////////////////////////////////////////////////////////////////
// process and thread
BASE_EXPORT
DWORD
WINAPI
GetThreadId(
  _In_ HANDLE thread_handle);

BASE_EXPORT
BOOL
WINAPI
QueryFullProcessImageNameW(
  _In_ HANDLE process_handle,
  _In_ DWORD flags,
  _Out_writes_to_(*dw_size, *dw_size) LPWSTR exe_name,
  _Inout_ DWORD& dw_size
);
//////////////////////////////////////////////////////////////////////////
// file
BASE_EXPORT
BOOL
WINAPI
GetFileInformationByHandleEx(
  __in  HANDLE file_handle,
  __in  FILE_INFO_BY_HANDLE_CLASS file_information_class,
  __out_bcount(dwBufferSize) LPVOID file_information,
  __in  DWORD buffer_size,
  __out bool& is_comptible_mode);

BASE_EXPORT
BOOL
WINAPI
SetFileInformationByHandle(
  __in  HANDLE file_handle,
  __in  FILE_INFO_BY_HANDLE_CLASS file_information_class,
  __in_bcount(buffer_size)  LPVOID file_information,
  __in  DWORD buffer_size
);
//////////////////////////////////////////////////////////////////////////
// shl
BASE_EXPORT
HRESULT
WINAPI
SHGetKnownFolderPath(
  _In_ REFKNOWNFOLDERID rfid,
  _In_ DWORD /* KNOWN_FOLDER_FLAG */ dwFlags,
  _In_opt_ HANDLE hToken,
  _Outptr_ PWSTR *ppszPath); // free *ppszPath with CoTaskMemFree

BASE_EXPORT
HRESULT
WINAPI SHGetPropertyStoreForWindow(
  __in HWND hwnd,
  __in REFIID riid,
  __out void** ppv);

BASE_EXPORT
HRESULT
WINAPI
SHOpenWithDialog(
  __in_opt HWND hwnd_parent,
  __in const OPENASINFO* poainfo);
//////////////////////////////////////////////////////////////////////////
//
BASE_EXPORT
BOOL
WINAPI
QueryUnbiasedInterruptTime(
  __out PULONGLONG unbiased_time);

//////////////////////////////////////////////////////////////////////////
BASE_EXPORT
BOOL
WINAPI
InitOnceExecuteOnce(
  _Inout_ LONG volatile *once_control,
  _Inout_ PINIT_ONCE init_once,
  _In_ __callback PINIT_ONCE_FN init_fn,
  _Inout_opt_ PVOID parameter,
  _Outptr_opt_result_maybenull_ LPVOID * context);

//////////////////////////////////////////////////////////////////////////
BASE_EXPORT
BOOL
WINAPI
SymGetSearchPathW(
  __in HANDLE process,
  __out_ecount(search_path_length) PWSTR search_path,
  __in DWORD search_path_length
);

BASE_EXPORT
BOOL
WINAPI
SymSetSearchPathW(
  __in HANDLE process,
  __in_opt PCWSTR search_path
);


//////////////////////////////////////////////////////////////////////////
//time
BASE_EXPORT
BOOL
WINAPI
QueryThreadCycleTime(
  _In_ HANDLE ThreadHandle,
  _Out_ PULONG64 CycleTime
);

}  // namespace wechat_support_xp
}  //namespace base
#endif  // BASE_WECHAT_SUPPORT_XP_SUPPORT_XP_H_
