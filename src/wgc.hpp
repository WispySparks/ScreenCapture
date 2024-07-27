#ifndef WGC_HPP
#define WGC_HPP

#include <windows.h>
#include <winrt/windows.foundation.h>

#include <vector>

// Format: RGBA 32bpp
struct Frame {
    unsigned int width, height;
    std::vector<uint8_t> data;
    winrt::Windows::Foundation::TimeSpan timestamp;
};

std::vector<Frame> CaptureDisplay(HMONITOR display, bool captureCursor);
std::vector<Frame> CaptureWindow(HWND window, bool captureCursor);

#endif