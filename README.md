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

## ドキュメント

- [要件定義](docs/requirements.md)
