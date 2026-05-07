#include "files_screen.h"
#include "../ui.h"
#include "../sdcard.h"
#include <algorithm>

static int currentPage = 0;
static const int itemsPerPage = 9;
static int totalPages = 1;

namespace screens {

    template <typename T>
    T customClamp(T value, T minVal, T maxVal) {
        return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
    }


    String getLastFolder(const String& path) {
        if (path == "/") return "/";
        String cleanPath = path;
        if (cleanPath.endsWith("/")) {
            cleanPath = cleanPath.substring(0, cleanPath.length() - 1);
        }
        int lastSlash = cleanPath.lastIndexOf('/');
        if (lastSlash >= 0) {
            return cleanPath.substring(lastSlash + 1);
        }
        return cleanPath;
    }


    void resetPagination() {
        currentPage = 0;
    }

    void drawFilesScreen() {


        displayedFilesCount = 0;
        for (int i = 0; i < MAX_DISPLAYED_FILES; i++) {
            displayedFiles[i] = "";
        }
        bufferRow("Files Manager", 2, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        String lastFolder = getLastFolder(currentPath);
        bufferRow("Path: " + lastFolder, 3, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);


        if (currentPath != "/") {
            bufferRow("...", 4, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);
        } else {
            bufferRow("", 4, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        }

        for (int row = 5; row <= 13; ++row) {
            bufferRow("", row, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        }


        String pathToOpen = currentPath;
        if (pathToOpen.length() > 1 && pathToOpen.endsWith("/")) {
            pathToOpen = pathToOpen.substring(0, pathToOpen.length() - 1);
        }
        File root = SD.open(pathToOpen);

        if (!root) {
            bufferRow("No files found", 5, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
            return;
        }

        static String folders[MAX_DISPLAYED_FILES];
        static String files[MAX_DISPLAYED_FILES];
        int foldersCount = 0;
        int filesCount = 0;
        while (true) {
            File file = root.openNextFile();
            if (!file) break;

            String filename = file.name();
            if (filename.startsWith(".") || filename == "settings.json") {
                file.close();
                continue;
            }

            if (file.isDirectory()) {
                if (foldersCount < MAX_DISPLAYED_FILES) {
                    folders[foldersCount++] = filename + "/";
                }
            } else {
                if (filesCount < MAX_DISPLAYED_FILES) {
                    files[filesCount++] = filename;
                }
            }
            file.close();
        }
        root.close();

        std::sort(folders, folders + foldersCount);
        std::sort(files, files + filesCount);

        displayedFilesCount = 0;

        for (int i = 0; i < foldersCount && displayedFilesCount < MAX_DISPLAYED_FILES; i++) {
            displayedFiles[displayedFilesCount++] = folders[i];
        }

        for (int i = 0; i < filesCount && displayedFilesCount < MAX_DISPLAYED_FILES; i++) {
            displayedFiles[displayedFilesCount++] = files[i];
        }


        totalPages = (displayedFilesCount + itemsPerPage - 1) / itemsPerPage;
        if (totalPages < 1) {
            totalPages = 1;
        }


        if (currentPage >= totalPages) {
            currentPage = totalPages - 1;
        }
        if (currentPage < 0) {
            currentPage = 0;
        }


        int startIdx = currentPage * itemsPerPage;
        int endIdx = std::min(startIdx + itemsPerPage, displayedFilesCount);
        for (int i = startIdx; i < endIdx; ++i) {
            bool isFolder = displayedFiles[i].endsWith("/");
            bufferRow(displayedFiles[i], 5 + (i - startIdx), TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);
        }


        if (totalPages > 1) {
            String paginationButtons[] = {"<<<--", "<--", String(currentPage + 1), "-->", "-->>>"};  
            int numButtons = 5;
            int sectionWidth = EPD_WIDTH / numButtons;
            RowPosition pos = getRowPosition(14);
            int underlineY = pos.y + 30;

            for(int i = 0; i < numButtons; ++i){
                String btn = paginationButtons[i];
                int btnX = pos.x + i * sectionWidth + 10;
                int btnY = pos.y + (pos.height - 20) / 2;
                M5.Display.setCursor(btnX, btnY);
                M5.Display.print(btn);
                int textWidth = M5.Display.textWidth(btn.c_str());
                int centeredUnderlineY = pos.y + pos.height - 10;
                M5.Display.drawLine(btnX, centeredUnderlineY, btnX + textWidth, centeredUnderlineY, TFT_BLACK);
            }
        }
    }

    void handlePagination(String button) {
        if (button == "<<<--") {
            if (currentPage != 0) {
                currentPage = 0;
                displayMessage("First page pressed");
                renderCurrentScreen();
                clearMessage();
            }
        }
        else if (button == "<--") {
            if (currentPage > 0) {
                currentPage--;
                displayMessage("Previous page pressed");
                renderCurrentScreen();
                clearMessage();
            }
        }
        else if (button == "-->") {
            if (currentPage < totalPages - 1) {
                currentPage++;
                displayMessage("Next page pressed");
                renderCurrentScreen();
                clearMessage();
            }
        }
        else if (button == "-->>>") {
            if (currentPage != totalPages - 1) {
                currentPage = totalPages - 1;
                displayMessage("Last page pressed");
                renderCurrentScreen();
                clearMessage();
            }
        }
    }

    void selectFile(const String& filename) {
        if (filename == "...") {
            navigateUp();
        } else if (filename.endsWith("/")) {
            navigateTo(currentPath + filename);
        } else {

            if (filename.endsWith(".txt")) {
                navigateTo(currentPath + filename);
                currentScreen = TXT_VIEWER_SCREEN;
                renderCurrentScreen();
            }

            else if (filename.endsWith(".bmp")) {
                navigateTo(currentPath + filename);
                currentScreen = IMG_VIEWER_SCREEN;
                renderCurrentScreen();
            }
            else {

                displayMessage("Unsupported file type");
                clearMessage();
            }
        }
    }

    void handleTouch(int touchRow, int touchX, int touchY) {
        if (currentScreen == FILES_SCREEN) {
            if (touchRow >= 5 && touchRow <= 13) {
                int index = currentPage * itemsPerPage + (touchRow - 5);
                if (index < displayedFilesCount) {
                    String selected = displayedFiles[index];
                    selectFile(selected);
                }
            } else if (touchRow == 4) {
                selectFile("...");
            } else if (touchRow == 14) {
                int width = EPD_WIDTH;

                int sectionWidth = width / 5;

                int firstBtnCenterX = sectionWidth / 2;
                int firstBtnStartX = customClamp(firstBtnCenterX - (int)(sectionWidth * 0.75), 0, width);
                int firstBtnEndX = customClamp(firstBtnCenterX + (int)(sectionWidth * 0.75), 0, width);

                int secondBtnCenterX = 3 * sectionWidth / 2;
                int secondBtnStartX = customClamp(secondBtnCenterX - (int)(sectionWidth * 0.75), 0, width);
                int secondBtnEndX = customClamp(secondBtnCenterX + (int)(sectionWidth * 0.75), 0, width);

                int thirdBtnCenterX = 5 * sectionWidth / 2;
                int thirdBtnStartX = customClamp(thirdBtnCenterX - (int)(sectionWidth * 0.75), 0, width);
                int thirdBtnEndX = customClamp(thirdBtnCenterX + (int)(sectionWidth * 0.75), 0, width);

                int fourthBtnCenterX = 7 * sectionWidth / 2;
                int fourthBtnStartX = customClamp(fourthBtnCenterX - (int)(sectionWidth * 0.75), 0, width);
                int fourthBtnEndX = customClamp(fourthBtnCenterX + (int)(sectionWidth * 0.75), 0, width);

                int fifthBtnCenterX = 9 * sectionWidth / 2;
                int fifthBtnStartX = customClamp(fifthBtnCenterX - (int)(sectionWidth * 0.75), 0, width);
                int fifthBtnEndX = customClamp(fifthBtnCenterX + (int)(sectionWidth * 0.75), 0, width);


                if (touchX >= firstBtnStartX && touchX <= firstBtnEndX) {
                    handlePagination("<<<--");
                }
                else if (touchX >= secondBtnStartX && touchX <= secondBtnEndX) {
                    handlePagination("<--");
                }
                else if (touchX >= fourthBtnStartX && touchX <= fourthBtnEndX) {
                    handlePagination("-->");
                }
                else if (touchX >= fifthBtnStartX && touchX <= fifthBtnEndX) {
                    handlePagination("-->>>");
                }
            }
        }
    }
}
