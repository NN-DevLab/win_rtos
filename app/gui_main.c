#include "tx_api.h"
#include "fx_api.h"
#include "gx_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "gui_main.h"
#include "data_thread.h"
#include "filex_media.h"
#include "s01_main.h"
#include "s02_main.h"
#include "s03_main.h"
#include "s04_main.h"
#include "s05_main.h"
#include "d01_main.h"
#include "d02_main.h"
#include "d03_main.h"
#include "d04_main.h"
#include "d05_main.h"
#include "d06_main.h"

/* S03→S04で読み込み完了をポーリングするタイマーのID */
#define ID_S03_IMAGE_LOAD_TIMER 303

extern UINT win32_graphics_driver_setup_24xrgb(GX_DISPLAY *display);
extern GX_FONT _gx_system_font_8bpp;

GX_DISPLAY     main_disp;
GX_CANVAS      main_canvas;
GX_COLOR       main_canvas_memory[DISPLAY_WIDTH * DISPLAY_HEIGHT];
GX_WINDOW_ROOT root;
S01_SCREEN     s01;
S02_SCREEN     s02;
S03_SCREEN     s03;
S04_SCREEN     s04;
S05_SCREEN     s05;
D01_SCREEN     d01;
D02_SCREEN     d02;
D03_SCREEN     d03;
D04_SCREEN     d04;
D05_SCREEN     d05;
D06_SCREEN     d06;

static GX_COLOR color_table[] =
{
    GX_SYSTEM_DEFAULT_COLORS_DECLARE,
    GX_COLOR_BLACK,
    GX_COLOR_WHITE
};

GX_FONT *font_table[] =
{
    &_gx_system_font_8bpp,
    &_gx_system_font_8bpp,
    &_gx_system_font_8bpp,
    &_gx_system_font_8bpp
};

/* 画面スタック用のメモリ(push/popできる深さは要素数/2) */
static GX_WIDGET *screen_stack_memory[8];

/* gx_image_reader_startが内部でピクセルバッファを確保するのに使うアロケータ。
   GUIXはデフォルトでは何も登録しないため、未登録のままデコードすると
   GX_SYSTEM_MEMORY_ERRORで失敗する。 */
static VOID *gx_heap_allocate(ULONG size)
{
    return malloc((size_t)size);
}

static VOID gx_heap_release(VOID *memory)
{
    free(memory);
}

/* S03→S04で表示する、ランタイムデコード済みの画像。gx_pixelmap_dataが
   NULLの間はまだ何も読み込んでいない(static変数なのでゼロ初期化される)。 */
static GX_PIXELMAP s04_pixelmap;

/* S04の画像表示エリアの描画関数。デコード済みなら中央でなく左上基準で描画する
   (demo_guix_industrialのmain_screen_drawと同じパターン)。 */
static VOID s04_image_window_draw(GX_WIDGET *widget)
{
GX_WINDOW *window = (GX_WINDOW *)widget;

    gx_window_background_draw(window);

    if (s04_pixelmap.gx_pixelmap_data)
    {
        gx_canvas_pixelmap_draw(window->gx_widget_size.gx_rectangle_left,
                                 window->gx_widget_size.gx_rectangle_top, &s04_pixelmap);
    }

    gx_widget_children_draw(window);
}

/* データ管理スレッドにファイル読み込みを依頼する。パスを共有データに書き、
   completeを0にリセットしてからキューでコマンドを送る。 */
static void request_read_file(GX_CONST CHAR *path)
{
DATA_MANAGE_MESSAGE message;

    tx_mutex_get(&data_manage_shared.mutex, TX_WAIT_FOREVER);
    strncpy(data_manage_shared.path, path, DATA_MANAGE_PATH_MAX - 1);
    data_manage_shared.path[DATA_MANAGE_PATH_MAX - 1] = 0;
    data_manage_shared.complete = 0;
    tx_mutex_put(&data_manage_shared.mutex);

    message.command = DATA_MANAGE_COMMAND_READ_FILE;
    tx_queue_send(&data_manage_thread_queue, &message, TX_WAIT_FOREVER);
}

/* ID_S03_IMAGE_LOAD_TIMERから呼ばれる。読み込みが完了していなければ何もしない。
   完了していれば、成功時のみPNGデコードしてからタイマーを止め、S04へ遷移する。 */
