#ifndef WGC_HPP
#define WGC_HPP

#include <windows.h>
#include <winrt/windows.foundation.h>

#include <vector>

struct Frame {
    const unsigned int width, height;
    std::vector<uint8_t> data;
    winrt::Windows::Foundation::TimeSpan timestamp;
};

// Format: BGRA 32bpp
std::vector<Frame> CaptureDisplay(HMONITOR display, bool captureCursor);
// Format: BGRA 32bpp
std::vector<Frame> CaptureWindow(HWND window, bool captureCursor);

#endif