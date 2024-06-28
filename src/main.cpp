#include <iostream>
#include <vector>

#include "util.h"

void CaptureFrames(bool captureWindow);

int main(int argc, char* argv[]) {
    CaptureFrames(true);
    std::cout << "---PROGRAM END---\n";
    return 0;
}

void CaptureFrames(bool captureWindow) {
    HDC srcDC = GetDC(NULL);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (captureWindow) {
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
        for (HWND window : windows) {
            std::cout << GetWindowTitle(window) << "\n";
        }
        HWND window = windows.at(0);
        RECT rect;
        GetWindowRect(window, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        std::cout << width << " " << height << "\n";
        srcDC = GetWindowDC(window);
    }
    HDC memoryDC =                  // IDXGISurface1?
        CreateCompatibleDC(srcDC);  // Place in memory that we're gonna copy the actual screen to
    HBITMAP bitmap = CreateCompatibleBitmap(srcDC, width, height);
    SelectObject(memoryDC, bitmap);

    while (true) {
        BitBlt(memoryDC, 0, 0, width, height, srcDC, 0, 0, SRCCOPY);
        Sleep(1000);
    }

    DeleteObject(bitmap);
    DeleteDC(memoryDC);
    DeleteDC(srcDC);
}