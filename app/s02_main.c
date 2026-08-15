#include "gx_api.h"

#include "gui_main.h"
#include "s02_main.h"

void s02_create(S02_SCREEN *screen, GX_WIDGET *parent)
{
    GX_RECTANGLE rect;

    /* S02自体(画面全体を覆うウィンドウ) */
    gx_utility_rectangle_define(&rect, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_create(&screen->window, "s02_window", parent,
                      GX_STYLE_BORDER_NONE, GX_ID_NONE, &rect);
    gx_widget_fill_color_set(&screen->window, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 接続状態表示 */
    gx_utility_rectangle_define(&rect, 10, 10, DISPLAY_WIDTH - 10, 30);
    gx_prompt_create(&screen->status_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_LEFT, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->status_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    /* 日本語フォント未対応のため仮の英語表示(「接続状態: 未接続」) */
    gx_prompt_text_set(&screen->status_prompt, "Status: Not connected");

    /* フォルダ一覧(未実装。実際はFTPで取得した内容をGX_VERTICAL_LISTで表示する想定) */
    gx_utility_rectangle_define(&rect, 10, 35, DISPLAY_WIDTH - 10, 210);
    gx_prompt_create(&screen->folder_list_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_THIN | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->folder_list_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&screen->folder_list_prompt, "(Folder list - not yet implemented)");

    /* 戻るボタン: S01(メイン画面)へ戻る */
    gx_utility_rectangle_define(&rect, 10, 220, 80, 260);
    gx_text_button_create(&screen->back_button, "back_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S02_BACK_BUTTON, &rect);
    gx_text_button_text_set(&screen->back_button, "Back");

    /* 接続ボタン: D06(接続先入力ダイアログ)を開く(未実装) */
    gx_utility_rectangle_define(&rect, 90, 220, 190, 260);
    gx_text_button_create(&screen->connect_button, "connect_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S02_CONNECT_BUTTON, &rect);
    gx_text_button_text_set(&screen->connect_button, "Connect");

    /* ファイル情報表示(未実装。実際は選択中ファイルの詳細を表示する想定) */
    gx_utility_rectangle_define(&rect, 200, 220, 330, 260);
    gx_prompt_create(&screen->file_info_prompt, NULL, &screen->window, GX_ID_NONE,
                      GX_STYLE_BORDER_THIN | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &rect);
    gx_prompt_text_color_set(&screen->file_info_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&screen->file_info_prompt, "(No file selected)");

    /* 取得開始ボタン: 選択したファイルのダウンロードを開始する(未実装) */
    gx_utility_rectangle_define(&rect, 340, 220, DISPLAY_WIDTH - 10, 260);
    gx_text_button_create(&screen->acquire_button, "acquire_button", &screen->window,
                           GX_ID_NONE, GX_STYLE_ENABLED, ID_S02_ACQUIRE_BUTTON, &rect);
    gx_text_button_text_set(&screen->acquire_button, "Start Acquire");
}
