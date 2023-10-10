#pragma once
#include <memory>
#define MTU 1500

enum struct PacketType : uint8_t {
    Sensor,
    Stats,
    Ping
};

struct Packet {
    Packet(PacketType type, uint16_t length) :
        type(type), length(length) {
        data.reset(new uint8_t[length]());
    }
    Packet(PacketType type, uint16_t length, void *buf) :
        type(type), length(length) {
        data.reset(new uint8_t[length]());
        std::copy_n((uint8_t *)buf, length, data.get());
    }
    Packet(void *buf) {
        uint8_t *_buf = reinterpret_cast<uint8_t *>(buf);
        std::copy_n(_buf, header_len(), (uint8_t *)&type);
        data.reset(new uint8_t[data_len()]());
        std::copy_n(&_buf[header_len()], data_len(), data.get());
    };
    PacketType type;
    uint8_t _pad;
    uint16_t length;
    std::unique_ptr<uint8_t[]> data;
    const size_t data_len() const { return length; };
    const size_t header_len() const { return sizeof(Packet) - sizeof(data); };
    const size_t len() const { return (header_len() + data_len()); };
    const uint8_t *header_ptr() const { return reinterpret_cast<const uint8_t *>(&type); };
    const uint8_t *data_ptr() const { return data.get(); };
    std::unique_ptr<uint8_t[]> buf() const {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[len()]());
        std::copy_n(header_ptr(), header_len(), buffer.get());
        std::copy_n(data_ptr(), data_len(), &(buffer.get()[header_len()]));
        return buffer;
    };
};