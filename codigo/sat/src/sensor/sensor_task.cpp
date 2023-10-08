#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include "sensor_task.hpp"

void SensorTask::run() {
	xTaskCreate(_thread<SensorTask>, tag.c_str(), stack, this, 1, nullptr);
}

void SensorTask::thread() {
	for (Sensor *sensor : sensors) {
		delay(2);
		sensor->setup();
	}
	while (1) {
		for (Sensor *sensor : sensors) {
			sensor->loop();
		}
	}
}

void SensorTask::add(Sensor *sensor) {
	sensors.push_back(sensor);
}

void SensorTask::get(std::vector<Packet> &dest) {
	for (Sensor *sensor : sensors) {
		if (sensor->available()) {
			dest.push_back(sensor->get());
		}
	}
}