static void check_read_file_complete(void)
{
UINT complete;
UINT status = FX_SUCCESS;
ULONG size = 0;

    tx_mutex_get(&data_manage_shared.mutex, TX_WAIT_FOREVER);
    complete = data_manage_shared.complete;
    if (complete)
    {
        status = data_manage_shared.status;
        size = data_manage_shared.size;
        if (status == FX_SUCCESS)
        {
            GX_IMAGE_READER reader;
            UINT decode_status;

            gx_image_reader_create(&reader, data_manage_shared.buffer, (INT)size,
                                    GX_COLOR_FORMAT_24XRGB, 0);
            decode_status = gx_image_reader_start(&reader, &s04_pixelmap);
            if (decode_status != GX_SUCCESS)
            {
                printf("gui: gx_image_reader_start failed: 0x%x\n", decode_status);
            }
        }
        data_manage_shared.complete = 0;
    }
    tx_mutex_put(&data_manage_shared.mutex);

    if (!complete)
    {
        return;
    }

    if (status != FX_SUCCESS)
    {
        printf("gui: read_file failed, status=0x%x\n", status);
    }

    gx_system_timer_stop((GX_WIDGET *)&s03.window, ID_S03_IMAGE_LOAD_TIMER);

    gx_system_screen_stack_push((GX_WIDGET *)&s03.window);
    gx_widget_attach(&root, &s04.window);
    gx_widget_show((GX_WIDGET *)&s04.window);
}

/* S01: 画像取得ボタンでS02へ、ビューアボタンでS03へ進む */
static UINT s01_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_S01_ACQUIRE_BUTTON, GX_EVENT_CLICKED):
        gx_system_screen_stack_push((GX_WIDGET *)&s01.window);
        gx_widget_attach(&root, &s02.window);
        gx_widget_show((GX_WIDGET *)&s02.window);
        return GX_SUCCESS;

    case GX_SIGNAL(ID_S01_VIEWER_BUTTON, GX_EVENT_CLICKED):
        gx_system_screen_stack_push((GX_WIDGET *)&s01.window);
        gx_widget_attach(&root, &s03.window);
        gx_widget_show((GX_WIDGET *)&s03.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* S02: 戻るボタンでS01へ戻る。接続ボタンでD06(接続先入力ダイアログ)を開く */
static UINT s02_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_S02_BACK_BUTTON, GX_EVENT_CLICKED):
        gx_widget_detach(&s02.window);
        gx_system_screen_stack_pop();
        gx_widget_show((GX_WIDGET *)&s01.window);
        return GX_SUCCESS;

    case GX_SIGNAL(ID_S02_CONNECT_BUTTON, GX_EVENT_CLICKED):
        /* S02はS01からのattachでroot直下の最前面に移動済みなので、D06も
           最前面に移動してから表示する(でないとS02の下に隠れる) */
        gx_widget_front_move((GX_WIDGET *)&d06.window, GX_NULL);
        gx_widget_show((GX_WIDGET *)&d06.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* S03: 戻るボタンでS01へ戻る。モード変更ボタンでビューア/コラージュを切り替える。
   次へボタンはモードに応じてS04またはS05へ進む */
static UINT s03_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_S03_BACK_BUTTON, GX_EVENT_CLICKED):
        gx_widget_detach(&s03.window);
        gx_system_screen_stack_pop();
        gx_widget_show((GX_WIDGET *)&s01.window);
        return GX_SUCCESS;

    case GX_SIGNAL(ID_S03_MODE_BUTTON, GX_EVENT_CLICKED):
        if (s03.mode == S03_MODE_VIEWER)
        {
            s03.mode = S03_MODE_COLLAGE;
            gx_prompt_text_set(&s03.mode_prompt, "Mode: Collage");
        }
        else
        {
            s03.mode = S03_MODE_VIEWER;
            gx_prompt_text_set(&s03.mode_prompt, "Mode: Viewer");
        }
        return GX_SUCCESS;

    case GX_SIGNAL(ID_S03_NEXT_BUTTON, GX_EVENT_CLICKED):
        if (s03.mode == S03_MODE_VIEWER)
        {
            /* ファイル一覧が未実装のため、選択ファイルの代わりに固定のテスト
               画像を読み込む(TODO: 一覧で選択したファイル名に差し替える)。
               読み込み完了はID_S03_IMAGE_LOAD_TIMERのポーリングで検知し、
               完了後にcheck_read_file_completeがS04へ遷移する。 */
            request_read_file(FILEX_TEST_IMAGE_NAME);
            gx_system_timer_start((GX_WIDGET *)&s03.window, ID_S03_IMAGE_LOAD_TIMER, 5, 5);
        }
        else
        {
            gx_system_screen_stack_push((GX_WIDGET *)&s03.window);
            gx_widget_attach(&root, &s05.window);
            gx_widget_show((GX_WIDGET *)&s05.window);
        }
        return GX_SUCCESS;

    case GX_EVENT_TIMER:
        if (event_ptr->gx_event_payload.gx_event_timer_id == ID_S03_IMAGE_LOAD_TIMER)
        {
            check_read_file_complete();
        }
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* S04: 戻るボタンでS03へ戻る。編集ボタンでD01(編集ツールバー)を開く。
   保存ボタンはそもそも無効状態なので押せない(D05未実装) */
