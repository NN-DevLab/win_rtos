#ifndef DATA_THREAD_H
#define DATA_THREAD_H

#include "tx_api.h"

#define DATA_MANAGE_THREAD_QUEUE_DEPTH 10
#define DATA_MANAGE_PATH_MAX 128
#define DATA_MANAGE_RESULT_BUFFER_SIZE (256 * 1024)

/* データ管理スレッドへの指示コマンド。今後、書き込み等を追加する。 */
typedef enum
{
    DATA_MANAGE_COMMAND_NONE = 0,
    DATA_MANAGE_COMMAND_READ_FILE
} DATA_MANAGE_COMMAND;

/* 
 * キューでやり取りするメッセージ。message_sizeをTX_1_ULONGにできるよう、
 * 今はcommandだけの1ワード構成にしておく(パラメータが増えたら見直す)。
 */
typedef struct
{
    DATA_MANAGE_COMMAND command;
} DATA_MANAGE_MESSAGE;

extern TX_THREAD data_manage_thread;
extern TX_QUEUE   data_manage_thread_queue;

/*
 * データ管理スレッドとGUIXスレッドの間で共有するデータ。mutexで保護すること
 * (どちらのスレッドから触る場合もtx_mutex_get/putで挟む)。
 *
 * リクエスト: GUIX側がpathに読みたいファイルパスを書き、completeを0にして
 * からキューにコマンドを送る(completeを0に戻すのはGUIX側の責任)。
 * 結果: データ管理スレッドがbuffer/size/statusを書いてからcompleteを1にする。
 * GUIX側はGX_EVENT_TIMERでcompleteをポーリングし、1になったら
 * buffer[0..size)を読み出す(読み出し終わったら次のリクエストのために
 * completeを0に戻す)。
 */
typedef struct
{
    TX_MUTEX mutex;
    CHAR     path[DATA_MANAGE_PATH_MAX];
    UINT     complete;
    UINT     status;   /* FX_SUCCESSなら成功、それ以外はFileXのエラーコード */
    ULONG    size;      /* bufferのうち実際に読み込めたバイト数 */
    UCHAR    buffer[DATA_MANAGE_RESULT_BUFFER_SIZE];
} data_manage_shared_t;

extern data_manage_shared_t data_manage_shared;

/* 
 * データ管理スレッドを作成・起動する(main_thread_entryから呼ぶ想定、
 * launch_gui_application()と同じ位置づけ)。
 */
void launch_data_manage_thread(void);

#endif /* DATA_THREAD_H */
