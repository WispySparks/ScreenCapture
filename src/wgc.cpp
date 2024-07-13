#include <d3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/base.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.graphics.capture.h>

#include "util.hpp"

namespace winrt {
using namespace ::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
}

using winrt::com_ptr;

com_ptr<ID3D11Device> device{};
com_ptr<ID3D11DeviceContext> context{};
FILE* pipe = nullptr;

void OnFrameArrived(const winrt::Direct3D11CaptureFramePool& framePool,
                    const winrt::IInspectable&) {
    winrt::Direct3D11CaptureFrame frame = framePool.TryGetNextFrame();
    if (frame == NULL) return;
    winrt::IDirect3DSurface rtSurface = frame.Surface();
    com_ptr<IDXGISurface> surface{};
    com_ptr<winrt::IDirect3DDxgiInterfaceAccess> dxgiInterfaceAccess =
        rtSurface.as<winrt::IDirect3DDxgiInterfaceAccess>();
    com_ptr<ID3D11Texture2D> frameTexture{};
    com_ptr<ID3D11Texture2D> stagingTexture{};
    HRESULT hr = dxgiInterfaceAccess->GetInterface(IID_PPV_ARGS(&surface));
    HandleError(hr, "Couldn't get IDXGISurface!");
    hr = surface->QueryInterface(frameTexture.put());
    HandleError(hr, "Couldn't get ID3D11Texture2D!");

    D3D11_TEXTURE2D_DESC textureDesc;
    frameTexture->GetDesc(&textureDesc);
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    textureDesc.MiscFlags = 0;
    hr = device->CreateTexture2D(&textureDesc, NULL, stagingTexture.put());
    HandleError(hr, "Couldn't create ID3D11Texture2D!");

    context->CopyResource(stagingTexture.get(), frameTexture.get());
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = context->Map(stagingTexture.get(), 0, D3D11_MAP_READ, 0, &mappedResource);
    HandleError(hr, "Couldn't map DXGI_MAPPED_RECT!");
    context->Unmap(stagingTexture.get(), 0);

    const int size = mappedResource.RowPitch * textureDesc.Height;
    std::vector<uint8_t> buffer(static_cast<uint8_t*>(mappedResource.pData),
                                static_cast<uint8_t*>(mappedResource.pData) + size);
    std::fwrite(buffer.data(), sizeof(uint8_t), buffer.size(), pipe);

    frame.Close();
}

void CaptureDisplayWGC(HMONITOR display) {
    winrt::GraphicsCaptureAccess::RequestAccessAsync(winrt::GraphicsCaptureAccessKind::Borderless)
        .get();
    com_ptr<IDXGIDevice> idxgiDevice{};
    winrt::IDirect3DDevice iDevice{};
    winrt::DirectXPixelFormat pixelFormat{winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized};
    winrt::GraphicsCaptureItem item =
        winrt::GraphicsCaptureItem::TryCreateFromDisplayId({reinterpret_cast<uint64_t>(display)});
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // DEBUG FLAG
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
                                   D3D11_SDK_VERSION, device.put(), NULL, context.put());
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
    pipe = _popen(GetCommand("bgra", item.Size().Width, item.Size().Height, 60).c_str(), "wb");
    session.StartCapture();
    while (!GetAsyncKeyState(VK_RSHIFT)) {
    }
    std::fclose(pipe);
}