static UINT s04_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_S04_BACK_BUTTON, GX_EVENT_CLICKED):
        gx_widget_detach(&s04.window);
        gx_system_screen_stack_pop();
        gx_widget_show((GX_WIDGET *)&s03.window);
        return GX_SUCCESS;

    case GX_SIGNAL(ID_S04_EDIT_BUTTON, GX_EVENT_CLICKED):
        /* S04はS03からのattachでroot直下の最前面に移動済みなので、D01も
           最前面に移動してから表示する(でないとS04の下に隠れて見えない) */
        gx_widget_front_move((GX_WIDGET *)&d01.window, GX_NULL);
        gx_widget_show((GX_WIDGET *)&d01.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* S05: 戻るボタンでS03へ戻る。編集ボタンでD01(編集ツールバー)を開く。
   保存ボタンでD03(名前入力ダイアログ)を開く */
static UINT s05_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_S05_BACK_BUTTON, GX_EVENT_CLICKED):
        gx_widget_detach(&s05.window);
        gx_system_screen_stack_pop();
        gx_widget_show((GX_WIDGET *)&s03.window);
        return GX_SUCCESS;

    case GX_SIGNAL(ID_S05_EDIT_BUTTON, GX_EVENT_CLICKED):
        gx_widget_front_move((GX_WIDGET *)&d01.window, GX_NULL);
        gx_widget_show((GX_WIDGET *)&d01.window);
        return GX_SUCCESS;

    case GX_SIGNAL(ID_S05_SAVE_BUTTON, GX_EVENT_CLICKED):
        gx_widget_front_move((GX_WIDGET *)&d03.window, GX_NULL);
        gx_widget_show((GX_WIDGET *)&d03.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* D02: OKボタンでパネルを閉じる。取得完了処理(S02)が未実装のため、
   今のところ開くトリガーは無い */
static UINT d02_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_D02_OK_BUTTON, GX_EVENT_CLICKED):
        gx_widget_hide((GX_WIDGET *)&d02.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* D03: キャンセル/保存のどちらでもダイアログを閉じる。
   入力チェック・実際の保存・D04(エラーパネル)への遷移は未実装 */
static UINT d03_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_D03_CANCEL_BUTTON, GX_EVENT_CLICKED):
    case GX_SIGNAL(ID_D03_SAVE_BUTTON, GX_EVENT_CLICKED):
        gx_widget_hide((GX_WIDGET *)&d03.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* D04: OKボタンでパネルを閉じる。D03の入力チェックが未実装のため、
   今のところ開くトリガーは無い */
static UINT d04_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_D04_OK_BUTTON, GX_EVENT_CLICKED):
        gx_widget_hide((GX_WIDGET *)&d04.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* D05: 上書き保存・キャンセルはダイアログを閉じるだけ(上書き保存の実処理は未実装)。
   別名で保存はD03(名前入力ダイアログ)を開く。S04の保存ボタンが常に無効のため、
   今のところ開くトリガーは無い */
static UINT d05_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_D05_OVERWRITE_BUTTON, GX_EVENT_CLICKED):
    case GX_SIGNAL(ID_D05_CANCEL_BUTTON, GX_EVENT_CLICKED):
        gx_widget_hide((GX_WIDGET *)&d05.window);
        return GX_SUCCESS;

    case GX_SIGNAL(ID_D05_SAVE_AS_BUTTON, GX_EVENT_CLICKED):
        gx_widget_hide((GX_WIDGET *)&d05.window);
        gx_widget_front_move((GX_WIDGET *)&d03.window, GX_NULL);
        gx_widget_show((GX_WIDGET *)&d03.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

/* D06: キャンセル/接続開始のどちらでもダイアログを閉じてS02に戻る。
   接続開始の実処理(FTP接続)は未実装 */
static UINT d06_window_event_process(GX_WIDGET *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(ID_D06_CANCEL_BUTTON, GX_EVENT_CLICKED):
    case GX_SIGNAL(ID_D06_CONNECT_BUTTON, GX_EVENT_CLICKED):
        gx_widget_hide((GX_WIDGET *)&d06.window);
        return GX_SUCCESS;

    default:
        return gx_window_event_process((GX_WINDOW *)widget, event_ptr);
    }
}

