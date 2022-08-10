#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <vector>
#include "DeviceParameter.h"

class Device
{
private:
	int id;
	std::string name;
	std::vector<DeviceParameter*> parameters;
	bool status; // true = ON, false = OFF
public:
	Device();
	Device(int id, std::string name, std::vector<DeviceParameter*> parameters);
	bool isParameterExist(std::string paramName);
	bool addParameter(DeviceParameter* param);
	bool setParameter(std::string parameterName, std::string parameterValue);
	bool getStatus();
	bool setStatus(bool status);
	int getId();
	void setId(int id);
	std::string getName();
	void setName(std::string name);
	std::string getParameter(std::string name);
	DeviceParameter* getParameterReference(std::string name);
	virtual std::string getDeviceType();
	virtual bool start();
	std::string getAllParam();
};

#endif // !DEVICE_H
