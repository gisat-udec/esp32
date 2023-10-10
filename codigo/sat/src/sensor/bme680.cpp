#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <bsec.h>
#include "sensor.hpp"

Bsec iaqSensor;

void BME680_c::setup() {
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
}

void BME680_c::loop() {
    if (iaqSensor.run()) {
        container data = {
            pdTICKS_TO_MS(xTaskGetTickCount()),
            iaqSensor.temperature,
            iaqSensor.pressure,
            iaqSensor.humidity
        };
        xQueueOverwrite(queue, &data);
    }
}