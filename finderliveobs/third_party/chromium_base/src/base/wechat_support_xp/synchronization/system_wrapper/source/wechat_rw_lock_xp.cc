/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "base/wechat_support_xp/synchronization/system_wrapper/include/wechat_rw_lock_wrapper_xp.h"

#include <assert.h>

#include "base/wechat_support_xp/synchronization/system_wrapper/source/wechat_rw_lock_win_xp.h"
#include "base/wechat_support_xp/synchronization/system_wrapper/source/wechat_rw_lock_winxp_win_xp.h"


namespace base {
namespace wechat_support_xp {
RWLockWrapper* RWLockWrapper::CreateRWLock() {
#ifdef _WIN32
  // Native implementation is faster, so use that if available.
  RWLockWrapper* lock = RWLockWin::Create();
  if (lock) {
    return lock;
  }
  return new RWLockWinXP();
#else
  return RWLockPosix::Create();
#endif
}
}  // namespace wechat_support_xp
}  // namespace base
