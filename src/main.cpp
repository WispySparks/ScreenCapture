#include <iostream>
#include <vector>

#include "encoder.hpp"
#include "util.hpp"
#include "wgc.hpp"

int main() {
    auto windows = GetWindows();
    HWND window = windows.at(0);
    std::cout << GetWindowName(window) << "\n";
    auto displays = GetDisplays();
    HMONITOR display = displays.at(0);
    std::cout << GetDisplayName(display) << "\n";
    auto frames = CaptureDisplay(display, true);
    // auto frames = CaptureWindow(window, true);
    WriteToFile(L"output.mp4", 30, frames);
    std::cout << "\n---PROGRAM END---\n\n";
    return 0;
}