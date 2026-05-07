#include "img_viewer_screen.h"
#include "../ui.h"
#include "../sdcard.h"
#include "../buttons/rotate.h"

enum ImageFormat {
    FORMAT_UNKNOWN,
    FORMAT_BMP
};

struct ImageBounds {
    int left;
    int top;
    int width;
    int height;
};

struct BmpInfo {
    int32_t width;
    int32_t height;
    uint32_t dataOffset;
    uint32_t rowSize;
    bool topDown;
};

ImageFormat getImageFormat(const String& filename) {
    String ext = filename.substring(filename.lastIndexOf('.') + 1);
    ext.toLowerCase();

    if (ext == "bmp") return FORMAT_BMP;

    return FORMAT_UNKNOWN;
}

static uint16_t readLe16(const uint8_t* data, size_t offset) {
    return data[offset] | (data[offset + 1] << 8);
}

static uint32_t readLe32(const uint8_t* data, size_t offset) {
    return static_cast<uint32_t>(data[offset]) |
           (static_cast<uint32_t>(data[offset + 1]) << 8) |
           (static_cast<uint32_t>(data[offset + 2]) << 16) |
           (static_cast<uint32_t>(data[offset + 3]) << 24);
}

static int32_t readLe32Signed(const uint8_t* data, size_t offset) {
    return static_cast<int32_t>(readLe32(data, offset));
}

namespace screens {
    static String currentImgOpened = "";

    static constexpr int frameLeft = 0;
    static constexpr int frameTop = 100;
    static constexpr int frameRight = 540;
    static constexpr int frameBottom = 700;
    static constexpr size_t MAX_IMAGE_FILE_SIZE = 8 * 1024 * 1024;

    static void drawImageError(const String& message, const ImageBounds& bounds) {
        ::setUniversalFont();
        M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
        M5.Display.drawString(message, bounds.left + 20, bounds.top + 20);
    }

    static bool parseBmpInfo(const uint8_t* data, size_t size, BmpInfo& info) {
        if (size < 54 || data[0] != 'B' || data[1] != 'M') {
            return false;
        }

        uint32_t dataOffset = readLe32(data, 10);
        uint32_t dibHeaderSize = readLe32(data, 14);
        int32_t width = readLe32Signed(data, 18);
        int32_t height = readLe32Signed(data, 22);
        uint16_t planes = readLe16(data, 26);
        uint16_t bitsPerPixel = readLe16(data, 28);
        uint32_t compression = readLe32(data, 30);

        if (dibHeaderSize < 40 || planes != 1 || bitsPerPixel != 24 || compression != 0) {
            return false;
        }
        if (width <= 0 || height == 0 || dataOffset >= size) {
            return false;
        }

        int32_t absHeight = height < 0 ? -height : height;
        uint32_t rowSize = ((24U * static_cast<uint32_t>(width) + 31U) / 32U) * 4U;
        uint64_t pixelBytes = static_cast<uint64_t>(rowSize) * static_cast<uint32_t>(absHeight);
        if (pixelBytes == 0 || static_cast<uint64_t>(dataOffset) + pixelBytes > size) {
            return false;
        }

        info.width = width;
        info.height = absHeight;
        info.dataOffset = dataOffset;
        info.rowSize = rowSize;
        info.topDown = height < 0;
        return true;
    }

    static bool scaleBmpImage(const uint8_t* srcData, const BmpInfo& info, uint16_t* destBuffer, int destWidth, int destHeight) {
        if (!srcData || !destBuffer || destWidth <= 0 || destHeight <= 0 || info.width <= 0 || info.height <= 0) {
            return false;
        }

        for (int y = 0; y < destHeight; y++) {
            for (int x = 0; x < destWidth; x++) {
                int srcX = x * info.width / destWidth;
                int sampledY = y * info.height / destHeight;
                int srcY = info.topDown ? sampledY : (info.height - 1 - sampledY);

                srcX = constrain(srcX, 0, static_cast<int>(info.width - 1));
                srcY = constrain(srcY, 0, static_cast<int>(info.height - 1));

                size_t srcIndex = static_cast<size_t>(srcY) * info.rowSize + static_cast<size_t>(srcX) * 3;
                uint8_t blue = srcData[srcIndex];
                uint8_t green = srcData[srcIndex + 1];
                uint8_t red = srcData[srcIndex + 2];
                destBuffer[y * destWidth + x] = M5.Display.color565(red, green, blue);
            }
        }

        return true;
    }

