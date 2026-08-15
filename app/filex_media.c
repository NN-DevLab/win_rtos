#include "fx_api.h"
#include <stdio.h>

#include "filex_media.h"

#define TOTAL_SECTORS 256
#define SECTOR_SIZE   512
#define DISK_FILE_NAME "win_rtos_disk.dat"

#define SEED_IMAGE_BUFFER_SIZE 65536

FX_MEDIA disk_media;

static UCHAR media_memory[512];

VOID fx_win32_file_driver(FX_MEDIA *media_ptr);

/*
 * ディスクにテスト用PNGがまだ無ければ、ソースツリーのapp/data/test.pngを
 * 標準Cのfopen/freadで読み込み、fx_file_writeでディスクへ書き込む。
 * S02のダウンロード機能ができるまでの、暫定的なテストデータ投入処理。
 */
static void seed_test_image(void)
{
    FX_FILE      file;
    UINT         status;
    FILE        *source;
    static UCHAR buffer[SEED_IMAGE_BUFFER_SIZE];
    size_t       read_size;

    /* 既に書き込み済みなら何もしない。 */
    status = fx_file_open(&disk_media, &file, FILEX_TEST_IMAGE_NAME, FX_OPEN_FOR_READ);
    if (status == FX_SUCCESS)
    {
        fx_file_close(&file);
        return;
    }

    source = fopen(PROJECT_SOURCE_DIR "/app/data/test.png", "rb");
    if (!source)
    {
        printf("[FileX] seed image not found at " PROJECT_SOURCE_DIR "/app/data/test.png\n");
        return;
    }

    read_size = fread(buffer, 1, sizeof(buffer), source);
    fclose(source);

    /*
     * 開発中に強制終了(taskkill)を繰り返すと、ディレクトリエントリだけ残って
     * 中身が不整合な状態になることがある。fx_file_createが失敗する可能性を
     * 考慮し、先に(存在しなくても害のない)fx_file_deleteで掃除してから作る。
     */
    fx_file_delete(&disk_media, FILEX_TEST_IMAGE_NAME);

    status = fx_file_create(&disk_media, FILEX_TEST_IMAGE_NAME);
    if (status != FX_SUCCESS)
    {
        printf("[FileX] fx_file_create failed: 0x%x\n", status);
        return;
    }

    status = fx_file_open(&disk_media, &file, FILEX_TEST_IMAGE_NAME, FX_OPEN_FOR_WRITE);
    if (status != FX_SUCCESS)
    {
        printf("[FileX] fx_file_open (write) failed: 0x%x\n", status);
        return;
    }

    status = fx_file_write(&file, buffer, (ULONG)read_size);
    fx_file_close(&file);

    if (status != FX_SUCCESS)
    {
        printf("[FileX] fx_file_write failed: 0x%x\n", status);
        return;
    }

    printf("[FileX] seeded %s (%lu bytes)\n", FILEX_TEST_IMAGE_NAME, (ULONG)read_size);
}

void launch_filex_media(void)
{
    UINT status;

    /* 前回起動時のディスクファイルが残っていればそのまま開き、
       無ければフォーマットしてから開く(app_poc/main.cと同じパターン)。 */
    status = fx_media_open(&disk_media, "WIN_RTOS_DISK", fx_win32_file_driver,
                            (VOID *)DISK_FILE_NAME, media_memory, sizeof(media_memory));

    if (status != FX_SUCCESS)
    {
        printf("[FileX] no existing disk found, formatting %s\n", DISK_FILE_NAME);

        status = fx_media_format(&disk_media, fx_win32_file_driver, (VOID *)DISK_FILE_NAME,
                                  media_memory, sizeof(media_memory),
                                  "WIN_RTOS_DISK", 1, 32, 0, TOTAL_SECTORS, SECTOR_SIZE, 8, 1, 1);
        if (status != FX_SUCCESS)
        {
            printf("[FileX] fx_media_format failed: 0x%x\n", status);
            return;
        }

        status = fx_media_open(&disk_media, "WIN_RTOS_DISK", fx_win32_file_driver,
                                (VOID *)DISK_FILE_NAME, media_memory, sizeof(media_memory));
        if (status != FX_SUCCESS)
        {
            printf("[FileX] fx_media_open failed after format: 0x%x\n", status);
            return;
        }
    }

    seed_test_image();
}
