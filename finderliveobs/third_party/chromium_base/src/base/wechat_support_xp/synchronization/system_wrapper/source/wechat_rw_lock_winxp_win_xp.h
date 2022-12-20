/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_RW_LOCK_WINXP_WIN_H_
#define BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_RW_LOCK_WINXP_WIN_H_

#include "base/macros.h"

#include "base/wechat_support_xp/synchronization/system_wrapper/include/wechat_rw_lock_wrapper_xp.h"
#include "base/wechat_support_xp/synchronization/system_wrapper/source/wechat_condition_variable_event_win_xp.h"

namespace base {
namespace wechat_support_xp {
class RWLockWinXP : public RWLockWrapper {
 public:
  RWLockWinXP();
  ~RWLockWinXP() override;

  void AcquireLockExclusive() override;
  void ReleaseLockExclusive() override;

  void AcquireLockShared() override;
  void ReleaseLockShared() override;

 private:
  CRITICAL_SECTION critical_section_;
  ConditionVariableEventWin read_condition_;
  ConditionVariableEventWin write_condition_;

  int readers_active_ = 0;
  bool writer_active_ = false;
  int readers_waiting_ = 0;
  int writers_waiting_ = 0;

private:
  DISALLOW_COPY_AND_ASSIGN(RWLockWinXP);
};
}  // namespace support_xp
}  // namespace base

#endif  // BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_RW_LOCK_WINXP_WIN_H_
