#include <Wire.h>
#include <BleKeyboard.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define WHITE 1
#define BLACK 0
#define PIN_IN1 32
#define PIN_IN2 33
#define OLED_RESET -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C


BleKeyboard bleKeyboard("CronoDeck", "Trident Apollo", 100);
Adafruit_SH1106G oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);

    if(!oled.begin(SCREEN_ADDRESS, true)) {
        Serial.println(F("SH1106 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }

    oled.setRotation(2);
    startBluetooth();
    connect();
    showScreen();
}

void loop() {
  

}

void showScreen() {
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setTextColor(WHITE, BLACK);
    oled.setCursor(12, 15);
    oled.println("Executing");
    oled.setCursor(25, 32);
    oled.println("Macro...");
    oled.display();
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