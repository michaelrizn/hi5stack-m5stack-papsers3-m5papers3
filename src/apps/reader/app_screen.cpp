#include "app_screen.h"
#include "../../ui.h"
#include "../../sdcard.h"
#include <SD.h>
#include <algorithm>
#include <ArduinoJson.h>

namespace apps_reader {
    const int MAX_DISPLAYED_FILES = 50;
    

    const int WORK_AREA_X = 0;
    const int WORK_AREA_Y = 120;
    const int WORK_AREA_WIDTH = 540;
    const int WORK_AREA_BOTTOM = 845;
    
    const int READER_ROW_HEIGHT = 50;
    const int LINES_PER_PAGE = (WORK_AREA_BOTTOM - WORK_AREA_Y - READER_ROW_HEIGHT) / READER_ROW_HEIGHT;
    const int CHARS_PER_LINE = 45;
    const int MAX_FILE_SIZE = 50000;
    

    static bool initialized = false;
    static bool fileIsOpen = false;
    static String currentFileName = "";
    static String fileContent = "";
    static String* pages = nullptr;
    static int totalPagesCount = 0;
    static int currentPageIndex = 0;
    static bool showingFileList = true;
    

    static String bookFiles[MAX_DISPLAYED_FILES];
    static int bookFilesCount = 0;
    static int currentPage = 0;
    static const int itemsPerPage = 9;
    static int totalPages = 1;
    

    const String STATE_FILE_PATH = "/books/reader_state.json";
    

    struct ReaderRowPosition {
        int x;
        int y;
        int width;
        int height;
    };
    
    ReaderRowPosition getReaderRowPosition(int row) {
        int y = WORK_AREA_Y + (row * READER_ROW_HEIGHT);

        if (y + READER_ROW_HEIGHT > WORK_AREA_BOTTOM) {
            y = WORK_AREA_BOTTOM - READER_ROW_HEIGHT;
        }
        return ReaderRowPosition{WORK_AREA_X, y, WORK_AREA_WIDTH, READER_ROW_HEIGHT};
    }
    
    void drawReaderRow(const String& text, int row, uint16_t textColor = TFT_BLACK, uint16_t bgColor = TFT_WHITE) {
        ReaderRowPosition pos = getReaderRowPosition(row);
        M5.Display.fillRect(pos.x, pos.y, pos.width, pos.height, bgColor);
        

        M5.Display.setFont(UNIVERSAL_FONT);
        M5.Display.setTextDatum(middle_left);
        M5.Display.setTextColor(textColor, bgColor);
        M5.Display.setTextSize(FONT_SIZE_ALL);
        

        M5.Display.drawString(text, pos.x + 10, pos.y + pos.height / 2);
        

        M5.Display.setTextDatum(top_left);
    }
    

    void saveReadingState() {
        if (!fileIsOpen || currentFileName.isEmpty()) {
            return;
        }
        
        JsonDocument doc;
        

        if (SD.exists(STATE_FILE_PATH)) {
            File stateFile = SD.open(STATE_FILE_PATH);
            if (stateFile) {
                deserializeJson(doc, stateFile);
                stateFile.close();
            }
        }
        

        doc[currentFileName] = currentPageIndex;
        
        if (SD.exists(STATE_FILE_PATH)) {
            SD.remove(STATE_FILE_PATH);
        }

        File stateFile = SD.open(STATE_FILE_PATH, FILE_WRITE);
        if (stateFile) {
            serializeJson(doc, stateFile);
            stateFile.close();
            Serial.println("[Reader] State saved: " + currentFileName + ", page " + String(currentPageIndex + 1));
        } else {
            Serial.println("[Reader] Failed to save state");
        }
    }
    

    int loadReadingStateForFile(const String& filename) {
        if (!SD.exists(STATE_FILE_PATH)) {
            return 0;
        }
        
        File stateFile = SD.open(STATE_FILE_PATH);
        if (!stateFile) {
            return 0;
        }
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, stateFile);
        stateFile.close();
        
        if (error) {
            Serial.println("[Reader] Failed to parse state file");
            return 0;
        }
        
        if (doc[filename].is<int>()) {
            int page = doc[filename].as<int>();
            Serial.println("[Reader] State loaded for " + filename + ": page " + String(page + 1));
            return page;
        }
        
