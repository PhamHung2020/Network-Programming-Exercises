#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <string.h>
#include <queue>
#include "Time.h"
#include "Validation.h"
#include "Utility.h"
#include "ProtocolHandler.h"
#include "IOHelper.h"


#pragma comment(lib, "Ws2_32.lib")

//HANDLE completionPort;
std::string serverCenterAddr = "";
std::string configFile = "";
int serverCenterPort = 5500;
std::string id = "1";
std::string pass = "123";
char buff[DATA_BUFSIZE];
int serverPort = 5500;
sockaddr_in serverAddr;
SessionInfo sessionInfo;
SOCKET client;
Device* device;


bool connectServer()
{
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create client socket. Please try again", WSAGetLastError());
		return 0;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, serverCenterAddr.c_str(), &serverAddr.sin_addr);
	if (res == 0)
	{
		printf("The provided IP address format is invalid\n");
		return 0;
	}
	else if (res == -1)
	{
		printf("Error %d: Something wrong with provided IP address. Please try again\n", WSAGetLastError());
		return 0;
	}

	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot connect server. Please check IP address and port number, or ensure server is running, then run program again.\n", WSAGetLastError());
		return 0;
	}

	sessionInfo.setConnSock(client);

	int ret = sendMessage(client, "DEVICE " + id + ' ' + pass, buff);
	if (ret == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAECONNABORTED)
			printf("Server is unavailable for now. Please restart program or use this program in another time\n");
		else
			printf("Error %d: Cannot send data. Please try again\n", WSAGetLastError());
		return 0;
	}

	while (true)
	{
		int ret = recv(client, buff, DATA_BUFSIZE - 1, 0);
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAECONNABORTED)
				printf("Server is unavailable for now. Please restart program or use this program in another time\n");
			else
				printf("Error %d: Cannot receive data. Please restart program or use this program in another time\n", WSAGetLastError());
			break;
		}
		else if (ret == 0)
		{
			printf("Server is unavailable for now. Please restart program or use this program in another time\n");
			break;
		}
		else if (strlen(buff) > 0)
		{
			buff[ret] = 0;
			sessionInfo.receiveMessages(buff);
			if (sessionInfo.getMessageNumber() > 0)
				break;
		}
	}

	if (sessionInfo.getMessageNumber() > 0)
	{
		std::string message = sessionInfo.getNextMessage();
		if (message == ProtocolHandler::DEVICE_APPROVED)
		{
			return true;
		}
		else
		{
			printf("Device is not approved. Please check id or password\n");
			return false;
		}
	}
	else
	{
		return false;
	}
}

void reconnect()
{
	while (true)
	{
		printf("Reconnecting to server...\n");
		closesocket(client);
		if (connectServer())
		{
			printf("Server connected\n");
			break;
		}
		Sleep(2000);
	}
}

void initializeDevice()
{
	int deviceId;
	device = readDevice(configFile, device, deviceId, pass);
	id = std::to_string(deviceId);
}

int main(int argc, char* argv[])
{
	int serverport = checkCommandLineArgument(argc, argv);
	if (serverport == -1)
		return 0;
	serverCenterAddr = argv[1];
	configFile = argv[3];
	initializeDevice();
	device->start();
	

	WSADATA wsaData;
	if (WSAStartup((2, 2), &wsaData) != 0) {
		printf("WSAStartup() failed with error %d\n", GetLastError());
		return 1;
	}

	if (!connectServer())
	{
		closesocket(client);
		WSACleanup();
		return 0;
	}
	printf("Server connected!\n");

	while (true) 
	{
		bool isServerDisconnect = false;
		while (true)
		{
			int ret = recv(client, buff, DATA_BUFSIZE - 1, 0);
			if (ret == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAECONNABORTED || WSAGetLastError() == WSAECONNRESET)
					printf("Server is unavailable for now\n");
				else
					printf("Error %d: Cannot receive data\n", WSAGetLastError());
				isServerDisconnect = true;
				break;
			}
			else if (ret == 0)
			{
				printf("Server is unavailable for now\n");
				isServerDisconnect = true;
				break;
			}
			else if (strlen(buff) > 0)
			{
				buff[ret] = 0;
				sessionInfo.receiveMessages(buff);
				if (sessionInfo.getMessageNumber() > 0)
					break;
			}
		}

		if (isServerDisconnect)
		{
			sessionInfo.reset();
			reconnect();
		}
		else
		{
			while (sessionInfo.getMessageNumber() > 0)
			{
				std::string message = sessionInfo.getNextMessage();
				std::string response = ProtocolHandler::handleMessage(message, *device);
				int ret = sendMessage(client, response, buff);
				if (ret == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAECONNABORTED)
						printf("Server is unavailable for now. Please restart program or use this program in another time\n");
					else
						printf("Error %d: Cannot send data. Please try again\n", WSAGetLastError());
					continue;
				}
			}
		}
	}

	closesocket(client);
	WSACleanup();

	getchar();
	return 0;
}

