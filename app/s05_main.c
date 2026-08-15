#include "gx_api.h"

#include "gui_main.h"
#include "s05_main.h"

void s05_create(S05_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* S05自体(画面全体を覆うウィンドウ) */
    gx_utility_rectangle_define(&rect, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_create(&screen->window, "s05_window", parent,
                      GX_STYLE_BORDER_NONE, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 画像1表示エリア(未実装。実際は画像1枚につき1つのウィジェットとして表示する想定) */
    gx_utility_rectangle_define(&rect, 10, 10, 235, 210);
    gx_prompt_create(&screen->image1_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_THIN | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->image1_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&screen->image1_prompt, "(Image 1)");

    /* 画像2表示エリア(未実装) */
    gx_utility_rectangle_define(&rect, 245, 10, DISPLAY_WIDTH - 10, 210);
    gx_prompt_create(&screen->image2_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_THIN | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->image2_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&screen->image2_prompt, "(Image 2)");

    /* 戻るボタン: S03(ファイル一覧画面)へ戻る */
    gx_utility_rectangle_define(&rect, 10, 220, 150, 260);
    gx_text_button_create(&screen->back_button, "back_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S05_BACK_BUTTON, &rect);
    gx_text_button_text_set(&screen->back_button, "Back");

    /* 編集ボタン: D01(編集ツールバー)を開く(未実装) */
    gx_utility_rectangle_define(&rect, 170, 220, 310, 260);
    gx_text_button_create(&screen->edit_button, "edit_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S05_EDIT_BUTTON, &rect);
    gx_text_button_text_set(&screen->edit_button, "Edit");

    /* 保存ボタン: D03(名前入力ダイアログ)を開く(未実装)。S04と違い常に有効 */
    gx_utility_rectangle_define(&rect, 330, 220, DISPLAY_WIDTH - 10, 260);
    gx_text_button_create(&screen->save_button, "save_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S05_SAVE_BUTTON, &rect);
    gx_text_button_text_set(&screen->save_button, "Save");
}
