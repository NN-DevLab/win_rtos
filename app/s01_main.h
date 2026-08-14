#ifndef S01_MAIN_H
#define S01_MAIN_H

#include "gx_api.h"

/* S01: メイン画面(アプリ起動時に表示される最初の画面)
   ウィジェット一式を1つの構造体にまとめ、s01_create()を「コンストラクタ」として扱う。 */
typedef struct
{
    GX_WINDOW               window;
    GX_MULTI_LINE_TEXT_VIEW description_view;
    GX_TEXT_BUTTON          acquire_button;   /* 画像取得ボタン: S02へ遷移する */
    GX_TEXT_BUTTON          viewer_button;    /* ビューアボタン: S03へ遷移する */
} S01_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。
   まだボタンを押しても何も起きない(画面遷移は別途実装)。 */
void s01_create(S01_SCREEN *screen, GX_WIDGET *parent);

#endif /* S01_MAIN_H */
