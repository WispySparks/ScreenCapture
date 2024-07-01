#include <d3d11.h>
#include <dxgi1_3.h>

#include <format>
#include <iostream>
#include <vector>

#include "util.h"

void CaptureDesktop();
void CaptureWindow();

int main(int argc, char* argv[]) {
    //! use a swap chain but keep this for desktops, also use a static size like obs
    // https://stackoverflow.com/a/43631781
    // CaptureDesktop();
    CaptureWindow();
    std::cout << "---PROGRAM END---\n\n";
    return 0;
}

void CaptureWindow() {
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGIFactory2* factory;
    IDXGISwapChain1* swapChain;
    ID3D11Texture2D* frameBuffer;
    ID3D11Texture2D* copiedFrame;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    auto windows = GetWindows();
    HWND window = windows.at(0);
    std::cout << GetWindowTitle(window) << "\n";
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
                                   D3D11_SDK_VERSION, &device, NULL, &context);
    HandleError(hr, "Couldn't create device!");
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    HandleError(hr, "Couldn't create factory!");
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
    HandleError(hr, "Couldn't create swap chain!");
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&frameBuffer));
    HandleError(hr, "Couldn't get swap chain buffer!");
    context->CopyResource(copiedFrame, frameBuffer);
    // D3D11_TEXTURE2D_DESC textureDesc;
    // device->CreateTexture2D(&textureDesc, NULL, &copiedFrame);
    hr = context->Map(copiedFrame, 0, D3D11_MAP_READ, 0, &mappedResource);
    HandleError(hr, "Couldn't map the frame buffer!");
    // Cleanup
    copiedFrame->Release();
    frameBuffer->Release();
    swapChain->Release();
    factory->Release();
    device->Release();
}

void CaptureDesktop() {
    HWND handle = NULL;
    HDC srcDC = GetDC(handle);
    HandleError(srcDC == NULL, "GetDC failed!");
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    HandleError(width == 0, "GetSystemMetrics(Width) failed!");
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    HandleError(height == 0, "GetSystemMetrics(Height) failed!");
    HDC destDC =
        CreateCompatibleDC(srcDC);  // Place in memory that we're gonna copy the actual screen to
    HandleError(destDC == NULL, "CreateCompatibleDC failed!");
    HBITMAP bitmap = CreateCompatibleBitmap(srcDC, width, height);
    HandleError(bitmap == NULL, "CreateCompatibleBitmap failed!");
    BITMAPINFOHEADER infoHeader = {sizeof(infoHeader), width, -height, 1, 24, BI_RGB};
    std::vector<char> buffer(
        std::abs(infoHeader.biWidth * infoHeader.biHeight * (infoHeader.biBitCount / CHAR_BIT)));
    // Bitmaps are stored as BGR, BI_RGB simply means uncompressed data.
    std::string cmd = std::format(
        "ffmpeg -hide_banner -y -f rawvideo -pix_fmt bgr24 -s {}x{} -i - -c:v "
        "libx264 -pix_fmt yuv420p -r 30 -an out_vid.mp4",
        width, height);
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