# hi5stack दस्तावेज़

## परिचय

hi5stack M5Stack/M5Paper ESP32 डिवाइसों के लिए PlatformIO Arduino firmware प्रोजेक्ट है। इसमें SD card file management, TXT/BMP viewer, Wi-Fi, built-in apps, games और SD Gateway web interface वाला touch UI है।

यह प्रोजेक्ट अभी शुरुआती अवस्था में है। Build सफल होना यह साबित नहीं करता कि हर workflow hardware पर verify हो चुका है, खासकर PaperS3 power-off और battery drain behavior।

## Build और Development

- Build: `pio run`
- Upload: `pio run -t upload`
- Serial monitor: `pio device monitor -b 115200`

Active PlatformIO environment `PaperS3` है। Library versions `platformio.ini` में pinned हैं: M5Unified, M5GFX, ArduinoJson, AnimatedGIF और fixed epdiy commit।

## Architecture

मुख्य code `src/` में है।

- `main.cpp`: setup, loop, touch dispatch, animation updates, SD Gateway polling।
- `ui.cpp` / `ui.h`: screen state, footer buttons, row buffering, rendering, navigation, word wrap।
- `footer.cpp` / `button.cpp`: footer और generic button।
- `sdcard.cpp`: SD initialization, mount status, directory listing।
- `settings.cpp`: SD पर JSON settings, Wi-Fi data।
- `battery.cpp`: voltage और battery percentage।
- `power_shutdown.cpp`: Off और Freeze actions के लिए common shutdown।
- `sd_gateway.cpp`: WebServer based SD file interface।

Folders:

- `screens/`: main, files, TXT/BMP viewer, Wi-Fi, apps, games, SD Gateway, clear, off।
- `apps/`: calculator, reader, geometry test, swipe test, test2, text language test।
- `games/`: minesweeper, sudoku, test game।
- `buttons/`: Home, Files, Refresh, Rotate, Freeze, Off।
- `keyboards/`: English और numeric on-screen keyboards।
- `network/`: Wi-Fi manager।

## मुख्य Workflows

Main screen battery, SD card, Wi-Fi, SD Gateway status और Apps/Games links दिखाती है। Touch handling `main.cpp` में होता है और `currentScreen` के आधार पर सही screen को भेजा जाता है।

File manager SD entries पढ़ता है, folders और files अलग करता है, sort करता है, pagination दिखाता है, `.txt` को text viewer और `.bmp` को image viewer में खोलता है।

Reader app `/books/` folder इस्तेमाल करता है, `.txt` files पढ़ता है, text को pages में बांटता है और reading position `/books/reader_state.json` में save करता है।

Wi-Fi screen `WiFiManager` से asynchronous scan करती है और password input के लिए English keyboard खोलती है।

SD Gateway Wi-Fi connected होने पर ही start होता है। Port `8080` पर browser UI file list, upload, delete, batch delete और safe `.txt`/`.json` editing देता है।

## Apps और Games

Apps:

- Calculator: basic arithmetic और AC।
- Reader: book list, pagination, saved reading position।
- Geometry test: animated random shapes।
- Swipe test: touch/swipe tracking।
- Test2: minimal test app।
- Text language test: English, Russian, Japanese और Chinese rendering sample।

Games:

- Minesweeper: 10x15 grid और 25 mines।
- Sudoku: 6x6 puzzle with numeric keyboard।
- Test game: simple drawing/touch test।

## UI Notes

UI fixed row layout पर आधारित है: `EPD_WIDTH = 540`, `EPD_HEIGHT = 960`, row height 60 px। Footer touch areas और pagination भी fixed coordinates पर निर्भर हैं। Layout changes के बाद hardware पर verify करना जरूरी है।

## SD Card और Debugging

Settings `/settings.json` में हैं। Reader books `/books/` में रखता है। SD paths को root-based safe paths की तरह handle करें और `..` जैसे traversal reject करें।

Debug flags `src/debug_config.h` में हैं। Normal build में verbose debug off रखें और investigation के समय targeted flags enable करें।

## Known Risks

- Power-off behavior board/PMIC पर निर्भर है और device validation मांगता है।
- SD Gateway में authentication नहीं है, इसलिए इसे केवल trusted local network पर use करें।
- Large files ESP32 heap pressure और fragmentation पैदा कर सकती हैं।
