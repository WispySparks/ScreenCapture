#include <comdef.h>

#include <iostream>

void handleError(HRESULT hr, std::string msg) {
    if (FAILED(hr)) {
        _com_error err{hr};
        std::cout << "Error: 0x" << std::hex << hr << ", " << err.ErrorMessage() << " " << msg
                  << "\n";
        exit(1);
    }
}