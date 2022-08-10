#ifndef FEEDER_H
#define FEEDER_H

#include "Device.h"
#include "DeviceParameter.h"
#include <chrono>
#include <ctime>
#include <process.h>
#include "Time.h"
#include <iostream>
#include <Windows.h>


class Feeder : public Device
{

public:
	Feeder(int id, std::string name, std::vector<DeviceParameter*> parameters);
	bool start();
};

#endif // !FEEDER_H
