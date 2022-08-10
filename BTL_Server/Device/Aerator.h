#ifndef AERATOR_H
#define AERATOR_H

#include "DeviceParameter.h"
#include "Device.h"
#include <chrono>
#include <ctime>
#include <process.h>
#include <Windows.h>
#include <iostream>

class Aerator : public Device
{

public:
	Aerator(int id, std::string name, std::vector<DeviceParameter*> parameters);
	bool getTimeRanges();
	bool start();
	//unsigned _stdcall AeratorThread(void *params);
};

#endif // !AERATOR_H
