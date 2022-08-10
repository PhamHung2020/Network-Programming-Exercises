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
#define BUFF_SIZE 10
#pragma comment(lib, "Ws2_32.lib")

/**
* @function isNumber: check if a string only contains number characters
*
* @param str: An input string
*
* @ return: true, if input string only contains number characters
false, otherwise
**/
bool isNumber(char* str)
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
		exit(-1);
	}

	if (!isNumber(argv[1]))
	{
		printf("The only argument must be a number, which is a port number");
		exit(-1);
	}

	int portNumber = atoi(argv[1]);
	if (portNumber < 0 || portNumber > 65536)
	{
		printf("Port number must be in range [0, 65536]");
		exit(-1);
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


/**
* @function splitString: splits a string into two or more substrings depending on delimiter, but doesn't modify original string
*
* @param str: A pointer to a input string
* @param delimiter: A constant pointer to a delimiter string
* @param arrStr: An empty array of strings that would contain all substrings after splitting
*
* @ return: number of substrings after splitting
**/
int splitString(char* str, const char* delimiter, std::vector<std::string>& arrStr)
{
	// copy input string to another string and use the new string to split
	char copyOfStr[BUFF_SIZE];
	strcpy_s(copyOfStr, str);
	// used to store position information between calls to the function strtok_s
	char* nextToken = NULL;
	// used to store next substring after each call to the function strtok_s
	char* ptr = strtok_s(copyOfStr, delimiter, &nextToken);
	while (ptr != NULL)
	{
		arrStr.push_back(std::string(ptr));
		ptr = strtok_s(NULL, delimiter, &nextToken);
	}
	return arrStr.size();
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
		ret = send(*connSockPtr, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Error %d: Cannot send data\n", WSAGetLastError());
			index = ret;
			break;
		}
		leftBytes -= ret;
		index += ret;
	}
	memset(buff, 0, BUFF_SIZE);
	message.clear();
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
	int res = caculateSumFromString(message);
	// used to store return value
	int ret;
	// if message is invalid
	if (res == -1)
	{
		//ret = send(*connSockPtr, "-Failed: String contains non-number character", 45, 0);
		ret = sendMessage(connSockPtr, "-Failed: String contains non-number character", buff);
		printf("Sent: -Failed: String contains non-number character\n");
	}
	else
	{
		// add prefix to result before sending
		std::string resStr = "+" + std::to_string(res);
		//ret = send(*connSockPtr, resStr.c_str(), resStr.length(), 0);
		ret = sendMessage(connSockPtr, resStr, buff);
		printf("Sent: %s\n", resStr.c_str());
	}

	/*if (ret == SOCKET_ERROR)
	{
	printf("Error %d: Cannot send data.\n", WSAGetLastError());
	}*/
	return ret;
}


/**
* @function handleReceivedMessages: handles messages sequentialy in the buffer of application
*
* @param currentMessage: A string that stores the last incomplete message from previous buffer
* @param message: A pointer to buffer
* @param connSockPtr: A pointer to the socket that is connecting with client
*
* @ return: number of messages handled
**/
int handleReceivedMessages(std::string& currentMessage, char* buff, SOCKET* connSockPtr)
{
	// convert buff from char* to string for easier handles
	std::string buffStr = std::string(buff);
	int posOfDelimiter = buffStr.find(ENDING_DELIMITER);

	// if buffer doesn't contain ending delimiter
	if (posOfDelimiter == std::string::npos)
	{
		currentMessage.append(buffStr);
		return 0;
	}

	// split buffer into multiple messages
	std::vector<std::string> messages = std::vector<std::string>();
	int numOfMessages = splitString(buff, ENDING_DELIMITER, messages);
	// used to store position of first unhandled message in array messages
	int posUnhandledMessage = 0;

	if (!currentMessage.empty())
	{
		// if buffer doesn't start with ending delimiter
		if (posOfDelimiter != 0)
		{
			currentMessage += messages[0];
			posUnhandledMessage = 1;
		}
		sendResult(connSockPtr, currentMessage, buff);
		currentMessage.clear();
	}

	// if buffer hasn't any new messages (posUnhandledMessage = numOfMessages = 0) or 
	// the only one message has been handled (posUnhandledMessage = numOfMessages = 1) --> return
	if (posUnhandledMessage == numOfMessages)
		return posUnhandledMessage;

	// handle remaining messages, except the last one
	for (int i = posUnhandledMessage; i < numOfMessages - 1; ++i)
	{
		sendResult(connSockPtr, messages[i], buff);
	}

	int lengthOfBuff = buffStr.length();
	// if buff ends with ending delimiter
	if (buffStr.rfind(ENDING_DELIMITER) == lengthOfBuff - ENDING_DELIMITER_LENGTH)
	{
		// handle the last message
		sendResult(connSockPtr, messages[numOfMessages - 1], buff);
		return numOfMessages;
	}
	// if buff doesn't end with ending delimiter, that means the last message is incomplete
	currentMessage = messages[numOfMessages - 1];
	return numOfMessages - 1;
}

int handleReceivedMessages2(std::string& messageQueue, char* buff, SOCKET* connSockPtr)
{
	int posOfDelimiter = std::string::npos;
	messageQueue.append(buff);
	while ((posOfDelimiter = messageQueue.find(ENDING_DELIMITER)) != std::string::npos)
	{
		std::string message = messageQueue.substr(0, posOfDelimiter);
		printf("%s\n", &message[0]);
		/*if (sendResult(connSockPtr, message, buff) == SOCKET_ERROR)
		{
		continue;
		}*/
		sendResult(connSockPtr, message, buff);
		messageQueue.erase(0, posOfDelimiter + ENDING_DELIMITER_LENGTH);
	}
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
		SOCKET connSock;
		connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);
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
		std::string currentMessage = "";
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
				handleReceivedMessages2(currentMessage, buff, &connSock);
			}
			memset(buff, 0, BUFF_SIZE);
		} //end communicating

		closesocket(connSock);
		memset(&clientAddr, 0, sizeof(clientAddr));
	}

	//Step 5: Close socket
	closesocket(listenSock);

	//Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}