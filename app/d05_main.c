#include "gx_api.h"

#include "gui_main.h"
#include "d05_main.h"

#define D05_LEFT   90
#define D05_TOP    60
#define D05_RIGHT  389
#define D05_BOTTOM 210

void d05_create(D05_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* D05自体(ダイアログ枠) */
    gx_utility_rectangle_define(&rect, D05_LEFT, D05_TOP, D05_RIGHT, D05_BOTTOM);
    gx_window_create(&screen->window, "d05_window", parent,
                      GX_STYLE_BORDER_THIN, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 案内テキスト */
    gx_utility_rectangle_define(&rect, D05_LEFT + 20, D05_TOP + 10, D05_RIGHT - 20, D05_TOP + 30);
    gx_prompt_create(&screen->instruction_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_LEFT, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->instruction_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    /* 日本語フォント未対応のため仮の英語表示(「保存方法を選んでください」) */
    gx_prompt_text_set(&screen->instruction_prompt, "Choose how to save");

    /* 上書き保存ボタン(未実装) */
    gx_utility_rectangle_define(&rect, D05_LEFT + 20, D05_TOP + 40, D05_LEFT + 140, D05_TOP + 90);
    gx_text_button_create(&screen->overwrite_button, "overwrite_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D05_OVERWRITE_BUTTON, &rect);
    gx_text_button_text_set(&screen->overwrite_button, "Overwrite");

    /* 別名で保存ボタン: D03(名前入力ダイアログ)を開く(未実装) */
    gx_utility_rectangle_define(&rect, D05_LEFT + 160, D05_TOP + 40, D05_RIGHT - 20, D05_TOP + 90);
    gx_text_button_create(&screen->save_as_button, "save_as_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D05_SAVE_AS_BUTTON, &rect);
    gx_text_button_text_set(&screen->save_as_button, "Save As");

    /* キャンセルボタン: 何もせずダイアログを閉じる */
    gx_utility_rectangle_define(&rect, D05_LEFT + 110, D05_BOTTOM - 40, D05_LEFT + 190, D05_BOTTOM - 10);
    gx_text_button_create(&screen->cancel_button, "cancel_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D05_CANCEL_BUTTON, &rect);
    gx_text_button_text_set(&screen->cancel_button, "Cancel");
}
