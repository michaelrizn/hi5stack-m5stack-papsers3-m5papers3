# hi5stack 文档

## 概述

hi5stack 是面向 M5Stack/M5Paper ESP32 设备的 PlatformIO Arduino 固件项目。它提供基于触摸的模块化界面，支持 SD 卡文件管理、TXT/BMP 查看、Wi-Fi、内置应用、游戏以及 SD Gateway 网页文件界面。

项目仍处于早期阶段。编译成功不代表所有功能都已在硬件上验证，尤其是 PaperS3 的关机和耗电行为。

## 构建与开发

- 构建: `pio run`
- 上传: `pio run -t upload`
- 串口监视器: `pio device monitor -b 115200`

当前 PlatformIO 环境是 `PaperS3`。库版本固定在 `platformio.ini` 中，包括 M5Unified、M5GFX、ArduinoJson、AnimatedGIF 和指定 commit 的 epdiy。

## 架构

主要代码位于 `src/`。

- `main.cpp`: 初始化、主循环、触摸分发、动画更新、SD Gateway 循环。
- `ui.cpp` / `ui.h`: 全局屏幕状态、footer 按钮、行缓冲、渲染、导航、文本换行。
- `footer.cpp` / `button.cpp`: 底部按钮栏和通用按钮。
- `sdcard.cpp`: SD 初始化、挂载状态、目录列表。
- `settings.cpp`: SD 上的 JSON 设置，包括 Wi-Fi 信息。
- `battery.cpp`: 电压和电量百分比。
- `power_shutdown.cpp`: Off 和 Freeze 共用的关机流程。
- `sd_gateway.cpp`: 基于 WebServer 的 SD 文件网页界面。

目录分组:

- `screens/`: 主屏、文件、TXT/BMP 查看器、Wi-Fi、应用、游戏、SD Gateway、清屏、关机。
- `apps/`: 计算器、reader、geometry test、swipe test、test2、text language test。
- `games/`: 扫雷、数独、测试游戏。
- `buttons/`: Home、Files、Refresh、Rotate、Freeze、Off。
- `keyboards/`: 英文和数字屏幕键盘。
- `network/`: Wi-Fi 管理器。

## 主要流程

主屏显示电池、SD、Wi-Fi、SD Gateway 状态，并提供 Apps/Games 入口。触摸在 `main.cpp` 中处理，然后根据 `currentScreen` 分发给对应屏幕。

文件管理器读取 SD 内容，将文件夹和文件分组排序并分页显示。`.txt` 使用文本查看器打开，`.bmp` 使用图片查看器打开。

Reader 使用 `/books/`，读取 `.txt`，分页显示，并把阅读位置保存到 `/books/reader_state.json`。

Wi-Fi 屏幕通过 `WiFiManager` 异步扫描网络，密码输入使用英文键盘。

SD Gateway 只在 Wi-Fi 已连接时启动，在 `8080` 端口提供文件列表、上传、删除、批量删除，以及安全 `.txt`/`.json` 编辑。

## 应用和游戏

应用:

- Calculator: 基本算术和 AC。
- Reader: 书籍列表、分页、阅读位置保存。
- Geometry test: 随机几何图形动画。
- Swipe test: 触摸和滑动跟踪。
- Test2: 最小测试应用。
- Text language test: 英文、俄文、日文、中文显示测试。

游戏:

- Minesweeper: 10x15 网格，25 个雷。
- Sudoku: 6x6 数独，数字键盘输入和验证。
- Test game: 简单绘制和触摸测试。

## UI 注意事项

界面使用固定行布局: `EPD_WIDTH = 540`，`EPD_HEIGHT = 960`，行高 60 px。很多屏幕、footer 触摸区域和分页都依赖这些坐标。修改 UI 后需要在设备上验证。

## SD 和调试

设置文件为 `/settings.json`，reader 书籍位于 `/books/`。SD 路径必须是安全的根路径，拒绝 `..` 等目录穿越。

调试开关在 `src/debug_config.h` 中。普通构建应关闭详细调试，只在排查触摸、键盘、文件、Wi-Fi 或 SD Gateway 时启用对应开关。

## 已知风险

- 关机行为依赖板卡和 PMIC，需要硬件验证。
- SD Gateway 没有认证，只适合可信本地网络。
- 大文件可能造成 ESP32 heap 压力或碎片化。
