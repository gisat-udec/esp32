#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <SparkFun_BNO080_Arduino_Library.h>
#include "sensor.hpp"

BNO080 imu;

void BNO080_c::setup() {
	imu.begin();
	imu.enableRotationVector(10);
}

void BNO080_c::loop() {
	if (imu.dataAvailable() == true) {
		container data = {
			imu.getQuatI(),
			imu.getQuatJ(),
			imu.getQuatK()
		};
		xQueueOverwrite(queue, &data);
	}
}