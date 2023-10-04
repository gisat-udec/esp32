#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <Wire.h>

#include <vector>
#include <cstring>
#include <atomic>

#include "src/sensor/sensor.hpp"
#include "src/wifi.hpp"

#define LED_PIN 33
std::atomic<uint32_t> packets_sent;
std::vector<Sensor *> sensors;
void setup() {
	packets_sent = 0;
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	Serial.begin(115200);
	Wire.begin(14, 15);
	Wire.setClock(400000);
	sensors.push_back(dynamic_cast<Sensor *>(new BNO080_c()));
	sensors.push_back(dynamic_cast<Sensor *>(new BME680_c()));
	sensors.push_back(dynamic_cast<Sensor *>(new Camera_c()));
	sensors.push_back(dynamic_cast<Sensor *>(new GPS_c()));
	wifi();
	pinMode(LED_PIN, OUTPUT);
	TimerHandle_t logtimer = xTimerCreate(
		"log", pdMS_TO_TICKS(1000), pdTRUE, NULL,
		[](TimerHandle_t xTimer) {
			Serial.print("\npkt tx: ");
			Serial.print(packets_sent);
			packets_sent = 0;
			digitalWrite(LED_PIN, LOW);
			vTaskDelay(pdMS_TO_TICKS(100));
			digitalWrite(LED_PIN, HIGH);
		});
	xTimerStart(logtimer, 0);
}

std::vector<Packet> tosend;
IRAM_ATTR void loop() {
	for (Sensor *sensor : sensors) {
		if (sensor->available()) {
			tosend.push_back(sensor->get());
			packets_sent++;
		}
	}
	if (!tosend.empty()) {
		send(tosend);
		tosend.clear();
	}
}