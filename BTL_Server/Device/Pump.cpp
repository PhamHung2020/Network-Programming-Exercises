#include "Pump.h"


Pump::Pump(int id, std::string name, std::vector<DeviceParameter*> parameters) : Device(id, name, parameters)
{

	// initial status = false (Pump is turned off)

	// parameter of pump : V (flow rate) and operating time T	
	//SingleDeviceParameter<double> *flowRate = new SingleDeviceParameter<double>("V", "Flow Rate", 100);
	//SingleDeviceParameter<int> *operatingTime = new SingleDeviceParameter<int>("T", "Operating Time", 7);

	////add parameters when creating an instance of Pump
	//this->addParameter(flowRate);
	//this->addParameter(operatingTime);
}




// Thread used to track pump, print status of the pump
unsigned _stdcall PumpThread(void *params)
{
	std::cout << "start thread\n";
	Pump *p = (Pump*)params;
	while (true)
	{
		if (p->getStatus() == true)
		{
			std::cout << "Pump is ON with Flow Rate " << p->getParameter("V") << std::endl;;

			auto now = std::chrono::system_clock::now();
			std::time_t nowTime = std::chrono::system_clock::to_time_t(now);


			int opTime = 10;
			std::time_t pumpOffTime = nowTime + 10;
			if (isInt(p->getParameter("T"), opTime))
				pumpOffTime = nowTime + opTime;

			while (p->getStatus() == true)
			{
				now = std::chrono::system_clock::now();
				nowTime = std::chrono::system_clock::to_time_t(now);

				if (nowTime == pumpOffTime)
					p->setStatus(false);

			}
			if (p->getStatus() == false)
				std::cout << "Pump is OFF";
		}
		
	}
	std::cout << "End thread\n";
}

bool Pump::start()
{
	// start a thread to track the Pump, turn On or Off in realtime
	if (_beginthreadex(0, 0, PumpThread, this, 0, 0))
	{
		std::cout << "Device tracking started!" << std::endl;
		return true;
	}
	else
		return false;
}
