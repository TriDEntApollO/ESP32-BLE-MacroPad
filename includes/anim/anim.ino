#include <Wire.h>
#include <Adafruit_GFX.h>         // https://randomnerdtutorials.com/esp8266-0-96-inch-oled-display-with-arduino-ide/
#include <Adafruit_SSD1306.h>     // https://javl.github.io/image2cpp/
#include "graphics.h"
#include "Animator.h"

// screen; uses pins D1 & D2 (I2C)
unsigned int SCREEN_WIDTH = 128; // OLED display width, in pixels
unsigned int SCREEN_HEIGHT = 64; // OLED display height, in pixels
int OLED_RESET = -1; // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// animation helper class
Animator animator;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(2);
  
  animator.begin(&display, SCREEN_WIDTH, SCREEN_HEIGHT, &Serial);
  int seqId = animator.addAnimation(_cat, len, 4, WHITE);
  animator.playLoopedSequence(seqId);
}

void loop() {
  animator.update();
}