

#include "flui/dpi_win.h"

#include <windows.h>

namespace display {
namespace win {

class ScopedGetDC {
 public:
  explicit ScopedGetDC(HWND hwnd) : hwnd_(hwnd), hdc_(GetDC(hwnd)) {}

  ScopedGetDC(const ScopedGetDC&) = delete;
  ScopedGetDC& operator=(const ScopedGetDC&) = delete;

  ~ScopedGetDC() {
    if (hdc_)
      ReleaseDC(hwnd_, hdc_);
  }

  operator HDC() { return hdc_; }

 private:
  HWND hwnd_;
  HDC hdc_;
};

const float kDefaultDPI = 96.f;

float GetDPIScale() {
  return GetUnforcedDeviceScaleFactor();
}

int GetDefaultSystemDPI() {
  static int dpi_x = 0;
  static int dpi_y = 0;
  static bool should_initialize = true;

  if (should_initialize) {
    should_initialize = false;
    ScopedGetDC screen_dc(NULL);
    // This value is safe to cache for the life time of the app since the
    // user must logout to change the DPI setting. This value also applies
    // to all screens.
    dpi_x = GetDeviceCaps(screen_dc, LOGPIXELSX);
    dpi_y = GetDeviceCaps(screen_dc, LOGPIXELSY);
  }
  return dpi_x;
}

float GetUnforcedDeviceScaleFactor() {
  return GetScalingFactorFromDPI(GetDefaultSystemDPI());
}

float GetScalingFactorFromDPI(int dpi) {
  return static_cast<float>(dpi) / kDefaultDPI;
}

}  // namespace win
}  // namespace display
