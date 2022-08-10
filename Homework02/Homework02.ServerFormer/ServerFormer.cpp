#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include "Account.h"
#pragma comment(lib, "Ws2_32.lib")
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"
#define ENDING_DELIMITER_LENGTH 2

#define FILE_ACCOUNT "account.txt"
#define RESPONSE_LENGTH 4
#define MESSAGE_TYPE_NOT_EXIST_CODE "99\r\n"
#define MESSAGE_INVALID_FORMAT "98\r\n"
#define USER_LOGIN_OK "10\r\n"
#define USER_ACCOUNT_NOT_ACTIVE "11\r\n"
#define USER_ACCOUNT_ALREADY_LOGIN_CODE "13\r\n"
#define USER_ACCOUNT_NOT_EXIST_CODE "12\r\n"
#define POST_OK "20\r\n"
#define POST_ACCOUNT_NOT_LOGIN_YET "21\r\n"
#define BYE_OK "30\r\n"

struct ClientInfo
{
	SOCKET connectedSocket;
	char* clientIP;
	int clientPort;

	ClientInfo(SOCKET connectedSocket, char* clientIP, int clientPort)
		: connectedSocket(connectedSocket), clientIP(clientIP), clientPort(clientPort) {}
};

std::vector<Account> accounts = std::vector<Account>();
int numOfAccount = 0;
CRITICAL_SECTION critical;

/**
* @function splitString: splits a string into two or more substrings depending on delimiter, but doesn't modify original string
*
* @param str: A pointer to a input string
* @param delimiter: A constant pointer to a delimiter string
* @param arrStr: An empty array of strings that would contain all substrings after splitting
*
* @ return: number of substrings after splitting
**/
int splitString(std::string& str, const char* delimiter, int delimiterLength, std::vector<std::string>& arrStr, int count)
{
	//used to store first occurrence of ending delimiter in message queue
	int posOfDelimiter = std::string::npos;
	int numSubString = 0;

	while (count != 0 && (posOfDelimiter = str.find(delimiter)) != std::string::npos)
	{
		std::string substring = str.substr(0, posOfDelimiter);
		if (substring != "")
		{
			arrStr.push_back(substring);
			++numSubString;
			--count;
		}
		str.erase(0, posOfDelimiter + delimiterLength);
	}
	if (count > 0 && str != "")
	{
		++numSubString;
		arrStr.push_back(str);
	}
	return numSubString;
}

void readAccounts(std::string filename)
{
	std::ifstream accountFile(filename);
	if (accountFile.fail())
	{
		std::cout << "Open file failed\n";
		exit(0);
	}
	std::string line = "";
	while (getline(accountFile, line))
	{
		accounts.push_back(Account::parseAccount(line));
	}
	numOfAccount = accounts.size();
	accountFile.close();
}

