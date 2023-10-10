#pragma once
#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include "../packet.hpp"

enum struct SensorType : uint8_t {
    BNO080,
    BME680,
    OV2640,
    GPS
};

typedef std::pair<std::string, std::vector<std::string>> info_pair;

struct SensorHeader {
    SensorHeader(SensorType type, uint32_t time) :
        type(type), time(time) {
    };
    SensorType type;
    uint8_t _pad;
    uint16_t _pad2;
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
protected:
    const size_t payload;
    const bool peek;
public:
    const SensorType type;
    Sensor(SensorType type, size_t payload, bool peek)
        : type(type), payload(payload + sizeof(SensorHeader)), peek(peek) {
        queue = xQueueCreate(1, payload + sizeof(SensorHeader));
    };
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual info_pair info() = 0;
    template<class Payload>
    void reading(Payload &payload, bool block) {
        SensorData<Payload> data{
            { type, pdTICKS_TO_MS(xTaskGetTickCount()) },
            std::move(payload)
        };
        if (block) {
            xQueueSend(queue, &data, portMAX_DELAY);
        } else {
            xQueueOverwrite(queue, &data);
        }
    }
    bool available() {
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

class BNO080_c : public Sensor {
private:
    const std::string name = "BNO080";
    const std::vector<std::string> vars = {
        "x",
        "y",
        "z"
    };
    typedef std::tuple<
        float_t,
        float_t,
        float_t
    > container;
public:
    BNO080_c() : Sensor(SensorType::BNO080, sizeof(container), false) {};
    void setup() override;
    void loop() override;
    info_pair info() override { return std::make_pair(name, vars); };
};

class BME680_c : public Sensor {
private:
    const std::string name = "BME680";
    const std::vector<std::string> vars = {
        "time",
        "temperature",
        "pressure",
        "humidity"
    };
    typedef std::tuple<
        float_t,
        float_t,
        float_t,
        float_t
    > container;
public:
    BME680_c() : Sensor(SensorType::BME680, sizeof(container), true) {}
    void setup() override;
    void loop() override;
    info_pair info() override { return std::make_pair(name, vars); };
};


class Camera_c : public Sensor {
private:
    const std::string name = "OV2640";
    const std::vector<std::string> vars = {
        "frame",
        "k",
        "v",
        "id",
        "chunk"
    };
    typedef std::tuple<
        uint32_t,
        uint8_t,
        uint8_t,
        uint8_t,
        std::array<uint8_t, 1000>
    > container;
public:
    Camera_c() : Sensor(SensorType::OV2640, sizeof(container), false) {};
    void setup() override;
    void loop() override;
    info_pair info() override { return std::make_pair(name, vars); };
};

class GPS_c : public Sensor {
private:
    const std::string name = "GPS";
    const std::vector<std::string> vars = {
        "time",
        "latitude",
        "longitude",
        "altitude"
    };
    typedef std::tuple<
        float_t,
        float_t,
        float_t,
        float_t
    > container;
public:
    GPS_c() : Sensor(SensorType::GPS, sizeof(container), true) {};
    void setup() override;
    void loop() override;
    info_pair info() override { return std::make_pair(name, vars); };
};