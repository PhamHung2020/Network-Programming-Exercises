#include "Feeder.h"

Feeder::Feeder(int id, std::string name, std::vector<DeviceParameter*> parameters) : Device(id, name, parameters)
{
	/*SingleDeviceParameter<double> *amount = new SingleDeviceParameter<double>("W", "Feeding Amount", 100);

	std::vector<Time> tps;
	MultipleDeviceParameter<Time> *times = new MultipleDeviceParameter<Time>("Tp", "Times", tps);

	this->addParameter(amount);
	this->addParameter(times);*/
}

unsigned _stdcall FeederThread(void *params)
{
	Feeder *feeder = (Feeder*)params;
	DeviceParameter *parameter = feeder->getParameterReference("Tp");
	MultipleDeviceParameter<Time> *times = (MultipleDeviceParameter<Time>*) parameter;

	std::vector<std::pair<int, Time>> FeedingTimes = times->getParameters();

	while (true)
	{
		std::time_t nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		struct tm nowTm;
		localtime_s(&nowTm, &nowTime);
		Time t = Time(nowTm.tm_hour, nowTm.tm_min, nowTm.tm_sec);

		for (auto ft : FeedingTimes)
		{
			if (t == ft.second)
			{
				feeder->setStatus(true);
				std::cout << "Fed the fish " << feeder->getParameter("W") << std::endl;
				Sleep(1000);
			}

			else
				feeder->setStatus(false);
		}
	}

}



bool Feeder::start()
{
	return false;
}