#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#include <format>
#include <functional>
#include <string>
#include <vector>

void HandleError(HRESULT hr, std::string msg, std::function<void()> beforeExit = {});
void HandleError(bool failed, std::string msg, std::function<void()> beforeExit = {});
std::string GetWindowTitle(HWND window);
std::vector<HWND> GetWindows();
std::vector<HMONITOR> GetDisplays();
inline std::string GetCommand(std::string pixelFormat, int width, int height) {
    return std::format(
        "ffmpeg -hide_banner -y -f rawvideo -pix_fmt {} -s {}x{} -i - -c:v "
        "libx264 -pix_fmt yuv420p -r 30 -an out_vid.mp4",
        pixelFormat, width, height);
}

#endif