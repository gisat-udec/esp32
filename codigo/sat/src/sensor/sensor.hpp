#pragma once

#include <vector>
#include <tuple>
#include <string>
#include <memory>

#include "../packet.hpp"

#define BNO080_STACK 1800
#define BME680_STACK 3000
#define CAMERA_STACK 6400
#define GPS_STACK 3000

enum struct SensorType : uint8_t {
	BNO080,
	BME680,
	OV2640,
	GPS
};

typedef std::pair<std::string, std::vector<std::string>> info_pair;

class Sensor {
protected:
	TaskHandle_t handle;
	const SensorType type;
	template<class T>
	static void _thread(void *parm) {
		static_cast<T *>(parm)->thread();
	}
public:
	Sensor(SensorType type) : type(type) {};
	virtual bool available();
	virtual Packet get();
	virtual info_pair info();
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
	QueueHandle_t queue = xQueueCreate(1, sizeof(container));
public:
	BNO080_c() : Sensor(SensorType::BNO080) {
		xTaskCreate(_thread<BNO080_c>, name.c_str(), BNO080_STACK, this, 1, &handle);
	};
	void thread();
	bool available() override;
	Packet get() override;
	info_pair info() override { return std::make_pair(name, vars); };
};

class BME680_c : public Sensor {
private:
	const std::string name = "BME680";
	const std::vector<std::string> vars = {
		"temperature",
		"pressure",
		"humidity"
	};
	typedef std::tuple<
		float_t,
		float_t,
		float_t
	> container;
	QueueHandle_t queue = xQueueCreate(1, sizeof(container));
public:
	BME680_c() : Sensor(SensorType::BME680) {
		xTaskCreate(_thread<BME680_c>, name.c_str(), BME680_STACK, this, 1, &handle);
	};
	void thread();
	bool available() override;
	Packet get() override;
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
	std::unique_ptr<container> _data;
public:
	Camera_c() : Sensor(SensorType::OV2640) {
		xTaskCreate(_thread<Camera_c>, name.c_str(), CAMERA_STACK, this, 1, &handle);
	};
	void thread();
	bool available() override;
	Packet get() override;
	info_pair info() override { return std::make_pair(name, vars); };
};

class GPS_c : public Sensor {
private:
	const std::string name = "GPS";
	const std::vector<std::string> vars = {
		"latitude",
		"longitude",
		"altitude"
	};
	typedef std::tuple<
		float_t,
		float_t,
		float_t
	> container;
	QueueHandle_t queue = xQueueCreate(1, sizeof(container));
public:
	GPS_c() : Sensor(SensorType::GPS) {
		xTaskCreate(_thread<GPS_c>, name.c_str(), GPS_STACK, this, 1, &handle);
	};
	void thread();
	bool available() override;
	Packet get() override;
	info_pair info() override { return std::make_pair(name, vars); };
};