/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_RW_LOCK_WIN_XP_H_
#define BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_RW_LOCK_WIN_XP_H_

#include <Windows.h>

#include "base/macros.h"
#include "base/wechat_support_xp/synchronization/system_wrapper/include/wechat_rw_lock_wrapper_xp.h"



namespace base {
namespace wechat_support_xp {
class RWLockWin : public RWLockWrapper {
 public:
  static RWLockWin* Create();
  virtual ~RWLockWin() {}

  virtual void AcquireLockExclusive();
  virtual void ReleaseLockExclusive();

  virtual void AcquireLockShared();
  virtual void ReleaseLockShared();

 private:
  RWLockWin();
  static bool LoadModule();

  SRWLOCK lock_;

private:
  DISALLOW_COPY_AND_ASSIGN(RWLockWin);
};
}  // namespace support_xp
}  // namespace base

#endif  // BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_RW_LOCK_WIN_XP_H_
