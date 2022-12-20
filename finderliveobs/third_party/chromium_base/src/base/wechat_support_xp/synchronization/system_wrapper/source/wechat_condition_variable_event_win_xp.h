/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_CONDITION_VARIABLE_EVENT_WIN_XP_H_
#define BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_CONDITION_VARIABLE_EVENT_WIN_XP_H_

#include <windows.h>

#include "base/macros.h"

namespace base {
namespace wechat_support_xp {
class ConditionVariableEventWin {
 public:
  ConditionVariableEventWin();
  ~ConditionVariableEventWin();

  void SleepCS(CRITICAL_SECTION* crit_sect);
  bool SleepCS(CRITICAL_SECTION* crit_sect, unsigned long max_time_inMS);
  void Wake();
  void WakeAll();

 private:
  enum EventWakeUpType {
    WAKEALL_0   = 0,
    WAKEALL_1   = 1,
    WAKE        = 2,
    EVENT_COUNT = 3
  };

  unsigned int     num_waiters_[2];
  EventWakeUpType  eventID_;
  CRITICAL_SECTION num_waiters_crit_sect_;
  HANDLE           events_[EVENT_COUNT];

private:
  DISALLOW_COPY_AND_ASSIGN(ConditionVariableEventWin);
};

}  // namespace wechat_support_xp
}  // namespace base

#endif  // BASE_WECHAT_SUPPORT_XP_SYNCHRONIZATION_SYSTEM_WRAPPER_SOURCE_WECHAT_CONDITION_VARIABLE_EVENT_WIN_XP_H_
