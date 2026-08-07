/* win_rtos_poc - smoke test proving NetX Duo, FileX, and GUIX can all be
   pulled into a win_rtos app together.

   - NetX Duo: two loopback IP instances exchange UDP packets over the
     stock nx_ram_network_driver (no Npcap/real NIC needed for this test).
   - FileX:    the "disk" is backed by a real Windows file via our own
     fx_win32_file_driver (port/win32/filex/fx_win32_file_driver.c), not
     the stock RAM driver, so a run counter written to it survives
     process restarts - close this exe, run it again, and the count goes
     up instead of resetting to 1.
   - GUIX:     a window opens showing NetX Duo's packet counters and the
     FileX-persisted run counter.
*/

#include "tx_api.h"
#include "nx_api.h"
#include "fx_api.h"
#include "gx_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define DEMO_STACK_SIZE 4096

/* ---------------------------------------------------------------- ThreadX */

TX_THREAD thread_netx_tx;
TX_THREAD thread_netx_rx;
TX_THREAD thread_filex;

void thread_netx_tx_entry(ULONG thread_input);
void thread_netx_rx_entry(ULONG thread_input);
void thread_filex_entry(ULONG thread_input);

/* -------------------------------------------------------------- NetX Duo */

#define PACKET_SIZE 1536
#define POOL_SIZE   ((sizeof(NX_PACKET) + PACKET_SIZE) * 16)

NX_PACKET_POOL pool_0;
NX_IP          ip_0;
NX_IP          ip_1;
NX_UDP_SOCKET  socket_0;
NX_UDP_SOCKET  socket_1;
UCHAR          pool_buffer[POOL_SIZE];

volatile ULONG netx_sent_count = 0;
volatile ULONG netx_received_count = 0;

void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

/* ----------------------------------------------------------------- FileX */

#define TOTAL_SECTORS 256
#define SECTOR_SIZE   512
#define DISK_FILE_NAME "win_rtos_poc_disk.dat"
#define COUNTER_FILE_NAME "RUNCOUNT.TXT"

FX_MEDIA disk_media;
FX_FILE  counter_file;

volatile ULONG persisted_run_count = 0;

VOID fx_win32_file_driver(FX_MEDIA *media_ptr);

/* ------------------------------------------------------------------ GUIX */

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 272

extern UINT win32_graphics_driver_setup_24xrgb(GX_DISPLAY *display);
extern GX_FONT _gx_system_font_8bpp;

GX_DISPLAY     win_rtos_display;
GX_CANVAS      win_rtos_canvas;
GX_WINDOW_ROOT win_rtos_root;
GX_PROMPT      status_prompt;
GX_PROMPT      netx_prompt;
GX_PROMPT      filex_prompt;
GX_COLOR       win_rtos_canvas_memory[DISPLAY_WIDTH * DISPLAY_HEIGHT];

#define ID_REFRESH_TIMER 1

GX_FONT *win_rtos_font_table[] =
{
    &_gx_system_font_8bpp,     /* GX_FONT_ID_DEFAULT / GX_FONT_ID_SYSTEM */
    &_gx_system_font_8bpp,     /* GX_FONT_ID_BUTTON                      */
    &_gx_system_font_8bpp,     /* GX_FONT_ID_PROMPT                      */
    &_gx_system_font_8bpp      /* GX_FONT_ID_TEXT_INPUT                  */
};

#define GX_COLOR_ID_BLACK GX_FIRST_USER_COLOR
#define GX_COLOR_ID_WHITE (GX_FIRST_USER_COLOR + 1)

