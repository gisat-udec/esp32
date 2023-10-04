#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include "bsec.h"

#include "../packet.hpp"
#include "sensor.hpp"

void BME680_c::thread() {
	Bsec iaqSensor;
	iaqSensor.begin(0x77, Wire);
	bsec_virtual_sensor_t sensorList[13] = {
		BSEC_OUTPUT_IAQ,
		BSEC_OUTPUT_STATIC_IAQ,
		BSEC_OUTPUT_CO2_EQUIVALENT,
		BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
		BSEC_OUTPUT_RAW_TEMPERATURE,
		BSEC_OUTPUT_RAW_PRESSURE,
		BSEC_OUTPUT_RAW_HUMIDITY,
		BSEC_OUTPUT_RAW_GAS,
		BSEC_OUTPUT_STABILIZATION_STATUS,
		BSEC_OUTPUT_RUN_IN_STATUS,
		BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
		BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
		BSEC_OUTPUT_GAS_PERCENTAGE
	};
	iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
	while (1) {
		if (iaqSensor.run()) {
			container temp{
				iaqSensor.temperature,
				iaqSensor.pressure,
				iaqSensor.humidity
			};
			xQueueOverwrite(queue, &temp);
		}
		const int64_t wait = iaqSensor.nextCall - iaqSensor.getTimeMs();
		if (wait > 0) {
			vTaskDelay(pdMS_TO_TICKS(wait));
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