#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>

#define BUFF_SIZE 10
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define ENDING_DELIMITER "\r\n"
#define ENDING_DELIMITER_LENGTH 2

#pragma comment (lib, "Ws2_32.lib")

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
	if (argc != 3)
	{
		printf("This program accepts 2 arguments: ip address and port number");
		exit(-1);
	}

	if (!isNumber(argv[2]))
	{
		printf("The second argument must be a number, which is a port number");
		exit(-1);
	}

	int portNumber = atoi(argv[2]);
	if (portNumber < 0 || portNumber > 65536)
	{
		printf("Port number must be in range [0, 65536]");
		exit(-1);
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
int sendMessage(SOCKET* connSockPtr, std::string& message, char* buff)
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

/**
* @function handleReceivedMessages: handles messages sequentialy in the buffer of application
*
* @param currentMessage: A string that stores the last incomplete message from previous buffer
* @param message: A pointer to buffer
* @param connSockPtr: A pointer to the socket that is connecting with client
*
* @ return: number of messages handled
**/
int handleReceivedMessages(std::string& currentMessage, char* buff)
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
		printServerMessage(currentMessage);
		currentMessage.clear();
	}

	// if buffer hasn't any new messages (posUnhandledMessage = numOfMessages = 0) or 
	// the only one message has been handled (posUnhandledMessage = numOfMessages = 1) --> return
	if (posUnhandledMessage == numOfMessages)
		return posUnhandledMessage;

	// handle remaining messages, except the last one
	for (int i = posUnhandledMessage; i < numOfMessages - 1; ++i)
	{
		printServerMessage(messages[i]);
	}

	int lengthOfBuff = buffStr.length();
	// if buff ends with ending delimiter
	if (buffStr.rfind(ENDING_DELIMITER) == lengthOfBuff - ENDING_DELIMITER_LENGTH)
	{
		// handle the last message
		printServerMessage(messages[numOfMessages - 1]);
		return numOfMessages;
	}
	// if buff doesn't end with ending delimiter, that means the last message is incomplete
	currentMessage = messages[numOfMessages - 1];
	return numOfMessages - 1;
}

int handleReceivedMessages2(std::string& messageQueue, char* buff)
{
	int posOfDelimiter = std::string::npos;
	messageQueue.append(buff);
	while ((posOfDelimiter = messageQueue.find(ENDING_DELIMITER)) != std::string::npos)
	{
		std::string message = messageQueue.substr(0, posOfDelimiter);
		/*if (sendResult(connSockPtr, message, buff) == SOCKET_ERROR)
		{
		continue;
		}*/
		printServerMessage(message);
		messageQueue.erase(0, posOfDelimiter + ENDING_DELIMITER_LENGTH);
	}
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
	std::string currentServerMessage = "";
	while (1) {
		//Send message
		printf("Send to server: ");
		getline(std::cin, userMessage);
		messageLen = userMessage.length();
		if (messageLen == 0)
			break;

		sendMessage(&client, userMessage, buff);

		while (1)
		{
			//Receive message from server until the currentServerMessage is empty
			ret = recv(client, buff, BUFF_SIZE - 1, 0);
			if (ret == SOCKET_ERROR)
			{
				printf("Error %d: Cannot receive data\n", WSAGetLastError());
				break;
			}
			else if (strlen(buff) > 0)
			{
				buff[ret] = 0;
				handleReceivedMessages2(currentServerMessage, buff);
				if (currentServerMessage.empty())
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