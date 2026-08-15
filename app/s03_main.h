#ifndef S03_MAIN_H
#define S03_MAIN_H

#include "gx_api.h"

#define ID_S03_BACK_BUTTON 300
#define ID_S03_MODE_BUTTON 301
#define ID_S03_NEXT_BUTTON 302

#define S03_MODE_VIEWER  0
#define S03_MODE_COLLAGE 1

/* S03: ファイル一覧画面(保存済み画像の一覧、ビューア/コラージュのモード選択)
   ウィジェット一式を1つの構造体にまとめ、s03_create()を「コンストラクタ」として扱う。
   現時点では静的レイアウトのみ。ファイル一覧の取得(FileX)とS04/S05への遷移は未実装
   (S04/S05自体が未実装のため)。モード切り替えの表示更新のみ実装する。 */
typedef struct
{
    GX_WINDOW      window;
    GX_PROMPT      mode_prompt;      /* 現在のモード(ビューア/コラージュ)を表示 */
    GX_PROMPT      file_list_prompt; /* ファイル一覧(未実装のため枠と仮テキストのみ) */
    GX_TEXT_BUTTON back_button;      /* S01(メイン画面)へ戻る */
    GX_TEXT_BUTTON mode_button;      /* ビューア/コラージュのモードを切り替える */
    GX_TEXT_BUTTON next_button;      /* 選択中のモードの画面(S04またはS05)へ進む(未実装) */
    UINT           mode;             /* S03_MODE_VIEWER または S03_MODE_COLLAGE */
} S03_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void s03_create(S03_SCREEN *screen, GX_WIDGET *parent);

#endif /* S03_MAIN_H */
