#ifndef S04_MAIN_H
#define S04_MAIN_H

#include "gx_api.h"

#define ID_S04_BACK_BUTTON 400
#define ID_S04_EDIT_BUTTON 401
#define ID_S04_SAVE_BUTTON 402

/* S04: ビューア画面(選択した1枚の画像を表示・編集)
   ウィジェット一式を1つの構造体にまとめ、s04_create()を「コンストラクタ」として扱う。
   現時点では静的レイアウトのみ。画像デコード表示、D01(編集ツールバー)、
   D05(保存方法選択ダイアログ)は未実装。保存ボタンは編集内容がない前提で
   常に無効(グレーアウト)にしておく。 */
typedef struct
{
    GX_WINDOW      window;
    GX_PROMPT      image_prompt; /* 画像表示エリア(未実装のため枠と仮テキストのみ) */
    GX_TEXT_BUTTON back_button;  /* S03(ファイル一覧画面)へ戻る */
    GX_TEXT_BUTTON edit_button;  /* D01(編集ツールバー)を開く(未実装) */
    GX_TEXT_BUTTON save_button;  /* D05(保存方法選択ダイアログ)を開く(編集内容がある時のみ有効、未実装のため常に無効) */
} S04_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void s04_create(S04_SCREEN *screen, GX_WIDGET *parent);

#endif /* S04_MAIN_H */
