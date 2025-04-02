#ifndef KEYMAP_CPP
#define KEYMAP_CPP

#include "bitmaps.h"
#include <Preferences.h>

#define DEBOUNCE 10

class KeyMap {
    public:
        uint8_t layers;
        uint8_t selectedLayer;
        const uint8_t modPin = 18;
        const uint8_t modPin2 = 19;
        const uint8_t macroPin = 25;
        uint8_t pinStates[3] = {1, 1, 1};
        static const uint8_t rows = 3;
        static const uint8_t cols = 3;
        const uint8_t rowPins[3] =  {14, 12, 13};
        const uint8_t colPins[3] = {15, 2, 4};
        uint8_t MATRIX_STATE[rows][cols] = { 0 };
        String macroMessage = "Greetings! Welcome to CronoDeck's Demo";
        String layerNames[10] = {
            "Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5",
            "Layer 6", "Layer 7", "Layer 8", "Layer 9", "Layer 10"
        };

        void loadAndInitialize() {
            loadSettings();
            printSettings();
            initMatrix();
        }

        char getKey(uint8_t row, uint8_t col) {
          return keyMap[selectedLayer - 1][row][col];
        }

        char getKey(uint8_t layer, uint8_t row, uint8_t col) {
          return keyMap[layer][row][col];
        }

        void updateKey(uint8_t layer, uint8_t row, uint8_t col, char newKey) {
          if (layer < maxLayers) {
            keyMap[layer][row][col] = newKey;
          }
        }

        void updateMacroMessage(const String &newMessage) {
            Serial.println("Updating Macro Message!");
            if (macroMessage != newMessage) {  // Avoid unnecessary writes
                prefs.begin("CronoDeck", false);  // Open in write mode
                macroMessage = newMessage;
                prefs.putString("macroMsg", macroMessage);
                prefs.end();
                Serial.println("Macro Message updated to: " + macroMessage);
            }
        }

        void updateActiveLayers(uint8_t newLayerCount) {
            Serial.println("Updating Active Layers!");
            if (layers != newLayerCount && newLayerCount <= maxLayers) {  
                prefs.begin("CronoDeck", false);  // Open in write mode
                layers = newLayerCount;
                if (selectedLayer > layers) {
                    selectedLayer = 1;
                    reloadLayerName = true;
                }
                prefs.putUChar("layers", layers);
                prefs.end();
                Serial.println("Layers updated to: " + String(layers));
            }
        }

        void updateLayerName(uint8_t layerIndex, const String &newName) {
            Serial.println("Updating Layer Name!");
            if (layerIndex < maxLayers && layerNames[layerIndex] != newName) {  
                prefs.begin("CronoDeck", false);  // Open in write mode
                layerNames[layerIndex] = newName;
                if (layerIndex + 1 == selectedLayer) {
                    reloadLayerName = true;
                }
                prefs.putString(("layer" + String(layerIndex)).c_str(), newName);
                prefs.end();
                Serial.println("Layer " + String(layerIndex) + " name updated to: " + newName);
            }
        }

        void updateKeyMap(uint8_t layer, char (&newKeyMap)[rows][cols]) {
            Serial.println("Updating KeyMap!");
            if (layer >= maxLayers) {
                Serial.println("Invalid layer index.");
                return;
            }
            bool changed = false;

            for (uint8_t r = 0; r < rows && !changed; r++) {
                for (uint8_t c = 0; c < cols; c++) {
                    if (keyMap[layer][r][c] != newKeyMap[r][c]) {
                        changed = true;
                        break;
                    }
                }
            }
            
            if (!changed) {
                Serial.println("No changes detected in keyMap. Skipping write.");
                return;
            }

            prefs.begin("CronoDeck", false);
            String keyString = "";
            for (uint8_t r = 0; r < rows; r++) {
                for (uint8_t c = 0; c < cols; c++) {
                    keyMap[layer][r][c] = newKeyMap[r][c];
                    keyString += newKeyMap[r][c];
                }
            }
            
            prefs.putString(("keyMap" + String(layer)).c_str(), keyString);
            prefs.end();
            Serial.println("KeyMap for Layer " + String(layer) + " updated successfully.");
        }

