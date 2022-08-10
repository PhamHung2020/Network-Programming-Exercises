#ifndef QUICKLIME_DISPENSER_H
#define QUICKLIME_DISPENSER_H
#include "Device.h"
#include "DeviceParameter.h"
#include <process.h>
#include <Windows.h>
#include <iostream>


class QuicklimeDispenser : public Device
{
public:
	QuicklimeDispenser(int id, std::string name, std::vector<DeviceParameter*> parameters);
	bool start();
};

#endif // !QUICKLIME_DISPENSER_H
