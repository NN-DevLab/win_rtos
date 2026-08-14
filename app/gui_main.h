#ifndef GX_MAIN_H
#define GX_MAIN_H

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 270

/* 独自の色(GUIXが予約する29個の標準色の直後、GX_FIRST_USER_COLOR以降)を、
   全画面から共通の名前で参照できるようにする */
#define GX_COLOR_ID_BLACK GX_FIRST_USER_COLOR
#define GX_COLOR_ID_WHITE (GX_FIRST_USER_COLOR + 1)

#define GX_SYSTEM_DEFAULT_COLORS_DECLARE \
    GX_COLOR_BLACK, \
    0x00787c78, \
    0x00e2e2e2, \
    0x009b9b73, \
    0x007599aa, \
    GX_COLOR_BLACK, \
    GX_COLOR_WHITE, \
    GX_COLOR_BLUE, \
    GX_COLOR_DARKGRAY, \
    0x00dadada, \
    0x00e0c060, \
    0x00f8f8e0, \
    0x00f8ecb0, \
    GX_COLOR_BLACK, \
    0x00e8d8f8, \
    0x00e8ccb0, \
    GX_COLOR_BLACK, \
    GX_COLOR_WHITE, \
    GX_COLOR_BLACK, \
    GX_COLOR_LIGHTGRAY, \
    GX_COLOR_WHITE, \
    GX_COLOR_BLACK, \
    GX_COLOR_DARKGRAY, \
    GX_COLOR_LIGHTGRAY, \
    0x00e0c060, \
    GX_COLOR_DARKGRAY, \
    GX_COLOR_LIGHTGRAY, \
    GX_COLOR_BLACK, \
    GX_COLOR_LIGHTGRAY


void launch_gui_application();
void start_guix();

#endif /* GX_MAIN_H */