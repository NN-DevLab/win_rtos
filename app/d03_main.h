#ifndef D03_MAIN_H
#define D03_MAIN_H

#include "gx_api.h"

#define D03_NAME_INPUT_BUFFER_SIZE 40

#define ID_D03_CANCEL_BUTTON 901
#define ID_D03_SAVE_BUTTON   902

/* D03: 名前入力ダイアログ(S05の保存ボタン、S04のD05「別名で保存」から開く想定)
   現時点では静的レイアウトのみ。入力チェック・実際の保存処理・D04(エラー
   パネル)への遷移は未実装(保存ボタンは押すとダイアログを閉じるだけ)。 */
typedef struct
{
    GX_WINDOW                 window;
    GX_PROMPT                 instruction_prompt; /* 「保存する名前を入力してください」 */
    GX_SINGLE_LINE_TEXT_INPUT name_input;
    CHAR                      name_input_buffer[D03_NAME_INPUT_BUFFER_SIZE];
    GX_TEXT_BUTTON            cancel_button;
    GX_TEXT_BUTTON            save_button;
} D03_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。 */
void d03_create(D03_SCREEN *screen, GX_WIDGET *parent);

#endif /* D03_MAIN_H */
