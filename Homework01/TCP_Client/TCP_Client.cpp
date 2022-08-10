#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>

#define BUFF_SIZE 2049
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define ENDING_DELIMITER "\r\n"
#define ENDING_DELIMITER_LENGTH 2

#pragma comment (lib, "Ws2_32.lib")

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

/**
* @function printServerMessage: print server's message to screen without prefix
*
* @param message: A message that need to be printed
*
**/
void printServerMessage(std::string& message)
{
	printf("%s\n", &message[1]);
}

/*
* @function sendMessage: handle TCP streaming when sending message
*
* @param connSockPtr: A pointer to a socket that is connecting with server
* @param message: A message that need to be sent
* @param buff: Application's buffer
*
* @return: number of bytes sent if no error occurs
		   SOCKET_ERROR if an error occurs
*/
int sendMessage(SOCKET* connSockPtr, std::string message, char* buff)
{
	// this code is inspired by Mr.Tung's lecture
	message += ENDING_DELIMITER;
	int leftBytes = message.length();
	int index = 0, ret = 0;
	while (leftBytes > 0)
	{
		int numCopiedChar = message.copy(buff, BUFF_SIZE, index);
		buff[numCopiedChar] = 0;
		ret = send(*connSockPtr, buff, numCopiedChar, 0);
		if (ret == SOCKET_ERROR)
		{
			index = ret;
			break;
		}
		leftBytes -= ret;
		index += ret;
	}
	return index;
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
		printf("Error %d: Cannot connect server\n", WSAGetLastError());
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