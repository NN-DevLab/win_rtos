# 設計判断の記録(ADR)

このディレクトリには、本番アプリ(PNGダウンロード&ビューア)の方式設計・機能設計を進める中で行った判断を、ADR(Architecture Decision Record)として記録する。

## 一覧

- [0001. プラットフォームにWindows(Win32シミュレータ)を採用する](0001-windows-platform.md)
- [0002. 画像取得は「FTPで探索、HTTPで転送」の二段構成にする](0002-ftp-discovery-http-transfer.md)
- [0003. PNG保存(エンコード)にlibpngを組み込む](0003-libpng-for-png-encoding.md)
- [0004. PNGエンコード(libpng)とFileXの間に書き込みIFを挟んで依存を分離する](0004-png-encode-filex-write-sink.md)
- [0005. 機器検索は自分のサブネット内を総当たりスキャンする](0005-subnet-scan-device-discovery.md)(→ 0006に置き換え)
- [0006. 機器検索(スキャン)をやめ、IPアドレス直接指定に変更する](0006-fixed-ip-connect-instead-of-scan.md)
- [0007. 日本語フォント対応まで画面表示を英語/ローマ字の仮表示にする](0007-placeholder-english-text-pending-japanese-font.md)

## 目的

「最終的にどう設計したか」だけでなく、「なぜその設計にしたか」「他にどんな案を検討して、なぜ採用しなかったか」を残すこと。

## ファイルの作り方

1つの判断ごとに、連番でファイルを作る。

```
0001-xxxxx.md
0002-xxxxx.md
...
```

`0000-template.md` をコピーして使う。

## テンプレートの項目

- **ステータス**: 検討中 / 決定 / 却下 / 別の決定に置き換え
- **背景(Context)**: なぜこの判断が必要になったか
- **決定(Decision)**: 何を選んだか
- **検討した代替案(Alternatives)**: 他にどんな選択肢があったか、なぜ採用しなかったか
- **結果・影響(Consequences)**: この判断によって何が決まる/制約されるか
