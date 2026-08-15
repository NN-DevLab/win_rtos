#include "gx_api.h"

#include "gui_main.h"
#include "d01_main.h"

/* ミニダイアログの表示範囲(絶対座標)。D06と同様、ウィジェット矩形は
   キャンバス上の絶対座標で指定する(親の原点によるオフセットはない)。 */
#define D01_LEFT   20
#define D01_TOP    90
#define D01_RIGHT  460
#define D01_BOTTOM 180

void d01_create(D01_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* D01自体(ツールバー枠) */
    gx_utility_rectangle_define(&rect, D01_LEFT, D01_TOP, D01_RIGHT, D01_BOTTOM);
    gx_window_create(&screen->window, "d01_window", parent,
                      GX_STYLE_BORDER_THIN, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 移動: 有効 */
    gx_utility_rectangle_define(&rect, D01_LEFT + 10, D01_TOP + 10, D01_LEFT + 75, D01_BOTTOM - 10);
    gx_text_button_create(&screen->move_button, "move_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_D01_MOVE_BUTTON, &rect);
    gx_text_button_text_set(&screen->move_button, "Move");

    /* 落書き: 無効(将来実装) */
    gx_utility_rectangle_define(&rect, D01_LEFT + 80, D01_TOP + 10, D01_LEFT + 145, D01_BOTTOM - 10);
    gx_text_button_create(&screen->doodle_button, "doodle_button", &screen->window,
                           GX_ID_NONE, 0, GX_ID_NONE, &rect);
    gx_text_button_text_set(&screen->doodle_button, "Doodle");

    /* フレーム挿入: 無効(将来実装) */
    gx_utility_rectangle_define(&rect, D01_LEFT + 150, D01_TOP + 10, D01_LEFT + 215, D01_BOTTOM - 10);
    gx_text_button_create(&screen->frame_button, "frame_button", &screen->window,
                           GX_ID_NONE, 0, GX_ID_NONE, &rect);
    gx_text_button_text_set(&screen->frame_button, "Frame");

    /* 拡大縮小: 無効(将来実装) */
    gx_utility_rectangle_define(&rect, D01_LEFT + 220, D01_TOP + 10, D01_LEFT + 285, D01_BOTTOM - 10);
    gx_text_button_create(&screen->scale_button, "scale_button", &screen->window,
                           GX_ID_NONE, 0, GX_ID_NONE, &rect);
    gx_text_button_text_set(&screen->scale_button, "Scale");

    /* 回転: 無効(将来実装) */
    gx_utility_rectangle_define(&rect, D01_LEFT + 290, D01_TOP + 10, D01_LEFT + 355, D01_BOTTOM - 10);
    gx_text_button_create(&screen->rotate_button, "rotate_button", &screen->window,
                           GX_ID_NONE, 0, GX_ID_NONE, &rect);
    gx_text_button_text_set(&screen->rotate_button, "Rotate");

    /* テキスト追加: 無効(将来実装) */
    gx_utility_rectangle_define(&rect, D01_LEFT + 360, D01_TOP + 10, D01_LEFT + 425, D01_BOTTOM - 10);
    gx_text_button_create(&screen->text_button, "text_button", &screen->window,
                           GX_ID_NONE, 0, GX_ID_NONE, &rect);
    gx_text_button_text_set(&screen->text_button, "Text");
}
