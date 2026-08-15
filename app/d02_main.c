#include "gx_api.h"

#include "gui_main.h"
#include "d02_main.h"

#define D02_LEFT   110
#define D02_TOP    75
#define D02_RIGHT  369
#define D02_BOTTOM 194

void d02_create(D02_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* D02自体(ダイアログ枠) */
    gx_utility_rectangle_define(&rect, D02_LEFT, D02_TOP, D02_RIGHT, D02_BOTTOM);
    gx_window_create(&screen->window, "d02_window", parent,
                      GX_STYLE_BORDER_THIN, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 結果メッセージ */
    gx_utility_rectangle_define(&rect, D02_LEFT + 10, D02_TOP + 10, D02_RIGHT - 10, D02_TOP + 70);
    gx_prompt_create(&screen->message_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->message_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&screen->message_prompt, "(Acquisition result message)");

    /* OKボタン */
    gx_utility_rectangle_define(&rect, D02_LEFT + 90, D02_BOTTOM - 40, D02_RIGHT - 90, D02_BOTTOM - 10);
    gx_text_button_create(&screen->ok_button, "ok_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D02_OK_BUTTON, &rect);
    gx_text_button_text_set(&screen->ok_button, "OK");
}
