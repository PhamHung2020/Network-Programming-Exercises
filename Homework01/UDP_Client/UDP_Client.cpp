#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>

#define BUFF_SIZE 2049
#define FAILURE_MESSAGE_PREFIX '-'
#define LAST_SUCCESS_MESSAGE_PREFIX '+'
#define SUCCESS_MESSAGE_PREFIX '!'

#pragma comment(lib, "Ws2_32.lib")

/**
* @function doesContainNumberOnly: check if a string only contains number characters
*
* @param str: An input string
*
* @ return: true, if input string only contains number characters
			false, otherwise
**/
bool doesContainNumberOnly(char* str)
{
	int length = strlen(str);
	for (int i = 0; i < length; ++i)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

/**
* @function checkCommandLineArgument: check if arguments passed in command line are valid
*
* @param argc: Number of arguments
* @param argv: An array of pointers to arguments
*
* @ return: port number passed in command line if arguments are valid
			exit, otherwise
**/
int checkCommandLineArgument(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("This program accepts 2 arguments: ip address and port number");
		exit(0);
	}

	if (!doesContainNumberOnly(argv[2]))
	{
		printf("The second argument must be a number, which is a port number");
		exit(0);
	}

	int portNumber = atoi(argv[2]);
	if (portNumber < 0 || portNumber > 65536)
	{
		printf("Port number must be in range [0, 65536]");
		exit(0);
	}
	return portNumber;
}

/*
* @function printServerMessage: print server's message to screen without prefix
*
* @param message: A message that need to be printed
* @param isFirstMessage: A flag indicates the message is the first received message or not
*
* @return: -1 if input message is a failure message
			0 if input message is the last success message
			1 if input message is a success message but not the last one
*/
int printServerMessage(char* message, bool isFirstMessage)
{
	if (message[0] == FAILURE_MESSAGE_PREFIX)
	{
		printf("%s\n", &message[1]);
		return -1;
	}
	else if (message[0] == SUCCESS_MESSAGE_PREFIX || message[0] == LAST_SUCCESS_MESSAGE_PREFIX)
	{
		if (isFirstMessage)
		{
			printf("IP Addresses: \n%s", &message[1]);
		}
		else
		{
			printf("%s", &message[1]);
		}
	}
	return message[0] == SUCCESS_MESSAGE_PREFIX;
}

int main(int argc, char* argv[])
{
	int serverPort = checkCommandLineArgument(argc, argv);

	// Step 1: Initiate Winsock
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata))
	{
		printf("Winsock 2.2 is not supported");
		return 0;
	}

	// Step 2: Construct socket
	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == SOCKET_ERROR)
	{
		printf("Error %d: Cannot create client socket", WSAGetLastError());
		return 0;
	}

	// Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
	if (res == 0)
	{
		printf("The provided IP address format is invalid\n");
		closesocket(client);
		return 0;
	}
	else if (res == -1)
	{
		printf("Error: Something wrong with provided IP address. Please try again\n");
		closesocket(client);
		return 0;
	}

	printf("Client is started\n");


	// Step 4: Communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(serverAddr), messageLen;
	while (1)
	{
		//Send message to server
		std::string userMessage = "";
		printf("\nSend to server: ");
		getline(std::cin, userMessage);
		//gets_s(buff, BUFF_SIZE - 1);
		messageLen = userMessage.length();
		if (messageLen >= BUFF_SIZE)
		{
			printf("Your message is too long. Please send another message\n");
			continue;
		}
		if (messageLen == 0) break;

		userMessage.copy(buff, messageLen);
		ret = sendto(client, buff, messageLen, 0, (sockaddr*)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR)
		{
			printf("Error %d: Cannot send message\n", WSAGetLastError());
			continue;
		}

		//Receive messages from server

		//if received message is the first one
		bool isFirstMessage = true;
		//loop until receive all messages or failed receiving limit is reached
		while (1)
		{
			ret = recvfrom(client, buff, BUFF_SIZE - 1, 0, (sockaddr*)&serverAddr, &serverAddrLen);
			if (ret == SOCKET_ERROR)
			{
				printf("Error %d: Cannot receive message\n", WSAGetLastError());
				break;
			}
			else if (ret > 0)
			{
				buff[ret] = 0;
				int res = printServerMessage(buff, isFirstMessage);
				if (res == 0 || res == -1) //if this is a failure or last message
				{
					break;
				}
				isFirstMessage = false;
			}
		}
		
	} //end communicating

	//Step 5: Close socket
	closesocket(client);

	//Step 6: Clean up Winsock
	WSACleanup();

	return 0;
}