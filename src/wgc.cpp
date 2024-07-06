#include <d3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/windows.graphics.capture.h>

#include <iostream>

namespace winrt {
using namespace ::Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
}

using winrt::check_hresult;
using winrt::com_ptr;

void OnFrameArrived(winrt::Direct3D11CaptureFramePool const& framePool,
                    IInspectable const& inspectable) {
    winrt::Direct3D11CaptureFrame frame = framePool.TryGetNextFrame();
    winrt::IDirect3DSurface rtSurface = frame.Surface();
    com_ptr<IDXGISurface> surface;
    com_ptr<winrt::IDirect3DDxgiInterfaceAccess> dxgiInterfaceAccess{
        rtSurface.as<winrt::IDirect3DDxgiInterfaceAccess>()};
    dxgiInterfaceAccess->GetInterface(IID_PPV_ARGS(&surface));
    DXGI_SURFACE_DESC desc;
    surface->GetDesc(&desc);
    DXGI_MAPPED_RECT data;
    surface->Map(&data, DXGI_MAP_READ);

    const int size = data.Pitch * desc.Height;
    std::vector<BYTE> buffer(data.pBits, data.pBits + size);

    for (int i = 0; i < 5000; i++) {
        std::cout << *data.pBits;
        ++data.pBits;
    }
}

void CaptureWindowWGC(HWND window) {
    com_ptr<ID3D11Device> device{};
    com_ptr<IDXGIDevice> dxgiDevice{};
    winrt::IDirect3DDevice iDevice{};
    winrt::DirectXPixelFormat pixelFormat{winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized};
    winrt::GraphicsCaptureItem item =
        winrt::GraphicsCaptureItem::TryCreateFromWindowId({reinterpret_cast<uint64_t>(window)});
    std::wcout << item.DisplayName().c_str() << "\n";
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // DEBUG FLAG
    check_hresult(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
                                    D3D11_SDK_VERSION, device.put(), NULL, NULL));
    check_hresult(CreateDirect3D11DeviceFromDXGIDevice(
        dxgiDevice.get(), reinterpret_cast<IInspectable**>(winrt::put_abi(iDevice))));
    winrt::Direct3D11CaptureFramePool framePool =
        winrt::Direct3D11CaptureFramePool::Create(iDevice, pixelFormat, 1, item.Size());
    framePool.FrameArrived(&OnFrameArrived);
    winrt::GraphicsCaptureSession session = framePool.CreateCaptureSession(item);
    session.StartCapture();
}