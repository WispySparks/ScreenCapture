#ifndef UTIL_HPP
#define UTIL_HPP

#include <windows.h>

#include <functional>
#include <string>
#include <vector>

void HandleError(HRESULT hr, std::string msg, std::function<void()> beforeExit = {});
void HandleError(bool failed, std::string msg, std::function<void()> beforeExit = {});
std::string GetWindowName(HWND window);
std::vector<HWND> GetWindows();
std::string GetDisplayName(HMONITOR display);
std::vector<HMONITOR> GetDisplays();

#endif