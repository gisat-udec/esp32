#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <Wire.h>

#include "src/sensor/sensor.hpp"
#include "src/sensor/sensor_task.hpp"
#include "src/wifi.hpp"

#define LED_PIN GPIO_NUM_33

SensorTask *i2c_task;
SensorTask *camera_task;
SensorTask *gps_task;

void setup() {
    Serial.begin(115200);
    Wire.begin(14, 15);
    Wire.setClock(400000);

    i2c_task = new SensorTask("I2C", 6000);
    i2c_task->add(dynamic_cast<Sensor *>(new BNO080_c()));
    i2c_task->add(dynamic_cast<Sensor *>(new BME680_c()));
    i2c_task->run(1);

    camera_task = new SensorTask("CAMERA", 9000);
    camera_task->add(dynamic_cast<Sensor *>(new Camera_c()));
    camera_task->run(1);

    gps_task = new SensorTask("GPS", 7000);
    gps_task->add(dynamic_cast<Sensor *>(new GPS_c()));
    gps_task->run(0);

    delay(2);
    wifi::init();

    xTaskCreatePinnedToCore([](void *) -> void {
        gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
        TickType_t wake;
        const TickType_t freq = pdMS_TO_TICKS(500);
        while (1) {
            gpio_set_level(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_PIN, 1);
            vTaskDelayUntil(&wake, freq);
        }
        }, "led flash", configMINIMAL_STACK_SIZE, NULL, 1, nullptr, 0);
}

std::vector<Packet> tosend;
void loop() {
    i2c_task->get(tosend);
    camera_task->get(tosend);
    gps_task->get(tosend);
    if (!tosend.empty()) {
        wifi::send(tosend);
        tosend.clear();
    }
}