#define GX_SYSTEM_DEFAULT_COLORS_DECLARE \
    GX_COLOR_BLACK,   /* CANVAS           */ \
    0x00787c78,       /* WIDGET_FILL      */ \
    0x00e2e2e2,       /* WINDOW_FILL      */ \
    0x009b9b73,       /* STANDARD_BORDER  */ \
    0x007599aa,       /* WINDOW_BORDER    */ \
    GX_COLOR_BLACK,   /* NORMAL_TEXT      */ \
    GX_COLOR_WHITE,   /* SELECTED_TEXT    */ \
    GX_COLOR_BLUE,    /* SELECTED_FILL    */ \
    GX_COLOR_DARKGRAY,/* SHADOW           */ \
    0x00dadada,       /* SHINE            */ \
    0x00e0c060,       /* BUTTON_BORDER    */ \
    0x00f8f8e0,       /* BUTTON_UPPER     */ \
    0x00f8ecb0,       /* BUTTON_LOWER     */ \
    GX_COLOR_BLACK,   /* BUTTON_TEXT      */ \
    0x00e8d8f8,       /* SCROLL_FILL      */ \
    0x00e8ccb0,       /* SCROLL_BUTTON    */ \
    GX_COLOR_BLACK,   /* TEXT_INPUT_TEXT  */ \
    GX_COLOR_WHITE,   /* TEXT_INPUT_FILL  */ \
    GX_COLOR_BLACK,   /* SLIDER_TICK      */ \
    GX_COLOR_LIGHTGRAY,/* SLIDER_GROOVE_TOP    */ \
    GX_COLOR_WHITE,   /* SLIDER_GROOVE_BOTTOM */ \
    GX_COLOR_BLACK,   /* SLIDER_NEEDLE_OUTLINE */ \
    GX_COLOR_DARKGRAY,/* SLIDER_NEEDLE_FILL    */ \
    GX_COLOR_LIGHTGRAY,/* SLIDER_NEEDLE_LINE1  */ \
    0x00e0c060        /* SLIDER_NEEDLE_LINE2   */

static GX_COLOR win_rtos_color_table[] =
{
    GX_SYSTEM_DEFAULT_COLORS_DECLARE,
    GX_COLOR_BLACK,
    GX_COLOR_WHITE
};

static UINT root_event_handler(GX_WIDGET *widget, GX_EVENT *event)
{
CHAR buf[80];

    switch (event->gx_event_type)
    {
    case GX_EVENT_SHOW:
        gx_widget_event_process(widget, event);
        gx_system_timer_start(widget, ID_REFRESH_TIMER, 10, 10);
        return GX_SUCCESS;

    case GX_EVENT_TIMER:
        if (event->gx_event_payload.gx_event_timer_id == ID_REFRESH_TIMER)
        {
            sprintf(buf, "NetX Duo: sent %lu / received %lu (loopback UDP)",
                    netx_sent_count, netx_received_count);
            gx_prompt_text_set(&netx_prompt, buf);

            sprintf(buf, "FileX: run count = %lu (persisted in %s)",
                    persisted_run_count, DISK_FILE_NAME);
            gx_prompt_text_set(&filex_prompt, buf);
        }
        return GX_SUCCESS;

    default:
        return gx_window_root_event_process((GX_WINDOW_ROOT *)widget, event);
    }
}

