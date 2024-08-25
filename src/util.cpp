#include <comdef.h>

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void HandleError(HRESULT hr, std::string msg, std::function<void()> beforeExit = {}) {
    if (FAILED(hr)) {
        _com_error err{hr};
        std::cerr << "Error: 0x" << std::hex << hr << ", " << err.ErrorMessage() << "\n";
        beforeExit();
        throw std::runtime_error(msg);
    }
}

void HandleError(bool failed, std::string msg, std::function<void()> beforeExit = {}) {
    if (failed) {
        DWORD code = GetLastError();
        std::cerr << "Error: 0x" << std::hex << code << "\n";
        beforeExit();
        throw std::runtime_error(msg);
    }
}

std::string GetWindowName(HWND window) {
    int length = GetWindowTextLengthA(window) + 1;
    std::vector<char> title(length);
    GetWindowTextA(window, title.data(), length);
    return std::string{title.data()};
}

std::vector<HWND> GetWindows() {
    std::vector<HWND> windows;
    EnumWindows(
        [](HWND window, LPARAM lParam) {
            std::vector<HWND>* windows = reinterpret_cast<std::vector<HWND>*>(lParam);
            if (IsWindowVisible(window) && GetWindowTextLengthA(window) > 0 &&
                GetWindowName(window) != "Program Manager") {
                windows->push_back(window);
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&windows));
    return windows;
}

std::string GetDisplayName(HMONITOR display) {
    MONITORINFOEXA info{sizeof(MONITORINFOEXA)};
    int result = GetMonitorInfoA(display, &info);
    HandleError(result == 0, "Couldn't get display name!");
    return std::string{info.szDevice};
}

std::vector<HMONITOR> GetDisplays() {
    std::vector<HMONITOR> displays;
    EnumDisplayMonitors(
        NULL, NULL,
        [](HMONITOR display, HDC, LPRECT, LPARAM lParam) {
            std::vector<HMONITOR>* displays = reinterpret_cast<std::vector<HMONITOR>*>(lParam);
            displays->push_back(display);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&displays));
    return displays;
}