#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <Wire.h>

#include <vector>

#include "src/wifi.hpp"

void setup() {
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	Serial.begin(115200);
	Wire.begin(14, 15);
	Wire.setClock(400000);
	wifi();
}

IRAM_ATTR void loop() {
	vTaskDelay(1000);
}