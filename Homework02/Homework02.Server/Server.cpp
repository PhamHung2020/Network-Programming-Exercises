#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include "Account.h"
#include "AccountManager.h"
#include "Helper.h"
#include "MessageHandler.h"

#pragma comment(lib, "Ws2_32.lib")
#define SERVER_ADDR "127.0.0.1"
#define FILE_ACCOUNT "account.txt"

struct ClientInfo
{
	SOCKET connectedSocket;
	std::string clientIP;
	int clientPort;

	ClientInfo() {}

	ClientInfo(SOCKET connectedSocket, std::string clientIP, int clientPort)
		: connectedSocket(connectedSocket), clientIP(clientIP), clientPort(clientPort) {}
};

AccountManager accountManager;
//critical for accountManager
CRITICAL_SECTION critical;

/* clientThread - Thread to receive the message from a client and response*/
unsigned __stdcall clientThread(void *param) {
	//create a message handler for this client
	EnterCriticalSection(&critical);
	MessageHandler messageHandler(&accountManager, &critical);
	LeaveCriticalSection(&critical);

	ClientInfo* clientInfoPtr = (ClientInfo*)param;
	SOCKET connectedSocket = clientInfoPtr->connectedSocket;

	std::string messageQueue = "";
	char buff[BUFF_SIZE];
	int ret;
	
	while (1) {
		//receives message from client
		ret = recv(connectedSocket, buff, BUFF_SIZE - 1, 0);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAECONNRESET)
			{
				printf("Client [%s:%d] closed the connection forcibly\n", &(clientInfoPtr->clientIP[0]), clientInfoPtr->clientPort);
				messageHandler.handleByeMessage(false, connectedSocket, buff);
			}
			else
			{
				printf("Error %d: Cannot receive data from client [%s:%d]\n", WSAGetLastError(), &(clientInfoPtr->clientIP[0]), clientInfoPtr->clientPort);
			}
			break;
		}
		else if (ret == 0) {
			messageHandler.handleByeMessage(false, connectedSocket, buff);
			printf("Client [%s:%d] disconnects.\n", &(clientInfoPtr->clientIP[0]), clientInfoPtr->clientPort);
			break;
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			printf("Receive from client [%s:%d] %s\n", &(clientInfoPtr->clientIP[0]), clientInfoPtr->clientPort, buff);
			messageQueue.append(buff);
			//used to store messages splitted from message queue
			std::vector<std::string> messages = std::vector<std::string>();
			//split message queue into messages
			int numOfMessages = Helper::splitString(messageQueue, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
			if (numOfMessages == 0)
				continue;

			for (int i = 0; i < numOfMessages - 1; ++i)
			{
				messageHandler.handleMessage(messages[i], connectedSocket, buff);
			}

			// if message queue is empty after splitting,
			// that means the last message in messages array is a complete message
			// (complete message = message ends with ending delimiter)
			if (messageQueue.empty())
				messageHandler.handleMessage(messages[numOfMessages - 1], connectedSocket, buff);
		}
	}

	closesocket(connectedSocket);
	delete clientInfoPtr;
	return 0;
}

int main(int argc, char* argv[])
{
	int serverPort = Helper::checkCommandLineArgument(argc, argv);
	if (serverPort == -1)
		return 0;

	if (!accountManager.readAccountFromFile(FILE_ACCOUNT))
		return 0;

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
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
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
			printf("Accept incoming connection from [%s:%d]\n", clientIP, clientPort);
			
			ClientInfo* clientInfoPtr = new ClientInfo;
			if (clientInfoPtr == nullptr)
			{
				printf("Error: Cannot serve more client. Heap allocation failed\n");
				closesocket(connSocket);
			}
			clientInfoPtr->connectedSocket = connSocket;
			clientInfoPtr->clientIP = std::string(clientIP);
			clientInfoPtr->clientPort = clientPort;
			HANDLE hThread = (HANDLE)_beginthreadex(0, 0, clientThread, (void*)clientInfoPtr, 0, 0); //start thread
			if (!hThread)
			{
				printf("Error: Cannot serve more client. Thread creation failed\n");
				closesocket(connSocket);
			}
		}
	}

	closesocket(listenSock);

	DeleteCriticalSection(&critical);

	WSACleanup();

	return 0;
}
