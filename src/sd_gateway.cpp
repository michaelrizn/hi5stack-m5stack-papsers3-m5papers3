#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include "sd_gateway.h"
#include "debug_config.h"
#include "ui.h"

namespace sd_gateway {
    static bool active = false;
    static uint16_t serverPort = 8080;
    static WebServer* server = nullptr;

    bool isActive() { return active; }
    uint16_t getPort() { return serverPort; }

    String htmlEscape(const String& value) {
        String escaped;
        escaped.reserve(value.length());
        for (size_t i = 0; i < value.length(); ++i) {
            char c = value[i];
            if (c == '&') escaped += F("&amp;");
            else if (c == '<') escaped += F("&lt;");
            else if (c == '>') escaped += F("&gt;");
            else if (c == '"') escaped += F("&quot;");
            else if (c == '\'') escaped += F("&#39;");
            else escaped += c;
        }
        return escaped;
    }

    String urlEncode(const String& value) {
        static const char hex[] = "0123456789ABCDEF";
        String encoded;
        encoded.reserve(value.length());
        for (size_t i = 0; i < value.length(); ++i) {
            uint8_t c = static_cast<uint8_t>(value[i]);
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') || c == '-' || c == '_' ||
                c == '.' || c == '~' || c == '/') {
                encoded += static_cast<char>(c);
            } else {
                encoded += '%';
                encoded += hex[c >> 4];
                encoded += hex[c & 0x0F];
            }
        }
        return encoded;
    }

    String jsonEscape(const String& value) {
        String escaped;
        escaped.reserve(value.length());
        for (size_t i = 0; i < value.length(); ++i) {
            char c = value[i];
            if (c == '"' || c == '\\') {
                escaped += '\\';
                escaped += c;
            } else if (c == '\n') {
                escaped += F("\\n");
            } else if (c == '\r') {
                escaped += F("\\r");
            } else if (c == '\t') {
                escaped += F("\\t");
            } else {
                escaped += c;
            }
        }
        return escaped;
    }

    bool normalizePath(const String& input, String& normalized) {
        String path = input;
        path.trim();

        if (path.isEmpty()) return false;
        path.replace('\\', '/');
        if (!path.startsWith("/")) path = "/" + path;
        while (path.indexOf("//") >= 0) {
            path.replace("//", "/");
        }

        if (path == "/" || path.indexOf("/../") >= 0 || path.endsWith("/..") ||
            path.indexOf("/./") >= 0 || path.endsWith("/.") || path.indexOf('\0') >= 0) {
            return false;
        }

        normalized = path;
        return true;
    }

    bool normalizeUploadName(const String& input, String& normalized) {
        String name = input;
        name.trim();
        name.replace('\\', '/');
        int slash = name.lastIndexOf('/');
        if (slash >= 0) {
            name = name.substring(slash + 1);
        }
        if (name.isEmpty() || name == "." || name == ".." || name.indexOf("..") >= 0) {
            return false;
        }
        return normalizePath(name, normalized);
    }

    bool isEditablePath(const String& filename) {
        String lower = filename;
        lower.toLowerCase();
        return lower.endsWith(".txt") || lower.endsWith(".json");
    }

    void handleRoot() {
        String html = "<html><head><title>SD Gateway</title></head><body>";
        html += "<h2>SD Gateway</h2>";
        html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
        html += "<input type='file' name='file'><input type='submit' value='Upload'></form>";
        html += "<h3>Files:</h3>";
        html += "<form method='POST' action='/delete_multi' onsubmit='return confirm(\"Delete selected files?\");'>";
        html += "<ul>";
        File root = SD.open("/");
        while (true) {
            File entry = root.openNextFile();
            if (!entry) break;
            String name = entry.name();
            String safeName;
            if (!normalizePath(name, safeName)) {
                entry.close();
                continue;
            }
            String escapedName = htmlEscape(safeName);
            String encodedName = urlEncode(safeName);
            html += "<li>";
            html += "<input type='checkbox' name='file' value='" + escapedName + "'> ";
            html += escapedName + " <a href='/delete?file=" + encodedName + "'>[delete]</a>";
            if (isEditablePath(safeName)) {
                html += " <a href='/edit?file=" + encodedName + "'>[edit]</a>";
            }
            html += "</li>";
            entry.close();
        }
        root.close();
        html += "</ul>";
        html += "<input type='submit' value='Delete selected'>";
        html += "</form>";
        html += "</body></html>";
        server->send(200, "text/html", html);
    }

    void handleUpload() {
        HTTPUpload& upload = server->upload();
        static File uploadFile;
        static bool uploadAccepted = false;
        if (upload.status == UPLOAD_FILE_START) {
            if (uploadFile) {
                uploadFile.close();
            }
            uploadAccepted = false;
            String filename;
            if (!normalizeUploadName(upload.filename, filename)) {
                return;
            }
            if (SD.exists(filename)) {
                SD.remove(filename);
            }
            uploadFile = SD.open(filename, FILE_WRITE);
            uploadAccepted = static_cast<bool>(uploadFile);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (uploadAccepted && uploadFile) uploadFile.write(upload.buf, upload.currentSize);
        } else if (upload.status == UPLOAD_FILE_END) {
            if (uploadFile) uploadFile.close();
            uploadAccepted = false;
            server->sendHeader("Location", "/");
            server->send(303);
        }
    }

    void handleDelete() {
        if (!server->hasArg("file")) {
            server->send(400, "text/plain", "Missing file param");
            return;
        }
        String filename;
        if (!normalizePath(server->arg("file"), filename)) {
            server->send(400, "text/plain", "Invalid file param");
            return;
        }
        #ifdef DEBUG_SD_GATEWAY
        Serial.print("[SD Gateway] Delete request for: ");
        Serial.println(filename);
        #endif
        if (SD.exists(filename)) {
            SD.remove(filename);
            server->sendHeader("Location", "/");
            server->send(303);
        } else {
            server->send(404, "text/plain", "File not found: " + filename);
        }
    }

    void handleDeleteMulti() {
        if (!server->hasArg("file")) {
            server->sendHeader("Location", "/");
            server->send(303);
            return;
        }
        int n = server->args();
        for (int i = 0; i < n; ++i) {
            if (server->argName(i) == "file") {
                String filename;
                if (!normalizePath(server->arg(i), filename)) {
                    continue;
                }
                #ifdef DEBUG_SD_GATEWAY
                Serial.print("[SD Gateway] Multi-delete: ");
                Serial.println(filename);
                #endif
                if (SD.exists(filename)) {
                    SD.remove(filename);
                }
            }
        }
        server->sendHeader("Location", "/");
        server->send(303);
    }

    void handleEditGet() {
        if (!server->hasArg("file")) {
            server->send(400, "text/plain", "Missing file param");
            return;
        }
        String filename;
        if (!normalizePath(server->arg("file"), filename)) {
            server->send(400, "text/plain", "Invalid file param");
            return;
        }
        if (!isEditablePath(filename)) {
            server->send(403, "text/plain", "Editing only allowed for .txt and .json files");
            return;
        }
        File file = SD.open(filename, FILE_READ);
        if (!file) {
            server->send(404, "text/plain", "File not found");
            return;
        }
        String content;
        while (file.available()) {
            content += (char)file.read();
        }
        file.close();
        String escapedFilename = htmlEscape(filename);
        String html = "<html><head><title>Edit " + escapedFilename + "</title></head><body>";
        html += "<h2>Edit: " + escapedFilename + "</h2>";
        html += "<form method='POST' action='/edit'>";
        html += "<input type='hidden' name='file' value='" + escapedFilename + "'>";
        html += "<textarea name='content' rows='25' cols='80'>";
        html += htmlEscape(content);
        html += "</textarea><br>";
        html += "<input type='submit' value='Save'> ";
        html += "<a href='/'>Cancel</a>";
        html += "</form></body></html>";
        server->send(200, "text/html", html);
    }

    void handleEditPost() {
        if (!server->hasArg("file") || !server->hasArg("content")) {
            server->send(400, "text/plain", "Missing file or content param");
            return;
        }
        String filename;
        if (!normalizePath(server->arg("file"), filename)) {
            server->send(400, "text/plain", "Invalid file param");
            return;
        }
        if (!isEditablePath(filename)) {
            server->send(403, "text/plain", "Editing only allowed for .txt and .json files");
            return;
        }
        String content = server->arg("content");
        if (SD.exists(filename)) {
            SD.remove(filename);
        }
        File file = SD.open(filename, FILE_WRITE);
        if (!file) {
            server->send(500, "text/plain", "Failed to open file for writing");
            return;
        }
        file.print(content);
        file.close();
        server->sendHeader("Location", "/");
        server->send(303);
    }

    void handleList() {
        String json = "[";
        File root = SD.open("/");
        bool first = true;
        while (true) {
            File entry = root.openNextFile();
            if (!entry) break;
            if (!first) json += ",";
            json += "\"" + jsonEscape(String(entry.name())) + "\"";
            first = false;
            entry.close();
        }
        root.close();
        json += "]";
        server->send(200, "application/json", json);
    }

    void startServer() {
        if (active) return;
        if (!SD.begin()) {
            displayMessage("SD init error");
            return;
        }
        server = new WebServer(serverPort);
        server->on("/", HTTP_GET, handleRoot);
        server->on("/upload", HTTP_POST, [](){ server->send(200); }, handleUpload);
        server->on("/delete", HTTP_GET, handleDelete);
        server->on("/delete_multi", HTTP_POST, handleDeleteMulti);
        server->on("/edit", HTTP_GET, handleEditGet);
        server->on("/edit", HTTP_POST, handleEditPost);
        server->on("/list", HTTP_GET, handleList);
        server->begin();
        active = true;
    }

    void stopServer() {
        if (server) {
            server->close();
            delete server;
            server = nullptr;
        }
        active = false;
    }

    void toggleOrShow() {
        if (WiFi.status() != WL_CONNECTED) {
            displayMessage("Wi-Fi not connected");
            return;
        }
        if (active) {
            stopServer();
            displayMessage("SD Gateway: Off");
        } else {
            startServer();
            displayMessage("SD Gateway: On (port " + String(serverPort) + ")");
        }
    }

    void loop() {
        if (active && server) {
            server->handleClient();
        }
    }
}
