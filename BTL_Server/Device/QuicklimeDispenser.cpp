#include "QuicklimeDispenser.h"

QuicklimeDispenser::QuicklimeDispenser(int id, std::string name, std::vector<DeviceParameter*> parameters) : Device(id, name, parameters)
{
	SingleDeviceParameter<double> *limeAmount = new SingleDeviceParameter<double>("L", "Lime Amount", 100);
	this->addParameter(limeAmount);
}

bool QuicklimeDispenser::start()
{
	return false;
}