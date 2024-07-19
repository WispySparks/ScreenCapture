#include <iostream>

#include "util.hpp"
#include "wgc.hpp"

int main() {
    // Figure out holding certain frames for longer or dropping frames
    // Make writing to ffmpeg faster
    auto windows = GetWindows();
    HWND window = windows.at(0);
    std::cout << GetWindowName(window) << "\n";
    auto displays = GetDisplays();
    HMONITOR display = displays.at(0);
    std::cout << GetDisplayName(display) << "\n";
    // CaptureDisplayWGC(display, true);
    CaptureWindowWGC(window, true);
    std::cout << "\n---PROGRAM END---\n\n";
    return 0;
}