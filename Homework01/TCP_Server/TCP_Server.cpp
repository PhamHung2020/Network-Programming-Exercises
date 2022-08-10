#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>

#define SERVER_ADDR "127.0.0.1"
#define ENDING_DELIMITER "\r\n"
#define ENDING_DELIMITER_LENGTH 2
#define BUFF_SIZE 65537

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
	if (argc != 2)
	{
		printf("This program only accepts 1 argument, which is a port number.");
		exit(0);
	}

	if (!doesContainNumberOnly(argv[1]))
	{
		printf("The only argument must be a number, which is a port number");
		exit(0);
	}

	int portNumber = atoi(argv[1]);
	if (portNumber < 0 || portNumber > 65536)
	{
		printf("Port number must be in range [0, 65536]");
		exit(0);
	}

	return portNumber;
}

/**
* @function calculateSumFromString: calculates sum of all digits in a string if the string contains only number characters
*
* @param str: An input string
*
* @ return: sum of all digits in the input string if it contains only number characters
			-1 if the input string contains non-number characters
**/
int caculateSumFromString(std::string str)
{
	int sum = 0;
	int length = str.length();
	for (int i = 0; i < length; ++i)
	{
		if (isdigit(str[i]))
			sum += (str[i] - '0');
		else
			return -1;
	}
	return sum;
}


/*
* @function sendMessage: handle TCP streaming when sending message
*
* @param serverSock: A pointer to a socket that is connecting with client
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

/**
* @function sendResult: handle a message and sends reuslt to client
*
* @param connSockPtr: A pointer to the socket that is connecting with client
* @param message: A string that need to be handled
*
* @ return: number of bytes sent if no error occurs
			SOCKET_ERROR if an error occurs
**/
int sendResult(SOCKET* connSockPtr, std::string& message, char* buff)
{
	// calculate result from message
	int sum = caculateSumFromString(message);
	// used to store return value
	int ret;
	// if message is invalid
	if (sum == -1)
	{
		ret = sendMessage(connSockPtr, "-Failed: String contains non-number character", buff);
		printf("Sent: -Failed: String contains non-number character\n");
	}
	else
	{
		// add prefix to result before sending
		std::string resStr = "+" + std::to_string(sum);
		//ret = send(*connSockPtr, resStr.c_str(), resStr.length(), 0);
		ret = sendMessage(connSockPtr, resStr, buff);
		printf("Sent: %s\n", resStr.c_str());
	}

	return ret;
}

/*
* @function handleReceivedMessages: handle messages received from server
*
* @param messageQueue: A string represents a queue which may contains multiple messages
* @param buff: Application's buffer
* @param connSockPtr: A pointer to a socket associated with a connection
*
* @return: number of handled messages
*/
int handleReceivedMessages(std::string& messageQueue, char* buff, SOCKET* connSockPtr)
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
		if (sendResult(connSockPtr, message, buff) == SOCKET_ERROR)
		{
			printf("Error %d: Cannot send data\n", WSAGetLastError());
			break;
		}
		++numHandledMessages;
		messageQueue.erase(0, posOfDelimiter + ENDING_DELIMITER_LENGTH);
	}
	return numHandledMessages;
}

int main(int argc, char* argv[])
{
	int serverPort = checkCommandLineArgument(argc, argv);

	// Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
	{
		printf("Winsock 2.2 is supported\n");
		return 0;
	}

	// Step 2: Construct socket
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create server socket", WSAGetLastError());
		return 0;
	}

	// Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("(Error: %d) Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	// Step 4: Listen request from client
	if (listen(listenSock, 10))
	{
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	//Step 5: Communicate with client
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;
	while (1)
	{
		//reset clientAddr before using for new connection
		memset(&clientAddr, 0, sizeof(clientAddr));
		SOCKET connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);
		if (connSock == SOCKET_ERROR)
		{
			printf("Error %d: Cannot permit incoming connection.", WSAGetLastError());
			continue;
		}
		else
		{
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Accept incoming connection from %s:%d\n", clientIP, clientPort);
		}

		char buff[BUFF_SIZE];
		std::string messageQueue = "";
		while (1)
		{
			//receive message from client
			ret = recv(connSock, buff, BUFF_SIZE - 1, 0);
			if (ret == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("The connection was forcibly closed\n");
				}
				else
				{
					printf("Error %d: Cannot receive data\n", WSAGetLastError());
				}
				break;
			}
			else if (ret == 0)
			{
				printf("Client disconnect\n");
				break;
			}
			else
			{
				buff[ret] = 0;
				handleReceivedMessages(messageQueue, buff, &connSock);
			}
		} //end communicating

		closesocket(connSock);
	}

	//Step 5: Close socket
	closesocket(listenSock);

	//Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}