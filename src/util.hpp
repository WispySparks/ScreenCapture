#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#include <format>
#include <functional>
#include <string>
#include <vector>

void HandleError(HRESULT hr, std::string msg, std::function<void()> beforeExit = {});
void HandleError(bool failed, std::string msg, std::function<void()> beforeExit = {});
std::string GetWindowName(HWND window);
std::vector<HWND> GetWindows();
std::string GetDisplayName(HMONITOR display);
std::vector<HMONITOR> GetDisplays();
inline std::string GetCommand(std::string pixelFormat, int width, int height, int fps,
                              std::string loglevel = "info") {
    return std::format(
        "ffmpeg -hide_banner -v {} -y -f rawvideo -pix_fmt {} -s {}x{} -r {} -i - -c:v "
        "libx264 -pix_fmt yuv420p -an out_vid.mp4",
        loglevel, pixelFormat, width, height, fps);
}

#endif