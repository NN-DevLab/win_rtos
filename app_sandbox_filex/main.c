/* win_rtos_sandbox_filex - minimal FileX API sandbox.

   Just ThreadX + FileX, nothing else (no NetX Duo, no GUIX). Uses
   FileX's own stock RAM driver (not our persistence driver from
   win_rtos_poc) to keep this focused purely on the FileX API itself:
   format -> open -> create -> write -> read -> directory listing.
*/

#include "tx_api.h"
#include "fx_api.h"
#include <stdio.h>

#define DEMO_STACK_SIZE 4096
#define TOTAL_SECTORS 256
#define SECTOR_SIZE   512

TX_THREAD thread_filex;

FX_MEDIA      ram_disk;
FX_FILE       test_file;
CHAR         *ram_disk_memory;
unsigned char media_memory[512];

VOID _fx_ram_driver(FX_MEDIA *media_ptr);

void thread_filex_entry(ULONG thread_input)
{
UINT  status;
ULONG actual;
CHAR  read_buffer[64];

    (void)thread_input;

    status = fx_media_format(&ram_disk, _fx_ram_driver, ram_disk_memory, media_memory, sizeof(media_memory),
                              "SANDBOX_DISK", 1, 32, 0, TOTAL_SECTORS, SECTOR_SIZE, 8, 1, 1);
    printf("fx_media_format -> 0x%x\n", status);

    status = fx_media_open(&ram_disk, "SANDBOX RAM DISK", _fx_ram_driver, ram_disk_memory, media_memory, sizeof(media_memory));
    printf("fx_media_open -> 0x%x\n", status);

    status = fx_file_create(&ram_disk, "HELLO.TXT");
    printf("fx_file_create -> 0x%x\n", status);

    status = fx_file_open(&ram_disk, &test_file, "HELLO.TXT", FX_OPEN_FOR_WRITE);
    printf("fx_file_open(write) -> 0x%x\n", status);

    status = fx_file_write(&test_file, "Hello from FileX sandbox!\n", 27);
    printf("fx_file_write -> 0x%x\n", status);

    status = fx_file_close(&test_file);
    printf("fx_file_close -> 0x%x\n", status);

    status = fx_file_open(&ram_disk, &test_file, "HELLO.TXT", FX_OPEN_FOR_READ);
    printf("fx_file_open(read) -> 0x%x\n", status);

    status = fx_file_seek(&test_file, 0);
    printf("fx_file_seek -> 0x%x\n", status);

    status = fx_file_read(&test_file, read_buffer, sizeof(read_buffer) - 1, &actual);
    read_buffer[actual] = 0;
    printf("fx_file_read -> 0x%x, %lu bytes: %s", status, actual, read_buffer);

    fx_file_close(&test_file);

    {
    CHAR  name[FX_MAX_LONG_NAME_LEN];
    UINT  attributes;
    ULONG size;
    UINT  year, month, day, hour, minute, second;

        printf("\ndirectory listing:\n");
        status = fx_directory_first_full_entry_find(&ram_disk, name, &attributes, &size,
                                                      &year, &month, &day, &hour, &minute, &second);
        while (status == FX_SUCCESS)
        {
            printf("  %-16s %lu bytes\n", name, size);
            status = fx_directory_next_full_entry_find(&ram_disk, name, &attributes, &size,
                                                         &year, &month, &day, &hour, &minute, &second);
        }
    }
}

int main(void)
{
    printf("=== win_rtos_sandbox_filex ===\n");
    printf("FileX only (RAM disk): format, create, write, read, list directory.\n\n");

    tx_kernel_enter();
    return 0;
}

void tx_application_define(void *first_unused_memory)
{
CHAR *pointer = (CHAR *)first_unused_memory;

    tx_thread_create(&thread_filex, "filex", thread_filex_entry, 0,
                      pointer, DEMO_STACK_SIZE, 5, 5, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer += DEMO_STACK_SIZE;

    fx_system_initialize();

    ram_disk_memory = pointer;
    pointer += TOTAL_SECTORS * SECTOR_SIZE;
}
