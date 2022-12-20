// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/synchronization/lock_impl.h"

#include "base/debug/activity_tracker.h"

//wechat_support_xp {
namespace base {
namespace internal {

LockImpl::LockImpl() {
  // The second parameter is the spin count, for short-held locks it avoid the
  // contending thread from going to sleep which helps performance greatly.
  ::InitializeCriticalSectionAndSpinCount(&native_handle_, 2000);
}

LockImpl::~LockImpl() {
  ::DeleteCriticalSection(&native_handle_);
}

bool LockImpl::Try() {
  if (::TryEnterCriticalSection(&native_handle_) != FALSE) {
    return true;
  }
  return false;
}

void LockImpl::Lock() {
  base::debug::ScopedLockAcquireActivity lock_activity(this);
  ::EnterCriticalSection(&native_handle_);
}

}  // namespace internal
}  // namespace base
//} wechat_support_xp