# hi5stack

> **Status:** early firmware. This project was created with help from an LLM, and some behavior still needs hardware verification.
>
> **Known issue:** power-off/deep-sleep behavior is still being improved. Battery drain while the device appears powered off may still occur depending on board and PMIC state.

**hi5stack** is a PlatformIO Arduino firmware project for M5Stack/M5Paper ESP32 devices, focused on a touch UI for SD files, Wi-Fi, built-in apps, games, text/image viewing, and an SD Gateway web interface.

## Documentation

- [English documentation](docs/en/README.md)
- [Русская документация](docs/ru/README.md)
- [日本語ドキュメント](docs/ja/README.md)
- [中文文档](docs/zh/README.md)
- [हिंदी दस्तावेज़](docs/hi/README.md)

## Build

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

## Platform

- Hardware target: M5Stack/M5Paper ESP32 class device, currently configured as `PaperS3`
- Framework: Arduino via PlatformIO
- Main code: `src/`
- Configuration: `platformio.ini`

## Repository Layout

```text
src/
├── apps/       # calculator, reader, gesture/font/geometry tests
├── buttons/    # footer button actions
├── games/      # minesweeper, sudoku, test game
├── keyboards/  # on-screen keyboards
├── network/    # Wi-Fi manager
├── screens/    # main UI screens
└── *.cpp/*.h   # core firmware modules
```
