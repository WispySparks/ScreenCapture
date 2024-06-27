#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <windows.h>
#include <winnt.h>

#include <cstddef>
#include <iostream>
#include <vector>

#include "util.h"

void getDisplayOutputs(std::vector<IDXGIOutput1*>* outputs);
void writeFrameToDisk(IDXGIOutputDuplication* display);
namespace {
const int frameTime = 1.0 / 30.0 * 1000;
}

int main(int argc, char* argv[]) {
    std::vector<IDXGIOutput1*> outputs;
    ID3D11Device* device;
    IDXGIOutputDuplication* duplicateDisplay;
    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
                                   D3D11_SDK_VERSION, &device, NULL, NULL);
    handleError(hr, "Couldn't create Direct 3D device.");
    getDisplayOutputs(&outputs);
    hr = outputs.at(0)->DuplicateOutput(device, &duplicateDisplay);
    handleError(hr, "Couldn't create duplicate output.");
    writeFrameToDisk(duplicateDisplay);
    std::cout << "---PROGRAM END---\n";
    return 0;
}

void getDisplayOutputs(std::vector<IDXGIOutput1*>* outputs) {
    std::vector<IDXGIAdapter1*> adapters;
    IDXGIFactory1* factory;
    IDXGIAdapter1* adapter;
    IDXGIOutput* output;
    DXGI_ADAPTER_DESC adapterDesc;
    DXGI_OUTPUT_DESC outputDesc;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
    handleError(hr, "Couldn't create DXGI factory.");
    for (int i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
        adapters.push_back(adapter);
    }
    for (int i = 0; i < adapters.size(); i++) {
        adapters.at(i)->GetDesc(&adapterDesc);
        std::wcout << "Adapter: " << adapterDesc.Description << "\n";
        for (int j = 0; adapters.at(i)->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; j++) {
            IDXGIOutput1* temp;
            hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&temp);
            handleError(hr, "Couldn't convert IDXGIOutput to IDXGIOutput1.");
            outputs->push_back(temp);
            output->GetDesc(&outputDesc);
            std::wcout << "Output: " << outputDesc.DeviceName << "\n";
        }
    }
}

void writeFrameToDisk(IDXGIOutputDuplication* display) {
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* desktopResource;
    ID3D11Texture2D* texture;
    D3D11_TEXTURE2D_DESC desc;
    HRESULT hr = display->AcquireNextFrame(frameTime, &frameInfo, &desktopResource);
    handleError(hr, "Couldn't get next frame.");
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
    handleError(hr, "Couldn't convert frame to texture2D.");
    texture->GetDesc(&desc);
    std::wcout << desc.Width << "\n";
    std::wcout << desc.Height << "\n";
    std::wcout << desc.Usage << "\n";
    std::wcout << desc.Format << "\n";
    // write texture to disk
}