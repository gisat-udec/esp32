#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <TinyGPSPlus.h>
#include "sensor.hpp"

TinyGPSPlus gps;

void ATGM336H_c::setup() {
    Serial2.begin(9600, SERIAL_8N1, 13, 12);
}

void ATGM336H_c::loop() {
    while (Serial2.available() > 0) {
        if (gps.encode(Serial2.read())) {
            if (gps.location.isUpdated()) {
                container data = {
                    gps.location.lat(),
                    gps.location.lng(),
                    gps.altitude.meters(),
                };
                reading(data, false);
            }
        }
    }
}