# ScreenCapture

A Windows 10/11 application used to record displays and windows.

## Building
CMake is required to build this program which can be downloaded [here](https://cmake.org/download/). <br> A mimimum version of `3.28` is required. <br>
You will also need MSVC and the Windows SDK which can both be downloaded through the [Visual Studio Installer](https://visualstudio.microsoft.com/vs/community/). <br>
First clone the repository.
```
git clone https://github.com/WispySparks/ScreenCapture.git
```
Then navigate to the project and run this command to set up the build system. 
```
cmake -S . -B build/
```
Then run this command to build the program.
```
cmake --build build/
```
Finally to run the program.
```
./main.exe
```