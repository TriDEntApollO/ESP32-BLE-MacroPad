#ifndef WEBINTERFACE_CPP
#define WEBINTERFACE_CPP

#include <WiFi.h>
#include <WebServer.h>


#include "webpage.h"
#include "bitmaps.h"
#include "keymap.cpp"


class WebInterface {
    public:
        bool running = false;

        WebInterface(uint32_t port, KeyMap &keyMapMatix) : server(port), keyMap(keyMapMatix) {}

        void startWebInterface() {
            Serial.println("Starting Web Interface...");
            spawnWiFiAP();
            startWebServer();
            Serial.println("Web Interface started!");
        }

        void stopWebInterface() {
            Serial.println("Stopping Web Interface...");
            server.close();
            WiFi.softAPdisconnect(true);
            Serial.println("Web Interface stopped!");
        }
        
        static void webServerTask(void* parameter) {
            WebInterface* webInterface = static_cast<WebInterface*>(parameter);
            webInterface->startWebInterface();
        
            while (webInterface->running) {  // Keep running while `running` is true
                webInterface->handleClient();
                vTaskDelay(10 / portTICK_PERIOD_MS);  // Avoid watchdog reset
            }
        
            webInterface->stopWebInterface();  // Properly stop AP and server
            webInterface->taskHandle = NULL;   // Mark task as stopped
            vTaskDelete(NULL);                 // Delete the task
        }

        void startWebInterfaceTask() {
            if (taskHandle != NULL) {
                Serial.println("Web Interface is already running!");
                return;
            }

            running = true;  // Mark as running
        
            xTaskCreatePinnedToCore(
                webServerTask,       // Task function
                "WebServerTask",     // Task name
                8192,                // Stack size (adjust if needed)
                this,                // Pass `this` instance
                1,                   // Priority
                &taskHandle,         // Store task handle
                0                    // Core 0
            );
        
            Serial.println("Web Interface Task started on Core 0");
        }

