#include <iostream>
#include <windows.h>

int main(int argc, char* argv[]) {
    const int width = 1280;
    const int height = 720;
    HDC displayContext = CreateDC("DISPLAY", NULL, NULL, NULL);
    HDC memoryDisplayContext = CreateCompatibleDC(displayContext);
    HBITMAP memoryBitmap = CreateCompatibleBitmap(memoryDisplayContext, width, height);
    SelectObject(memoryDisplayContext, memoryBitmap);
    BitBlt(memoryDisplayContext, 0, 0, width, height, displayContext, 0, 0, CAPTUREBLT);
    BitBlt(displayContext, 0, 0, width, height, memoryDisplayContext, 0, 0, CAPTUREBLT);
    DeleteObject(memoryBitmap);
    DeleteDC(memoryDisplayContext);
    DeleteDC(displayContext);
    std::cout << "---END---";
    return 0;
}