VOID start_guix(VOID)
{
GX_RECTANGLE root_window_size;
GX_RECTANGLE size;

    gx_system_initialize();

    gx_display_create(&win_rtos_display, "win_rtos display", win32_graphics_driver_setup_24xrgb,
                       DISPLAY_WIDTH, DISPLAY_HEIGHT);

    gx_display_color_table_set(&win_rtos_display, win_rtos_color_table,
                                sizeof(win_rtos_color_table) / sizeof(GX_COLOR));

    gx_display_font_table_set(&win_rtos_display, win_rtos_font_table,
                               sizeof(win_rtos_font_table) / sizeof(GX_FONT *));

    gx_canvas_create(&win_rtos_canvas, "win_rtos canvas", &win_rtos_display,
                      GX_CANVAS_MANAGED | GX_CANVAS_VISIBLE,
                      DISPLAY_WIDTH, DISPLAY_HEIGHT,
                      win_rtos_canvas_memory, sizeof(win_rtos_canvas_memory));

    gx_utility_rectangle_define(&root_window_size, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_root_create(&win_rtos_root, "win_rtos root window", &win_rtos_canvas,
                           GX_STYLE_BORDER_NONE, GX_ID_NONE, &root_window_size);
    gx_widget_fill_color_set(&win_rtos_root, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_widget_event_process_set(&win_rtos_root, root_event_handler);

    gx_utility_rectangle_define(&size, 10, 90, DISPLAY_WIDTH - 10, 115);
    gx_prompt_create(&status_prompt, NULL, &win_rtos_root, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &size);
    gx_prompt_text_color_set(&status_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&status_prompt, "win_rtos_poc: NetX Duo + FileX + GUIX");

    gx_utility_rectangle_define(&size, 10, 125, DISPLAY_WIDTH - 10, 150);
    gx_prompt_create(&netx_prompt, NULL, &win_rtos_root, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &size);
    gx_prompt_text_color_set(&netx_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);

    gx_utility_rectangle_define(&size, 10, 160, DISPLAY_WIDTH - 10, 185);
    gx_prompt_create(&filex_prompt, NULL, &win_rtos_root, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &size);
    gx_prompt_text_color_set(&filex_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);

    gx_widget_show(&win_rtos_root);

    gx_system_start();
}

/* --------------------------------------------------------------- ThreadX */

int main(int argc, char **argv)
{
FILE *console_stream;

    (void)argc;
    (void)argv;

    /* GUIX's win32 port runs us as a GUI-subsystem app (WinMain), which has
       no console by default. Log to a file instead of AllocConsole()'ing a
       window, so status messages are easy to inspect after the fact. */
    freopen_s(&console_stream, "win_rtos_poc.log", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);

    tx_kernel_enter();
    return 0;
}

void tx_application_define(void *first_unused_memory)
{
CHAR *pointer = (CHAR *)first_unused_memory;
UINT  status;

    tx_thread_create(&thread_netx_tx, "netx tx", thread_netx_tx_entry, 0,
                      pointer, DEMO_STACK_SIZE, 4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer += DEMO_STACK_SIZE;

    tx_thread_create(&thread_netx_rx, "netx rx", thread_netx_rx_entry, 0,
                      pointer, DEMO_STACK_SIZE, 3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer += DEMO_STACK_SIZE;

    tx_thread_create(&thread_filex, "filex", thread_filex_entry, 0,
                      pointer, DEMO_STACK_SIZE, 5, 5, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer += DEMO_STACK_SIZE;

    /* --- NetX Duo --- */
    nx_system_initialize();

    status = nx_packet_pool_create(&pool_0, "win_rtos_poc packet pool", PACKET_SIZE, pool_buffer, POOL_SIZE);
    if (status != NX_SUCCESS)
    {
        printf("nx_packet_pool_create failed: 0x%x\n", status);
    }

    status = nx_ip_create(&ip_0, "win_rtos_poc IP 0", IP_ADDRESS(1, 2, 3, 4), 0xFFFFF000UL,
                           &pool_0, _nx_ram_network_driver, pointer, 2048, 1);
    pointer += 2048;

    status += nx_ip_create(&ip_1, "win_rtos_poc IP 1", IP_ADDRESS(1, 2, 3, 5), 0xFFFFF000UL,
                            &pool_0, _nx_ram_network_driver, pointer, 2048, 1);
    pointer += 2048;
    if (status != NX_SUCCESS)
    {
        printf("nx_ip_create failed: 0x%x\n", status);
    }

    status = nx_arp_enable(&ip_0, (void *)pointer, 1024);
    pointer += 1024;
    status += nx_arp_enable(&ip_1, (void *)pointer, 1024);
    pointer += 1024;
    if (status != NX_SUCCESS)
    {
        printf("nx_arp_enable failed: 0x%x\n", status);
    }

    status = nx_udp_enable(&ip_0);
    status += nx_udp_enable(&ip_1);
    if (status != NX_SUCCESS)
    {
        printf("nx_udp_enable failed: 0x%x\n", status);
    }

    /* --- FileX --- */
    fx_system_initialize();

    /* --- GUIX --- */
    start_guix();
}

/* ---------------------------------------------------------- NetX threads */

void thread_netx_tx_entry(ULONG thread_input)
{
UINT       status;
NX_PACKET *my_packet;
NXD_ADDRESS ipv4_address;

    (void)thread_input;

    tx_thread_sleep(NX_IP_PERIODIC_RATE);

    ipv4_address.nxd_ip_version = NX_IP_VERSION_V4;
    ipv4_address.nxd_ip_address.v4 = IP_ADDRESS(1, 2, 3, 5);

    status = nx_udp_socket_create(&ip_0, &socket_0, "win_rtos_poc socket 0", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);
    if (status != NX_SUCCESS)
    {
        printf("nx_udp_socket_create (tx) failed: 0x%x\n", status);
        return;
    }

    status = nx_udp_socket_bind(&socket_0, 0x88, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("nx_udp_socket_bind (tx) failed: 0x%x\n", status);
        return;
    }

    nx_udp_socket_checksum_disable(&socket_0);
    nx_arp_dynamic_entry_set(&ip_0, IP_ADDRESS(1, 2, 3, 5), 0, 0);

    while (1)
    {
        status = nx_packet_allocate(&pool_0, &my_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            break;
        }

        nx_packet_data_append(my_packet, "win_rtos_poc NetX Duo ping", 27, &pool_0, TX_WAIT_FOREVER);

        status = nxd_udp_socket_send(&socket_0, my_packet, &ipv4_address, 0x89);
        if (status != NX_SUCCESS)
        {
            printf("nxd_udp_socket_send failed: 0x%x\n", status);
            break;
        }

        netx_sent_count++;
        tx_thread_sleep(50);
    }
}

void thread_netx_rx_entry(ULONG thread_input)
{
UINT       status;
NX_PACKET *my_packet;

    (void)thread_input;

    tx_thread_sleep(NX_IP_PERIODIC_RATE);

    status = nx_udp_socket_create(&ip_1, &socket_1, "win_rtos_poc socket 1", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);
    if (status != NX_SUCCESS)
    {
        printf("nx_udp_socket_create (rx) failed: 0x%x\n", status);
        return;
    }

    status = nx_udp_socket_bind(&socket_1, 0x89, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("nx_udp_socket_bind (rx) failed: 0x%x\n", status);
        return;
    }

    while (1)
    {
        status = nx_udp_socket_receive(&socket_1, &my_packet, TX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            break;
        }

        nx_packet_release(my_packet);
        netx_received_count++;
    }
}

/* --------------------------------------------------------- FileX thread */

void thread_filex_entry(ULONG thread_input)
{
UINT  status;
ULONG actual;
CHAR  local_buffer[32];
static UCHAR media_memory[512];

    (void)thread_input;

    /* Try to open the disk as-is first: if the backing file already
       exists from a previous run, this succeeds without reformatting
       and whatever was written last time is still there. */
    status = fx_media_open(&disk_media, "WIN_RTOS_POC_DISK", fx_win32_file_driver,
                            (VOID *)DISK_FILE_NAME, media_memory, sizeof(media_memory));

    if (status != FX_SUCCESS)
    {
        /* First run (or the file was missing/corrupt): format it fresh.  */
        printf("[FileX] no existing disk found, formatting %s\n", DISK_FILE_NAME);

        status = fx_media_format(&disk_media, fx_win32_file_driver, (VOID *)DISK_FILE_NAME,
                                  media_memory, sizeof(media_memory),
                                  "WIN_RTOS_POC_DISK", 1, 32, 0, TOTAL_SECTORS, SECTOR_SIZE, 8, 1, 1);
        if (status != FX_SUCCESS)
        {
            printf("[FileX] fx_media_format failed: 0x%x\n", status);
            return;
        }

        status = fx_media_open(&disk_media, "WIN_RTOS_POC_DISK", fx_win32_file_driver,
                                (VOID *)DISK_FILE_NAME, media_memory, sizeof(media_memory));
        if (status != FX_SUCCESS)
        {
            printf("[FileX] fx_media_open failed after format: 0x%x\n", status);
            return;
        }
    }
    else
    {
        printf("[FileX] found existing disk %s, opened without reformatting\n", DISK_FILE_NAME);
    }

    /* Read the persisted run count, if any (0 if this is the first run).  */
    status = fx_file_open(&disk_media, &counter_file, COUNTER_FILE_NAME, FX_OPEN_FOR_READ);
    if (status == FX_SUCCESS)
    {
        fx_file_seek(&counter_file, 0);
        status = fx_file_read(&counter_file, local_buffer, sizeof(local_buffer) - 1, &actual);
        if (status == FX_SUCCESS)
        {
            local_buffer[actual] = 0;
            persisted_run_count = (ULONG)atol(local_buffer);
        }
        fx_file_close(&counter_file);
    }

    persisted_run_count++;
    printf("[FileX] this is run number %lu\n", persisted_run_count);

    /* Write the incremented count straight back out - close the app and
       run it again to see this number keep climbing.  */
    fx_file_delete(&disk_media, COUNTER_FILE_NAME);
    status = fx_file_create(&disk_media, COUNTER_FILE_NAME);
    if (status == FX_SUCCESS)
    {
        status = fx_file_open(&disk_media, &counter_file, COUNTER_FILE_NAME, FX_OPEN_FOR_WRITE);
        if (status == FX_SUCCESS)
        {
        INT len = sprintf(local_buffer, "%lu", persisted_run_count);

            fx_file_write(&counter_file, local_buffer, (ULONG)len);
            fx_file_close(&counter_file);
        }
    }

    fx_media_flush(&disk_media);
}
