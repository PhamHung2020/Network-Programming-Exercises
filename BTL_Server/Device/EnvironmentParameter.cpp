#include "EnvironmentParameter.h"

EnvironmentParameter::EnvironmentParameter(std::string name, std::string unitOfMeasurement, double min, double max)
	: name(name), unitOfMeasurement(unitOfMeasurement), min(min), max(max)
{
}

std::string EnvironmentParameter::getName()
{
	return name;
}

std::string EnvironmentParameter::getUnitOfMeasurement()
{
	return unitOfMeasurement;
}

double EnvironmentParameter::getValue()
{
	double random = (double)std::rand() / RAND_MAX;
	return random * (max - min) + min;
}

