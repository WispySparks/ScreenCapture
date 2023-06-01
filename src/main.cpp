#include <iostream>
#include <string>
#include <windows.h>

namespace {
    const int width = 1280;
    const int height = 720;
    const std::string basePath = "C:\\Users\\wispy\\ProgrammingProjects\\C++\\ScreenCapture\\output\\image-";
}

int main(int argc, char* argv[]) {
    HDC screen = GetDC(NULL);
    HDC memoryScreen = CreateCompatibleDC(screen); // Place in memory that we're gonna copy the actual screen to
    HBITMAP bitmap = CreateCompatibleBitmap(screen, width, height);
    SelectObject(memoryScreen, bitmap);
    int i = 0;
    while (true) {
        // Copy the screen to our memory that currently has a bitmap
        BitBlt(memoryScreen, 0, 0, width, height, screen, 0, 0, SRCCOPY);
        std::string filePath = basePath + std::to_string(i) + ".txt";
        HANDLE file = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        std::string data = "data!!!";
        WriteFile(file, &data, data.length(), NULL, NULL);
        CloseHandle(file);
        std::cout << "Wrote file: " << filePath << "\n";
        Sleep(1000);
        i++;
    }

    DeleteObject(bitmap);
    DeleteDC(memoryScreen);
    DeleteDC(screen);
    std::cout << "---PROGRAM END---";
    return 0;
}