    static bool renderBmpFile(const String& filename, const ImageBounds& bounds, bool useClip) {
        File file = SD.open(filename, FILE_READ);
        if (!file) {
            drawImageError("Error opening file", bounds);
            return false;
        }

        size_t fileSize = file.size();
        if (fileSize == 0 || fileSize > MAX_IMAGE_FILE_SIZE) {
            drawImageError("Invalid file size", bounds);
            file.close();
            return false;
        }

        uint8_t* fileData = static_cast<uint8_t*>(malloc(fileSize));
        if (!fileData) {
            drawImageError("Out of memory", bounds);
            file.close();
            return false;
        }

        size_t bytesRead = file.read(fileData, fileSize);
        file.close();
        if (bytesRead != fileSize) {
            drawImageError("Error reading file", bounds);
            free(fileData);
            return false;
        }

        BmpInfo info{};
        if (!parseBmpInfo(fileData, fileSize, info)) {
            drawImageError("Invalid BMP file", bounds);
            free(fileData);
            return false;
        }

        float scale = min(static_cast<float>(bounds.width) / info.width, static_cast<float>(bounds.height) / info.height);
        int scaledWidth = min(static_cast<int>(floor(info.width * scale)), bounds.width);
        int scaledHeight = min(static_cast<int>(floor(info.height * scale)), bounds.height);
        if (scaledWidth <= 0 || scaledHeight <= 0) {
            drawImageError("Invalid image size", bounds);
            free(fileData);
            return false;
        }

        uint64_t pixels = static_cast<uint64_t>(scaledWidth) * static_cast<uint64_t>(scaledHeight);
        uint64_t bufferBytes = pixels * sizeof(uint16_t);
        if (bufferBytes == 0 || bufferBytes > SIZE_MAX) {
            drawImageError("Image too large", bounds);
            free(fileData);
            return false;
        }

        uint16_t* scaledBuffer = static_cast<uint16_t*>(malloc(static_cast<size_t>(bufferBytes)));
        if (!scaledBuffer) {
            drawImageError("Out of memory for scaling", bounds);
            free(fileData);
            return false;
        }

        const uint8_t* pixelData = fileData + info.dataOffset;
        bool success = scaleBmpImage(pixelData, info, scaledBuffer, scaledWidth, scaledHeight);
        if (success) {
            int posX = bounds.left + (bounds.width - scaledWidth) / 2;
            int posY = bounds.top + (bounds.height - scaledHeight) / 2;
            if (useClip) {
                M5.Display.setClipRect(bounds.left, bounds.top, bounds.width, bounds.height);
            }
            M5.Display.pushImage(posX, posY, scaledWidth, scaledHeight, scaledBuffer);
            if (useClip) {
                M5.Display.setClipRect(0, 0, EPD_WIDTH, EPD_HEIGHT);
            }
        } else {
            drawImageError("Error displaying image", bounds);
        }

        free(scaledBuffer);
        free(fileData);
        return success;
    }

    static void displayImageFileInBounds(const String& filename, const ImageBounds& bounds, bool useClip) {
        ImageFormat format = getImageFormat(filename);
        if (format == FORMAT_UNKNOWN) {
            drawImageError("Unsupported file format", bounds);
            return;
        }

        if (format == FORMAT_BMP && !renderBmpFile(filename, bounds, useClip)) {
            return;
        }
    }

    void drawImgViewerScreen(const String& filename) {
        currentImgOpened = filename;

        M5.Display.fillScreen(TFT_WHITE);
        M5.Display.drawRect(frameLeft, frameTop, frameRight - frameLeft, frameBottom - frameTop, TFT_BLACK);

        displayImgFile(filename);

        FooterButton viewerFooterButtons[] = {
            {"Home", homeAction},
            {"180°", rotateImg180Action},
            {"Freeze", freezeAction},
            {"Files", filesAction}
        };
        footer.setButtons(viewerFooterButtons, 4);

        ::drawRowsBuffered();
        footer.draw(footer.isVisible());
        M5.Display.display();
    }

    void displayImgFile(const String& filename) {
        ImageBounds bounds{frameLeft, frameTop, frameRight - frameLeft, frameBottom - frameTop};
        displayImageFileInBounds(filename, bounds, true);
    }

    void clearImgViewerScreen() {
        M5.Display.fillScreen(TFT_WHITE);
        M5.Display.display();
    }

    String getCurrentImgFile() {
        return screens::currentImgOpened;
    }

    void displayFullScreenImgFile(const String& filename) {
        ImageBounds bounds{0, 0, EPD_WIDTH, EPD_HEIGHT};
        displayImageFileInBounds(filename, bounds, false);
    }

    void setupImgViewerButtons() {
        FooterButton viewerFooterButtons[] = {
            {"Home", homeAction},
            {"180°", rotateImg180Action},
            {"Freeze", freezeAction},
            {"Files", filesAction}
        };
        footer.setButtons(viewerFooterButtons, 4);
    }

    void setupImgViewerRotateButtons() {
        FooterButton rotateButtons[] = {
            {"Home", homeAction},
            {"180°", rotateImg180Action},
            {"Freeze", freezeAction},
            {"Files", filesAction}
        };
        footer.setButtons(rotateButtons, 4);
    }
}
