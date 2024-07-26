cmake_minimum_required(VERSION 3.28)
project("ScreenCapture")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(TARGET "main")
set(SOURCE_FILES "src/main.cpp" "src/util.cpp" "src/wgc.cpp" "src/encoder.cpp")
set(LIBRARIES "d3d11" "mfplat" "mfreadwrite")
add_executable(${TARGET} ${SOURCE_FILES})
target_compile_options(${TARGET} PRIVATE "/W4")
target_link_libraries(${TARGET} ${LIBRARIES})
# Copy binary to working directory
add_custom_command(
  TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET}>
          ${PROJECT_SOURCE_DIR}/${TARGET}.exe)
