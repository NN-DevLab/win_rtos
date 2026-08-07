/* win_rtos_sandbox_guix - minimal GUIX API sandbox.

   Just ThreadX + GUIX, nothing else (no NetX Duo, no FileX). A window
   with a single prompt that a timer updates, showing the basic GUIX
   API flow on its own: system_initialize -> display_create ->
   canvas_create -> window_root_create -> widget_show -> system_start,
   plus a GX_EVENT_TIMER handler.
*/

#include "tx_api.h"
#include "gx_api.h"
#include <stdio.h>
#include <windows.h>

#define DISPLAY_WIDTH  400
#define DISPLAY_HEIGHT 200
#define ID_TIMER 1

extern UINT win32_graphics_driver_setup_24xrgb(GX_DISPLAY *display);
extern GX_FONT _gx_system_font_8bpp;

GX_DISPLAY     sandbox_display;
GX_CANVAS      sandbox_canvas;
GX_WINDOW_ROOT sandbox_root;
GX_PROMPT      sandbox_prompt;
GX_COLOR       sandbox_canvas_memory[DISPLAY_WIDTH * DISPLAY_HEIGHT];

GX_FONT *sandbox_font_table[] =
{
    &_gx_system_font_8bpp, &_gx_system_font_8bpp, &_gx_system_font_8bpp, &_gx_system_font_8bpp
};

#define GX_COLOR_ID_BLACK GX_FIRST_USER_COLOR
#define GX_COLOR_ID_WHITE (GX_FIRST_USER_COLOR + 1)

#define GX_SYSTEM_DEFAULT_COLORS_DECLARE \
    GX_COLOR_BLACK, 0x00787c78, 0x00e2e2e2, 0x009b9b73, 0x007599aa, \
    GX_COLOR_BLACK, GX_COLOR_WHITE, GX_COLOR_BLUE, GX_COLOR_DARKGRAY, 0x00dadada, \
    0x00e0c060, 0x00f8f8e0, 0x00f8ecb0, GX_COLOR_BLACK, 0x00e8d8f8, 0x00e8ccb0, \
    GX_COLOR_BLACK, GX_COLOR_WHITE, GX_COLOR_BLACK, GX_COLOR_LIGHTGRAY, GX_COLOR_WHITE, \
    GX_COLOR_BLACK, GX_COLOR_DARKGRAY, GX_COLOR_LIGHTGRAY, 0x00e0c060

static GX_COLOR sandbox_color_table[] =
{
    GX_SYSTEM_DEFAULT_COLORS_DECLARE,
    GX_COLOR_BLACK,
    GX_COLOR_WHITE
};

static ULONG tick_count = 0;

static UINT root_event_handler(GX_WIDGET *widget, GX_EVENT *event)
{
CHAR buf[64];

    switch (event->gx_event_type)
    {
    case GX_EVENT_SHOW:
        gx_widget_event_process(widget, event);
        gx_system_timer_start(widget, ID_TIMER, 10, 10);
        return GX_SUCCESS;

    case GX_EVENT_TIMER:
        if (event->gx_event_payload.gx_event_timer_id == ID_TIMER)
        {
            tick_count++;
            sprintf(buf, "GX_EVENT_TIMER fired %lu times", tick_count);
            gx_prompt_text_set(&sandbox_prompt, buf);
        }
        return GX_SUCCESS;

    default:
        return gx_window_root_event_process((GX_WINDOW_ROOT *)widget, event);
    }
}

int main(int argc, char **argv)
{
FILE *log_stream;

    (void)argc;
    (void)argv;

    /* GUIX's win32 port runs us as a GUI-subsystem app (WinMain), which
       has no console by default. Log to a file instead. */
    freopen_s(&log_stream, "win_rtos_sandbox_guix.log", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("=== win_rtos_sandbox_guix ===\n");
    printf("GUIX only: a window with a timer-driven prompt.\n\n");

    tx_kernel_enter();
    return 0;
}

void tx_application_define(void *first_unused_memory)
{
GX_RECTANGLE root_size;
GX_RECTANGLE prompt_size;

    (void)first_unused_memory;

    gx_system_initialize();

    gx_display_create(&sandbox_display, "sandbox display", win32_graphics_driver_setup_24xrgb,
                       DISPLAY_WIDTH, DISPLAY_HEIGHT);
    gx_display_color_table_set(&sandbox_display, sandbox_color_table, sizeof(sandbox_color_table) / sizeof(GX_COLOR));
    gx_display_font_table_set(&sandbox_display, sandbox_font_table, sizeof(sandbox_font_table) / sizeof(GX_FONT *));

    gx_canvas_create(&sandbox_canvas, "sandbox canvas", &sandbox_display,
                      GX_CANVAS_MANAGED | GX_CANVAS_VISIBLE,
                      DISPLAY_WIDTH, DISPLAY_HEIGHT, sandbox_canvas_memory, sizeof(sandbox_canvas_memory));

    gx_utility_rectangle_define(&root_size, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_root_create(&sandbox_root, "sandbox root", &sandbox_canvas,
                           GX_STYLE_BORDER_NONE, GX_ID_NONE, &root_size);
    gx_widget_fill_color_set(&sandbox_root, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_widget_event_process_set(&sandbox_root, root_event_handler);

    gx_utility_rectangle_define(&prompt_size, 10, 90, DISPLAY_WIDTH - 10, 115);
    gx_prompt_create(&sandbox_prompt, NULL, &sandbox_root, GX_ID_NONE,
                      GX_STYLE_BORDER_NONE | GX_STYLE_TEXT_CENTER, GX_ID_NONE, &prompt_size);
    gx_prompt_text_color_set(&sandbox_prompt, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_set(&sandbox_prompt, "GX_EVENT_TIMER fired 0 times");

    gx_widget_show(&sandbox_root);

    gx_system_start();
}
