#ifndef WGC_HPP
#define WGC_HPP

#include <windows.h>
#include <winrt/windows.foundation.h>

#include <vector>

struct Frame {
    std::vector<uint8_t> data;
    winrt::Windows::Foundation::TimeSpan timestamp;
};

std::vector<Frame> CaptureDisplay(HMONITOR display, bool captureCursor);
std::vector<Frame> CaptureWindow(HWND window, bool captureCursor);

#endif