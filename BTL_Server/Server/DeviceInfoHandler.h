#ifndef DEVICE_HANDLER_H
#define DEVICE_HANDLER_H

#include <fstream>
#include "Constant.h"
#include "SessionInfo.h"

struct DeviceInfo
{
	SessionInfo sessionInfo;
	int deviceId;
	std::string deviceName;
	std::string devicePass;
	std::string deviceIP;
	int devicePort;
	char buff[DATA_BUFSIZE];

	/*void handleClientMessage()
	{
		if (clientMess.empty())
			return;
		ClientMessage clientMessage = clientMess.front();
		int ret = sendMessage(messageHandler.connSock, clientMessage.message, buff);
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAECONNABORTED)
				printf("Device %d is unavailable for now\n", deviceId);
			else
				printf("Error %d: Cannot send data to device %d. Please try again\n", WSAGetLastError(), deviceId);
		}
		while (1)
		{
			ret = recv(messageHandler.connSock, buff, DATA_BUFSIZE - 1, 0);
			if (ret == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAECONNABORTED)
					printf("Device %d is unavailable for now\n", deviceId);
				else
					printf("Error %d: Cannot receive data to device %d.\n", WSAGetLastError(), deviceId);
				break;
			}
			else if (ret == 0)
			{
				printf("Device %d is unavailable for now\n", deviceId);
				break;
			}
			else if (strlen(buff) > 0)
			{
				buff[ret] = 0;
				messageHandler.handleReceivedMessages(buff);
				if (messageHandler.getMessageNumber() > 0)
					break;
			}
		}

		std::string returnedMessage = messageHandler.getNextMessage() + ENDING_DELIMITER;

		DWORD transferredBytes;
		clientMess.pop();
	}*/
};

bool parseDeviceInfo(std::string deviceInfoStr, DeviceInfo & deviceInfo);

bool readDeviceInfoFromFile(const std::string & filename, std::vector<DeviceInfo>& deviceInfos);

#endif // !DEVICE_HANDLER_H
