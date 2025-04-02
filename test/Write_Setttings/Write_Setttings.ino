#include <Preferences.h>

Preferences prefs;

#define maxLayers 10
#define rows 3
#define cols 3

void setup() {
    Serial.begin(115200);
    prefs.begin("CronoDeck", false);  // Open Preferences in write mode

    Serial.println("Writing default settings to Flash...");

    // Save default values
    prefs.putUChar("layers", 5);
    prefs.putUChar("selLayer", 1);
    prefs.putString("macroMsg", "Greetings! Welcome to CronoDeck's Demo");

    // Default layer names
    String layerNames[maxLayers] = {
        "Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5",
        "Layer 6", "Layer 7", "Layer 8", "Layer 9", "Layer 10"
    };

    for (uint8_t i = 0; i < maxLayers; i++) {
        prefs.putString(("layer" + String(i)).c_str(), layerNames[i]);
    }

    // Default keymap
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

    for (uint8_t l = 0; l < maxLayers; l++) {
        String keyString = "";
        for (uint8_t r = 0; r < rows; r++) {
            for (uint8_t c = 0; c < cols; c++) {
                keyString += keyMap[l][r][c];
            }
        }
        prefs.putString(("keyMap" + String(l)).c_str(), keyString);
    }

    Serial.println("Default settings saved.");
    prefs.end();
}

void loop() {
    // This sketch is used only once, no need for loop code.
}
