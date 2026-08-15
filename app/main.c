#include "tx_api.h"
#include "fx_api.h"
#include "gx_api.h"
#include <stdio.h>

#include "gui_main.h"
#include "data_thread.h"
#include "filex_media.h"

#define MAIN_THREAD_STACK_SIZE 4096

/* メインスレッド */
TX_THREAD main_thread;
UCHAR main_thread_stack[MAIN_THREAD_STACK_SIZE];


/**
 * @brief ライブラリを初期化する
 */
void init_libraries()
{
    fx_system_initialize();
    start_guix();
}

/**
 * @brief メインスレッドのエントリ関数
 *
 * ThreadX起動後に実行されるメインスレッドです。
 * 定期的にメッセージを出力します。
 *
 * @param arg スレッド生成時に指定した引数(未使用)
 */
void main_thread_entry(ULONG arg)
{
    TX_THREAD *self;

    (void)arg;

    self = tx_thread_identify();
    printf("main_thread is %p\n", (void *)self);

    init_libraries();

    launch_filex_media();

    launch_data_manage_thread();

    launch_gui_application();

    while (1)
    {
        printf("main_thread running\n");
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

/**
 * @brief ThreadXアプリケーションの初期化処理
 *
 * ThreadXカーネル起動時に呼び出されます。
 * 必要なスレッドや同期オブジェクトを生成します。
 *
 * @param first_unused_memory 未使用メモリの先頭アドレス(未使用)
 */
void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;

    /* メインスレッド 起動 */
    tx_thread_create(&main_thread, "main_thread", main_thread_entry, 0,
                      main_thread_stack, sizeof(main_thread_stack),
                      21, 21, TX_NO_TIME_SLICE, TX_AUTO_START);
}

/**
 * @brief アプリケーションのエントリポイント
 *
 * ThreadXカーネルを起動します。
 *
 * @return プロセスの終了コード
 */
int main(int argc, char **argv)
{
    FILE *console_stream;

    freopen_s(&console_stream, "win_rtos.log", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);

    tx_kernel_enter();

    return 0;
}
