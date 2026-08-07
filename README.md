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

## ビルド・実行方法(VSCode)

**前提**

- Visual Studio 2022/2026(C++によるデスクトップ開発ワークロード)がインストールされていること
- VSCodeに以下の拡張機能が入っていること
  - C/C++(`ms-vscode.cpptools`)
  - CMake Tools(`ms-vscode.cmake-tools`)

**手順**

1. このフォルダをVSCodeで開く
2. `repos/` にThreadXをclone(未取得の場合)
   ```bash
   git clone https://github.com/eclipse-threadx/threadx.git repos/threadx
   ```
3. ステータスバーの **「Select a Kit」** をクリックし、`Visual Studio Community 20xx Release - x86`(または `amd64_x86`)を選択
   - 表示されない場合は、コマンドパレット(`Ctrl+Shift+P`)から `CMake: Select a Kit` を実行する
   - それでも一覧に出てこない場合は `CMake: Scan for Kits` を試す
4. Kit選択後、自動的にconfigureが走る(初回は数十秒かかる)
5. ステータスバーの **▶(Run)** ボタンを押すと `win_rtos.exe` がビルド・実行され、統合ターミナルにログが表示される
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
