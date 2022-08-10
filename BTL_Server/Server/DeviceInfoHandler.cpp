#include "DeviceInfoHandler.h"

bool parseDeviceInfo(std::string deviceInfoStr, DeviceInfo & deviceInfo)
{
	// deviceId ip port
	std::vector<std::string> deviceInfoComponents;
	int numComponent = splitString(deviceInfoStr, " ", 1, deviceInfoComponents, 3);
	if (numComponent != 3)
	{
		printf("Device information is in invalid format");
		return false;
	}

	int deviceId;
	std::string devicePass;
	if (!isInt(deviceInfoComponents[0], deviceId))
	{
		printf("Device information is in invalid format");
		return false;
	}

	deviceInfo.deviceId = deviceId;
	deviceInfo.devicePass = deviceInfoComponents[1];
	deviceInfo.deviceName = deviceInfoComponents[2];
	return true;
}


bool readDeviceInfoFromFile(const std::string & filename, std::vector<DeviceInfo>& deviceInfos)
{
	std::ifstream deviceInfoFile(filename);
	if (deviceInfoFile.fail())
	{
		std::cout << "Open device info file failed\n";
		return false;
	}

	std::string line = "";
	DeviceInfo deviceInfo;
	//read file line by line, each line is an account
	while (getline(deviceInfoFile, line))
	{
		if (!parseDeviceInfo(line, deviceInfo))
			return false;
		deviceInfos.push_back(deviceInfo);
	}

	deviceInfoFile.close();
	return true;
}