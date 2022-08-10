#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <string.h>
#include <conio.h>
#include "Helper.h"
#include "Notification.h"
#include "UI.h"
#include "InputHandler.h"
#include "MessageSender.h"

#pragma comment(lib, "Ws2_32.lib")
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 64000
#define ENDING_DELIMITER "\r\n"
#define BREAK_CODE 100
#define GET_DEVICE_CODE 101


HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

SOCKET client;
std::string response;
char buff[BUFF_SIZE];
bool isLogin = false;
Sensor sensor;

int main(int argc, char* argv[]) {

	/*int serverPort = Helper::checkCommandLineArgument(argc, argv);
	if (serverPort == -1) {
	return 0;
	}*/

	// Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}


	// Construct socket
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	// Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	int res = inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
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

	//Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot connect server. Please check IP address and port number, or ensure server is running, then run program again.\n", WSAGetLastError());
		return 0;
	}

	printf("Connected server!\n");
	int ret = MessageSender::sendHello();
	HandleActionClient::handleRecv(ret);
	if (response == CLIENT_APPROVED) {
		UI::App(UI::LOGIN_PAGE);
	}
	else {
		Notification::errorMessage("Client is not accepted");
	}

	std::cout << "Enter any character to exit the program";
	getchar();
	return 0;
}