#include <iostream>
#include <string>
#include <comdef.h>

void handleError(HRESULT hr, std::string msg) {
    if (FAILED(hr)) {
        _com_error err = _com_error(hr);
        std::cout << "Error: 0x" << std::hex << hr << ", " << err.ErrorMessage() << " " << msg << "\n";
        exit(1);
    }
}