        return 0;
    }
    

    void splitTextIntoPages() {
        if (pages != nullptr) {
            delete[] pages;
            pages = nullptr;
        }
        
        totalPagesCount = 0;
        if (fileContent.length() == 0) return;
        

        ::setUniversalFont();
        M5.Display.setTextSize(FONT_SIZE_ALL);
        

        int estimatedPages = (fileContent.length() / (LINES_PER_PAGE * 50)) + 5;
        if (estimatedPages < 5) estimatedPages = 5;
        if (estimatedPages > 200) estimatedPages = 200;
        
        pages = new String[estimatedPages];
        

        int maxLineWidth = EPD_WIDTH - 40;
        
        int pageIndex = 0;
        int textPos = 0;
        
        while (textPos < fileContent.length() && pageIndex < estimatedPages) {
            String page = "";
            int linesOnPage = 0;
            
            while (linesOnPage < LINES_PER_PAGE && textPos < fileContent.length()) {
    
                String currentLine = "";
                while (textPos < fileContent.length()) {
                    char c = fileContent.charAt(textPos);
                    textPos++;
                    
                    if (c == '\r') {
                        continue;
                    } else if (c == '\n') {
                        break;
                    } else {
                        currentLine += c;
                    }
                }
                
    
                if (currentLine.length() == 0) {
                    continue;
                }
                
    
                String wrappedLines[10];
                int wrappedCount = 0;
                ::wordWrap(currentLine, maxLineWidth, wrappedLines, wrappedCount, 10);
                

                for (int i = 0; i < wrappedCount && linesOnPage < LINES_PER_PAGE; i++) {
                    if (page.length() > 0) page += "\n";
                    page += wrappedLines[i];
                    linesOnPage++;
                }
                
                
                if (wrappedCount > 0 && linesOnPage >= LINES_PER_PAGE) {
                    break;
                }
            }
            
            pages[pageIndex] = page;
            pageIndex++;
        }
        
        totalPagesCount = pageIndex;
    }
    

    void loadBooksList() {
        bookFilesCount = 0;
        for (int i = 0; i < MAX_DISPLAYED_FILES; i++) {
            bookFiles[i] = "";
        }
        
        String booksPath = "/books";
        File booksDir = SD.open(booksPath);
        
        if (!booksDir) {
            return;
        }
        
        while (true) {
            File file = booksDir.openNextFile();
            if (!file) break;
            
            String filename = file.name();
            if (!file.isDirectory() && filename.endsWith(".txt") && bookFilesCount < MAX_DISPLAYED_FILES) {
                bookFiles[bookFilesCount++] = filename;
            }
            file.close();
        }
        booksDir.close();
        

        std::sort(bookFiles, bookFiles + bookFilesCount);
        

        totalPages = (bookFilesCount + itemsPerPage - 1) / itemsPerPage;
        if (totalPages < 1) totalPages = 1;
        if (currentPage >= totalPages) currentPage = totalPages - 1;
        if (currentPage < 0) currentPage = 0;
    }
    
    void ensureBooksFolder() {

        if (!isSDCardMounted()) {
            displayMessage("SD card not available");
            return;
        }
        

        if (!SD.exists("/books")) {
            Serial.println("[Reader] Books folder not found, creating...");
            
    
            if (SD.mkdir("/books")) {
                Serial.println("[Reader] Books folder created successfully");
                displayMessage("Created books folder");
                
        
                if (SD.exists("/books")) {
                    Serial.println("[Reader] Books folder verified");
                } else {
                    Serial.println("[Reader] ERROR: Books folder creation failed verification");
                    displayMessage("Failed to create books folder");
                }
            } else {
                Serial.println("[Reader] ERROR: Failed to create books folder");
                displayMessage("Failed to create books folder");
            }
        } else {
            Serial.println("[Reader] Books folder already exists");
        }
    }
    
    void initApp() {
        ensureBooksFolder();
        fileIsOpen = false;
        showingFileList = true;
        currentPageIndex = 0;
        currentPage = 0;
        loadBooksList();
        


    }
    
    void drawFileListScreen() {
        bufferRow("Reader - Books", 2, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        bufferRow("Select a book to read:", 3, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        bufferRow("", 4, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        

        for (int row = 5; row <= 13; ++row) {
            bufferRow("", row, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        }
        
        if (bookFilesCount == 0) {
            bufferRow("No .txt files found", 5, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
            bufferRow("Place books in /books/", 6, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
            return;
        }
        

        int startIdx = currentPage * itemsPerPage;
        int endIdx = std::min(startIdx + itemsPerPage, bookFilesCount);
        for (int i = startIdx; i < endIdx; ++i) {
            bufferRow(bookFiles[i], 5 + (i - startIdx), TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);
        }
        

        if (totalPages > 1) {
            String paginationButtons[] = {"<<<--", "<--", String(currentPage + 1), "-->", "-->>>"};  
            int numButtons = 5;
            int sectionWidth = EPD_WIDTH / numButtons;
            RowPosition pos = getRowPosition(14);
            
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
    
    void drawReaderScreen() {
        if (!fileIsOpen || currentPageIndex >= totalPagesCount) {
            bufferRow("Error: No file open", 7, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
            return;
        }
        

        ::setUniversalFont();
        

        int workAreaHeight = WORK_AREA_BOTTOM - WORK_AREA_Y;
        M5.Display.fillRect(WORK_AREA_X, WORK_AREA_Y, WORK_AREA_WIDTH, workAreaHeight, TFT_WHITE);
        

        String pageText = pages[currentPageIndex];
        int lineNum = 0;
        int pos = 0;
        const int maxLines = LINES_PER_PAGE;
        

        if (pageText.length() == 0) {
            drawReaderRow("[Empty page]", maxLines / 2);
        } else {
    
            while (pos < pageText.length() && lineNum < maxLines) {

                int lineY = WORK_AREA_Y + (lineNum * READER_ROW_HEIGHT);
                if (lineY + READER_ROW_HEIGHT > WORK_AREA_BOTTOM) {
                    break;
                }
                
                int nextNewline = pageText.indexOf('\n', pos);
                String line;
                
                if (nextNewline == -1) {
                    line = pageText.substring(pos);
                    pos = pageText.length();
                } else {
                    line = pageText.substring(pos, nextNewline);
                    pos = nextNewline + 1;
                }
                
                drawReaderRow(line, lineNum);
                lineNum++;
            }
        }
        

        String pageInfo = String(currentPageIndex + 1) + "/" + String(totalPagesCount);
        String navLine = "<< Prev  Menu " + pageInfo +"  Next >>";
        bufferRow(navLine, 14, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);
    }
    
    void drawAppScreen() {
        if (!initialized) {
            initApp();
            initialized = true;
        }
        
        if (showingFileList) {
            drawFileListScreen();
        } else {
            drawReaderScreen();
        }
    }
    
    void openFile(const String& filename) {
        String filepath = "/books/" + filename;
        File file = SD.open(filepath);
        
        if (!file) {
            displayMessage("Cannot open file");
            return;
        }
        
        if (file.size() > MAX_FILE_SIZE) {
            displayMessage("File too large");
            file.close();
            return;
        }
        
        fileContent = "";
        while (file.available()) {
            fileContent += (char)file.read();
        }
        file.close();
        
        currentFileName = filename;
        splitTextIntoPages();
        
        if (totalPagesCount > 0) {
    
            int savedPage = loadReadingStateForFile(filename);
            if (savedPage >= 0 && savedPage < totalPagesCount) {
                currentPageIndex = savedPage;
            } else {
                currentPageIndex = 0;
            }
            
            fileIsOpen = true;
            showingFileList = false;
            saveReadingState();
        } else {
            displayMessage("Empty file");
        }
    }
    
    void nextPage() {
        if (fileIsOpen && currentPageIndex < totalPagesCount - 1) {
            currentPageIndex++;
            saveReadingState();
        }
    }
    
    void prevPage() {
        if (fileIsOpen && currentPageIndex > 0) {
            currentPageIndex--;
            saveReadingState();
        }
    }
    
    void returnToFileList() {
        showingFileList = true;
        fileIsOpen = false;
        if (pages != nullptr) {
            delete[] pages;
            pages = nullptr;
        }
        loadBooksList();
    }
    
    void handlePagination(String button) {
        if (button == "<<<--") {
            if (currentPage != 0) {
                currentPage = 0;
                drawFileListScreen();
            }
        }
        else if (button == "<--") {
            if (currentPage > 0) {
                currentPage--;
                drawFileListScreen();
            }
        }
        else if (button == "-->") {
            if (currentPage < totalPages - 1) {
                currentPage++;
                drawFileListScreen();
            }
        }
        else if (button == "-->>>") {
            if (currentPage != totalPages - 1) {
                currentPage = totalPages - 1;
                drawFileListScreen();
            }
        }
    }
    
    void handleTouch(int touchRow, int touchX, int touchY) {
        if (showingFileList) {
    
            if (touchRow >= 5 && touchRow <= 13) {
                int index = currentPage * itemsPerPage + (touchRow - 5);
                if (index < bookFilesCount) {
                    openFile(bookFiles[index]);
                }
            } else if (touchRow == 14 && totalPages > 1) {
    
                int sectionWidth = EPD_WIDTH / 5;
                
                if (touchX >= 0 && touchX < sectionWidth) {
                    handlePagination("<<<--");
                } else if (touchX >= sectionWidth && touchX < 2 * sectionWidth) {
                    handlePagination("<--");
                } else if (touchX >= 3 * sectionWidth && touchX < 4 * sectionWidth) {
                    handlePagination("-->");
                } else if (touchX >= 4 * sectionWidth && touchX < 5 * sectionWidth) {
                    handlePagination("-->>>");
                }
            }
        } else {
    
            if (touchRow == 14) {
    
                int navSectionWidth = EPD_WIDTH / 3;
                
                if (touchX >= 0 && touchX < navSectionWidth) {
    
                    prevPage();
                } else if (touchX >= navSectionWidth && touchX < 2 * navSectionWidth) {

                    returnToFileList();
                } else if (touchX >= 2 * navSectionWidth && touchX < 3 * navSectionWidth) {

                    nextPage();
                }
            } else {

                int textStartY = WORK_AREA_Y;
                int textEndY = WORK_AREA_BOTTOM;
                
                if (touchY >= textStartY && touchY < textEndY) {
    
                    if (touchX < EPD_WIDTH / 2) {
    
                        prevPage();
                    } else {
    
                        nextPage();
                    }
                }
            }
        }
    }
    

    bool isFileOpen() {
        return fileIsOpen;
    }
    
    String getCurrentFile() {
        return currentFileName;
    }
    
    int getCurrentPage() {
        return currentPageIndex + 1;
    }
    
    int getTotalPages() {
        return totalPagesCount;
    }
}
