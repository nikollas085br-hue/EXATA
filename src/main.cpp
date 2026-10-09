#include <Arduino.h>
#include <M5Cardputer.h>
#include "App.h"

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(180);
    App::begin();
}

void loop() {
    M5Cardputer.update();
    App::update();
    delay(4);
}
