#include <Wire.h>
#include <BleKeyboard.h>
#include <Adafruit_GFX.h>
#include <RotaryEncoder.h>
#include <Adafruit_SH110X.h>

#include "includes/bitmaps.h"
#include "includes/KeyMap.cpp"
#include "includes/WebInterface.cpp"

#define WHITE 1
#define BLACK 0
#define PIN_IN1 32
#define PIN_IN2 33
#define OLED_RESET -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C


KeyMap keyMap;
WebInterface webInterface(80, keyMap);
BleKeyboard bleKeyboard("CronoDeck", "Trident Apollo", 100);
Adafruit_SH1106G oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

uint8_t state = 0;
uint8_t menuSelLayer;
uint8_t menuPrevLayer;
uint8_t menuNextLayer;
bool menuScreen = false;
uint8_t tickCount = 0;
bool webInterfaceState = false;
uint8_t PREV_MATRIX_STATE[keyMap.rows][keyMap.cols] = { 0 };

void IRAM_ATTR checkPosition() {
    encoder.tick(); // just call tick() to check the state.
}

void setup() {
    Serial.begin(115200);

    keyMap.loadAndInitialize();

    if(!oled.begin(SCREEN_ADDRESS, true)) {
        Serial.println(F("SH1106 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    

    // use TWO03 mode when PIN_IN1, PIN_IN2 signals are both LOW or HIGH in latch position.
    // encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

    // register interrupt routine
    attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);

    oled.setRotation(2);
    startBluetooth();
    connect();

    showScreen(3);
}

void loop() {
    if (!bleKeyboard.isConnected()) {
        connect();
    }

    static int pos = 0;
    keyMap.scanMatrix();
    
    for (uint8_t i = 0; i < keyMap.rows; i++) {
        for (uint8_t j = 0; j < keyMap.cols; j++) {
        state = keyMap.MATRIX_STATE[i][j];
        if (PREV_MATRIX_STATE[i][j] != state && bleKeyboard.isConnected()) {
            if (state == 1) {
            bleKeyboard.press(keyMap.getKey(i, j));
            showScreen(0);
            Serial.print("Button Pressed: ");
            Serial.println(keyMap.getKey(i, j));
            }
            else if (state == 0){
            bleKeyboard.release(keyMap.getKey(i, j));
            showScreen(3);
            Serial.print("Button Released: ");
            Serial.println(keyMap.getKey(i, j));
            }
            PREV_MATRIX_STATE[i][j] = state;
        }
        }
    }

    encoder.tick(); // just call tick() to check the state.

    int newPos = encoder.getPosition();
    if (pos != newPos) {
        tickCount++;
        if (tickCount >= 2) {
            if (((int)(encoder.getDirection())) == 1) {
                if (menuScreen) menuSelLayer = menuSelLayer + 1 > keyMap.layers ? 1 : menuSelLayer + 1;
                else bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
            }
            else {
                if (menuScreen) menuSelLayer = menuSelLayer - 1 < 1 ? keyMap.layers : menuSelLayer - 1;
                else bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
            }
            if (menuScreen) showScreen(3);
            tickCount = 0;
        }
        pos = newPos;
    }

    if (digitalRead(keyMap.modPin) != keyMap.pinStates[0]) {
        keyMap.pinStates[0] = digitalRead(keyMap.modPin);
        if (!keyMap.pinStates[0]) {
            Serial.println("Mod key 1 pressed");
            menuScreen = !menuScreen;
            if (menuScreen) {
                menuSelLayer = keyMap.selectedLayer;
            } else {
                keyMap.selectedLayer = menuSelLayer;
            }
            showScreen(3);
        }
    }

    if (digitalRead(keyMap.modPin2) != keyMap.pinStates[1]) {
        keyMap.pinStates[1] = digitalRead(keyMap.modPin2);
        if (!keyMap.pinStates[1]) {
            Serial.println("Mod key 2 pressed");
            showScreen(0);
            webInterfaceState = !webInterfaceState;
            webInterfaceState ? webInterface.startWebInterfaceTask() : webInterface.stopWebInterfaceTask();
        } else {
            showScreen(3);
        }
    }

    if (digitalRead(keyMap.macroPin) != keyMap.pinStates[2]) {
        keyMap.pinStates[2] = digitalRead(keyMap.macroPin);
        if (!keyMap.pinStates[2]) {   
            Serial.println("Macro key pressed");
            executeMacro();
        }
    }

    if (reloadLayerName) {
        reloadLayerName = false;
        showScreen(3);
    }

}

void startBluetooth() {
    Serial.println("Starting BLE work!");
    bleKeyboard.begin();

    oled.clearDisplay(); // Clear the buffer
    oled.setTextSize(2);
    oled.setTextColor(WHITE, BLACK);
    oled.setCursor(12, 15);
    oled.println("BlueTooth");
    oled.setCursor(25, 32);
    oled.println("Started");
    oled.display();
    delay(1000);
}

void connect() {
    oled.clearDisplay();
    oled.setTextSize(1);
    
    for (int i = 0; !bleKeyboard.isConnected(); i++) {
        oled.clearDisplay();
        oled.setCursor(34, 28);
        oled.print("Connecting");  
        for (int j = 0; j < (i % 4); j++) {
            oled.print(".");
        }
        oled.display();
        delay(500);
    }

    oled.setTextSize(2);
    Serial.println("Connected Successfully");

    oled.clearDisplay();
    oled.setCursor(12, 15);
    oled.println("BlueTooth");
    oled.setCursor(12, 32);
    oled.println("Connected");
    oled.display();
    delay(1000);
}

void executeMacro() {
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.press('r');
    delay(100);
    bleKeyboard.releaseAll();
    delay(500);

    String command = "powershell";
    for (char c : command) {
        bleKeyboard.write(c);
        delay(10);
    }
    delay(100);
    bleKeyboard.write(KEY_RETURN);
    delay(1500);

    String message = "echo \"" + keyMap.macroMessage + "\"";
    for (char c : message) {
        bleKeyboard.write(c);
        delay(10); 
    }
    bleKeyboard.write(KEY_RETURN);
    Serial.println("Macro executed!");
}

void showScreen(int catPos) {
    if (menuScreen) {
        drawMenu();
    } else {
        oled.clearDisplay();
        oled.drawBitmap(0, 0, _cat[catPos] , 128, 40, WHITE);
        if (webInterfaceState) {
            oled.drawBitmap(100, 4, _wifi, 20, 18, WHITE);
        }
        oled.setTextSize(1);
        oled.setCursor(10, oled.height() - 20);
        oled.print("Layer: ");
        oled.print(keyMap.layerNames[keyMap.selectedLayer - 1]);
        oled.display();
    }
}

void drawMenu() {
    char buffer[15];
    oled.clearDisplay();
    
    oled.drawXBitmap(0, 44, _bullet, 7, 16, WHITE); // Bullet 3
    oled.drawXBitmap(0, 3, _bullet, 7, 16, WHITE); // Bullet 1
    oled.drawXBitmap(0, 23, _selected_box, 110, 16, WHITE); // Selected
    // Set font and draw strings
    oled.setTextSize(1);
    oled.setTextColor(WHITE);

    menuNextLayer = menuSelLayer + 1 > keyMap.layers ? 1 : menuSelLayer + 1;
    menuPrevLayer = menuSelLayer - 1 < 1 ? keyMap.layers : menuSelLayer - 1;

    oled.setCursor(11, 7);
    sprintf(buffer, "%02d %s", menuPrevLayer, keyMap.layerNames[menuPrevLayer-1]);
    oled.println(buffer);
    oled.setCursor(11, 27);
    sprintf(buffer, "%02d %s", menuSelLayer, keyMap.layerNames[menuSelLayer-1]);
    oled.println(buffer);
    oled.setCursor(11, 48);
    sprintf(buffer, "%02d %s", menuNextLayer, keyMap.layerNames[menuNextLayer-1]);
    oled.println(buffer);

    int scrollbarHeight = 64 / keyMap.layers;
    oled.drawXBitmap(123, 1, _scroll_line, 1, 61, WHITE); // Scroll line
    oled.fillRect(121, scrollbarHeight * menuSelLayer - scrollbarHeight, 5, scrollbarHeight, WHITE); // Scroll bar
    oled.display();
}