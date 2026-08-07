# win_rtos
Eclipse ThreadX公式Win32ポートを用いた、Windows向けアプリ開発・検証環境

## 狙い

PNG画像のダウンロード&ビューアアプリを実際に開発することを通して、Eclipse ThreadXおよびそのエコシステム(NetX Duo・FileX・GUIX)についての理解を深める。

## 概要

NetX Duo・FileX・GUIXを組み合わせた、PNG画像のダウンロード&ビューアアプリです。

- HTTPで指定サイトからPNG画像をダウンロード(NetX Duo + Npcapによる実通信)
- ダウンロードしたPNGを永続化(FileX、アプリを再起動しても残る)
- 保存済み画像の一覧をGUI上に表示し、タッチした画像をその場でデコードして表示(GUIX)

詳細は[要件定義](docs/requirements.md)を参照してください。

## 実行ファイル

| exe | 内容 |
|---|---|
| `win_rtos.exe` | ThreadXのみの最小構成。`main_thread`が定期的にログを出す |
| `win_rtos_poc.exe` | NetX Duo・FileX・GUIXを導入できるかを確認するPOC |

### win_rtos_poc.exe について

NetX Duo・FileX・GUIXを一通り組み込めるかを確認するための検証用アプリです。

- **NetX Duo**: 2つのループバックIPインスタンス間でUDPパケットを送受信
- **FileX**: 実ファイル(`win_rtos_poc_disk.dat`)にバックする自作ドライバ([port/win32/filex/fx_win32_file_driver.c](port/win32/filex/fx_win32_file_driver.c))を使用。起動回数をカウントして書き込み、**アプリを再起動しても値が残る**ことを確認済み
- **GUIX**: ウィンドウにNetXの送受信数とFileXの起動回数カウンタを表示

実行すると、実行ディレクトリに以下の2ファイルが作られる。

- `win_rtos_poc_disk.dat` — FileXのディスクイメージ本体(削除すると次回起動時に再フォーマットされる)
- `win_rtos_poc.log` — 実行ログ(GUIサブシステムのためコンソールが無く、代わりにファイルへ出力している。実行のたびに上書きされる)

## ビルド・実行方法(VSCode)

**前提**

- Visual Studio 2022/2026(C++によるデスクトップ開発ワークロード)がインストールされていること
- VSCodeに以下の拡張機能が入っていること
  - C/C++(`ms-vscode.cpptools`)
  - CMake Tools(`ms-vscode.cmake-tools`)

**手順**

1. このフォルダをVSCodeで開く
2. `repos/` に必要なリポジトリをclone(未取得の場合)
   ```bash
   git clone https://github.com/eclipse-threadx/threadx.git repos/threadx
   git clone https://github.com/eclipse-threadx/netxduo.git repos/netxduo
   git clone https://github.com/eclipse-threadx/filex.git repos/filex
   git clone https://github.com/eclipse-threadx/guix.git repos/guix
   ```
   (`win_rtos.exe` だけなら threadx のみで良い)
3. ステータスバーの **「Select a Kit」** をクリックし、`Visual Studio Community 20xx Release - x86`(または `amd64_x86`)を選択
   - 表示されない場合は、コマンドパレット(`Ctrl+Shift+P`)から `CMake: Select a Kit` を実行する
   - それでも一覧に出てこない場合は `CMake: Scan for Kits` を試す
4. Kit選択後、自動的にconfigureが走る(初回は数十秒かかる)
5. ステータスバーの **「Select a Build Target」** で `win_rtos` または `win_rtos_poc` を選び、**▶(Run)** ボタンを押すとビルド・実行される。`win_rtos.exe` はそのまま統合ターミナルにログが表示される
   ```
   main start
   main_thread is 00A3F800
   main_thread running
   main_thread running
   ...
   ```

**コマンドラインでビルドする場合(参考)**

VSCodeを使わない場合は、Visual Studio Developer コマンドプロンプト(x86)から以下でも同じことができる。

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=repos/threadx/cmake/win32.cmake
cmake --build build
build\win_rtos.exe
```

## ドキュメント

- [要件定義](docs/requirements.md)

---

# ライセンス

本プロジェクトにおいて作者が作成したソースコードおよびドキュメントは、MIT License のもとで公開しています。
詳細はリポジトリに含まれる `LICENSE` ファイルを参照してください。

`repos` ディレクトリには、Eclipse ThreadX に由来するソースコードが含まれています。
これらのファイルの著作権は作者には帰属せず、各ファイルに記載されている著作権表示およびライセンスに従います。

本プロジェクトは、作者個人による学習・検証を目的とした非公式プロジェクトです。
Eclipse Foundation、Microsoft、その他関連団体、ならびに作者の所属企業とは一切関係ありません。

本ソフトウェアは開発途中のものであり、不具合、未実装の機能、仕様変更等が含まれる場合があります。

本ソフトウェアは現状(AS IS)のまま提供されます。
作者は、本ソフトウェアの正確性、完全性、有用性、特定目的への適合性等について、明示または黙示を問わず、いかなる保証も行いません。

本ソフトウェアの利用、改変、再配布等により生じたいかなる損害についても、作者は一切の責任を負いません。
本ソフトウェアをご利用の際は、利用者ご自身の責任においてご利用ください。
