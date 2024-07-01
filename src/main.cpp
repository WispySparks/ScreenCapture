#include <format>
#include <iostream>
#include <vector>

#include "util.h"

void CaptureFrames(bool captureWindow);

int main(int argc, char* argv[]) {
    CaptureFrames(true);
    std::cout << "---PROGRAM END---\n\n";
    return 0;
}

void CaptureFrames(bool captureWindow) {
    HWND handle = NULL;
    HDC srcDC = GetDC(handle);
    HandleError(srcDC == NULL, "GetDC failed!");
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    HandleError(width == 0, "GetSystemMetrics(Width) failed!");
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    HandleError(height == 0, "GetSystemMetrics(Height) failed!");
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
        int result = GetWindowRect(handle, &rect);
        HandleError(result == 0, "GetWindowRect failed!");
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
        srcDC = GetWindowDC(handle);
        HandleError(srcDC == NULL, "GetWindowDC failed!");
    }
    std::cout << "Width: " << width << " Height: " << height << "\n";
    HDC destDC =                    // IDXGISurface1?
        CreateCompatibleDC(srcDC);  // Place in memory that we're gonna copy the actual screen to
    HandleError(destDC == NULL, "CreateCompatibleDC failed!");
    HBITMAP bitmap = CreateCompatibleBitmap(srcDC, width, height);
    HandleError(bitmap == NULL, "CreateCompatibleBitmap failed!");
    BITMAPINFOHEADER infoHeader = {sizeof(infoHeader), width, -height, 1, 24, BI_RGB};
    std::vector<char> buffer(
        std::abs(infoHeader.biWidth * infoHeader.biHeight * (infoHeader.biBitCount / CHAR_BIT)));
    // Bitmaps are stored as BGR, BI_RGB simply means uncompressed data.
    std::string cmd = std::format(
        "ffmpeg -hide_banner -y -f rawvideo -pix_fmt bgr24 -s {}x{} -i - -c:v "
        "libx264 -pix_fmt yuv420p -r 30 -an out_vid.mp4",
        width, height);
    FILE* pipe = _popen(cmd.c_str(), "wb");
    while (!GetAsyncKeyState(VK_RSHIFT)) {
        HGDIOBJ prevObj = SelectObject(destDC, bitmap);
        HandleError(prevObj == NULL, "SelectObject(Bitmap) failed!");
        int result = BitBlt(destDC, 0, 0, width, height, srcDC, 0, 0, SRCCOPY);
        HandleError(result == 0, "BitBlt failed!");
        prevObj = SelectObject(destDC, prevObj);
        HandleError(prevObj == NULL, "SelectObject(Prev) failed!");
        // Fill buffer with screen data as BGR, 8bpp, last parameter is for unused color table
        result = GetDIBits(srcDC, bitmap, 0, height, buffer.data(), (BITMAPINFO*)&infoHeader,
                           DIB_RGB_COLORS);
        HandleError(result == 0, "GetDIBits failed!");
        std::fwrite(buffer.data(), sizeof(char), buffer.size(), pipe);
    }
    std::fclose(pipe);
    DeleteObject(bitmap);
    DeleteDC(destDC);
    ReleaseDC(handle, srcDC);
}