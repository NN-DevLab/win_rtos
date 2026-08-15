#include "gx_api.h"

#include "gui_main.h"
#include "d06_main.h"

/* ダイアログの表示範囲(絶対座標)。GUIXのウィジェット矩形は親からの相対位置ではなく、
   キャンバス上の絶対座標で指定する(gx_widget_create.cでsizeがそのままgx_widget_sizeに
   代入されるだけで、親の原点によるオフセットは行われないため)。 */
#define D06_LEFT   90
#define D06_TOP    55
#define D06_RIGHT  389
#define D06_BOTTOM 214

void d06_create(D06_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* D06自体(ダイアログ枠) */
    gx_utility_rectangle_define(&rect, D06_LEFT, D06_TOP, D06_RIGHT, D06_BOTTOM);
    gx_window_create(&screen->window, "d06_window", parent,
                      GX_STYLE_BORDER_THIN, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 入力案内テキスト */
    gx_utility_rectangle_define(&rect, D06_LEFT + 20, D06_TOP + 10, D06_RIGHT - 20, D06_TOP + 30);
    gx_prompt_create(&screen->instruction_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_LEFT, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->instruction_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    /* 日本語フォント未対応のため仮の英語表示(「接続先のIPアドレスを入力してください」) */
    gx_prompt_text_set(&screen->instruction_prompt, "Enter the IP address to connect to");

    /* IPアドレス入力欄 */
    screen->ip_input_buffer[0] = 0;
    gx_utility_rectangle_define(&rect, D06_LEFT + 20, D06_TOP + 35, D06_RIGHT - 20, D06_TOP + 65);
    gx_single_line_text_input_create(&screen->ip_input, "ip_input", &screen->window,
                                      screen->ip_input_buffer, D06_IP_INPUT_BUFFER_SIZE,
                                      GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, GX_ID_NONE, &rect);

    /* キャンセルボタン: 何もせずダイアログを閉じる(未実装) */
    gx_utility_rectangle_define(&rect, D06_LEFT + 60, D06_BOTTOM - 45, D06_LEFT + 140, D06_BOTTOM - 15);
    gx_text_button_create(&screen->cancel_button, "cancel_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D06_CANCEL_BUTTON, &rect);
    gx_text_button_text_set(&screen->cancel_button, "Cancel");

    /* 接続開始ボタン: ダイアログを閉じてS02側で接続処理を開始する(未実装) */
    gx_utility_rectangle_define(&rect, D06_LEFT + 160, D06_BOTTOM - 45, D06_LEFT + 240, D06_BOTTOM - 15);
    gx_text_button_create(&screen->connect_button, "connect_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D06_CONNECT_BUTTON, &rect);
    gx_text_button_text_set(&screen->connect_button, "Connect");
}
