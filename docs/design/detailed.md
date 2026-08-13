# 詳細設計書

[方式設計書](architecture.md)・[機能設計書](functional.md)で決めた内容を、実際の処理順序・関数レベルまで落とし込む。

## 画面遷移の実装方式

GUIXの**画面スタック機能**を使う。ただし、名前から連想される「画面を重ねる」機能ではなく、**「今表示中の画面を1つ外して、後で元に戻せるように覚えておく」だけの機能**であることに注意(GUIXソースコードを読んで確認済み)。

- `gx_system_screen_stack_create(memory, size)`: 起動時に1回、スタックを初期化する
- `gx_system_screen_stack_push(screen)`: 指定した画面を親から切り離し(`detach`)、画面と元の親を記憶する
- `gx_system_screen_stack_pop()`: 直前に記憶した画面を、元の親に再接続する(`attach`)

**次の画面を実際に表示するには、`push`とは別に`gx_widget_attach` (+ 必要なら`gx_widget_show`) を呼ぶ必要がある。**

- **進む(例: S01→S02)**: `gx_system_screen_stack_push(&S01)` で今の画面(S01)を外して記憶 → `gx_widget_attach(root, &S02)` (+`gx_widget_show(&S02)`) で次の画面(S02)を表示
- **戻る(例: S02→S01)**: `gx_widget_detach(&S02)` で今の画面を外す → `gx_system_screen_stack_pop()` で記憶していたS01を自動的に元の場所へ復元

(`attach`した後に`show`が明示的に必要かどうかは、ソースだけでは断定できなかったため、実装時に実際に確認する)

いずれも、ボタンのイベントハンドラの中から呼ぶだけでよい(GUIXシステムスレッドの中で完結する、軽い処理)。

## ダイアログ(D01〜D05)の実装方式

画面(S)とは異なり、ダイアログは**元の画面を表示したまま、その上に重ねて表示する**必要があるため、画面スタックは使わない。代わりに`gx_widget_front_move()`で最前面に持ってくる。

- あらかじめ、ダイアログのウィジェットは対象の画面(S04・S05等)の子として作成しておく(初期状態は非表示)
- 開く: `gx_widget_show()` + `gx_widget_front_move()` で表示しつつ最前面へ
- 閉じる: `gx_widget_hide()` で非表示に戻す(元の画面はそのまま表示され続けている)

## シーケンス図: 画面遷移(共通の仕組み)

画面遷移は、**どの画面同士でも同じ仕組み**(タッチで次の画面に進む、「戻る」で1つ前に戻る)で行う。個別の画面ペアごとに別の実装をするわけではない。以下はS01⇔S02を代表例とした図で、他の画面ペア(S03⇔S04など)も同様の流れになる。

![画面遷移のシーケンス](images/sequence-screen-transition.svg)

## シーケンス図: ダイアログの開閉(D01 編集ツールバー)

S04(ビューア画面)で「編集ボタン」を押してD01を開き、閉じるまでの例。

![ダイアログの開閉シーケンス](images/sequence-dialog-open-close.svg)

## ビューア画面(S04)表示の実装方式

S03で1枚選択して「次へ」を押してからS04に画像が表示されるまでは、ファイル読み込み(FileX)とPNGデコード(メモリ上、GUIXランタイムの`gx_image_reader`)の2段階に分かれる。

