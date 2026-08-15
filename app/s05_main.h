#ifndef S05_MAIN_H
#define S05_MAIN_H

#include "gx_api.h"

#define ID_S05_BACK_BUTTON 500
#define ID_S05_EDIT_BUTTON 501
#define ID_S05_SAVE_BUTTON 502

/* S05: コラージュ画面(選択した2枚の画像を横並びで表示・編集・合成保存)
   ウィジェット一式を1つの構造体にまとめ、s05_create()を「コンストラクタ」として扱う。
   現時点では静的レイアウトのみ。画像デコード表示、D01(編集ツールバー)、
   D03(名前入力ダイアログ)は未実装。 */
typedef struct
{
    GX_WINDOW      window;
    GX_PROMPT      image1_prompt; /* 画像1表示エリア(未実装のため枠と仮テキストのみ) */
    GX_PROMPT      image2_prompt; /* 画像2表示エリア(未実装のため枠と仮テキストのみ) */
    GX_TEXT_BUTTON back_button;   /* S03(ファイル一覧画面)へ戻る */
    GX_TEXT_BUTTON edit_button;   /* D01(編集ツールバー)を開く(未実装) */
    GX_TEXT_BUTTON save_button;   /* D03(名前入力ダイアログ)を開く(未実装) */
} S05_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void s05_create(S05_SCREEN *screen, GX_WIDGET *parent);

#endif /* S05_MAIN_H */