        void scanMatrix(void) {
            // Setting row pin as low, and checking if the input of a column pin changes.
            for(uint8_t col=0; col <cols; col++) {
                digitalWrite(colPins[col], LOW);

                for(uint8_t row=0; row < rows; row++) {
                curState = !digitalRead(rowPins[row]);

                if( PREV_MATRIX_STATE[row][col] != curState) {
                    DEBOUNCE_MATRIX[row][col] = millis();
                }
                
                PREV_MATRIX_STATE[row][col] = curState;
                if( (millis() - DEBOUNCE_MATRIX[row][col]) > DEBOUNCE) {
                    if( MATRIX_STATE[row][col] != curState) {
                    MATRIX_STATE[row][col] = curState;
                    }
                }
                }
                digitalWrite(colPins[col], HIGH);
            }
        }        
        
    private:
        Preferences prefs;
        uint8_t curState = 0;
        static const uint8_t maxLayers = 10; 
        
        // matrix states
        uint8_t PREV_MATRIX_STATE[rows][cols] = { 0 };
        uint32_t DEBOUNCE_MATRIX[rows][cols] = { 0 };

        // initialize default keymap
        char keyMap[maxLayers][rows][cols] = {
            {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}},
            {{'a', 'b', 'c'}, {'d', 'e', 'f'}, {'g', 'h', 'i'}},
            {{'j', 'k', 'l'}, {'m', 'n', 'o'}, {'p', 'q', 'r'}},
            {{'s', 't', 'u'}, {'v', 'w', 'x'}, {'y', 'z', ' '}},
            {{'A', 'B', 'C'}, {'D', 'E', 'F'}, {'G', 'H', 'I'}},
            {{'J', 'K', 'L'}, {'M', 'N', 'O'}, {'P', 'Q', 'R'}},
            {{'S', 'T', 'U'}, {'V', 'W', 'X'}, {'Y', 'Z', ' '}},
            {{'!', '@', '#'}, {'$', '%', '^'}, {'&', '*', ' '}},
            {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}},
            {{'0', '0', '0'}, {'0', '0', '0'}, {'0', '0', '0'}}
        };

        void loadSettings() {
            prefs.begin("CronoDeck", true);

            layers = prefs.getUChar("layers", 5);
            selectedLayer = prefs.getUChar("selLayer", 1);
            macroMessage = prefs.getString("macroMsg", "Greetings! Welcome to CronoDeck's Demo");

            for (uint8_t i = 0; i < maxLayers; i++) {
                layerNames[i] = prefs.getString(("layer" + String(i)).c_str(), ("Layer " + String(i + 1)).c_str());
            }

            for (uint8_t l = 0; l < maxLayers; l++) {
                String keyString = prefs.getString(("keyMap" + String(l)).c_str(), "");
                if (keyString.length() == rows * cols) {
                    uint8_t index = 0;
                    for (uint8_t r = 0; r < rows; r++) {
                        for (uint8_t c = 0; c < cols; c++) {
                            keyMap[l][r][c] = keyString[index++];
                        }
                    }
                }
            }

            prefs.end();
        }

        void initMatrix(void) {
            pinMode(modPin, INPUT_PULLUP);
            pinMode(modPin2, INPUT_PULLUP);
            pinMode(macroPin, INPUT_PULLUP);
            for (uint8_t i = 0; i < cols; i++) {
                pinMode(colPins[i], OUTPUT);
                digitalWrite(colPins[i], HIGH);
            }
            for (uint8_t j = 0; j < rows; j++) {
                pinMode(rowPins[j], INPUT_PULLUP);
            }
        }

        void printSettings() {
            Serial.println("Loaded Settings:");
            Serial.println("Layers: " + String(layers));
            Serial.println("Selected Layer: " + String(selectedLayer));
            Serial.println("Macro Message: " + macroMessage);

            for (uint8_t i = 0; i < maxLayers; i++) {
                Serial.println("Layer " + String(i + 1) + ": " + layerNames[i]);
            }

            Serial.println("\nKeymap Matrix:");
            for (uint8_t l = 0; l < maxLayers; l++) {
                Serial.println("Layer " + String(l) + " Keymap:");
                for (uint8_t r = 0; r < rows; r++) {
                    for (uint8_t c = 0; c < cols; c++) {
                        Serial.print(keyMap[l][r][c]);
                        Serial.print(" ");
                    }
                    Serial.println();
                }
                Serial.println();
            }
        }        
};

#endif