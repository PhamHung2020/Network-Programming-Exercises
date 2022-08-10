#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Helper.h"
#include "InputHandler.h"
#include "MessageSender.h"
#include "UIHandler.h"
#pragma comment (lib, "Ws2_32.lib")

/**
* @function handleMessage: handles a message from server
*
* @param message: A message from server.
* @param userChoice: User's choice from menu
* @param isLogin: User's login status
*
**/
void handleMessage(const std::string& message, int userChoice, bool& isLogin)
{
	if (userChoice == 1 && !isLogin && message == USER_LOGIN_OK)
	{
		isLogin = true;
	}
	else if (userChoice == 2 && isLogin && message == BYE_OK)
	{
		isLogin = false;
	}
	UIHandler::displayResponse(message);
}


int main(int argc, char* argv[]) {
	int serverPort = Helper::checkCommandLineArgument(argc, argv);
	if (serverPort == -1)
		return 0;

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD vVersion = MAKEWORD(2, 2);
	if (WSAStartup(vVersion, &wsaData))
	{
		printf("Version 2.2 is not supported");
		return 0;
	}

	//Step 2: Construct socket
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create client socket. Please try again", WSAGetLastError());
		return 0;
	}

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
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

	//Step 5: Communicate with server
	char buff[BUFF_SIZE];
	int ret, userChoice;
	std::string messageQueue = "";
	bool isLogin = false;

	while (1) {
		//Send message to server
		UIHandler::displayMenu(isLogin);
		if (!isLogin)
		{
			userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 2);
			if (userChoice == 1)
			{
				std::string username = InputHandler::inputString("Username: ");
				ret = MessageSender::sendUserMessage(client, username, buff);
			}
			else
				break;
		}
		else
		{
			userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 3);
			if (userChoice == 1)
			{
				std::string article = InputHandler::inputString("Your article: ");
				ret = MessageSender::sendPostMessage(client, article, buff);
			}
			else if (userChoice == 2)
			{
				ret = MessageSender::sendByeMessage(client, buff);
			}
			else
				break;
		}

		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAECONNABORTED)
				printf("Server is unavailable for now. Please restart program or use this program in another time\n");
			else
				printf("Error %d: Cannot send data. Please try again\n", WSAGetLastError());
			continue;
		}

		//Receive message from server
		while (1)
		{
			//Receive message from server until the message queue is empty
			ret = recv(client, buff, BUFF_SIZE - 1, 0);
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
				messageQueue.append(buff);
				//used to store messages splitted from message queue
				std::vector<std::string> messages = std::vector<std::string>();
				int numOfMessages = Helper::splitString(messageQueue, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
				if (numOfMessages == 0)
					continue;

				for (int i = 0; i < numOfMessages - 1; ++i)
				{
					handleMessage(messages[i], userChoice, isLogin);
				}

				// if message queue is empty after splitting,
				// that means the last message in messages array is a complete message
				// (complete message = message ends with ending delimiter)
				if (messageQueue.empty())
				{
					handleMessage(messages[numOfMessages - 1], userChoice, isLogin);
					break;
				}
			}
		}

	} //end communicating

	  //Step 6: Close socket
	shutdown(client, SD_SEND);
	closesocket(client);

	//Step 7: Clean up winsock
	WSACleanup();
	return 0;
}