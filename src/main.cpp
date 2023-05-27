#include <iostream>
#include <synchapi.h>
#include <windef.h>
#include <windows.h>
#include <wingdi.h>

namespace {
    const int width = 1280;
    const int height = 720;
}

int main(int argc, char* argv[]) {
    HDC screen = GetDC(NULL);
    HDC memoryScreen = CreateCompatibleDC(screen); // Place in memory that we're gonna copy the actual screen to
    HBITMAP bitmap = CreateCompatibleBitmap(screen, width, height);
    SelectObject(memoryScreen, bitmap);

    while (true) {
        // Copy the screen to our memory that currently has a bitmap
        BitBlt(memoryScreen, 0, 0, width, height, screen, 0, 0, SRCCOPY);
        EmptyClipboard();
        OpenClipboard(NULL);
        SetClipboardData(CF_BITMAP, bitmap);
        CloseClipboard();
        std::cout << "Copied screenshot to clipboard!\n";
        Sleep(1000);
    }

    DeleteObject(bitmap);
    DeleteDC(memoryScreen);
    DeleteDC(screen);
    std::cout << "---PROGRAM END---";
    return 0;
}
