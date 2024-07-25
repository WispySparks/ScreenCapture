#include <iostream>
#include <vector>

#include "util.hpp"
#include "wgc.hpp"

int main() {
    auto windows = GetWindows();
    HWND window = windows.at(0);
    std::cout << GetWindowName(window) << "\n";
    auto displays = GetDisplays();
    HMONITOR display = displays.at(0);
    std::cout << GetDisplayName(display) << "\n";
    CaptureDisplay(display, true);
    // CaptureWindow(window, true);
    std::cout << "\n---PROGRAM END---\n\n";
    return 0;
}