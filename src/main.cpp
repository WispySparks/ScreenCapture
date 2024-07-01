#include <d3d11.h>
#include <dxgi1_3.h>

#include <format>
#include <functional>
#include <iostream>
#include <vector>

#include "util.h"

void CaptureDesktop();
void CaptureWindow();

namespace {
const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
const std::string cmd = std::format(
    "ffmpeg -hide_banner -y -f rawvideo -pix_fmt bgr24 -s {}x{} -i - -c:v "
    "libx264 -pix_fmt yuv420p -r 30 -an out_vid.mp4",
    width, height);
}

int main(int argc, char* argv[]) {
    HandleError(width == 0, "GetSystemMetrics(Width) failed!");
    HandleError(height == 0, "GetSystemMetrics(Height) failed!");
    // https://stackoverflow.com/a/43631781
    // https://stackoverflow.com/questions/76567120/capturing-a-window-on-win-10-without-winrt
    // CaptureDesktop();
    CaptureWindow();
    std::cout << "---PROGRAM END---\n\n";
    return 0;
}

void CaptureWindow() {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGIFactory2* factory = nullptr;
    IDXGISwapChain1* swapChain = nullptr;
    ID3D11Texture2D* frameBuffer = nullptr;
    ID3D11Texture2D* copiedFrame = nullptr;
    std::function<void()> cleanup = [&]() {
        ReleaseIfExists(copiedFrame);
        ReleaseIfExists(frameBuffer);
        ReleaseIfExists(swapChain);
        ReleaseIfExists(factory);
        if (context != nullptr) {
            context->ClearState();
            context->Flush();
            context->Release();
        }
        ReleaseIfExists(device);
    };
    auto windows = GetWindows();
    HWND window = windows.at(0);
    std::cout << GetWindowTitle(window) << "\n";
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
                                   D3D11_SDK_VERSION, &device, NULL, &context);
    HandleError(hr, "Couldn't create device!", cleanup);
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    HandleError(hr, "Couldn't create factory!", cleanup);
    DXGI_SWAP_CHAIN_DESC1 desc;
    desc.Width = 0;
    desc.Height = 0;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc = {1, 0};
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = 0;
    hr = factory->CreateSwapChainForHwnd(device, window, &desc, NULL, NULL, &swapChain);
    HandleError(hr, "Couldn't create swap chain!", cleanup);
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&frameBuffer));
    HandleError(hr, "Couldn't get swap chain buffer!", cleanup);
    context->CopyResource(copiedFrame, frameBuffer);  //! currently fails cause copied frame is null
    // D3D11_TEXTURE2D_DESC textureDesc;
    // device->CreateTexture2D(&textureDesc, NULL, &copiedFrame);
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = context->Map(copiedFrame, 0, D3D11_MAP_READ, 0, &mappedResource);
    HandleError(hr, "Couldn't map the frame buffer!", cleanup);
    // Cleanup
    cleanup();
}

void CaptureDesktop() {
    HWND handle = NULL;
    HDC srcDC = GetDC(handle);
    HandleError(srcDC == NULL, "GetDC failed!");
    HDC destDC =
        CreateCompatibleDC(srcDC);  // Place in memory that we're gonna copy the actual screen to
    HandleError(destDC == NULL, "CreateCompatibleDC failed!");
    HBITMAP bitmap = CreateCompatibleBitmap(srcDC, width, height);
    HandleError(bitmap == NULL, "CreateCompatibleBitmap failed!");
    BITMAPINFOHEADER infoHeader = {sizeof(infoHeader), width, -height, 1, 24, BI_RGB};
    std::vector<char> buffer(
        std::abs(infoHeader.biWidth * infoHeader.biHeight * (infoHeader.biBitCount / CHAR_BIT)));
    // Bitmaps are stored as BGR, BI_RGB simply means uncompressed data.
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