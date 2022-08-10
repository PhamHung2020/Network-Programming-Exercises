#pragma once
#ifndef SENSOR_H
#define SENROR_H

#include "Device.h"
#include <vector>
#include "EnvironmentParameter.h"
#include <string>

class Sensor : public Device
{
private:
	std::vector <EnvironmentParameter*> enviromentVariables;

public:
	Sensor(int id, std::string name, std::vector<EnvironmentParameter*> enviromentVariables) : Device(id, name, {})
	{
		this->enviromentVariables = enviromentVariables;
	};

	double getEnvironmentParameterValue(std::string name, bool& isFound);
	std::string getDeviceType();
};

#endif // !SENSOR_H
