#ifndef FILEX_MEDIA_H
#define FILEX_MEDIA_H

#include "fx_api.h"

#define FILEX_TEST_IMAGE_NAME "TEST.PNG"

extern FX_MEDIA disk_media;

/*
 * FileXの仮想ディスクをオープンする(既存のディスクファイルが無ければ
 * フォーマットしてから開く。app_poc/main.cと同じパターン)。
 * main_thread_entryから、launch_gui_application()より前に呼ぶこと
 * (GUIXが動き出す前にディスクの準備を終わらせておく)。
 * テスト用PNG(app/data/test.png)がディスクに無ければ、このタイミングで
 * 書き込んでおく(S02のダウンロード機能がまだ無いための、暫定シード処理)。
 */
void launch_filex_media(void);

#endif /* FILEX_MEDIA_H */
