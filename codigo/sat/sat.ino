#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <Wire.h>

#include "src/sensor/sensor.hpp"
#include "src/sensor/sensor_task.hpp"
#include "src/wifi.hpp"

#define LED_PIN 33

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
    wifi();
}

std::vector<Packet> tosend;
void loop() {
    i2c_task->get(tosend);
    camera_task->get(tosend);
    gps_task->get(tosend);
    if (!tosend.empty()) {
        send(tosend);
        tosend.clear();
    }
}