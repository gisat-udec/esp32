#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <SparkFun_BNO080_Arduino_Library.h>
#include "sensor.hpp"

BNO080 imu;

void BNO080_c::setup() {
    imu.begin();
    imu.enableRotationVector(20);
    imu.enableLinearAccelerometer(20);
}

void BNO080_c::loop() {
    if (imu.dataAvailable() == true) {
        float qx, qy, qz, qw;
        float quatRadianAccuracy = 0;
        byte quatAccuracy = 0;
        imu.getQuat(qx, qy, qz, qw, quatRadianAccuracy, quatAccuracy);
        float ax, ay, az;
        byte linAccuracy = 0;
        imu.getLinAccel(ax, ay, az, linAccuracy);

        container data = {
            qx, qy, qz, qw,
            ax, ay, az
        };
        reading(data, false);
    }
}