/* The send() wrapper function*/
int Send(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = send(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot send data", WSAGetLastError());
	return n;
}

int handleUserMessage(std::vector<std::string> messageComponents, bool* isLogin, SOCKET* connSockPtr)
{
	int numOfParameter = messageComponents.size() - 1;
	if (numOfParameter != 1)
	{
		Send(*connSockPtr, MESSAGE_INVALID_FORMAT, RESPONSE_LENGTH, 0);
		return -1;
	}
	if (*isLogin)
	{
		Send(*connSockPtr, USER_ACCOUNT_ALREADY_LOGIN_CODE, RESPONSE_LENGTH, 0);
		return -1;
	}
	int posOfAccount = -1;
	Account account;
	for (int i = 0; i < numOfAccount; ++i)
	{
		EnterCriticalSection(&critical);
		if (accounts[i].username == messageComponents[1])
		{
			account = accounts[i];
			posOfAccount = i;
		}
		LeaveCriticalSection(&critical);
		if (posOfAccount != -1)
			break;
	}
	if (posOfAccount == -1)
	{
		Send(*connSockPtr, USER_ACCOUNT_NOT_EXIST_CODE, RESPONSE_LENGTH, 0);
		return -1;
	}
	if (account.isLogin)
	{
		Send(*connSockPtr, USER_ACCOUNT_ALREADY_LOGIN_CODE, RESPONSE_LENGTH, 0);
		return -1;
	}
	if (account.isLocked)
	{
		Send(*connSockPtr, USER_ACCOUNT_NOT_ACTIVE, RESPONSE_LENGTH, 0);
		return -1;
	}
	EnterCriticalSection(&critical);
	accounts[posOfAccount].isLogin = true;
	LeaveCriticalSection(&critical);
	*isLogin = true;
	Send(*connSockPtr, USER_LOGIN_OK, RESPONSE_LENGTH, 0);
	return posOfAccount;
}

bool handlePostMessage(std::vector<std::string> messageComponents, bool* isLogin, SOCKET* connSockPtr)
{
	int numOfParameter = messageComponents.size() - 1;
	if (numOfParameter != 1)
	{
		Send(*connSockPtr, MESSAGE_INVALID_FORMAT, RESPONSE_LENGTH, 0);
		return false;
	}
	if (!(*isLogin))
	{
		Send(*connSockPtr, POST_ACCOUNT_NOT_LOGIN_YET, RESPONSE_LENGTH, 0);
		return false;
	}
	Send(*connSockPtr, POST_OK, RESPONSE_LENGTH, 0);
	return true;
}

bool handleByeMessage(int* posOfAccount, bool* isLogin, SOCKET* connSockPtr)
{
	if (*posOfAccount >= numOfAccount)
		return false;
	if (!(*isLogin))
	{
		Send(*connSockPtr, POST_ACCOUNT_NOT_LOGIN_YET, RESPONSE_LENGTH, 0);
		return false;
	}
	EnterCriticalSection(&critical);
	accounts[*posOfAccount].isLogin = false;
	LeaveCriticalSection(&critical);
	*isLogin = false;
	Send(*connSockPtr, BYE_OK, RESPONSE_LENGTH, 0);
	*posOfAccount = -1;
	return true;

}

bool handleMessage(std::string message, SOCKET* connSockPtr, bool* isLogin, int* posOfAccount)
{
	// discard white space in the end of message
	int messageLength = message.length();
	while (message[messageLength - 1] == ' ')
	{
		message.erase(messageLength - 1, 1);
		--messageLength;
	}

	std::vector<std::string> messageComponents = std::vector<std::string>();
	int numOfComponent = splitString(message, " ", 1, messageComponents, 2);
	if (numOfComponent < 1)
	{
		Send(*connSockPtr, MESSAGE_TYPE_NOT_EXIST_CODE, RESPONSE_LENGTH, 0);
		return false;
	}

	std::string messageType = messageComponents[0];
	if (messageType == "USER")
	{
		*posOfAccount = handleUserMessage(messageComponents, isLogin, connSockPtr);
		return (*posOfAccount == -1) ? false : true;
	}
	else if (messageType == "POST")
	{
		return handlePostMessage(messageComponents, isLogin, connSockPtr);
	}
	else if (messageType == "BYE")
	{
		return handleByeMessage(posOfAccount, isLogin, connSockPtr);
	}
	else
	{
		Send(*connSockPtr, MESSAGE_TYPE_NOT_EXIST_CODE, RESPONSE_LENGTH, 0);
	}
	return false;
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
	int numHandledMessages = 0;
	messageQueue.append(buff);

	std::vector<std::string> messages = std::vector<std::string>();
	int numOfMessages = splitString(messageQueue, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
	for (int i = 0; i < numOfMessages; ++i)
	{
		/*messages[i] += "\r\n";
		int ret = send(*connSockPtr, messages[i].c_str(), messages[i].length(), 0);
		if (ret == SOCKET_ERROR)
		{
		printf("Error %d: Cannot send data\n", WSAGetLastError());
		continue;
		}*/
		++numHandledMessages;
	}
	return numHandledMessages;
}

/* echoThread - Thread to receive the message from client and echo*/
unsigned __stdcall echoThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;

	ClientInfo* clientInfoPtr = (ClientInfo*)param;
	SOCKET connectedSocket = clientInfoPtr->connectedSocket;
	char* clientIP = clientInfoPtr->clientIP;
	int clientPort = clientInfoPtr->clientPort;

	std::string messageQueue = "";
	bool isLogin = false;
	int posOfAccount = -1;
	while (1) {
		ret = recv(connectedSocket, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
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
		else if (ret == 0) {
			printf("Client[%s:%d] disconnects.\n", clientIP, clientPort);
			handleByeMessage(&posOfAccount, &isLogin, &connectedSocket);
			break;
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			printf("Receive from client[%s:%d] %s\n", clientIP, clientPort, buff);
			messageQueue.append(buff);
			std::vector<std::string> messages = std::vector<std::string>();
			int numOfMessages = splitString(messageQueue, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
			for (int i = 0; i < numOfMessages - 1; ++i)
			{
				handleMessage(messages[i], &connectedSocket, &isLogin, &posOfAccount);
			}
			if (messages[numOfMessages - 1] != messageQueue)
				handleMessage(messages[numOfMessages - 1], &connectedSocket, &isLogin, &posOfAccount);
			//Echo to client
			//ret = send(connectedSocket, buff, ret, 0);
			//handleReceivedMessages(messageQueue, buff, &connectedSocket);
		}
	}

	closesocket(connectedSocket);
	return 0;
}

int main(int argc, char* argv[])
{
	readAccounts(FILE_ACCOUNT);

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Step 2: Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 100)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");
	InitializeCriticalSection(&critical);

	//Step 5: Communicate with client
	SOCKET connSocket;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr), clientPort;
	while (1)
	{
		connSocket = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		if (connSocket == SOCKET_ERROR)
			printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
		else
		{
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Accept incoming connection from %s:%d\n", clientIP, clientPort);
			ClientInfo clientInfo(connSocket, clientIP, clientPort);
			_beginthreadex(0, 0, echoThread, (void *)&clientInfo, 0, 0); //start thread
		}
	}

	closesocket(listenSock);

	DeleteCriticalSection(&critical);

	WSACleanup();

	return 0;
}
