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
	i2c_task->run();

	camera_task = new SensorTask("CAMERA", 9000);
	camera_task->add(dynamic_cast<Sensor *>(new Camera_c()));
	camera_task->run();

	gps_task = new SensorTask("GPS", 7000);
	gps_task->add(dynamic_cast<Sensor *>(new GPS_c()));
	gps_task->run();

	delay(2);
	wifi();
	pinMode(LED_PIN, OUTPUT);
}

std::vector<Packet> tosend;
void loop() {
	i2c_task->get(tosend);
	camera_task->get(tosend);
	gps_task->get(tosend);
	if (!tosend.empty()) {
		for (Packet &packet : tosend) {
			Serial.print(packet.subtype);
		}
		send(tosend);
		tosend.clear();
	}
}