

#ifndef FLUI_DPI_WIN_H_
#define FLUI_DPI_WIN_H_

#include <stdint.h>

namespace display {
namespace win {

// Gets the system's scale factor. For example, if the system DPI is 96 then the
// scale factor is 1.0. This does not handle per-monitor DPI.
float GetDPIScale();

// Equivalent to GetDPIScale() but ignores the --force-device-scale-factor flag.
float GetUnforcedDeviceScaleFactor();

// Returns the equivalent scaling factor for |dpi|.
float GetScalingFactorFromDPI(int dpi);

// Gets the default DPI for the system.
int GetDefaultSystemDPI();

}  // namespace win
}  // namespace display

#endif  // FLUI_DPI_WIN_H_
