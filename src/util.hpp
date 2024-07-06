#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#include <functional>
#include <string>
#include <vector>

void HandleError(HRESULT hr, std::string msg, std::function<void()> beforeExit = {});
void HandleError(bool failed, std::string msg, std::function<void()> beforeExit = {});
std::string GetWindowTitle(HWND window);
std::vector<HWND> GetWindows();

#endif