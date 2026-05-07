# Audit Notes

## Verified Baseline

- `pio run` succeeds for the `PaperS3` environment.
- Current build size before refactoring: about 20.7% RAM and 20.3% Flash.

## Areas Reviewed

- SD Gateway file operations and generated HTML/JSON.
- Image viewer memory allocation and BMP parsing.
- Reader file loading and pagination state.
- File manager sorting and pagination.
- Debug macro defaults.
- Existing power shutdown changes in `power_shutdown.*`, `off.cpp`, `freeze.cpp`, and `off_screen.cpp`.

## Important Risks

- SD Gateway is unauthenticated. Use it only on trusted local Wi-Fi.
- UI layout depends on fixed screen coordinates. Hardware checks are required after visual changes.
- Large files and full-image buffers can stress ESP32 heap memory.
- Power-off behavior needs device validation because it depends on board and PMIC behavior.

## Recommended Manual Checks

- Main screen status rows.
- Files screen navigation and pagination.
- TXT viewer and BMP viewer.
- Reader book list, page navigation, and saved position.
- Wi-Fi scan, password entry, and reconnect.
- SD Gateway list/upload/delete/edit.
- Calculator, minesweeper, sudoku, and test game launch.
