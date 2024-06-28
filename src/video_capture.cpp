#include <Mfidl.h>
#include <mfapi.h>

#include <iostream>

#include "util.h"

WCHAR* getDeviceName(IMFAttributes* device);
void video() {
    IMFMediaSource* source;  // Video Capture Source
    IMFAttributes* attributes;
    IMFActivate** devices;  // Video Capture Devices
    UINT32 amount;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    handleError(hr, "Couldn't initialize the COM library.");
    hr = MFCreateAttributes(&attributes, 1);
    handleError(hr, "Couldn't create attribute store.");
    // Enumerate Video Devices
    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                             MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    handleError(hr, "Couldn't set source type to video capture devices.");
    hr = MFEnumDeviceSources(attributes, &devices, &amount);
    handleError(hr, "Couldn't enumerate video capture devices.");
    std::cout << amount << " video capture device(s).\n";
    hr = devices[0]->ActivateObject(IID_PPV_ARGS(&source));
    handleError(hr, "Couldn't activate video capture device.");
    std::wcout << "Activated video capture device: " << getDeviceName(devices[0]) << "\n";
    for (size_t i = 0; i < amount; i++) {
        devices[i]->Release();
    }
    source->Shutdown();
    CoUninitialize();
}

WCHAR* getDeviceName(IMFAttributes* device) {
    UINT32 strLen;
    WCHAR* deviceName = NULL;
    HRESULT hr =
        device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &deviceName, &strLen);
    handleError(hr, "Couldn't get device display name.");
    return deviceName;
}