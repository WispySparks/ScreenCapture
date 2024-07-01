#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#include <string>

void HandleError(HRESULT hr, std::string msg);
void HandleError(bool failed, std::string msg);
std::string GetWindowTitle(HWND window);

#endif