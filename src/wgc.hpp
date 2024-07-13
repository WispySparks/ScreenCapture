#ifndef WGC_HPP
#define WGC_HPP

#include <windows.h>

void CaptureDisplayWGC(HMONITOR display, bool captureCursor);
void CaptureWindowWGC(HWND window, bool captureCursor);

#endif