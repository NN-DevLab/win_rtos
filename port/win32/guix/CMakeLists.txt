# GUIX ships no CMake win32 port either (only a Visual Studio .sln/.vcxproj
# under ports/win32/build/vs_2019). This custom port compiles the plain
# 24xrgb GDI display driver directly from the cloned guix repo.

target_sources(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../../repos/guix/ports/win32/src/gx_win32_display_driver.c
    ${CMAKE_CURRENT_LIST_DIR}/../../../repos/guix/ports/win32/src/gx_win32_display_driver_24xrgb.c
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../../repos/guix/ports/win32/inc
)

# The driver sources are guarded by #ifdef WIN32, which MSVC does not
# predefine on its own (unlike _WIN32).
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    WIN32
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    user32
    gdi32
    winmm
)
