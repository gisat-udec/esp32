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
    // timer regular para reportar conexión a python
    TimerHandle_t ping_timer = xTimerCreate("ping timer", pdMS_TO_TICKS(1000), pdTRUE, NULL,
        [](TimerHandle_t xTimer) {
            int time = millis();
            eth::send(Packet(PacketType::Ping, 0, sizeof(time), &time));
        });
    xTimerStart(ping_timer, 0);
}

IRAM_ATTR void loop() {
    vTaskDelay(1000);
}