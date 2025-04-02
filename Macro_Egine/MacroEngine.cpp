#ifndef MACRO_ENGINE_CPP
#define MACRO_ENGINE_CPP

#include <map>
#include <vector>
#include <sstream>  
#include <Arduino.h>
#include <BleKeyboard.h>

class MacroEngine {
    public:
        MacroEngine(BleKeyboard &bleKeyboard) : bleKeyboard(bleKeyboard), intervalDelay(5) {} // Default 5ms delay

        // Executes a Ducky Script string
        void executeScript(const std::string &script) {
            std::istringstream stream(script);
            std::string line;
            activeModifiers.clear();

            while (std::getline(stream, line)) {
                processCommand(line);
            }
            releaseAllKeys(); // Ensure no stuck keys
        }

    private:
        int intervalDelay;
        BleKeyboard &bleKeyboard;
        std::vector<uint8_t> activeKeys;
        std::vector<uint8_t> activeModifiers;

        // Key map for supported keys
        std::map<std::string, uint8_t> keyMap = {
            {"ENTER", KEY_RETURN},
            {"CTRL", KEY_LEFT_CTRL},
            {"ALT", KEY_LEFT_ALT},
            {"SHIFT", KEY_LEFT_SHIFT},
            {"GUI", KEY_LEFT_GUI},
            {"ESC", KEY_ESC},
            {"TAB", KEY_TAB},
            {"SPACE", ' '},
            {"BACKSPACE", KEY_BACKSPACE},
            {"DELETE", KEY_DELETE},
            {"UP", KEY_UP_ARROW},
            {"DOWN", KEY_DOWN_ARROW},
            {"LEFT", KEY_LEFT_ARROW},
            {"RIGHT", KEY_RIGHT_ARROW},
        };

        // Processes a single command line from the script
        void processCommand(const std::string &line) {
            if (line.empty()) return;

            std::istringstream ss(line);
            std::string command;
            ss >> command;

            if (command == "REM" || command == "#") {
                std::string discard;
                std::getline(ss, discard); // Consume the rest of the line
                Serial.print("Ignoring command: ");
                Serial.println(discard);
                return; // Ignore the entire comment line
            }

            if (command == "STRING" || command == "ECHO" || command == "PRINT") {
                std::string text = extractString(ss);
                if (text.empty()) return;

                Serial.print("Printing string: ");
                Serial.println(text);
                
                if (command == "STRING" || command == "ECHO") {
                    typeString(text);
                } else {
                    typePrint(text);
                }
            }
            else if (command == "SEND") {
                char c;
                if (ss >> c) {
                    bleKeyboard.write(c);
                }
            }
            else if (command == "DELAY") {
                std::string arg;
                ss >> arg;
                if (arg == "INTERVAL") {
                    ss >> intervalDelay; // Update global delay interval
                } else {
                    int delayTime = std::stoi(arg);
                    delayExecution(delayTime);
                }
            } 
            else if (command == "PRESS") {
                std::string key;
                ss >> key;
                pressKey(key, true);
                releaseKey(key);
            }
            else if (command == "RELEASE") {
                std::string key;
                if (ss >> key) {
                    releaseKey(key, true);
                } else {
                    releaseAllKeys();
                }
            }
            else {
                std::vector<uint8_t> keysToRelease;
                bool isModifierLine = isModifierKey(keyMap[command]);
                
                if (isModifierLine) {}
                    do {
                        if (keyMap.find(command) != keyMap.end()) {
                            uint8_t keyCode = keyMap[command];
                            pressKey(command, false);
                            keysToRelease.push_back(keyCode);
                        }
                    } while (ss >> command);

                    for (uint8_t key : keysToRelease) {
                        releaseKey(key, false);
                    }
                } else {
                    return;
                }
            }
        }

        // Extracts a properly quoted string or a regular space-separated argument
        std::string extractString(std::istringstream &ss) {
            std::string text;
            ss >> std::ws; // Trim leading spaces
            char quoteChar = ss.peek();
            
            if (quoteChar == '"' || quoteChar == '\'') { // Detect opening quote
                ss.get(); // Consume opening quote
                std::getline(ss, text, quoteChar); // Read until closing quote
                
                // Handle escape sequences
                size_t pos = 0;
                while ((pos = text.find("\\", pos)) != std::string::npos) {
                    if (pos + 1 < text.size()) {
                        char nextChar = text[pos + 1];
                        if (nextChar == '"' || nextChar == '\'') {
                            text.erase(pos, 1); // Remove escape character
                        }
                    }
                    pos++;
                }
            } else {
                std::getline(ss, text);
            }
            return text;
        }

        // Presses a key
        void pressKey(const std::string &key, bool allowModifiers) {
            uint8_t keyCode;
            if (keyMap.find(key) != keyMap.end()) {
                keyCode = keyMap[key];
            } else {
                keyCode = key[0];
            }

            if (allowModifiers && isModifierKey(keyCode)) {
                activeModifiers.push_back(keyCode);
            } else {
                activeKeys.push_back(keyCode);
            }
            Serial.print("Pressing key: ");
            Serial.println(keyCode);
            bleKeyboard.press(keyCode);
        }

        // Releases a key
        void releaseKey(const std::string &key, bool allowModifiers) {
            if (key == "ALL") {
                releaseAllKeys();
                return;
            }

            uint8_t keyCode;
            if (keyMap.find(key) != keyMap.end()) {
                keyCode = keyMap[key];
            } else {
                keyCode = key[0];
            }

            if (allowModifiers && isModifierKey(keyCode)) {
                auto it = std::find(activeModifiers.begin(), activeModifiers.end(), keyCode);
                if (it != activeModifiers.end()) {
                    activeModifiers.erase(it);
                }
            } else {
                auto it = std::find(activeKeys.begin(), activeKeys.end(), keyCode);
                if (it != activeKeys.end()) {
                    activeKeys.erase(it);
                }
            }
            Serial.print("Releasing key: ");
            Serial.println(keyCode);
            bleKeyboard.release(keyCode);
        }

        // Types a string instantly
        void typeString(const std::string &text) {
            bleKeyboard.print(text.c_str());
        }

        // Types a string with per-character delay (PRINT)
        void typePrint(const std::string &text) {
            for (char c : text) {
                bleKeyboard.write(c);
                delay(intervalDelay);
            }
        }

        // Introduces a delay
        void delayExecution(int milliseconds) {
            delay(milliseconds);
        }

        // Checks if a key is a modifier
        bool isModifierKey(uint8_t keyCode) {
            return (keyCode == KEY_LEFT_CTRL || keyCode == KEY_LEFT_ALT ||
                    keyCode == KEY_LEFT_SHIFT || keyCode == KEY_LEFT_GUI);
        }

        // Removes a modifier key from activeModifiers
        void removeModifier(uint8_t keyCode) {
            activeModifiers.erase(std::remove(activeModifiers.begin(), activeModifiers.end(), keyCode), activeModifiers.end());
        }

        // Removes a non-modifier key from activeKeys
        void removeKey(uint8_t keyCode) {
            activeKeys.erase(std::remove(activeKeys.begin(), activeKeys.end(), keyCode), activeKeys.end());
        }

        // Releases all active keys
        void releaseAllKeys() {
            for (uint8_t keyCode : activeModifiers) {
                bleKeyboard.release(keyCode);
            }
            for (uint8_t keyCode : activeKeys) {
                bleKeyboard.release(keyCode);
            }
            activeModifiers.clear();
        }
};

#endif