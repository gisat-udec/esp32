#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <esp_event.h>
#include "src/ethernet.hpp"


void setup() {
	Serial.begin(115200);
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	eth::init();
}

IRAM_ATTR void loop() {
	vTaskDelay(1000);
	eth::send();
}