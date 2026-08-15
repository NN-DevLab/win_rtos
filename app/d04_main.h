#ifndef D04_MAIN_H
#define D04_MAIN_H

#include "gx_api.h"

#define ID_D04_OK_BUTTON 1001

/* D04: エラーパネル(D03の入力チェックに引っかかった時に自動表示する想定)
   現時点では静的レイアウトのみ。D03の入力チェック自体が未実装のため、
   このダイアログを開くトリガーはまだ無い(作成・非表示化のみ)。 */
typedef struct
{
    GX_WINDOW      window;
    GX_PROMPT      message_prompt; /* エラー内容(未実装のため仮テキストのみ) */
    GX_TEXT_BUTTON ok_button;      /* パネルを閉じてD03に戻る */
} D04_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void d04_create(D04_SCREEN *screen, GX_WIDGET *parent);

#endif /* D04_MAIN_H */
