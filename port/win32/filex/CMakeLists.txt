# FileX ships no ready-made CMake win32 port (upstream only provides an
# inc/fx_port.h under ports/win32/vs_2019). This custom port exposes that
# header and adds our own file-backed driver (fx_win32_file_driver.c),
# used instead of the stock RAM driver so the media survives restarts.

target_sources(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fx_win32_file_driver.c
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../../repos/filex/ports/win32/vs_2019/inc
)
