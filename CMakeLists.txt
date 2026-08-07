cmake_minimum_required(VERSION 3.13)

# Configure with:
#   cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=repos/threadx/cmake/win32.cmake
# (run from a Visual Studio Developer shell so cl.exe is on PATH)

project(win_rtos C)

set(THREADX_ARCH win32)
set(THREADX_TOOLCHAIN vs_2019)

add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/repos/threadx
    ${CMAKE_BINARY_DIR}/threadx
)

add_executable(win_rtos
    app/main.c
)

target_link_libraries(win_rtos
    threadx
)
