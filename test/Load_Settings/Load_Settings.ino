#include <Preferences.h>

Preferences prefs;

#define maxLayers 10
#define rows 3
#define cols 3

uint8_t layers;
uint8_t selectedLayer;
String macroMessage;
String layerNames[maxLayers];
char keyMap[maxLayers][rows][cols];

void loadSettings() {
    prefs.begin("CronoDeck", true);  // Read-only mode

    layers = prefs.getUChar("layers", 8);
    selectedLayer = prefs.getUChar("selLayer", 4);
    macroMessage = prefs.getString("macroMsg", "NOT");

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
        } else {
            // Fill with default values if not found
            for (uint8_t r = 0; r < rows; r++) {
                for (uint8_t c = 0; c < cols; c++) {
                    keyMap[l][r][c] = '?';  // Placeholder for missing data
                }
            }
        }
    }

    prefs.end();
}

void setup() {
    Serial.begin(115200);
    loadSettings();  // Load settings from Flash

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

void loop() {
    // The main firmware logic goes here.
}