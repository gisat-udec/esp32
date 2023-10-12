#pragma once
#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include "../packet.hpp"

#define PEEK_MS 100

enum struct SensorType : uint8_t {
    IMU,
    AMBIENT,
    CAMERA,
    GPS,
};

enum struct SensorSource : uint8_t {
    SATELLITE,
    GROUND_STATION,
};

struct SensorHeader {
    SensorHeader(SensorType type, SensorSource source, uint32_t time) :
        type(type), source(source), time(time) {
    };
    SensorType type;
    SensorSource source;
    uint16_t _pad;
    uint32_t time;
};

template<class SensorPayload>
struct SensorData {
    SensorHeader header;
    SensorPayload payload;
};

class Sensor {
private:
    QueueHandle_t queue;
    QueueHandle_t new_reading;
    uint32_t next_peek = 0;
protected:
    const size_t payload;
    const bool peek;
public:
    const SensorType type;
    const SensorSource source;
    Sensor(SensorType type, SensorSource source, size_t payload, bool peek)
        : type(type), source(source), payload(payload + sizeof(SensorHeader)), peek(peek) {
        queue = xQueueCreate(1, payload + sizeof(SensorHeader));
        if (peek) {
            new_reading = xQueueCreate(1, sizeof(bool));
            bool f = false;
            xQueueOverwrite(new_reading, &f);
        }
    };
    virtual void setup() = 0;
    virtual void loop() = 0;
    template<class Payload>
    void reading(Payload &payload, bool block) {
        SensorData<Payload> data{
            { type, source, pdTICKS_TO_MS(xTaskGetTickCount()) },
            std::move(payload)
        };
        if (block) {
            xQueueSend(queue, &data, portMAX_DELAY);
        } else {
            xQueueOverwrite(queue, &data);
        }
        if (peek) {
            bool t = true;
            xQueueOverwrite(new_reading, &t);
        }
    }
    bool available() {
        if (peek) {
            bool is_new_reading;
            xQueuePeek(new_reading, &is_new_reading, portMAX_DELAY);
            if (!is_new_reading && xTaskGetTickCount() < next_peek) {
                return false;
            }
            if (is_new_reading) {
                bool f = false;
                xQueueOverwrite(new_reading, &f);
            }
            next_peek = xTaskGetTickCount() + pdMS_TO_TICKS(PEEK_MS);
        }
        return (uxQueueMessagesWaiting(queue) > 0);
    }
    Packet get() {
        Packet packet{
            PacketType::Sensor,
            payload,
        };
        if (peek) {
            xQueuePeek(queue, packet.data.get(), portMAX_DELAY);
        } else {
            xQueueReceive(queue, packet.data.get(), portMAX_DELAY);
        }
        return packet;
    }
};

class SensorIMU : public Sensor {
protected:
    struct container {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };
public:
    SensorIMU(SensorSource source) :
        Sensor(SensorType::IMU, source, sizeof(container), false) {
    };
};

class SensorAmbient : public Sensor {
protected:
    struct container {
        uint32_t temperature;
        uint32_t humidity;
        uint32_t pressure;
    };
public:
    SensorAmbient(SensorSource source) :
        Sensor(SensorType::AMBIENT, source, sizeof(container), true) {
    }
};


class SensorCamera : public Sensor {
protected:
    struct container {
        uint32_t frame;
        uint8_t k;
        uint8_t v;
        uint8_t id;
        std::array<uint8_t, 1000> chunk;
    };
public:
    SensorCamera(SensorSource source) :
        Sensor(SensorType::CAMERA, source, sizeof(container), false) {
    };
};

class SensorGPS : public Sensor {
protected:
    struct container {
        uint32_t latitude;
        uint32_t longitude;
        uint32_t altitude;
    };
public:
    SensorGPS(SensorSource sensor) :
        Sensor(SensorType::GPS, source, sizeof(container), true) {
    };
};

/*
class BNO080_c : public SensorIMU {
public:
    BNO080_c() : SensorIMU(SensorSource::SATELLITE) {};
    void setup();
    void loop();
};
class BME680_c : public SensorAmbient {
public:
    BME680_c() : SensorAmbient(SensorSource::SATELLITE) {};
    void setup();
    void loop();
};
class OV2640_c : public SensorCamera {
public:
    OV2640_c() : SensorCamera(SensorSource::SATELLITE) {};
    void setup();
    void loop();
};
class ATGM336H_c : public SensorGPS {
public:
    ATGM336H_c() : SensorGPS(SensorSource::SATELLITE) {};
    void setup();
    void loop();
};
*/
class NEO6M_c : public SensorGPS {
public:
    NEO6M_c() : SensorGPS(SensorSource::GROUND_STATION) {};
    void setup();
    void loop();
};