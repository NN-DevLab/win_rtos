#ifndef D05_MAIN_H
#define D05_MAIN_H

#include "gx_api.h"

#define ID_D05_OVERWRITE_BUTTON 1101
#define ID_D05_SAVE_AS_BUTTON   1102
#define ID_D05_CANCEL_BUTTON    1103

/* D05: 保存方法選択ダイアログ(S04の保存ボタンから開く想定)
   現時点では静的レイアウトのみ。S04の保存ボタンは編集機能が無いため常に
   無効になっており、このダイアログを開くトリガーはまだ無い
   (作成・非表示化のみ)。 */
typedef struct
{
    GX_WINDOW      window;
    GX_PROMPT      instruction_prompt; /* 「保存方法を選んでください」 */
    GX_TEXT_BUTTON overwrite_button;   /* 上書き保存(未実装) */
    GX_TEXT_BUTTON save_as_button;     /* D03(名前入力ダイアログ)を開く(未実装) */
    GX_TEXT_BUTTON cancel_button;      /* 何もせずダイアログを閉じる */
} D05_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void d05_create(D05_SCREEN *screen, GX_WIDGET *parent);

#endif /* D05_MAIN_H */
