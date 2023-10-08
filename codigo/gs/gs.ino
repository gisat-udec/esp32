#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <esp_event.h>
#include "src/wifi.hpp"
#include "src/ethernet.hpp"
#include "src/packet.hpp"


void setup() {
	Serial.begin(115200);
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	wifi::init();
	eth::init();
	TimerHandle_t stats_timer = xTimerCreate("stats timer", pdMS_TO_TICKS(1000), pdTRUE, NULL,
		[](TimerHandle_t xTimer) {
			std::tuple<int8_t, uint32_t> stats{
				wifi::rssi,
				wifi::rx_bytes
			};
			eth::send(Packet(PacketType::Stats, 0, sizeof(stats), &stats));
			wifi::rssi = UINT8_MAX;
			wifi::rx_bytes = 0;
		});
	xTimerStart(stats_timer, 0);
}

IRAM_ATTR void loop() {
	vTaskDelay(1000);
}