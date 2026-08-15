#include "gx_api.h"

#include "gui_main.h"
#include "d04_main.h"

#define D04_LEFT   110
#define D04_TOP    75
#define D04_RIGHT  369
#define D04_BOTTOM 194

void d04_create(D04_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* D04自体(ダイアログ枠) */
    gx_utility_rectangle_define(&rect, D04_LEFT, D04_TOP, D04_RIGHT, D04_BOTTOM);
    gx_window_create(&screen->window, "d04_window", parent,
                      GX_STYLE_BORDER_THIN, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* エラーメッセージ */
    gx_utility_rectangle_define(&rect, D04_LEFT + 10, D04_TOP + 10, D04_RIGHT - 10, D04_TOP + 70);
    gx_prompt_create(&screen->message_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->message_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&screen->message_prompt, "(Error message)");

    /* OKボタン */
    gx_utility_rectangle_define(&rect, D04_LEFT + 90, D04_BOTTOM - 40, D04_RIGHT - 90, D04_BOTTOM - 10);
    gx_text_button_create(&screen->ok_button, "ok_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D04_OK_BUTTON, &rect);
    gx_text_button_text_set(&screen->ok_button, "OK");
}
