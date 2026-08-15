#ifndef D06_MAIN_H
#define D06_MAIN_H

#include "gx_api.h"

#define D06_IP_INPUT_BUFFER_SIZE 40

#define ID_D06_CANCEL_BUTTON  601
#define ID_D06_CONNECT_BUTTON 602

/* D06: 接続先入力ダイアログ(S02の「接続」ボタンから開く想定)
   現時点では静的レイアウトのみ。開閉の遷移処理・入力値を使った接続処理は未実装。 */
typedef struct
{
    GX_WINDOW                 window;
    GX_PROMPT                 instruction_prompt; /* 「接続先のIPアドレスを入力してください」 */
    GX_SINGLE_LINE_TEXT_INPUT ip_input;
    CHAR                      ip_input_buffer[D06_IP_INPUT_BUFFER_SIZE];
    GX_TEXT_BUTTON            cancel_button;
    GX_TEXT_BUTTON            connect_button;
} D06_SCREEN;

/* screenを初期化し、parentの子としてウィジェット一式を作成する。
   まだボタンを押しても何も起きない(開閉・接続処理は別途実装)。 */
void d06_create(D06_SCREEN *screen, GX_WIDGET *parent);

#endif /* D06_MAIN_H */
