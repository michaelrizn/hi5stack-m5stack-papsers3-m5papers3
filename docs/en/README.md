# hi5stack Documentation

## Overview

hi5stack is a PlatformIO Arduino firmware project for M5Stack/M5Paper ESP32 devices. It provides a modular touch interface with SD card file management, text and BMP viewing, Wi-Fi scanning/connection, an SD Gateway web file interface, built-in applications, and built-in games.

The project is early and experimental. Build success does not mean every workflow is hardware-verified, especially power-off behavior on PaperS3 boards.

## Build and Development

- Build: `pio run`
- Upload: `pio run -t upload`
- Serial monitor: `pio device monitor -b 115200`

The active PlatformIO environment is `PaperS3`. Library versions are pinned in `platformio.ini`, including M5Unified, M5GFX, ArduinoJson, AnimatedGIF, and a fixed epdiy commit.

## Architecture

Core firmware code lives in `src/`.

- `main.cpp`: setup, loop, touch dispatch, animation updates, SD Gateway loop polling.
- `ui.cpp` / `ui.h`: global screen state, footer sets, row buffering, rendering, navigation helpers, text wrapping.
- `footer.cpp` / `button.cpp`: footer actions and generic button rendering/touch handling.
- `sdcard.cpp`: SD initialization, mount status, directory listing.
- `settings.cpp`: JSON-backed settings on SD, including Wi-Fi credentials and last connection data.
- `battery.cpp`: voltage and percentage reporting.
- `power_shutdown.cpp`: shared shutdown sequence used by Off and Freeze actions.
- `sd_gateway.cpp`: WebServer-based file interface for the SD card.

Feature code is grouped by role:

- `screens/`: main, files, text viewer, image viewer, Wi-Fi, apps, games, SD Gateway, clear, and off screens.
- `apps/`: calculator, reader, geometry test, swipe test, test2, text language test.
- `games/`: minesweeper, sudoku, test game.
- `buttons/`: footer actions for home, files, refresh, rotate, freeze, and off.
- `keyboards/`: English and numeric on-screen keyboards.
- `network/`: Wi-Fi scanning and connection manager.
- `services/`: reserved for future service modules.

## Main Workflows

The main screen shows battery, SD card status, Wi-Fi status, SD Gateway status, and links to Apps and Games. Touch handling is centralized in `main.cpp` and routed by `currentScreen`.

The file manager reads SD card entries, separates folders and files, sorts each group, paginates the visible list, and opens `.txt` files in the text viewer or `.bmp` files in the image viewer.

The reader app uses `/books/` on the SD card, reads `.txt` files, paginates text, and stores reading position in `/books/reader_state.json`.

The Wi-Fi screen scans networks asynchronously through `WiFiManager`, opens a password screen with the English keyboard, and can reconnect using saved credentials.

The SD Gateway starts only when Wi-Fi is connected. It exposes a browser UI on port `8080` for listing, uploading, deleting, batch deleting, and editing safe text/JSON files on the SD card.

## Apps and Games

Applications:

- Calculator: basic arithmetic operations and AC.
- Reader: text book list, pagination, and saved reading position.
- Geometry test: animated random shapes.
- Swipe test: touch/swipe tracking.
- Test2: minimal test app.
- Text language test: English, Russian, Japanese, and Chinese font rendering sample.

Games:

- Minesweeper: 10x15 grid with 25 mines.
- Sudoku: 6x6 puzzle with numeric keyboard input and validation.
- Test game: simple drawing/touch test.

## UI Notes

The UI is row-based: `EPD_WIDTH = 540`, `EPD_HEIGHT = 960`, row height is 60 pixels, and the footer is drawn near the bottom of the screen. Many screens assume these constants. Change layout carefully and verify on device because text placement, footer touch areas, and pagination use fixed coordinates.

For visible changes, verify at least the main screen, file manager, text viewer, image viewer, reader, Wi-Fi screen, calculator, minesweeper, sudoku, and SD Gateway status screen.

## SD Card and Files

The SD card is initialized in `setup()` using configured SPI pins from `sdcard.h`. Settings are stored in `/settings.json`. Reader books live in `/books/`. File operations should treat SD paths as rooted paths and reject traversal such as `..`.

Supported user-facing file types:

- `.txt`: text viewer and reader.
- `.bmp`: image viewer.
- `.json`: SD Gateway editing for safe files.

## Debugging

Debug flags are controlled in `src/debug_config.h`. Keep verbose debug output disabled by default for normal builds, and enable targeted flags while investigating touch, keyboard, file, Wi-Fi, or SD Gateway behavior.

## Known Risks and Improvement Areas

- Power-off behavior depends on board/PMIC behavior and needs device validation.
- SD Gateway has no authentication and should only be used on trusted local networks.
- The UI relies on fixed dimensions and must be tested on hardware after layout changes.
- Large files can fragment heap memory on ESP32-class devices; keep file limits conservative.
