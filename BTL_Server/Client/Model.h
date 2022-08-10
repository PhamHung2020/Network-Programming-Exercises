#ifndef MODEL_FILE_
#define MODEL_FILE_

#include <string>
#include <vector>

struct Device {
	std::string id;
	std::string name;
	Device(std::string _id, std::string _name) {
		id = _id;
		name = _name;
	}

	Device() {
		id = "";
		name = "";
	}
};
struct Time {
	std::string value;
	std::string id;
	Time(std::string _value, std::string _id) {
		value = _value;
		id = _id;
	}
	Time() {
		value = "";
		id = "";
	}
};
struct TimeRange {
	std::string startTime;
	std::string endTime;
	std::string id;
	TimeRange(std::string _start, std::string _end, std::string _id) {
		startTime = _start;
		endTime = _end;
		id = _id;
	}
	TimeRange() {
		startTime = "";
		endTime = "";
		id = "";
	}
};

struct Param {
	std::string name;
	std::string fullName;
	Param(std::string _name, std::string _fullName) {
		name = _name;
		fullName = _fullName;
	}

	Param() {
		name = "";
		fullName = "";

	}
};
struct Sensor {
	std::vector<std::string> oxi;
	std::vector<std::string> ph;
	std::vector<std::string> salt;
	Sensor() {

	}
};

template <typename T>
struct ParamValue :Param {
	std::vector<T> value;
	ParamValue(std::string _name, std::string _fullname, std::vector<T> _value) {
		name = _name;
		fullName = _fullname;
		value = _value;
	}
};
#endif