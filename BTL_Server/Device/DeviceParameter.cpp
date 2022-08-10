#include "DeviceParameter.h"

DeviceParameter::DeviceParameter(std::string name, std::string fullName, DeviceParameterType parameterType) : name(name), fullName(fullName), parameterType(parameterType)
{

};

std::string DeviceParameter::getName()
{
	return name;
}

std::string DeviceParameter::getFullName()
{
	return fullName;
}

DeviceParameterType DeviceParameter::getParameterType()
{
	return parameterType;
}
;

