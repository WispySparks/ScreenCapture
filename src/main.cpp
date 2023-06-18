#include <iostream>
#include <vector>
#include <windows.h>
#include <mfapi.h>
#include <Mfidl.h>
#include <comdef.h>
#include <dxgi.h>

void handleError(HRESULT hr, std::string msg);
WCHAR* getDeviceName(IMFAttributes* device);
namespace {
    HRESULT hr;
    const int width = 1280;
    const int height = 720;
}

int main(int argc, char* argv[]) {
    std::vector<IDXGIAdapter*> adapters;
    IDXGIFactory1* factory;
    IDXGIAdapter* adapter;
    CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**) &factory);
    for (int i = 0; factory->EnumAdapters(0, &adapter) != DXGI_ERROR_NOT_FOUND; i++ ) {
        adapters.push_back(adapter);
    }
    std::cout << adapters.size();
    factory->Release();
    std::cout << "---PROGRAM END---\n";
    return 0;
}

WCHAR* getDeviceName(IMFAttributes* device) {
    UINT32 strLen;
    WCHAR* deviceName = NULL;
    hr = device->GetAllocatedString(
        MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
        &deviceName, 
        &strLen
    );
    handleError(hr, "Couldn't get device display name.");
    return deviceName;
}

void handleError(HRESULT hr, std::string msg) {
    if (FAILED(hr)) {
        _com_error err = _com_error(hr);
        std::cout << "Error: 0x" << std::hex << hr << ", " << err.ErrorMessage() << " " << msg << "\n";
        exit(1);
    }
}

// void video() {
    // IMFMediaSource* source; // Video Capture Source
    // IMFAttributes* attributes;
    // IMFActivate** devices; // Video Capture Devices
    // UINT32 amount; 
    // hr= CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    // handleError(hr, "Couldn't initialize the COM library.");
    // hr = MFCreateAttributes(&attributes, 1);
    // handleError(hr, "Couldn't create attribute store.");
    // // Enumerate Video Devices
    // hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID); 
    // handleError(hr, "Couldn't set source type to video capture devices.");
    // hr = MFEnumDeviceSources(attributes, &devices, &amount);
    // handleError(hr, "Couldn't enumerate video capture devices.");
    // std::cout << amount << " video capture device(s).\n";
    // hr = devices[0]->ActivateObject(IID_PPV_ARGS(&source));
    // handleError(hr, "Couldn't activate video capture device.");
    // std::wcout << "Activated video capture device: " << getDeviceName(devices[0]) << "\n";
    // for (int i = 0; i < amount; i++) {
    //     devices[i]->Release();
    // }
    // source->Shutdown();
    // CoUninitialize();
// }