void launch_gui_application()
{
    GX_RECTANGLE root_size;

    /* gx_image_reader_startがデコード結果のピクセルバッファを確保するのに使う。
       登録しないままデコードするとGX_SYSTEM_MEMORY_ERRORで失敗する。 */
    gx_system_memory_allocator_set(gx_heap_allocate, gx_heap_release);

    /* メイン画面を作成（windows32ドライバの場合はここで画面そのものが作られる） */
    gx_display_create(&main_disp, "main_disp", win32_graphics_driver_setup_24xrgb,
                       DISPLAY_WIDTH, DISPLAY_HEIGHT);

    /* ディスプレイ設定 */
    gx_display_color_table_set(&main_disp, color_table, sizeof(color_table) / sizeof(GX_COLOR));
    gx_display_font_table_set(&main_disp, font_table, sizeof(font_table) / sizeof(GX_FONT *));

    /* キャンバスを作成 */
    gx_canvas_create(&main_canvas, "main_canvas", &main_disp,
                      GX_CANVAS_MANAGED | GX_CANVAS_VISIBLE,
                      DISPLAY_WIDTH, DISPLAY_HEIGHT, main_canvas_memory, sizeof(main_canvas_memory));

    /* ルートを設定 */
    gx_utility_rectangle_define(&root_size, 0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    gx_window_root_create(&root, "root", &main_canvas,
                           GX_STYLE_BORDER_NONE, GX_ID_NONE, &root_size);
    gx_widget_fill_color_set(&root, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);

    /* 画面スタックを初期化(S01↔S02のような全画面遷移のpush/popに使う) */
    gx_system_screen_stack_create(screen_stack_memory, sizeof(screen_stack_memory));

    /* 各画面をrootの子として作成 */
    s01_create(&s01, (GX_WIDGET *)&root);
    s02_create(&s02, (GX_WIDGET *)&root);
    s03_create(&s03, (GX_WIDGET *)&root);
    s04_create(&s04, (GX_WIDGET *)&root);
    s05_create(&s05, (GX_WIDGET *)&root);
    d01_create(&d01, (GX_WIDGET *)&root);
    d02_create(&d02, (GX_WIDGET *)&root);
    d03_create(&d03, (GX_WIDGET *)&root);
    d04_create(&d04, (GX_WIDGET *)&root);
    d05_create(&d05, (GX_WIDGET *)&root);
    d06_create(&d06, (GX_WIDGET *)&root);

    /* S04の画像表示エリアは、デコード済みpixelmap(s04_pixelmap)を描く
       専用の描画関数に差し替える */
    gx_widget_draw_set(&s04.image_window, s04_image_window_draw);

    /* 各画面のボタンクリックを画面遷移につなげる */
    gx_widget_event_process_set(&s01.window, s01_window_event_process);
    gx_widget_event_process_set(&s02.window, s02_window_event_process);
    gx_widget_event_process_set(&s03.window, s03_window_event_process);
    gx_widget_event_process_set(&s04.window, s04_window_event_process);
    gx_widget_event_process_set(&s05.window, s05_window_event_process);
    gx_widget_event_process_set(&d02.window, d02_window_event_process);
    gx_widget_event_process_set(&d03.window, d03_window_event_process);
    gx_widget_event_process_set(&d04.window, d04_window_event_process);
    gx_widget_event_process_set(&d05.window, d05_window_event_process);
    gx_widget_event_process_set(&d06.window, d06_window_event_process);

    /* 起動時はS01のみ表示する。他の画面は画面遷移・ダイアログ表示で
       後から表示されるので、最初は隠しておく。 */
    gx_widget_hide(&s02.window);
    gx_widget_hide(&s03.window);
    gx_widget_hide(&s04.window);
    gx_widget_hide(&s05.window);
    gx_widget_hide(&d01.window);
    gx_widget_hide(&d02.window);
    gx_widget_hide(&d03.window);
    gx_widget_hide(&d04.window);
    gx_widget_hide(&d05.window);
    gx_widget_hide(&d06.window);

    /* ルートを表示 */
    gx_widget_show(&root);

    /* システムを開始 */
    gx_system_start();
}

void start_guix()
{
    gx_system_initialize();
}