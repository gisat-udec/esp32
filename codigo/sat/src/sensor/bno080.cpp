#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <SparkFun_BNO080_Arduino_Library.h>

#include <numeric>
#include <array>

#include "../packet.hpp"
#include "sensor.hpp"	

void BNO080_c::thread() {
	BNO080 imu;
	imu.begin();
	imu.enableRotationVector(10);
	while (1) {
		if (imu.dataAvailable() == true) {
			container data = {
				imu.getQuatI(),
				imu.getQuatJ(),
				imu.getQuatK()
			};
			xQueueOverwrite(queue, &data);
		}
	}
}

bool BNO080_c::available() {
	return (uxQueueMessagesWaiting(queue) > 0);
}

Packet BNO080_c::get() {
	Packet packet{
		static_cast<uint8_t>(PacketType::Sensor),
		static_cast<uint8_t>(type),
		sizeof(container),
		std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(container)]())
	};
	xQueueReceive(queue, packet.data.get(), 0);
	return packet;
}