#include "gx_api.h"

#include "gui_main.h"
#include "s03_main.h"

void s03_create(S03_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    screen->mode = S03_MODE_VIEWER;

    /* S03自体(画面全体を覆うウィンドウ) */
    gx_utility_rectangle_define(&rect, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_create(&screen->window, "s03_window", parent,
                      GX_STYLE_BORDER_NONE, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* モード表示(S02の接続状態表示と同じ位置・スタイル) */
    gx_utility_rectangle_define(&rect, 10, 10, DISPLAY_WIDTH - 10, 30);
    gx_prompt_create(&screen->mode_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_LEFT, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->mode_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    /* 日本語フォント未対応のため仮の英語表示(「モード: ビューア」) */
    gx_prompt_text_set(&screen->mode_prompt, "Mode: Viewer");

    /* ファイル一覧(未実装。実際はFileXから読んだ保存済み画像の一覧をGX_VERTICAL_LISTで表示する想定) */
    gx_utility_rectangle_define(&rect, 10, 35, DISPLAY_WIDTH - 10, 210);
    gx_prompt_create(&screen->file_list_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_THIN | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->file_list_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&screen->file_list_prompt, "(File list - not yet implemented)");

    /* 戻るボタン: S01(メイン画面)へ戻る */
    gx_utility_rectangle_define(&rect, 10, 220, 110, 260);
    gx_text_button_create(&screen->back_button, "back_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S03_BACK_BUTTON, &rect);
    gx_text_button_text_set(&screen->back_button, "Back");

    /* モード変更ボタン: ビューア/コラージュを切り替える */
    gx_utility_rectangle_define(&rect, 120, 220, 250, 260);
    gx_text_button_create(&screen->mode_button, "mode_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S03_MODE_BUTTON, &rect);
    gx_text_button_text_set(&screen->mode_button, "Change Mode");

    /* 次へボタン: 選択中のモードの画面(S04またはS05)へ進む(S04/S05未実装のため未対応) */
    gx_utility_rectangle_define(&rect, 260, 220, DISPLAY_WIDTH - 10, 260);
    gx_text_button_create(&screen->next_button, "next_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S03_NEXT_BUTTON, &rect);
    gx_text_button_text_set(&screen->next_button, "Next");
}
