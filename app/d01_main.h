#ifndef D01_MAIN_H
#define D01_MAIN_H

#include "gx_api.h"

#define ID_D01_MOVE_BUTTON 701

/* D01: 編集ツールバー(S04/S05から開くミニダイアログ)
   現時点では静的レイアウトのみ。「移動」ボタンは有効だが、ドラッグで画像を
   動かす実処理(ヒットテスト等)は未実装。落書き/フレーム挿入/拡大縮小/回転/
   テキスト追加は将来実装のため箱のみ配置しグレーアウト(無効)にしておく。 */
typedef struct
{
    GX_WINDOW      window;
    GX_TEXT_BUTTON move_button;    /* 有効。画像の移動モードに入る(未実装) */
    GX_TEXT_BUTTON doodle_button;  /* 無効(将来実装) */
    GX_TEXT_BUTTON frame_button;   /* 無効(将来実装) */
    GX_TEXT_BUTTON scale_button;   /* 無効(将来実装) */
    GX_TEXT_BUTTON rotate_button;  /* 無効(将来実装) */
    GX_TEXT_BUTTON text_button;    /* 無効(将来実装) */
} D01_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void d01_create(D01_SCREEN *screen, GX_WIDGET *parent);

#endif /* D01_MAIN_H */
