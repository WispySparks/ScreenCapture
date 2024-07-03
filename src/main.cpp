#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#include <format>
#include <iostream>
#include <vector>

#include "util.h"

using Microsoft::WRL::ComPtr;

void CaptureWindowGDI(HWND window);
void CaptureWindowDX(HWND window);
std::string GetCommand(std::string pixelFormat);

namespace {
const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

int main(int argc, char* argv[]) {
    HandleError(width == 0, "GetSystemMetrics(Width) failed!");
    HandleError(height == 0, "GetSystemMetrics(Height) failed!");
    // https://stackoverflow.com/questions/76567120/capturing-a-window-on-win-10-without-winrt
    // https://github.com/microsoft/DirectXTK/blob/main/Src/ScreenGrab.cpp
    // should also maybe unmap the texture
    auto windows = GetWindows();
    HWND window = windows.at(0);
    std::cout << GetWindowTitle(window) << "\n";
    // CaptureWindowGDI(window);
    CaptureWindowDX(window);
    std::cout << "---PROGRAM END---\n\n";
    return 0;
}

void CaptureWindowDX(HWND window) {
    ComPtr<ID3D11Device> device = nullptr;
    ComPtr<ID3D11DeviceContext> context = nullptr;
    ComPtr<IDXGIFactory2> factory = nullptr;
    ComPtr<IDXGISwapChain1> swapChain = nullptr;
    ComPtr<ID3D11Texture2D> frameBuffer = nullptr;
    ComPtr<ID3D11Texture2D> copiedFrame = nullptr;
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // DEBUG FLAG
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
                                   D3D11_SDK_VERSION, &device, NULL, &context);
    HandleError(hr, "Couldn't create device!");
    UINT factoryflags = 0;
    factoryflags |= DXGI_CREATE_FACTORY_DEBUG;  // DEBUG FLAG
    hr = CreateDXGIFactory2(factoryflags, IID_PPV_ARGS(&factory));
    HandleError(hr, "Couldn't create factory!");
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = {1, 0};
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = 0;
    hr = factory->CreateSwapChainForHwnd(device.Get(), window, &swapChainDesc, NULL, NULL,
                                         &swapChain);
    HandleError(hr, "Couldn't create swap chain!");
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&frameBuffer));
    HandleError(hr, "Couldn't get swap chain buffer!");
    D3D11_TEXTURE2D_DESC textureDesc;
    frameBuffer->GetDesc(&textureDesc);
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    hr = device->CreateTexture2D(&textureDesc, NULL, &copiedFrame);
    HandleError(hr, "Couldn't create texture 2D!");
    context->CopyResource(copiedFrame.Get(), frameBuffer.Get());
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = context->Map(copiedFrame.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
    HandleError(hr, "Couldn't map the frame buffer!");

    const int size = mappedResource.RowPitch * textureDesc.Height;
    std::vector<uint8_t> buffer(static_cast<uint8_t*>(mappedResource.pData),
                                static_cast<uint8_t*>(mappedResource.pData) + size);
    auto data = (char*)mappedResource.pData;
    for (int i = 0; i < 5000; i++) {
        std::cout << static_cast<int>(*data);
        ++data;
        // std::cout << +*(char*)mappedResource.pData;
        // ++((char*)mappedResource.pData);
    }
    // FILE* pipe = _popen(GetCommand("rgba").c_str(), "wb");
    // const int size = mappedResource.RowPitch * textureDesc.Height;
    // while (!GetAsyncKeyState(VK_RSHIFT)) {
    //     std::fwrite(mappedResource.pData, sizeof(char), size, pipe);
    // }
    // std::fclose(pipe);
}

// Pass in NULL to capture the desktop instead of a specific window.
void CaptureWindowGDI(HWND window) {
    HDC srcDC = GetDC(window);
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
    FILE* pipe = _popen(GetCommand("bgr24").c_str(), "wb");
    while (!GetAsyncKeyState(VK_RSHIFT)) {
        HGDIOBJ prevObj = SelectObject(destDC, bitmap);
        HandleError(prevObj == NULL, "SelectObject(Bitmap) failed!");
        if (window == NULL) {
            int result = BitBlt(destDC, 0, 0, width, height, srcDC, 0, 0, SRCCOPY);
            HandleError(result == 0, "BitBlt failed!");
        } else {
            int result = PrintWindow(window, destDC, 2);
            HandleError(result == 0, "PrintWindow Failed!");
        }
        prevObj = SelectObject(destDC, prevObj);
        HandleError(prevObj == NULL, "SelectObject(Prev) failed!");
        // Fill buffer with screen data as BGR, 8bpp, last parameter is for unused color table
        int result = GetDIBits(srcDC, bitmap, 0, height, buffer.data(), (BITMAPINFO*)&infoHeader,
                               DIB_RGB_COLORS);
        HandleError(result == 0, "GetDIBits failed!");
        std::fwrite(buffer.data(), sizeof(char), buffer.size(), pipe);
    }
    std::fclose(pipe);
    DeleteObject(bitmap);
    DeleteDC(destDC);
    ReleaseDC(window, srcDC);
}

std::string GetCommand(std::string pixelFormat) {
    return std::format(
        "ffmpeg -hide_banner -y -f rawvideo -pix_fmt {} -s {}x{} -i - -c:v "
        "libx264 -pix_fmt yuv420p -r 30 -an out_vid.mp4",
        pixelFormat, width, height);
}