#ifndef BASE_WECHAT_SUPPORT_XP_UTIL_NT_FUNCTION_H_
#define BASE_WECHAT_SUPPORT_XP_UTIL_NT_FUNCTION_H_
// add the winternl.h missing function

namespace base {
namespace wechat_support_xp {

__kernel_entry
NTSTATUS
NTAPI
NtQueryInformationFile(
  IN HANDLE                 file_handle,
  OUT PIO_STATUS_BLOCK      io_status_block,
  OUT PVOID                 file_information,
  IN ULONG                  length,
  IN FILE_INFORMATION_CLASS fileInformation_class);

__kernel_entry
NTSTATUS
NTAPI
NtSetInformationFile(
  IN HANDLE                 file_handle,
  OUT PIO_STATUS_BLOCK      io_status_block,
  IN PVOID                  file_information,
  IN ULONG                  length,
  IN FILE_INFORMATION_CLASS file_information_class);

}  // namespace wechat_support_xp
}  // namespace base
#endif  // BASE_WECHAT_SUPPORT_XP_UTIL_NT_FUNCTION_H_