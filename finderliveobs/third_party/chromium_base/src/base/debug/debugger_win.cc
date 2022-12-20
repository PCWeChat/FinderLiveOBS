// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/debug/debugger.h"

#include <stdlib.h>
#include <windows.h>

namespace base {
namespace debug {

bool BeingDebugged() { return ::IsDebuggerPresent() != 0; }

void BreakDebugger() {
// wechat {
  //if (IsDebugUISuppressed()) {
  //  _exit(1);
  //}
  ::RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr);
  _exit(1);
//} wechat
}

}  // namespace debug
}  // namespace base
