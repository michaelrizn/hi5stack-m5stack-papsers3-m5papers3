# Repository Guidelines

## Project Structure & Module Organization

This is a PlatformIO Arduino project for M5Stack/M5Paper ESP32 devices. Main firmware code lives in `src/`. Core modules such as `main.cpp`, `ui.cpp`, `sdcard.cpp`, `settings.cpp`, and `battery.cpp` sit at the top of `src/`. Feature code is grouped by role: `src/screens/` for UI screens, `src/buttons/` for footer/action buttons, `src/apps/` for built-in applications, `src/games/` for games, `src/keyboards/` for on-screen keyboards, and `src/network/` for Wi-Fi support.

## Build, Test, and Development Commands

- `pio run`: build the PaperS3 firmware using `platformio.ini`.
- `pio run -t upload`: build and upload firmware to the connected device.
- `pio device monitor -b 115200`: open the serial monitor at the configured speed.

## Coding Style & Naming Conventions

Use C++17-compatible Arduino code. Match the existing style: 4-space indentation, braces on the same line, `snake_case` for globals and free functions where already used, and PascalCase for classes such as `Settings`. Keep paired headers and implementations together as `name.h` and `name.cpp`. New screens, apps, and games should follow the existing directory pattern, for example `src/apps/my_app/app_screen.cpp` or `src/games/my_game/game.cpp`.

## Testing Guidelines

There is no dedicated unit test framework in this repository. Treat `pio run` as the required compile check before submitting changes. For UI or game changes, verify behavior on device when possible.

## Commit & Pull Request Guidelines

Recent history uses short release-style commits such as `0.40` and occasional conventional messages like `feat(swipe_test): ...` or `refactor: ...`. Prefer clear, scoped commits: `feat(reader): add page jump`, `fix(sd): handle missing card`, or `refactor(ui): simplify footer redraw`. Pull requests should describe the changed firmware behavior, mention hardware tested, include build results, and attach screenshots for visible UI changes.

## Security & Configuration Tips

Do not commit local Wi-Fi credentials, SD card contents, or generated temporary files. Keep library versions pinned in `platformio.ini` when updating dependencies, and confirm updates with `pio run`.
