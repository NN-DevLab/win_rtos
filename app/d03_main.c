#include "gx_api.h"

#include "gui_main.h"
#include "d03_main.h"

#define D03_LEFT   90
#define D03_TOP    55
#define D03_RIGHT  389
#define D03_BOTTOM 214

void d03_create(D03_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* D03自体(ダイアログ枠) */
    gx_utility_rectangle_define(&rect, D03_LEFT, D03_TOP, D03_RIGHT, D03_BOTTOM);
    gx_window_create(&screen->window, "d03_window", parent,
                      GX_STYLE_BORDER_THIN, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 入力案内テキスト */
    gx_utility_rectangle_define(&rect, D03_LEFT + 20, D03_TOP + 10, D03_RIGHT - 20, D03_TOP + 30);
    gx_prompt_create(&screen->instruction_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_LEFT, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->instruction_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    /* 日本語フォント未対応のため仮の英語表示(「保存する名前を入力してください」) */
    gx_prompt_text_set(&screen->instruction_prompt, "Enter a name to save");

    /* 名前入力欄 */
    screen->name_input_buffer[0] = 0;
    gx_utility_rectangle_define(&rect, D03_LEFT + 20, D03_TOP + 35, D03_RIGHT - 20, D03_TOP + 65);
    gx_single_line_text_input_create(&screen->name_input, "name_input", &screen->window,
                                      screen->name_input_buffer, D03_NAME_INPUT_BUFFER_SIZE,
                                      GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, GX_ID_NONE, &rect);

    /* キャンセルボタン: 何もせずダイアログを閉じる */
    gx_utility_rectangle_define(&rect, D03_LEFT + 60, D03_BOTTOM - 45, D03_LEFT + 140, D03_BOTTOM - 15);
    gx_text_button_create(&screen->cancel_button, "cancel_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D03_CANCEL_BUTTON, &rect);
    gx_text_button_text_set(&screen->cancel_button, "Cancel");

    /* 保存ボタン: 入力チェック・実際の保存は未実装(押すとダイアログを閉じるだけ) */
    gx_utility_rectangle_define(&rect, D03_LEFT + 160, D03_BOTTOM - 45, D03_LEFT + 240, D03_BOTTOM - 15);
    gx_text_button_create(&screen->save_button, "save_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D03_SAVE_BUTTON, &rect);
    gx_text_button_text_set(&screen->save_button, "Save");
}