        void stopWebInterfaceTask() {
            if (taskHandle == NULL) {
                Serial.println("Web Interface is not running!");
                return;
            }
        
            Serial.println("Stopping Web Interface Task...");
            running = false;  // Signal the task to exit
        
            // Wait for the task to exit
            while (taskHandle != NULL) {
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        
            Serial.println("Web Interface Task stopped.");
        }

    private:
        KeyMap &keyMap;
        WebServer server;
        TaskHandle_t taskHandle = NULL;
        const char* ssid = "CronoDeck";
        const char* password = "helloworld";
        IPAddress local_IP = IPAddress(192, 168, 4, 1);
        IPAddress gateway = IPAddress(192, 168, 4, 1);
        IPAddress subnet = IPAddress(255, 255, 255, 0);

        void spawnWiFiAP() {
            Serial.println("Setting up WiFi AP...");
            WiFi.mode(WIFI_AP);
            // WiFi.softAPConfig(local_IP, gateway, subnet);
            WiFi.softAP(ssid, password, 1, 0, 4);
            delay(500);
            Serial.println("WiFi AP setup complete.");
            Serial.print("CronoDeck AP IP: ");
            Serial.println(WiFi.softAPIP());
            Serial.println();
        }

        void handleClient() {
            server.handleClient();
        }

        void handleRoot() {
            Serial.println("Handling root request...");
            
            WiFiClient client = server.client();
            if (!client) {
                Serial.println("No client connected.");
                return;
            }
        
            // Send HTTP headers
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: text/html"));
            client.println(F("Connection: close"));
            client.println();
        
            // Send webpage in chunks
            const size_t CHUNK_SIZE = 1024;
            size_t len = strlen(webpage);
            size_t sent = 0;
        
            while (sent < len) {
                size_t remaining = len - sent;
                size_t toSend = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
                client.write((const uint8_t *)(webpage + sent), toSend);
                sent += toSend;
                delay(1);
            }
        
            Serial.println(F("Sent webpage in chunks..."));
            client.stop();
        }
        
        void handleStatus() {
            Serial.println("Handling status request...");
            server.send(200, "text/plain", "connected");
        }
        
        void handleGetActiveLayer() {
            Serial.println("Handling get active layer request...");
            server.send(200, "text/plain", String(keyMap.layers));
        }
        
        void handleChangeActiveLayers() {
            Serial.println("Handling change layers request...");
            
            if (!server.hasArg("value")) {
                server.send(400, "text/plain", "Missing parameter: value");
                return;
            }
            
            int value = server.arg("value").toInt();
        
            if (value < 1 || value > 10) {
                server.send(400, "text/plain", "Invliad Layers! Layers out of range");
            } else {
                keyMap.updateActiveLayers(value);
                server.send(200, "text/plain", "Active layers updated to " + String(keyMap.layers));
            }
        
        }

        void handleGetLayerNames() {
            String response = "[";
            for (int i = 0; i < keyMap.layers; i++) {
                response += "\"" + keyMap.layerNames[i] + "\"";
                if (i < keyMap.layers - 1) {
                    response += ",";
                }
            }
            response += "]";
            server.send(200, "application/json", response);  // Send the JSON response
        }

        void handleUpdateLayerName() {
            if (!server.hasArg("index") || !server.hasArg("name")) {
                server.send(400, "text/plain", "Invalid parameters");
                return;
            }

            int layerIndex = server.arg("index").toInt();
            String newLayerName = server.arg("name");

            if (layerIndex < 0 || layerIndex >= keyMap.layers || newLayerName.isEmpty()) {
                server.send(400, "text/plain", "Invalid index or name");
                return;
            }
            
            keyMap.updateLayerName(layerIndex, newLayerName);
            String message = "Layer " + String(layerIndex) + " name updated to \"" + newLayerName + "\" successfully.";
            server.send(200, "text/plain", message);
        }
        
        void handleSubmitKeymap() {
            Serial.println("Handling Submit Keymap request...");
            
            if (server.hasArg("layer") && server.hasArg("data")) {
                int layer = server.arg("layer").toInt();
                String data = server.arg("data");

                if (layer >= 0 && layer < keyMap.layers && data.length() == keyMap.rows * keyMap.cols) {
                    char newKeyMap[keyMap.rows][keyMap.cols];
                    int index = 0;
                    for (int row = 0; row < keyMap.rows; row++) {
                        for (int col = 0; col < keyMap.cols; col++) {
                            newKeyMap[row][col] = data[index++];
                        }
                    }
                    keyMap.updateKeyMap(layer, newKeyMap);
                    server.send(200, "text/plain", "OK");
                } else {
                    server.send(400, "text/plain", "Invalid Data");
                }
            } else {
                server.send(400, "text/plain", "Bad Request");
            }
        }
        
        void handleGetKeymap() {
            Serial.println("Handling get keymap request...");
            if (server.hasArg("layer")) {
                int layer = server.arg("layer").toInt();
                if (layer >= 0 && layer < 10) {
                    String json = "[";
                    for (int row = 0; row < 3; row++) {
                        json += "[";
                        for (int col = 0; col < 3; col++) {
                            json += "\""; json += keyMap.getKey(layer, row, col); 
                            json += "\"";
                            if (col < 2) json += ",";
                        }
                        json += "]";
                        if (row < 2) json += ",";
                    }
                    json += "]";
                    server.send(200, "application/json", json);
                } else {
                    server.send(400, "text/plain", "Invalid Layer");
                }
            } else {
                server.send(400, "text/plain", "Missing Parameter");
            }
        }

        void handleGetMacroMessage() {
            server.send(200, "text/plain", keyMap.macroMessage);
        }
        
        void handleUpdateMacroMessage() {
            if (server.hasArg("message")) {
                keyMap.updateMacroMessage(server.arg("message"));
                server.send(200, "text/plain", "Macro message updated successfully.");
            } else {
                server.send(400, "text/plain", "Error: No message received.");
            }
        }
        
        void handleNotFound() {
            Serial.print("Unknown request: ");
            Serial.println(server.uri());
            server.send(404, "text/plain", "Error 404: Page Not Found");
        }

        void setupRoutes() {
            server.on("/", HTTP_GET, [this]() { handleRoot(); });
            server.on("/status", HTTP_GET, [this]() { handleStatus(); });
            server.on("/getKeymap", HTTP_GET, [this]() { handleGetKeymap(); });
            server.on("/submitKeymap", HTTP_POST, [this]() { handleSubmitKeymap(); });
            server.on("/getActiveLayers", HTTP_GET, [this]() { handleGetActiveLayer(); });
            server.on("/changeActiveLayers", HTTP_POST, [this]() { handleChangeActiveLayers(); });
            server.on("/getMacroMessage", HTTP_GET, [this]() { handleGetMacroMessage(); });
            server.on("/updateMacroMessage", HTTP_POST, [this]() { handleUpdateMacroMessage(); });
            server.on("/getLayerNames", HTTP_GET, [this]() { handleGetLayerNames(); });
            server.on("/updateLayerName", HTTP_POST, [this]() { handleUpdateLayerName(); });
            server.onNotFound([this]() { handleNotFound(); });

            server.on("/test", HTTP_GET, [this]() {
                server.send(200, "text/plain", "Test OK");
            });
        }
        
        void startWebServer() {
            Serial.println("Starting Web Server...");
            setupRoutes();
            server.begin();
            Serial.println("Web Server started!");
            Serial.printf("Web Server running on: http://%s\n", WiFi.softAPIP().toString().c_str());
            Serial.println();
        }  
};

#endif