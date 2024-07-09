#include <d3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.graphics.capture.h>

#include <iostream>

#include "util.hpp"

namespace winrt {
using namespace ::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
}

using winrt::com_ptr;

void OnFrameArrived(const winrt::Direct3D11CaptureFramePool& framePool,
                    const winrt::IInspectable&) {
    std::cout << "Frame Arrived!\n";
    winrt::Direct3D11CaptureFrame frame = framePool.TryGetNextFrame();
    winrt::IDirect3DSurface rtSurface = frame.Surface();
    com_ptr<IDXGISurface> surface;
    com_ptr<winrt::IDirect3DDxgiInterfaceAccess> dxgiInterfaceAccess{
        rtSurface.as<winrt::IDirect3DDxgiInterfaceAccess>()};
    HRESULT hr = dxgiInterfaceAccess->GetInterface(IID_PPV_ARGS(&surface));
    HandleError(hr, "Couldn't get IDXGISurface!");
    DXGI_SURFACE_DESC desc;
    hr = surface->GetDesc(&desc);
    HandleError(hr, "Couldn't get DXGI_SURFACE_DESC!");
    DXGI_MAPPED_RECT data;
    hr = surface->Map(&data, DXGI_MAP_READ);
    HandleError(hr, "Couldn't map DXGI_MAPPED_RECT!");

    const int size = data.Pitch * desc.Height;
    std::vector<BYTE> buffer(data.pBits, data.pBits + size);

    for (int i = 0; i < 5000; i++) {
        std::cout << *data.pBits;
        ++data.pBits;
    }
}

void CaptureWindowWGC(HWND window) {
    winrt::GraphicsCaptureAccess::RequestAccessAsync(winrt::GraphicsCaptureAccessKind::Borderless)
        .get();
    com_ptr<ID3D11Device> device{};
    com_ptr<IDXGIDevice> idxgiDevice{};
    winrt::IDirect3DDevice iDevice{};
    winrt::DirectXPixelFormat pixelFormat{winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized};
    winrt::GraphicsCaptureItem item =
        winrt::GraphicsCaptureItem::TryCreateFromWindowId({reinterpret_cast<uint64_t>(window)});
    std::wcout << item.DisplayName().c_str() << "\n";
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // DEBUG FLAG
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
                                   D3D11_SDK_VERSION, device.put(), NULL, NULL);
    HandleError(hr, "Couldn't create ID3D11Device!");
    hr = device->QueryInterface(idxgiDevice.put());
    HandleError(hr, "Couldn't create IDXGIDevice!");
    hr = CreateDirect3D11DeviceFromDXGIDevice(idxgiDevice.get(),
                                              reinterpret_cast<IInspectable**>(&iDevice));
    HandleError(hr, "Couldn't create IDirect3DDevice!");
    winrt::Direct3D11CaptureFramePool framePool =
        winrt::Direct3D11CaptureFramePool::CreateFreeThreaded(iDevice, pixelFormat, 1, item.Size());
    framePool.FrameArrived(&OnFrameArrived);
    winrt::GraphicsCaptureSession session = framePool.CreateCaptureSession(item);
    session.IsBorderRequired(false);
    session.StartCapture();
    while (!GetAsyncKeyState(VK_RSHIFT)) {
    }
}