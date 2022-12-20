// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/synchronization/read_write_lock.h"

//wechat_support_xp {
//#include "base/wechat_support_xp/synchronization/system_wrapper/include/wechat_rw_lock_wrapper_xp.h"
namespace base {
namespace subtle {

ReadWriteLock::ReadWriteLock()
  : native_handle_(SRWLOCK_INIT)
  //: support_xp_native_handle_(base::wechat_support_xp::RWLockWrapper::CreateRWLock()) 
{
}

ReadWriteLock::~ReadWriteLock() = default;

void ReadWriteLock::ReadAcquire() {
  ::AcquireSRWLockShared(&native_handle_);
  //support_xp_native_handle_->AcquireLockShared();
}

void ReadWriteLock::ReadRelease() {
  ::ReleaseSRWLockShared(&native_handle_);
  //support_xp_native_handle_->ReleaseLockShared();
}

void ReadWriteLock::WriteAcquire() {
  ::AcquireSRWLockExclusive(&native_handle_);
  //support_xp_native_handle_->AcquireLockExclusive();
}

void ReadWriteLock::WriteRelease() {
  ::ReleaseSRWLockExclusive(&native_handle_);
  //support_xp_native_handle_->ReleaseLockExclusive();
}

}  // namespace subtle
}  // namespace base
//} wechat_support_xp
