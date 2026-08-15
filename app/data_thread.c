#include "tx_api.h"
#include "fx_api.h"
#include <stdio.h>

#include "data_thread.h"
#include "filex_media.h"

#define DATA_MANAGE_THREAD_STACK_SIZE 4096

TX_THREAD data_manage_thread;
TX_QUEUE  data_manage_thread_queue;
data_manage_shared_t data_manage_shared;

static UCHAR data_manage_thread_stack[DATA_MANAGE_THREAD_STACK_SIZE];
static ULONG data_manage_thread_queue_memory[DATA_MANAGE_THREAD_QUEUE_DEPTH];

static void handle_read_file(void)
{
    FX_FILE file;
    UINT    status;
    ULONG   actual = 0;

    /*
     * ファイルの読み込みが終わるまでmutexを持ちっぱなしにする。GUIX側は
     * ポーリング(GX_EVENT_TIMER)でcompleteを見に来るだけなので、読み込み中
     * ここでmutexが取れずに一瞬待たされても実害はない。
     */
    tx_mutex_get(&data_manage_shared.mutex, TX_WAIT_FOREVER);

    status = fx_file_open(&disk_media, &file, data_manage_shared.path, FX_OPEN_FOR_READ);
    if (status == FX_SUCCESS)
    {
        status = fx_file_read(&file, data_manage_shared.buffer,
                               sizeof(data_manage_shared.buffer), &actual);
        fx_file_close(&file);
    }

    if (status != FX_SUCCESS)
    {
        printf("data_manage_thread: read_file failed: 0x%x\n", status);
        actual = 0;
    }

    data_manage_shared.status = status;
    data_manage_shared.size = actual;
    data_manage_shared.complete = 1;

    tx_mutex_put(&data_manage_shared.mutex);
}

static void data_manage_thread_entry(ULONG arg)
{
    DATA_MANAGE_MESSAGE message;

    (void)arg;

    printf("data_manage_thread started\n");

    while (1)
    {
        tx_queue_receive(&data_manage_thread_queue, &message, TX_WAIT_FOREVER);

        switch (message.command)
        {
        case DATA_MANAGE_COMMAND_READ_FILE:
            handle_read_file();
            break;

        default:
            break;
        }
    }
}

void launch_data_manage_thread(void)
{
    tx_mutex_create(&data_manage_shared.mutex, "data_manage_shared_mutex", TX_NO_INHERIT);

    tx_queue_create(&data_manage_thread_queue, "data_manage_thread_queue", TX_1_ULONG,
                     data_manage_thread_queue_memory, sizeof(data_manage_thread_queue_memory));

    tx_thread_create(&data_manage_thread, "data_manage_thread", data_manage_thread_entry, 0,
                      data_manage_thread_stack, sizeof(data_manage_thread_stack),
                      21, 21, TX_NO_TIME_SLICE, TX_AUTO_START);
}
