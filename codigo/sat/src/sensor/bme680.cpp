#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <bme68xLibrary.h>

#include "../packet.hpp"
#include "sensor.hpp"

void BME680_c::thread() {
	Bme68x bme;
	bme.begin(0x77, Wire);
	if (bme.checkStatus()) {
		if (bme.checkStatus() == BME68X_ERROR) {
			Serial.println("Sensor error:" + bme.statusString());
			return;
		} else if (bme.checkStatus() == BME68X_WARNING) {
			Serial.println("Sensor Warning:" + bme.statusString());
		}
	}
	bme.setTPH();
	while (1) {
		bme68xData data;

		bme.setOpMode(BME68X_FORCED_MODE);
		delayMicroseconds(bme.getMeasDur());

		if (bme.fetchData()) {
			bme.getData(data);
			container temp{
				data.temperature,
				data.pressure,
				data.humidity
			};
			xQueueOverwrite(queue, &temp);
		}
	}
}

bool BME680_c::available() {
	return (uxQueueMessagesWaiting(queue) > 0);
}

Packet BME680_c::get() {
	Packet packet{
		static_cast<uint8_t>(PacketType::Sensor),
		static_cast<uint8_t>(type),
		sizeof(container),
		std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(container)]())
	};
	xQueueReceive(queue, packet.data.get(), 0);
	return packet;
}