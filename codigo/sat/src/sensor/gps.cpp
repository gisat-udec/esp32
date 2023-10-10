#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <TinyGPSPlus.h>
#include "sensor.hpp"

TinyGPSPlus gps;

void GPS_c::setup() {
    Serial2.begin(9600, SERIAL_8N1, 13, 12);
}

void GPS_c::loop() {
    while (Serial2.available() > 0) {
        if (gps.encode(Serial2.read())) {
            //if (gps.location.isUpdated()) {
            container data = {
                pdTICKS_TO_MS(xTaskGetTickCount()),
                gps.location.lat(),
                gps.location.lng(),
                gps.altitude.meters(),
            };
            xQueueOverwrite(queue, &data);
            //}
        }
    }
}