#pragma once
#include <memory>

enum struct PacketType : uint8_t {
	Sensor
};

struct Packet {
	uint8_t type;
	uint8_t subtype;
	uint16_t length;
	std::unique_ptr<uint8_t[]> data;
	const size_t data_len() const { return length; };
	const size_t header_len() const { return sizeof(Packet) - sizeof(data); };
	const size_t len() const { return (header_len() + data_len()); };
	const uint8_t *header_ptr() const { return reinterpret_cast<const uint8_t *>(&type); };
	const uint8_t *data_ptr() const { return reinterpret_cast<const uint8_t *>(data.get()); };
};