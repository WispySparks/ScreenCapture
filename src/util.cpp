#include <comdef.h>

#include <functional>
#include <iostream>
#include <string>
#include <vector>

void HandleError(HRESULT hr, std::string msg, std::function<void()> beforeExit = {}) {
    if (FAILED(hr)) {
        _com_error err{hr};
        std::cerr << "Error: 0x" << std::hex << hr << ", " << err.ErrorMessage() << "\n"
                  << msg << "\n";
        beforeExit();
        exit(hr);
    }
}

void HandleError(bool failed, std::string msg, std::function<void()> beforeExit = {}) {
    if (failed) {
        DWORD code = GetLastError();
        std::cerr << "Error: 0x" << std::hex << code << ", " << msg << "\n";
        beforeExit();
        exit(code);
    }
}

std::string GetWindowTitle(HWND window) {
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
                GetWindowTitle(window) != "Program Manager") {
                windows->push_back(window);
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&windows));
    return windows;
}