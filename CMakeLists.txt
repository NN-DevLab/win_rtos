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

# --- win_rtos_poc: NetX Duo + FileX + GUIX integration smoke test ---------

# FileX has no ready CMake win32 port upstream; point it at our own
# (adds fx_win32_file_driver.c, our file-backed persistence driver).
set(FILEX_CUSTOM_PORT ${CMAKE_CURRENT_LIST_DIR}/port/win32/filex)
add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/repos/filex
    ${CMAKE_BINARY_DIR}/filex
)

# NetX Duo's win32 port ships an empty CMakeLists.txt (no port-specific
# sources needed), so the default add_subdirectory path works as-is.
add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/repos/netxduo
    ${CMAKE_BINARY_DIR}/netxduo
)

# GUIX has no ready CMake win32 port either; point it at our own.
set(GUIX_CUSTOM_PORT ${CMAKE_CURRENT_LIST_DIR}/port/win32/guix)
add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/repos/guix
    ${CMAKE_BINARY_DIR}/guix
)

# WIN32 (GUI subsystem): GUIX's win32 port supplies its own WinMain, which
# calls gx_main() (our main(), renamed by a macro in gx_port.h).
add_executable(win_rtos_poc WIN32
    app_poc/main.c
)

target_link_libraries(win_rtos_poc
    threadx
    filex
    netxduo
    guix
)

# --- sandboxes: one component at a time, for looking at just its API ------

add_executable(win_rtos_sandbox_netx
    app_sandbox_netx/main.c
)

target_link_libraries(win_rtos_sandbox_netx
    threadx
    netxduo
)

add_executable(win_rtos_sandbox_filex
    app_sandbox_filex/main.c
)

target_link_libraries(win_rtos_sandbox_filex
    threadx
    filex
)

# WIN32 (GUI subsystem): see the win_rtos_poc comment above for why.
add_executable(win_rtos_sandbox_guix WIN32
    app_sandbox_guix/main.c
)

target_link_libraries(win_rtos_sandbox_guix
    threadx
    guix
)
