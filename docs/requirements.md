# 要件定義

## 目的

win_rtos上で、NetX Duo・FileX・GUIXを組み合わせた実用アプリを作り、Eclipse ThreadXエコシステムの実践的な使い方を検証する。

## 機能要件

| # | 内容 | 担当コンポーネント |
|---|---|---|
| FR1 | 指定URL(平文HTTP)からPNG画像をダウンロードする | NetX Duo(HTTPクライアント)+ Npcapによる実通信 |
| FR2 | ダウンロードしたPNGをファイルとして保存する。アプリを再起動しても残る | FileX(実ファイルにバックする永続化ドライバ) |
| FR3 | 保存済み画像の一覧(フォルダ)をGUI上に表示する | GUIX |
| FR4 | 一覧からアイテムをタッチすると、そのPNGをランタイムデコードして画面表示する | GUIX(`gx_image_reader_png_decode`) |

## 技術的裏付け

- ランタイムPNGデコードAPI `gx_image_reader_png_decode(data, size, &pixelmap)` がGUIX標準ライブラリに存在する。ビルド時のGUIX Studioリソース変換に頼らず、任意のバイト列をその場でデコードできる。
- NetX DuoにHTTPクライアントアドオンがある(`netxduo/addons/http`)。
- Npcapはインストール・稼働確認済み。

## 前提条件・未解決事項

- ビルドにはNpcap SDK(`pcap.h` / `wpcap.lib` / `Packet.lib`)が別途必要。手元にあるか未確認。
- 平文HTTPで配信しているテスト用PNG画像のURLを探す/用意する必要がある。

## スコープ外

- HTTPS/TLS対応
- 複数枚の一括ダウンロードやアルバム管理などの凝ったUI
