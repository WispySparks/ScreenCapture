cmake_minimum_required(VERSION 3.28)
project("ScreenCapture")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_FILES "src/main.cpp" "src/util.cpp")
set(LIBRARIES "gdi32" "d3d11" "dxgi")
add_executable(main ${SOURCE_FILES})
target_compile_options(main PRIVATE /W4)
target_link_libraries(main ${LIBRARIES})
# Copy binary to working directory
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:main> ${PROJECT_SOURCE_DIR}/main.exe 
)