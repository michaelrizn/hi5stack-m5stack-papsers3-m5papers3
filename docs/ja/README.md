# hi5stack ドキュメント

## 概要

hi5stack は、M5Stack/M5Paper 系 ESP32 デバイス向けの PlatformIO Arduino ファームウェアです。SD カードのファイル管理、TXT/BMP 表示、Wi-Fi、内蔵アプリ、ゲーム、SD Gateway Web インターフェースを備えたタッチ UI を提供します。

このプロジェクトは初期段階です。ビルド成功は、すべての操作が実機で確認済みであることを意味しません。特に PaperS3 の電源オフと消費電力は注意が必要です。

## ビルド

- ビルド: `pio run`
- アップロード: `pio run -t upload`
- シリアルモニター: `pio device monitor -b 115200`

PlatformIO 環境は `PaperS3` です。主要ライブラリのバージョンは `platformio.ini` に固定されています。

## アーキテクチャ

主要コードは `src/` にあります。

- `main.cpp`: 初期化、メインループ、タッチ分配、アニメーション更新、SD Gateway polling。
- `ui.cpp` / `ui.h`: 画面状態、footer ボタン、行バッファ、描画、ナビゲーション、テキスト折り返し。
- `footer.cpp` / `button.cpp`: footer と基本ボタン。
- `sdcard.cpp`: SD 初期化、状態確認、ディレクトリ一覧。
- `settings.cpp`: SD 上の JSON 設定、Wi-Fi 情報。
- `battery.cpp`: 電圧とバッテリー割合。
- `power_shutdown.cpp`: Off と Freeze で使う共通シャットダウン処理。
- `sd_gateway.cpp`: WebServer ベースの SD ファイル UI。

ディレクトリ:

- `screens/`: main、files、TXT/BMP viewer、Wi-Fi、apps、games、SD Gateway、clear、off。
- `apps/`: calculator、reader、geometry test、swipe test、test2、text language test。
- `games/`: minesweeper、sudoku、test game。
- `buttons/`: Home、Files、Refresh、Rotate、Freeze、Off。
- `keyboards/`: English keyboard と numeric keyboard。
- `network/`: Wi-Fi manager。

## 主な操作

main 画面はバッテリー、SD、Wi-Fi、SD Gateway 状態、Apps/Games への入口を表示します。タッチは `main.cpp` で処理され、`currentScreen` に応じて各画面へ渡されます。

file manager は SD のフォルダとファイルを分けてソートし、ページ表示します。`.txt` は text viewer、`.bmp` は image viewer で開きます。

reader は `/books/` の `.txt` を読み込み、ページ化し、読書位置を `/books/reader_state.json` に保存します。

Wi-Fi 画面は `WiFiManager` で非同期スキャンし、パスワード入力には English keyboard を使用します。

SD Gateway は Wi-Fi 接続時のみ起動し、ポート `8080` で SD ファイルの一覧、アップロード、削除、一括削除、安全な `.txt`/`.json` 編集を提供します。

## アプリとゲーム

アプリ: Calculator、Reader、Geometry test、Swipe test、Test2、Text language test。

ゲーム: Minesweeper、6x6 Sudoku、Test game。

## UI 注意点

UI は固定行レイアウトです。`EPD_WIDTH = 540`、`EPD_HEIGHT = 960`、行高 60 px を前提にしています。footer のタッチ領域やページングも固定座標に依存するため、見た目の変更後は実機確認が必要です。

## SD とデバッグ

設定は `/settings.json`、reader の本は `/books/` に保存されます。SD パスは root からの安全なパスとして扱い、`..` などの traversal は拒否します。

デバッグフラグは `src/debug_config.h` で管理します。通常ビルドでは verbose debug を無効にし、必要な対象だけを有効化してください。

## 既知のリスク

- 電源オフ処理は board/PMIC に依存します。
- SD Gateway には認証がないため、信頼できるローカルネットワークでのみ使ってください。
- 大きなファイルは ESP32 の heap を圧迫する可能性があります。
