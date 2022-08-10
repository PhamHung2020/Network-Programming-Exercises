#include "Sensor.h"


double Sensor::getEnvironmentParameterValue(std::string name, bool & isFound)
{
	for (auto &param : enviromentVariables)
	{
		if (param->getName() == name)
		{
			isFound = true;
			return param->getValue();
		}
	}
	isFound = false;
	return 0.0;
}

std::string Sensor::getDeviceType()
{
	return "Sensor";
}

