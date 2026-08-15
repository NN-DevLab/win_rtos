#ifndef S02_MAIN_H
#define S02_MAIN_H

#include "gx_api.h"

#define ID_S02_BACK_BUTTON     200
#define ID_S02_CONNECT_BUTTON 201
#define ID_S02_ACQUIRE_BUTTON 202

/* S02: 画像取得画面(接続・フォルダ一覧・ファイル取得)
   ウィジェット一式を1つの構造体にまとめ、s02_create()を「コンストラクタ」として扱う。
   現時点では静的レイアウトのみ。FTP接続・フォルダ一覧取得・ダウンロードは未実装。 */
typedef struct
{
    GX_WINDOW      window;
    GX_PROMPT      status_prompt;      /* 接続状態(未接続/接続中.../接続済み/接続失敗) */
    GX_PROMPT      folder_list_prompt; /* フォルダ一覧(未実装のため枠と仮テキストのみ) */
    GX_TEXT_BUTTON back_button;        /* S01(メイン画面)へ戻る */
    GX_TEXT_BUTTON connect_button;     /* D06(接続先入力ダイアログ)を開く */
    GX_PROMPT      file_info_prompt;   /* 選択中ファイルの情報(未実装のため枠と仮テキストのみ) */
    GX_TEXT_BUTTON acquire_button;     /* 選択したファイルの取得を開始する */
} S02_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。
   まだボタンを押しても何も起きない(接続・遷移処理は別途実装)。 */
void s02_create(S02_SCREEN *screen, GX_WIDGET *parent);

#endif /* S02_MAIN_H */
