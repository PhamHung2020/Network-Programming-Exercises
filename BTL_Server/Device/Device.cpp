#include "Device.h"

Device::Device()
{
	id = 0;
	name = "";
	status = false;
}

Device::Device(int id, std::string name, std::vector<DeviceParameter*> parameters) : id(id), name(name), parameters(parameters)
{
	status = false;
}

bool Device::isParameterExist(std::string paramName)
{
	for (DeviceParameter* parameter : parameters)
	{
		if (parameter->getName() == paramName)
			return true;
	}
	return false;
}

bool Device::addParameter(DeviceParameter * param)
{
	if (isParameterExist(param->getName()))
		return false;
	parameters.push_back(param);
	return true;
}

bool Device::setParameter(std::string parameterName, std::string parameterValue)
{
	DeviceParameter* paramFound = NULL;
	for (DeviceParameter* param : parameters)
	{
		if (parameterName == param->getName())
		{
			paramFound = param;
			break;
		}
	}
	if (paramFound == NULL)
		return false;
	if (paramFound->setValue(parameterValue))
		return true;
	return false;
}

bool Device::getStatus()
{
	return status;
}

bool Device::setStatus(bool status)
{
	if (this->status == status)
		return false;
	return true;
}

int Device::getId()
{
	return id;
}

void Device::setId(int id)
{
	this->id = id;
}

std::string Device::getName()
{
	return name;
}

void Device::setName(std::string name)
{
	this->name = name;
}

std::string Device::getParameter(std::string name)
{
	std::string retVal = "";
	for (DeviceParameter* param : parameters)
	{
		if (name == param->getName())
		{
			retVal = param->getValue();
			return retVal;
		}
	}
	return retVal;

}

DeviceParameter * Device::getParameterReference(std::string name)
{
	for (DeviceParameter* param : parameters)
	{
		if (name == param->getName())
		{
			return param;
		}
	}
	return nullptr;
}

std::string Device::getDeviceType()
{
	return "Device";
}

bool Device::start()
{
	return false;
}

std::string Device::getAllParam()
{
	std::string retVal = "";
	for (auto& deviceParam : parameters)
	{
		retVal += (deviceParam->getName() + '#' + deviceParam->getFullName());
		retVal += 6;
	}
	retVal.pop_back();
	return retVal;
}

;
