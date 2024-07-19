#include <iostream>
#include <vector>

#include "util.hpp"
#include "wgc.hpp"

// clang-format off
#include <winrt/windows.media.core.h>
// clang-format on

void writeToFile(std::vector<Frame>);

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

void writeToFile(std::vector<Frame> frames) {
    winrt::Windows::Media::MediaProperties::VideoEncodingProperties properties{};
    winrt::Windows::Media::Core::VideoStreamDescriptor descriptor{properties};
    winrt::Windows::Media::Core::MediaStreamSource source{descriptor};
    auto start = std::chrono::system_clock::now();

    std::cout << "Writing video file took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now() - start)
                         .count() /
                     1000.0
              << " seconds.\n";
}