- **ファイル読み込み**: [方式設計書](architecture.md#コンポーネント構成)で決めたスレッド間連携パターン(`TX_QUEUE`で指示・`TX_MUTEX`で保護した共有データ+`GX_EVENT_TIMER`のポーリングで結果を受け取る)をそのまま使い、データ管理スレッドに任せる
- **PNGデコード**: 読み込んだバイト列を受け取った後、GUIXシステムスレッドの中で`gx_image_reader_create`+`gx_image_reader_start`を同期的に呼ぶ。画像1枚分のデコードは軽いという想定に立ち、専用スレッドは作らない(デコード中はタッチ操作を含むGUIX全体が一瞬止まる)
- **画面遷移のタイミング**: デコードが完了するまではS03表示のまま待ち、完了後にpush/attachでS04へ遷移する(S04が常に画像の揃った状態で表示されるようにするため)

## シーケンス図: ビューア画面表示(S03 → S04)

![ビューア画面表示のシーケンス](images/sequence-viewer-display.svg)

## 保存処理(D03 別名で保存)の実装方式

D03(名前入力ダイアログ)の「保存」を押してからの流れ。D03自体の開閉はダイアログの実装方式([D01の開閉シーケンス](#シーケンス図-ダイアログの開閉d01-編集ツールバー)と同じ`show`/`front_move`/`hide`)を流用するため、ここでは「保存」ボタンを押した後の処理に絞る。

- **入力チェック**: 空欄→禁止文字→重複の順(詳細は[入出力仕様](functional.md#入出力仕様))。重複チェックはS03で既にメモリ上に読み込まれているファイル一覧と比較するだけで、FileXへの再問い合わせは行わない。GUIXシステムスレッドの中で完結する軽い処理
- **画面のキャプチャ**: 「保存」を押した時点でGUIXシステムスレッドが`gx_utility_canvas_to_bmp(canvas, &rect, write_callback)`を呼び、画像表示エリアの矩形を指定して「今見えている内容」をキャプチャする。S04(1枚)・S05(2枚横並び)どちらも、画像そのものではなく**表示エリアに実際に描画されている内容**をキャプチャするので、同じ仕組みで済む(S04で画像を`gx_widget_shift`で動かした後の位置、S05で2枚を並べた状態、どちらもそのまま反映される)。ソース([gx_utility_canvas_to_bmp.c](../../repos/guix/common/src/gx_utility_canvas_to_bmp.c))で確認した実装上の事実:
  - 54バイトの固定長ヘッダ(`bitmap_file_header`14バイト+`bmp_info`40バイト)の後にピクセルデータが続く
  - ピクセルデータは`y = rect.top`から`rect.bottom`まで**上から下の順**で書き出される。一般的なBMPの「下から上」とは逆(このGUIX実装独自の仕様)なので、**行の並び替えをせずそのままPNGの行順として使える**のは都合が良い
  - コールバック(`write_data`)は呼ばれるたびにバイト列を渡してくるだけなので、GUIXシステムスレッド側で1つのバッファに連結して溜めておく必要がある
  - 表示エリア分のキャプチャなので処理量は画面サイズに収まる(ビューア表示のPNGデコードと同様、GUIXシステムスレッドの中で同期的に行っても許容できる範囲という判断)
- **PNGエンコード(libpng)とFileX書き込み**: どちらもデータ管理スレッドに任せる。GUIXシステムスレッドは、キャプチャしたバッファ(54バイトのヘッダを除いたピクセル部分)への参照を`TX_QUEUE`で渡すだけ。`libpng`の書き込みコールバックがそのまま`fx_file_write`を呼ぶ形になるため、エンコードと書き込みを同じスレッドにまとめた方が自然という判断(ビューア表示のPNGデコードとは逆に、GUIXシステムスレッドでは行わない)。libpngはFileXやWindowsファイルシステムと違い、下にドライバ層を持たない純粋な計算ライブラリ([システム構成図](architecture.md#システム構成図)と同じ整理)
- **ファイル操作の順序**: `fx_file_create`(新規作成)→`png_write_image`等でPNGエンコードを開始(libpngの書き込みコールバックが内部で`fx_file_write`を繰り返し呼ぶ)→`fx_file_close`。作成を先に済ませてからエンコードを始めるのは、libpngのコールバックが書き込み先のファイルハンドルを必要とするため。上書き保存(既存ファイルへの安全な置き換え)は別名で保存とは異なる手順になるため、別途扱う
- **PNGエンコードとFileXの依存分離**: libpngの書き込みコールバックに`fx_file_write`を直接埋め込むと、「PNGエンコードを担当するコード」が「FileX」を直接知ってしまう。これを避けるため、自分で定義した薄い書き込みIF(`png_write_sink_t { UINT (*write)(void *context, const GX_UBYTE *data, ULONG length); }`)を間に挟む。libpngに登録するコールバックはこのIF越しに呼ぶだけでFileX非依存になり、FileX固有の実装(`fx_file_write`を呼ぶアダプタ)は別ファイルに閉じ込める。実行時のデータの流れ(逐次書き込み)は変えず、コードの依存方向だけを整理する目的
- **結果の受け渡し**: ビューア表示と同じ`TX_QUEUE`/`TX_MUTEX`で保護した共有データ/`GX_EVENT_TIMER`ポーリングのパターンを使う
- **保存後の画面遷移**: 画面遷移はせず、呼び出し元の画面(S04・S05)に留まる。D03を`hide`するだけでよい([機能設計書のS04・S05ふるまい](functional.md#s04-ビューア画面)で明記済み)

## シーケンス図: 別名で保存(D03、成功パターン)

![別名で保存のシーケンス](images/sequence-save-as.svg)

## 入力チェックNGの実装方式(D03 → D04)

入力チェック(空欄→禁止文字→重複)自体はGUIXシステムスレッドの中で完結する(FileXやデータ管理スレッドへは問い合わせない)ので、ワーカースレッドは登場しない。D04の表示・非表示は[D01の開閉シーケンス](#シーケンス図-ダイアログの開閉d01-編集ツールバー)と全く同じ`show`/`front_move`/`hide`の仕組みをそのまま使う。

- D04を閉じて(`hide`)D03に戻った後も、**D03のテキスト入力欄の内容は消さずに残す**想定(入力し直す手間を減らすため)。GUIXが自動でクリアするわけではないので、明示的にクリアする実装を入れない限りこの挙動になる

## シーケンス図: 入力チェックNG(D03 → D04)

![入力チェックNGのシーケンス](images/sequence-save-validation-error.svg)

## 上書き保存(D05)の実装方式

D05の「上書き保存」を押してからの流れ。PNGエンコード(libpng)・FileX書き込みの部分は[別名で保存](#保存処理d03-別名で保存の実装方式)と同じ仕組みをそのまま使う。異なるのは、書き込み先と、書き込み後の後処理。

- **一時ファイルに書く**: いきなり元ファイルに上書きするのではなく、別名(`temp_name`)で新規作成して書き込む。[方式設計書のデータ永続化方針](architecture.md#データ永続化方針)の「書き込みが完了してから置き換える(失敗時は元のファイルが残る)」を実現するための手順
- **書き込み完了後の置き換え**: `fx_file_delete(original_name)`で元ファイルを削除してから、`fx_file_rename(temp_name, original_name)`で一時ファイルを元の名前に変更する。FileXの`fx_file_rename`は、変更先の名前が既に存在すると`FX_ALREADY_CREATED`を返して失敗する([fx_file_rename.c](../../repos/filex/common/src/fx_file_rename.c)で確認済み)ため、削除してからリネームする以外の手順が取れない
- **残るリスク**: `fx_file_delete`が成功した直後、`fx_file_rename`が実行される前にクラッシュ・電源断が起きると、元ファイルも一時ファイルへのリネームも完了しておらず、データを失う可能性がある。「書き込み中の破損」は防げるが、「削除とリネームの間」は完全にはカバーできていない。今回はこの残存リスクを許容する(発生頻度・影響ともに小さいと判断)が、気になる場合は別途対策(削除ではなく元ファイルを一旦別名にリネームしておき、最後に不要になってから消す、等)を検討する

## シーケンス図: 上書き保存(D05、成功パターン)

![上書き保存のシーケンス](images/sequence-overwrite-save.svg)

## 接続(D06 → S02)の実装方式

D06で「接続開始」を押してからの流れ。機器の指定方法は[ADR-0006](../decisions/0006-fixed-ip-connect-instead-of-scan.md)の通り、ユーザーが直接入力したIPアドレスに接続する(スキャンはしない)。

- **D06→S02の切り替え**: 「接続開始」を押した時点でD06は`hide`して閉じ、S02側の接続状態表示を「接続中...」に切り替える。ここまではGUIXシステムスレッドの中で完結する軽い処理
- **FTP接続**: FTP探索スレッドが入力されたIPアドレスへ`nx_ftp_client_connect(ftp_client_ptr, server_ip, username, password, wait_option)`で接続する。IISのFTPサイトを匿名接続で使う想定なら、`username`/`password`は匿名ログイン用の固定値になる(実装時に確認)
- **ルートフォルダの取得**: 接続に成功したら`nx_ftp_client_directory_listing_get(ftp_client_ptr, "/", &packet_ptr, wait_option)`でルートフォルダの中身を取得する。接続に失敗した場合はここで打ち切り、エラーとして結果を返す
- **結果の受け渡し**: 他の操作と同じ`TX_QUEUE`/`TX_MUTEX`で保護した共有データ/`GX_EVENT_TIMER`ポーリングのパターンを使う。接続状態(成功/失敗)とルートフォルダの中身をまとめて1つの構造体で渡す

```c
#define MAX_LISTING_ENTRIES  64  /* 1回のフォルダ一覧表示に含める最大件数 */
#define MAX_NAME_LEN         64  /* ファイル/フォルダ名の最大長 */

typedef struct {
    CHAR   name[MAX_NAME_LEN];
    UCHAR  is_directory;
} listing_entry_t;

/* 接続(D06)〜フォルダ一覧取得(S02)用。以降のフォルダを掘る操作でも同じ構造体を使い回す */
typedef struct {
    op_status_t          status;         /* OP_PENDING(接続/取得中)/OP_SUCCESS/OP_ERROR */
    worker_error_code_t  error_code;
    ULONG                 connected_ip;   /* status == OP_SUCCESS の時、画面表示用 */
    UINT                  entry_count;
    listing_entry_t       entries[MAX_LISTING_ENTRIES];
} connection_status_t;

void set_connection_status(op_status_t status, worker_error_code_t error_code, ULONG ip,
                            const listing_entry_t *entries, UINT count);
UINT get_connection_status(connection_status_t *out);
```

## シーケンス図: 接続(D06 → S02)

![接続のシーケンス](images/sequence-device-connect.svg)

## フォルダを掘る(S02)の実装方式

フォルダ一覧表示中にフォルダをタッチしてからの流れ。接続(`nx_ftp_client_connect`)は既にD06での接続時に済んでいるFTPセッションをそのまま使うため、ここでは行わない。タッチされたパスに対して`nx_ftp_client_directory_listing_get`を呼ぶだけでよい。

結果の受け渡しも接続時と同じ`connection_status_t`/`set_connection_status`/`get_connection_status`をそのまま使い回す(`connected_ip`は変わらないので、そのまま維持するか呼び出し側で無視する)。専用の構造体を新設していないのは、接続時の「ルートフォルダ取得」と、掘る時の「指定フォルダ取得」が、実質的に同じ操作(パスを指定してFTPのLISTを呼ぶだけ)だから。

## シーケンス図: フォルダを掘る(S02)

![フォルダを掘るシーケンス](images/sequence-folder-drill.svg)

## 取得開始(S02)の実装方式

ファイルを選択して「取得開始」を押してからの流れ。[方式設計書](architecture.md#コンポーネント構成)の元々のスレッド責務分担(HTTP転送スレッド=転送、データ管理スレッド=永続化)を維持し、**HTTP転送スレッドはダウンロードのみ、FileXへの書き込みはデータ管理スレッドが行う**。ダウンロードとFileX保存で2つの独立したシーケンス図に分けている。

- **URLの組み立ては不要**: NetX DuoのHTTPクライアントAPI(`nx_http_client_get_start`)は`ip_address`と`resource`(パス)を別々の引数で受け取るため、`http://...`という文字列を自分で組み立てる必要はない。GUIXシステムスレッドが特定した「IP+パス」をそのままHTTP転送スレッドに渡すだけでよい
- **ボディの受信**: `nx_http_client_get_start`でリクエストを開始した後、`nx_http_client_get_packet`を繰り返し呼んでボディを受信しきるまでループする(FileX保存側の`fx_file_write`のような1回で終わる呼び出しではない)
- **HTTP転送スレッド→データ管理スレッドへの受け渡し**: ダウンロードしたバイト列は、HTTP転送スレッドが持つバッファに置いたまま、`TX_QUEUE`でそのポインタ・サイズ・保存先ファイル名だけをデータ管理スレッドに渡す(ビューア表示の`get_image`のような`memcpy`はしない)。次のダウンロードジョブが割り込む余地がない(UIの状態が排他的)ため、データ管理スレッドが読み終わるまでHTTP転送スレッド側のバッファはそのまま保持される、という前提に立つ
- **結果の受け渡し(データ管理スレッド→GUIXシステムスレッド)**: 他の操作と同じ`TX_QUEUE`/`TX_MUTEX`で保護した共有データ/`GX_EVENT_TIMER`ポーリングのパターンを使う。ペイロードは無く、成功/失敗だけで十分なので`save_status_t`と同じ形の構造体にする

```c
/* 取得開始(S02)用。ダウンロード→FileX保存の結果を1つにまとめて返す */
typedef struct {
    op_status_t          status;
    worker_error_code_t  error_code;
} download_status_t;

void set_download_status(op_status_t status, worker_error_code_t error_code);
UINT get_download_status(download_status_t *out);
```

## シーケンス図: 取得開始 1/2 ダウンロード(S02 → HTTP転送スレッド)

![取得開始のシーケンス1/2](images/sequence-download.svg)

## シーケンス図: 取得開始 2/2 FileX保存 → D02表示

![取得開始のシーケンス2/2](images/sequence-download-save.svg)

## 共有データ(ワーカースレッド→画面描画スレッド)の構造体設計

[方式設計書](architecture.md#コンポーネント構成)の「`TX_MUTEX`で保護した共有データ+`GX_EVENT_TIMER`ポーリング」パターンで実際にやり取りする構造体を定義する。

**操作ごとに別々の構造体を持つが、`TX_MUTEX`は共通で1つだけ**にする。理由は、GUIXシステムスレッドが一度に待てるジョブは1つだけ(D03表示中はビューア表示の読み込みが走らない、といったようにUIの状態が排他的)なので、構造体ごとに`TX_MUTEX`を増やしても同時にロックの奪い合いが起きる場面がなく、単に管理するオブジェクトが増えるだけになるため。

呼び出し側が`TX_MUTEX`を直接`tx_mutex_get`/`tx_mutex_put`する代わりに、構造体ごとにロック/アンロックを隠したセッター/ゲッター関数を用意する。

「成功/失敗/処理中」を表す`status`と、エラーの種類を表す`error_code`は操作をまたいで共通の語彙として再利用する(構造体そのものは共有しないが、値の分類は共有する)。

```c
/* 全操作で共通の分類(構造体そのものは共有しない) */
typedef enum {
    OP_PENDING = 0,  /* まだ処理中(データ管理スレッドが書き込み中) */
    OP_SUCCESS,
    OP_ERROR,
} op_status_t;

typedef enum {
    WORKER_ERROR_NONE = 0,
    WORKER_ERROR_FILE_NOT_FOUND,     /* ビューア表示: 対象ファイルが見つからない */
    WORKER_ERROR_IO_FAILURE,         /* 読み込み/書き込み中の失敗 */
    WORKER_ERROR_STORAGE_FULL,       /* 保存・取得開始: 空き容量不足 */
    WORKER_ERROR_CONNECTION_FAILED,  /* 接続: 指定IPアドレスに接続できない、タイムアウト */
} worker_error_code_t;

/* 全操作で共通の1つのTX_MUTEX(構造体側からのみ参照、呼び出し側は直接触らない) */
extern TX_MUTEX g_worker_result_mutex;

/* 想定する画像の最大サイズ。データ管理スレッドの読み込みバッファ、
   GUIXシステムスレッドの受け取りバッファの両方をこのサイズで固定確保する */
#define IMAGE_BUFFER_MAX_SIZE  (...)

/* ビューア表示(S03→S04)用 */
typedef struct {
    op_status_t          status;
    worker_error_code_t  error_code;  /* status == OP_ERROR の時だけ意味を持つ */
    ULONG                 size;        /* 実際のバイト数(status == OP_SUCCESS の時のみ有効) */
} image_data_t;

void set_image(op_status_t status, worker_error_code_t error_code, const GX_UBYTE *data, ULONG size);
UINT get_image(image_data_t *out, GX_UBYTE *dest_buffer, ULONG dest_capacity);

/* 保存(D03 別名で保存/D05 上書き保存 共通)用 */
typedef struct {
    op_status_t          status;
    worker_error_code_t  error_code;  /* status == OP_ERROR の時だけ意味を持つ */
} save_status_t;

void set_save_status(op_status_t status, worker_error_code_t error_code);
UINT get_save_status(save_status_t *out);
```

- **`set_image`**: データ管理スレッドが呼ぶ。処理開始前に`set_image(OP_PENDING, WORKER_ERROR_NONE, NULL, 0)`、完了後に結果を書き込む。`g_worker_result_mutex`を内部で取得/解放する
- **`get_image`**: GUIXシステムスレッドの`GX_EVENT_TIMER`が呼ぶ。`g_worker_result_mutex`を取得し、`status`が`OP_PENDING`でなければ、`data`を呼び出し側が用意した`dest_buffer`へ`memcpy`してから解放する。**ポインタではなく実データをコピーする**ことで、コピーが終わった瞬間にデータ管理スレッド側の読み込みバッファへの依存が切れ、データ管理スレッドは即座にそのバッファを再利用できる。`dest_buffer`は`IMAGE_BUFFER_MAX_SIZE`の固定サイズでGUIXシステムスレッド側があらかじめ確保しておく(実際のファイルサイズが`dest_capacity`を超える場合は失敗を返す)
- **`set_save_status`/`get_save_status`**: 保存はポインタを持たない小さな構造体なので、値のコピーだけで済む(`memcpy`は不要)
- `error_code`は内部的な分類にとどめ、実際にD04等のパネルに出すメッセージは[エラーハンドリング方針](architecture.md#エラーハンドリング方針)の通り簡潔な一言に変換する。対応表は以下の通り

| `error_code` | パネルに表示するメッセージ |
|---|---|
| `WORKER_ERROR_FILE_NOT_FOUND` | 「ファイルが見つかりません」 |
| `WORKER_ERROR_IO_FAILURE` | 「通信または書き込みに失敗しました」 |
| `WORKER_ERROR_STORAGE_FULL` | 「保存先の空き容量が不足しています」 |
| `WORKER_ERROR_CONNECTION_FAILED` | 「接続できませんでした」 |

## ワーカースレッド失敗時の実装方式(共通パターン)

別名で保存・上書き保存・接続(D06)・取得開始のいずれも、ワーカースレッド側で失敗した場合の見せ方は同じパターンになる。[エラーハンドリング方針](architecture.md#エラーハンドリング方針)の通り、種類によらず「メッセージ+OKボタンのみのパネル」で統一するため、D01と同じ`show`/`front_move`/`hide`の仕組みをそのまま使い、共有データの`status`が`OP_ERROR`になった場合にこのパネルを表示する。

**フローごとに異なるのは「エラーパネルを出す前に、書きかけのファイルを消すかどうか」**という後始末の部分だけ。

| フロー | 失敗時にFileXへ書きかけのファイルが残りうるか | 対応 |
|---|---|---|
| 別名で保存 | 残る(`fx_file_create`済みの新規ファイルへの書き込み/クローズが失敗) | `fx_file_delete`で書きかけの新規ファイルを消してから`set_save_status(OP_ERROR, ...)` |
| 上書き保存 | 残る(一時ファイル`temp_name`への書き込み/クローズが失敗) | `fx_file_delete`で一時ファイルを消してから`set_save_status(OP_ERROR, ...)`。**元ファイルには一切手を付けていない段階なので、これだけで元通りに戻せる**。ただし[上書き保存の実装方式](#上書き保存d05の実装方式)で触れた「削除は済んだがリネーム前」の残存リスクは、これとは別の話として残る |
| 接続(D06) | 残らない(まだファイルを作っていない) | クリーンアップ不要。そのまま`set_connection_status(OP_ERROR, ...)` |
| 取得開始 | 残りうる(HTTPダウンロード自体の失敗ならファイル未作成だが、FileX書き込み中の失敗なら書きかけのダウンロードファイルが残る) | 書き込み中に失敗した場合のみ`fx_file_delete`で書きかけのファイルを消してから`set_download_status(OP_ERROR, ...)` |

## シーケンス図: ワーカースレッド失敗時のエラー表示(共通パターン)

![ワーカースレッド失敗時のエラー表示シーケンス](images/sequence-worker-error.svg)

## D01「移動」の実装方式

D01(編集ツールバー)の「移動」を押してから、画像をドラッグして位置を動かすまでの流れ。ワーカースレッドは登場せず、GUIXシステムスレッドの中だけで完結する(FileXやNetX Duoへの問い合わせが無いため)。

- **モードに入る**: 「移動」を押した時点では画面上の見た目は変えず、内部的に「移動モード中」のフラグを立てるだけ
- **ドラッグ中**: GUIXのタッチイベントは`GX_EVENT_PEN_DOWN`(押した瞬間)→`GX_EVENT_PEN_DRAG`(押したまま動かしている間、繰り返し発生)→`GX_EVENT_PEN_UP`(離した瞬間)の3種類に分かれる。`PEN_DOWN`で開始位置を記憶し、`PEN_DRAG`のたびに直前位置との差分(dx, dy)を計算して`gx_widget_shift(&image, dx, dy, GX_TRUE)`を呼ぶ。第4引数`GX_TRUE`で再描画(dirty)まで一緒にやってくれる
- **ドラッグ終了**: `PEN_UP`で「編集内容あり」のフラグを立てる。これがS04のふるまい(「D01で画像を移動するなど、表示中の画像に変更を加えた時、保存ボタンがハイライトに変わる」)のトリガーになる

## シーケンス図: D01「移動」

![D01「移動」のシーケンス](images/sequence-toolbar-move.svg)

## コラージュ画面(S05)表示の実装方式

S03でコラージュモードのファイルを2枚選択して「次へ」を押してからS05に2枚が横並びで表示されるまでの流れ。**「合成」のための特別な画像処理は行わない**。

- **表示の仕組み**: S04と同じ「画像1枚につき1つのウィジェット」を、左右2つ分並べて配置するだけ。2枚の画像はそれぞれ独立したウィジェットとして持ち、それぞれが独立して`gx_widget_shift`で動かせる([D01「移動」のS05版](#d01移動のs05版差分のみ)参照)
- **読み込み**: [ビューア表示(S03→S04)](#ビューア画面s04表示の実装方式)と全く同じ`TX_QUEUE`/`TX_MUTEX`/`GX_EVENT_TIMER`/`get_image`/`gx_image_reader_start`のパターンを、選択した2枚それぞれについて繰り返すだけ。1枚読み込むたびに、対応するスロット(左/右)のウィジェットにセットする
- **画面遷移のタイミング**: ビューア表示と同様、2枚とも読み込み・デコードが完了してからS05へ遷移する(表示が半端な状態でS05に入らないようにするため)
- **「合成」の実体は保存時**: 2枚を1枚のPNGにまとめる処理は、この表示の時点では行わない。[保存の実装方式](#保存処理d03-別名で保存の実装方式)で触れた`gx_utility_canvas_to_bmp`によるキャプチャが、実質的な「合成」の役割を果たす(画面に見えている2枚の配置をそのまま1枚の画像として切り出す)

## シーケンス図: コラージュ画面表示(S03 → S05)

![コラージュ画面表示のシーケンス](images/sequence-collage-display.svg)

## D01「移動」のS05版(差分のみ)

S05では画像が2枚あるため、`PEN_DOWN`(タッチした瞬間)で「**どちらの画像がタッチされたか**」をヒットテストで判定する処理が1つ増える。それ以外([D01「移動」](#d01移動の実装方式)の「ドラッグで画像を動かす」④以降、`PEN_DRAG`の繰り返しで`gx_widget_shift`を呼ぶ→`PEN_UP`で編集フラグを立てる)は完全に同じで、`gx_widget_shift`の対象が判定したスロットの画像に変わるだけ。差分部分だけを別図にしている。

## シーケンス図: D01「移動」のS05版(差分のみ)

![D01「移動」のS05版シーケンス](images/sequence-toolbar-move-collage.svg)

## S03ファイル一覧読み込みの実装方式

S03(ファイル一覧画面)が表示する「保存済み画像の一覧」は、S02の「フォルダ一覧」(FTPでリモート機器を一覧する)とは別物で、**ローカル(FileX)に保存済みのファイルを列挙する**処理。S01→S03、S04/S05→S03のどちらの経路で入ってきても、S03画面のハンドラが呼ばれるたびに読み直す(S04/S05で新しく保存したファイルが一覧に反映されるようにするため)。

- **列挙方法**: `fx_directory_first_full_entry_find`(1件目)→`fx_directory_next_full_entry_find`(2件目以降)をエントリが無くなるまで繰り返す。データ管理スレッドが担当する(既存のFileX関連処理と同じ置き場所)
- **表示は名前のみ**: サムネイル表示は各画像を個別にデコードする必要があり別途大きな検討が要るため、今回はファイル名のみの一覧とする
- **結果の受け渡し**: 他の操作と同じ`TX_QUEUE`/`TX_MUTEX`で保護した共有データ/`GX_EVENT_TIMER`ポーリングのパターンを使う

```c
#define MAX_FILE_ENTRIES  64  /* 一覧に表示する最大件数 */

typedef struct {
    CHAR name[MAX_NAME_LEN];
} file_entry_t;

/* S03ファイル一覧用 */
typedef struct {
    op_status_t          status;
    worker_error_code_t  error_code;
    UINT                  entry_count;
    file_entry_t          entries[MAX_FILE_ENTRIES];
} file_list_t;

void set_file_list(op_status_t status, worker_error_code_t error_code, const file_entry_t *entries, UINT count);
UINT get_file_list(file_list_t *out);
```

## シーケンス図: S03ファイル一覧読み込み

![ファイル一覧読み込みのシーケンス](images/sequence-file-list.svg)
