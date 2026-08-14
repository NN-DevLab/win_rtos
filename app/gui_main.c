#include "tx_api.h"
#include "gx_api.h"
#include <stdio.h>
#include <windows.h>

#include "gui_main.h"
#include "s01_main.h"

extern UINT win32_graphics_driver_setup_24xrgb(GX_DISPLAY *display);
extern GX_FONT _gx_system_font_8bpp;

GX_DISPLAY     main_disp;
GX_CANVAS      main_canvas;
GX_COLOR       main_canvas_memory[DISPLAY_WIDTH * DISPLAY_HEIGHT];
GX_WINDOW_ROOT root;
S01_SCREEN     s01;

static GX_COLOR color_table[] =
{
    GX_SYSTEM_DEFAULT_COLORS_DECLARE,
    GX_COLOR_BLACK,
    GX_COLOR_WHITE
};

GX_FONT *font_table[] =
{
    &_gx_system_font_8bpp,
    &_gx_system_font_8bpp,
    &_gx_system_font_8bpp,
    &_gx_system_font_8bpp
};

void launch_gui_application()
{
    GX_RECTANGLE root_size;

    /* メイン画面を作成（windows32ドライバの場合はここで画面そのものが作られる） */
    gx_display_create(&main_disp, "main_disp", win32_graphics_driver_setup_24xrgb,
                       DISPLAY_WIDTH, DISPLAY_HEIGHT);

    /* ディスプレイ設定 */
    gx_display_color_table_set(&main_disp, color_table, sizeof(color_table) / sizeof(GX_COLOR));
    gx_display_font_table_set(&main_disp, font_table, sizeof(font_table) / sizeof(GX_FONT *));

    /* キャンバスを作成 */
    gx_canvas_create(&main_canvas, "main_canvas", &main_disp,
                      GX_CANVAS_MANAGED | GX_CANVAS_VISIBLE,
                      DISPLAY_WIDTH, DISPLAY_HEIGHT, main_canvas_memory, sizeof(main_canvas_memory));

    /* ルートを設定 */
    gx_utility_rectangle_define(&root_size, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_root_create(&root, "root", &main_canvas,
                           GX_STYLE_BORDER_NONE, GX_ID_NONE, &root_size);
    gx_widget_fill_color_set(&root, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* S01(メイン画面)をrootの子として作成 */
    s01_create(&s01, (GX_WIDGET *)&root);

    /* ルートを表示 */
    gx_widget_show(&root);

    /* システムを開始 */
    gx_system_start();
}

void start_guix()
{
    gx_system_initialize();
}