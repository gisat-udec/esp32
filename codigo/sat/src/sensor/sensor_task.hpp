#pragma once
#include <vector>
#include <string>
#include "sensor.hpp"
#include "../packet.hpp"

class SensorTask {
private:
	const std::string tag;
	const size_t stack;
	std::vector<Sensor *> sensors;
	void thread();
	template<class T>
	static void _thread(void *parm) {
		static_cast<T *>(parm)->thread();
	}
public:
	SensorTask(std::string tag, size_t stack) : tag(tag), stack(stack) {};
	void run();
	void add(Sensor *sensor);
	void get(std::vector<Packet> &dest);
};