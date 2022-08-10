#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include "Helper.h"

#define BUFF_SIZE 32769
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"


#pragma comment (lib, "Ws2_32.lib")

/**
* @function printServerMessage: print server's message to screen without prefix
*
* @param message: A message that need to be printed
*
**/
void printServerMessage(std::string& message)
{
	printf("%s\n", &message[0]);
}




/*
* @function handleReceivedMessages: handle messages received from server
*
* @param messageQueue: A string represents a queue which may contains multiple messages
* @param buff: Application's buffer
*
* @return: number of handled messages
*/
int handleReceivedMessages(std::string& messageQueue, char* buff)
{
	//used to store first occurrence of ending delimiter in message queue
	int posOfDelimiter = std::string::npos;
	int numHandledMessages = 0;
	messageQueue.append(buff);

	// loop until message queue doesn't contain ending delimiter
	while ((posOfDelimiter = messageQueue.find(ENDING_DELIMITER)) != std::string::npos)
	{
		//get a message from message queue
		std::string message = messageQueue.substr(0, posOfDelimiter);
		printServerMessage(message);
		++numHandledMessages;
		messageQueue.erase(0, posOfDelimiter + ENDING_DELIMITER_LENGTH);
	}
	return numHandledMessages;
}

int main(int argc, char* argv[]) {
	int serverPort = checkCommandLineArgument(argc, argv);
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
		printf("Error %d: Cannot create client socket", WSAGetLastError());
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
		printf("Error: Something wrong with provided IP address. Please try again\n");
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
	int ret, messageLen;
	std::string userMessage;
	std::string messageQueue = "";
	while (1) {
		//Send message to server
		printf("Send to server: ");
		getline(std::cin, userMessage);
		messageLen = userMessage.length();
		if (messageLen > BUFF_SIZE)
		{
			printf("Your message is too long. Please send another message\n");
			continue;
		}
		if (messageLen == 0)
			break;

		ret = sendMessage(&client, userMessage, buff);
		if (ret == SOCKET_ERROR)
		{
			printf("Error %d: Cannot send data\n", WSAGetLastError());
			break;
		}

		while (1)
		{
			//Receive message from server until the message queue is empty
			ret = recv(client, buff, BUFF_SIZE - 1, 0);
			if (ret == SOCKET_ERROR)
			{
				printf("Error %d: Cannot receive data\n", WSAGetLastError());
				break;
			}
			else if (strlen(buff) > 0)
			{
				buff[ret] = 0;
				handleReceivedMessages(messageQueue, buff);
				if (messageQueue.empty())
					break;
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