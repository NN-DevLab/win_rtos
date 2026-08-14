#include "gx_api.h"

#include "gui_main.h"
#include "s01_main.h"

void s01_create(S01_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* S01自体(画面全体を覆うウィンドウ) */
    gx_utility_rectangle_define(&rect, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_create(&screen->window, "s01_window", parent,
                      GX_STYLE_BORDER_NONE, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 機能説明(テキスト表示エリア、折り返し表示) */
    gx_utility_rectangle_define(&rect, 10, 10, DISPLAY_WIDTH - 10, 170);
    gx_multi_line_text_view_create(&screen->description_view, NULL, &screen->window,
                                    GX_ID_NONE, GX_STYLE_BORDER_NONE, GX_ID_NONE, &rect);
    gx_multi_line_text_view_text_color_set(&screen->description_view, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    /* 日本語フォント未対応(_gx_system_font_8bppはASCIIのみ)のため仮の英語表示 */
    gx_multi_line_text_view_text_set(&screen->description_view,
                                      "Get images from devices on the LAN, or view / merge saved images.");

    /* 画像取得ボタン: S02へ遷移する(遷移処理は未実装) */
    gx_utility_rectangle_define(&rect, 10, 185, 235, 245);
    gx_text_button_create(&screen->acquire_button, "acquire_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, GX_ID_NONE, &rect);
    /* 日本語フォント未対応のため仮の英語表示(「画像取得」) */
    gx_text_button_text_set(&screen->acquire_button, "Acquire");

    /* ビューアボタン: S03へ遷移する(遷移処理は未実装) */
    gx_utility_rectangle_define(&rect, 245, 185, DISPLAY_WIDTH - 10, 245);
    gx_text_button_create(&screen->viewer_button, "viewer_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, GX_ID_NONE, &rect);
    /* 日本語フォント未対応のため仮の英語表示(「ビューア」) */
    gx_text_button_text_set(&screen->viewer_button, "Viewer");
}
