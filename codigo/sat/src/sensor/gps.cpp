#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <TinyGPSPlus.h>

#include "../packet.hpp"
#include "sensor.hpp"

void GPS_c::thread() {
	TinyGPSPlus gps;
	Serial2.begin(9600, SERIAL_8N1, 13, 12);
	while (1) {
		while (Serial2.available() > 0) {
			if (gps.encode(Serial2.read())) {
				if (gps.location.isUpdated()) {
					container data = {
						gps.location.lat(),
						gps.location.lng(),
						gps.altitude.meters(),
					};
					xQueueOverwrite(queue, &data);
				}
			}
		}
		vTaskDelay(10);
	}
}

bool GPS_c::available() {
	return (uxQueueMessagesWaiting(queue) > 0);
}

Packet GPS_c::get() {
	Packet packet{
		static_cast<uint8_t>(PacketType::Sensor),
		static_cast<uint8_t>(type),
		sizeof(container),
		std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(container)]())
	};
	xQueueReceive(queue, packet.data.get(), 0);
	return packet;
}
