/* FileX driver that backs the "disk" onto a real Windows file, instead of
   the RAM buffer that FileX's stock fx_ram_driver.c uses. This makes the
   media survive process restarts.

   Modeled directly on fx_ram_driver.c (see repos/filex/common/src/
   fx_ram_driver.c for the full FX_DRIVER_* command reference); every
   command below does the same thing that driver does, just against a
   FILE* instead of a memory pointer.

   fx_media_driver_info must point to a NUL-terminated path naming the
   backing file (passed as the driver_info argument to fx_media_open /
   fx_media_format). Only one media using this driver may be open at a
   time, since the open FILE* is kept in a single static variable. */

#include "fx_api.h"
#include <stdio.h>
#include <string.h>

static FILE *fx_win32_file_driver_fp = NULL;

VOID fx_win32_file_driver(FX_MEDIA *media_ptr);

static UINT fx_win32_file_driver_seek(FX_MEDIA *media_ptr, ULONG logical_sector)
{
long offset;

    offset = (long)((logical_sector + media_ptr->fx_media_hidden_sectors) *
                     media_ptr->fx_media_bytes_per_sector);

    if (fseek(fx_win32_file_driver_fp, offset, SEEK_SET) != 0)
    {
        return FX_IO_ERROR;
    }
    return FX_SUCCESS;
}

VOID fx_win32_file_driver(FX_MEDIA *media_ptr)
{
    switch (media_ptr->fx_media_driver_request)
    {

    case FX_DRIVER_READ:
    {
    ULONG  total_bytes;
    size_t n;

        total_bytes = media_ptr->fx_media_driver_sectors * media_ptr->fx_media_bytes_per_sector;

        if (fx_win32_file_driver_seek(media_ptr, media_ptr->fx_media_driver_logical_sector) != FX_SUCCESS)
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }

        n = fread(media_ptr->fx_media_driver_buffer, 1, total_bytes, fx_win32_file_driver_fp);
        if (n < total_bytes)
        {
            /* The backing file grows lazily as sectors are first written,
               so reading a sector that has never been written yet (but is
               within the formatted media) hits real EOF early. Treat
               anything not physically on disk as zero, like a real block
               device's unused area would read. */
            memset((UCHAR *)media_ptr->fx_media_driver_buffer + n, 0, total_bytes - n);
            clearerr(fx_win32_file_driver_fp);
        }

        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_WRITE:
    {
    ULONG  total_bytes;
    UINT   seek_status;
    size_t written;

        total_bytes = media_ptr->fx_media_driver_sectors * media_ptr->fx_media_bytes_per_sector;
        seek_status = fx_win32_file_driver_seek(media_ptr, media_ptr->fx_media_driver_logical_sector);
        written = seek_status == FX_SUCCESS ? fwrite(media_ptr->fx_media_driver_buffer, 1, total_bytes, fx_win32_file_driver_fp) : 0;

        if ((seek_status != FX_SUCCESS) || (written != total_bytes))
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }

        fflush(fx_win32_file_driver_fp);
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_FLUSH:
    {
        if (fx_win32_file_driver_fp)
        {
            fflush(fx_win32_file_driver_fp);
        }
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_ABORT:
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_INIT:
    {
    const char *path = (const char *)media_ptr->fx_media_driver_info;

        /* Open the backing file if it already exists; otherwise create it.
           A freshly created file has no valid boot record yet, so the
           subsequent FX_DRIVER_BOOT_READ below will correctly report
           FX_MEDIA_INVALID until fx_media_format() has written one. */
        fx_win32_file_driver_fp = fopen(path, "r+b");
        if (fx_win32_file_driver_fp == NULL)
        {
            fx_win32_file_driver_fp = fopen(path, "w+b");
        }

        if (fx_win32_file_driver_fp == NULL)
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }

        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_UNINIT:
    {
        if (fx_win32_file_driver_fp)
        {
            fclose(fx_win32_file_driver_fp);
            fx_win32_file_driver_fp = NULL;
        }
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_BOOT_READ:
    {
    UCHAR   header[32];
    UINT    bytes_per_sector;

        if ((fx_win32_file_driver_seek(media_ptr, 0) != FX_SUCCESS) ||
            (fread(header, 1, sizeof(header), fx_win32_file_driver_fp) != sizeof(header)))
        {
            /* Can't even read a header's worth of bytes - either the file
               is brand new (empty) or too short. Not formatted yet.  */
            media_ptr->fx_media_driver_status = FX_MEDIA_INVALID;
            break;
        }

        if ((header[0] != (UCHAR)0xEB) ||
            ((header[1] != (UCHAR)0x34) && (header[1] != (UCHAR)0x76)) ||
            (header[2] != (UCHAR)0x90))
        {
            media_ptr->fx_media_driver_status = FX_MEDIA_INVALID;
            break;
        }

        bytes_per_sector = _fx_utility_16_unsigned_read(&header[FX_BYTES_SECTOR]);

        if (bytes_per_sector > media_ptr->fx_media_memory_size)
        {
            media_ptr->fx_media_driver_status = FX_BUFFER_ERROR;
            break;
        }

        if ((fx_win32_file_driver_seek(media_ptr, 0) != FX_SUCCESS) ||
            (fread(media_ptr->fx_media_driver_buffer, 1, bytes_per_sector, fx_win32_file_driver_fp) != bytes_per_sector))
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }

        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    case FX_DRIVER_BOOT_WRITE:
    {
        if ((fx_win32_file_driver_seek(media_ptr, 0) != FX_SUCCESS) ||
            (fwrite(media_ptr->fx_media_driver_buffer, 1, media_ptr->fx_media_bytes_per_sector, fx_win32_file_driver_fp) !=
             media_ptr->fx_media_bytes_per_sector))
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }

        fflush(fx_win32_file_driver_fp);
        media_ptr->fx_media_driver_status = FX_SUCCESS;
        break;
    }

    default:
    {
        printf("[fx_win32_file_driver] unhandled request=%u\n", media_ptr->fx_media_driver_request);
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        break;
    }
    }
}
