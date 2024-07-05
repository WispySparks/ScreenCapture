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
void CaptureWindowDD(HWND window);
std::string GetCommand(std::string pixelFormat);

namespace {
const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
const int timeoutMS = static_cast<int>(1.0 / 30.0 * 1000);
}

int main() {
    HandleError(width == 0, "GetSystemMetrics(Width) failed!");
    HandleError(height == 0, "GetSystemMetrics(Height) failed!");
    // Dpi aware
    // Figure out holding certain frames for longer or dropping frames
    // Should also maybe unmap the texture
    // If can't get swapchains to work have both GDI method and desktop dupe method with cropping
    auto windows = GetWindows();
    HWND window = windows.at(0);
    std::cout << GetWindowTitle(window) << "\n";
    // CaptureWindowGDI(window);
    // CaptureWindowDX(window);
    CaptureWindowDD(window);
    std::cout << "---PROGRAM END---\n\n";
    return 0;
}

void CaptureWindowDD(HWND window) {
    ComPtr<ID3D11Device> device{};
    ComPtr<ID3D11DeviceContext> context{};
    ComPtr<IDXGIDevice> idxgiDevice{};
    ComPtr<IDXGIAdapter> adapter{};
    std::vector<ComPtr<IDXGIOutput>> outputs{};
    ComPtr<IDXGIOutput1> output{};
    ComPtr<IDXGIOutputDuplication> duplication{};
    ComPtr<IDXGIResource> resource{};
    ComPtr<ID3D11Texture2D> frame{};
    ComPtr<ID3D11Texture2D> copiedFrame{};
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // DEBUG FLAG
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
                                   D3D11_SDK_VERSION, &device, NULL, &context);
    HandleError(hr, "Couldn't create ID3D11Device!");
    hr = device->QueryInterface(idxgiDevice.GetAddressOf());
    HandleError(hr, "Couldn't create IDXGIDevice!");
    hr = idxgiDevice->GetAdapter(&adapter);
    HandleError(hr, "Couldn't get IDXGIAdapter!");
    ComPtr<IDXGIOutput> tempOutput;
    int i = 0;
    while ((hr = adapter->EnumOutputs(i, &tempOutput)) != DXGI_ERROR_NOT_FOUND) {
        HandleError(hr, "Couldn't enumerate output!");
        outputs.push_back(tempOutput);
        ++i;
    }

    DXGI_OUTPUT_DESC outputDesc;
    for (auto o : outputs) {
        hr = o->GetDesc(&outputDesc);
        HandleError(hr, "Couldn't get DXGI_OUTPUT_DESC!");
        std::wcout << outputDesc.DeviceName << "\n";
    }

    hr = outputs.at(0)->QueryInterface(output.GetAddressOf());
    HandleError(hr, "Couldn't get IDXGIOutput1!");
    hr = output->DuplicateOutput(device.Get(), &duplication);
    HandleError(hr, "Couldn't get IDXGIOutputDuplication!");
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    hr = duplication->AcquireNextFrame(timeoutMS, &frameInfo, &resource);
    std::cout << std::hex << hr << "\n";
    std::cout << frameInfo.LastPresentTime.QuadPart << "\n";
    HandleError(hr, "Couldn't get IDXGIResource!");
    hr = resource->QueryInterface(frame.GetAddressOf());
    HandleError(hr, "Couldn't get ID3D11Texture2D!");
    D3D11_TEXTURE2D_DESC textureDesc;
    frame->GetDesc(&textureDesc);
    std::wcout << textureDesc.Width << "\n";
    std::wcout << textureDesc.Height << "\n";
    std::wcout << textureDesc.Usage << "\n";
    std::wcout << std::hex << textureDesc.Format << "\n";
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    textureDesc.MiscFlags = 0;
    hr = device->CreateTexture2D(&textureDesc, NULL, &copiedFrame);
    HandleError(hr, "Couldn't create texture 2D!");
    context->CopyResource(copiedFrame.Get(), frame.Get());
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = context->Map(copiedFrame.Get(), 0, D3D11_MAP_READ, 0,
                      &mappedResource);  // Need to make a copy before I read it
    HandleError(hr, "Couldn't map the frame buffer!");

    const int size = mappedResource.RowPitch * textureDesc.Height;
    std::vector<uint8_t> buffer(static_cast<uint8_t*>(mappedResource.pData),
                                static_cast<uint8_t*>(mappedResource.pData) + size);
    auto data = (char*)mappedResource.pData;
    for (int i = 0; i < 5000; i++) {
        std::cout << static_cast<int>(*data);
        ++data;
    }
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
    HDC srcDC = nullptr;
    HDC destDC = nullptr;
    HBITMAP bitmap = nullptr;
    FILE* pipe = nullptr;
    std::function<void()> cleanup = [&]() {
        if (pipe != nullptr) std::fclose(pipe);
        if (bitmap != nullptr) DeleteObject(bitmap);
        if (destDC != nullptr) DeleteDC(destDC);
        if (srcDC != nullptr) ReleaseDC(window, srcDC);
    };
    srcDC = GetDC(window);
    HandleError(srcDC == NULL, "GetDC failed!", cleanup);
    destDC =
        CreateCompatibleDC(srcDC);  // Place in memory that we're gonna copy the actual screen to
    HandleError(destDC == NULL, "CreateCompatibleDC failed!", cleanup);
    bitmap = CreateCompatibleBitmap(srcDC, width, height);
    HandleError(bitmap == NULL, "CreateCompatibleBitmap failed!", cleanup);
    BITMAPINFOHEADER infoHeader = {sizeof(infoHeader), width, -height, 1, 24, BI_RGB};
    std::vector<char> buffer(
        std::abs(infoHeader.biWidth * infoHeader.biHeight * (infoHeader.biBitCount / CHAR_BIT)));
    // Bitmaps are stored as BGR, BI_RGB simply means uncompressed data.
    pipe = _popen(GetCommand("bgr24").c_str(), "wb");
    while (!GetAsyncKeyState(VK_RSHIFT)) {
        HGDIOBJ prevObj = SelectObject(destDC, bitmap);
        HandleError(prevObj == NULL, "SelectObject(Bitmap) failed!", cleanup);
        if (window == NULL) {
            int result = BitBlt(destDC, 0, 0, width, height, srcDC, 0, 0, SRCCOPY);
            HandleError(result == 0, "BitBlt failed!", cleanup);
        } else {
            int result = PrintWindow(window, destDC, PW_RENDERFULLCONTENT);
            HandleError(result == 0, "PrintWindow Failed!", cleanup);
        }
        prevObj = SelectObject(destDC, prevObj);
        HandleError(prevObj == NULL, "SelectObject(Prev) failed!", cleanup);
        // Fill buffer with screen data as BGR, 8bpp, last parameter is for unused color table
        int result = GetDIBits(srcDC, bitmap, 0, height, buffer.data(), (BITMAPINFO*)&infoHeader,
                               DIB_RGB_COLORS);
        HandleError(result == 0, "GetDIBits failed!", cleanup);
        std::fwrite(buffer.data(), sizeof(char), buffer.size(), pipe);
    }
    cleanup();
}

std::string GetCommand(std::string pixelFormat) {
    return std::format(
        "ffmpeg -hide_banner -y -f rawvideo -pix_fmt {} -s {}x{} -i - -c:v "
        "libx264 -pix_fmt yuv420p -r 30 -an out_vid.mp4",
        pixelFormat, width, height);
}