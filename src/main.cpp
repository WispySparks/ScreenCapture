#include <iostream>
#include <vector>

#include "util.h"

void CaptureFrames(bool captureWindow);

int main(int argc, char* argv[]) {
    CaptureFrames(false);
    std::cout << "---PROGRAM END---\n";
    return 0;
}

void CaptureFrames(bool captureWindow) {
    HWND handle = NULL;
    HDC srcDC = GetDC(handle);
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
        handle = windows.at(0);
        RECT rect;
        GetWindowRect(handle, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
        srcDC = GetWindowDC(handle);
    }
    std::cout << "Width: " << width << " Height: " << height << "\n";
    HDC destDC =                    // IDXGISurface1?
        CreateCompatibleDC(srcDC);  // Place in memory that we're gonna copy the actual screen to
    HBITMAP bitmap = CreateCompatibleBitmap(srcDC, width, height);
    BITMAPINFOHEADER infoHeader = {sizeof(infoHeader), width, -height, 1, 24, BI_RGB};
    std::vector<char> buffer(abs(infoHeader.biWidth * infoHeader.biHeight * infoHeader.biBitCount));
    SelectObject(destDC, bitmap);
    while (!GetAsyncKeyState(VK_RSHIFT)) {
        BitBlt(destDC, 0, 0, width, height, srcDC, 0, 0, SRCCOPY);
        GetDIBits(srcDC, bitmap, 0, height, buffer.data(), (BITMAPINFO*)&infoHeader,
                  DIB_RGB_COLORS);
        for (size_t i = 0; i < buffer.size(); i++) {
            std::cout << static_cast<int>(buffer.at(i)) << "\n";
        }
        OpenClipboard(NULL);
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, bitmap);
        CloseClipboard();
        Sleep(1000);
        // https://stackoverflow.com/questions/51903888/is-it-possible-to-send-ffmpeg-images-by-using-pipe
        // ffmpeg -y -f rawvideo -pix_fmt argb -s 800x600 -r 25 -i - -c:v libx264 -profile:v baseline -level:v 3 -b:v 2500 -an out_vid.h264
    }
    DeleteObject(bitmap);
    DeleteDC(destDC);
    ReleaseDC(handle, srcDC);
}