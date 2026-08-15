#ifndef D02_MAIN_H
#define D02_MAIN_H

#include "gx_api.h"

#define ID_D02_OK_BUTTON 801

/* D02: 取得結果パネル(S02の画像取得完了時に自動表示する想定)
   現時点では静的レイアウトのみ。S02のダウンロード処理自体が未実装のため、
   このダイアログを開くトリガーはまだ無い(作成・非表示化のみ)。 */
typedef struct
{
    GX_WINDOW      window;
    GX_PROMPT      message_prompt; /* 成功/失敗メッセージ(未実装のため仮テキストのみ) */
    GX_TEXT_BUTTON ok_button;      /* パネルを閉じる */
} D02_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void d02_create(D02_SCREEN *screen, GX_WIDGET *parent);

#endif /* D02_MAIN_H */
