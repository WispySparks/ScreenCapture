#include <comdef.h>

#include <iostream>
#include <string>
#include <vector>

void HandleError(HRESULT hr, std::string msg) {
    if (FAILED(hr)) {
        _com_error err{hr};
        std::cerr << "Error: 0x" << std::hex << hr << ", " << err.ErrorMessage() << " " << msg
                  << "\n";
        exit(hr);
    }
}

void HandleError(bool failed, std::string msg) {
    if (failed) {
        DWORD code = GetLastError();
        std::cerr << "Error: 0x" << std::hex << code << ", " << msg << "\n";
        exit(code);
    }
}

std::string GetWindowTitle(HWND window) {
    int length = GetWindowTextLengthA(window) + 1;
    std::vector<char> title(length);
    GetWindowTextA(window, title.data(), length);
    return std::string{title.data()};
}