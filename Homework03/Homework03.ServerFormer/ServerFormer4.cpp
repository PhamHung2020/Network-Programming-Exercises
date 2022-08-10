#define FD_SETSIZE 1000

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include "Helper.h"
#include "AccountHandler.h"
#include "MessageHandler.h"

#pragma comment(lib, "Ws2_32.lib")
#define SERVER_ADDR "127.0.0.1"
#define FILE_ACCOUNT "account.txt"
#define MAX_CLIENT 5000


const int MAX_THREAD = MAX_CLIENT / FD_SETSIZE + 1;
std::vector<MessageHandler> clients = std::vector<MessageHandler>(MAX_CLIENT, MessageHandler(0));
fd_set initfdsThread[MAX_THREAD];
int numOfThread[MAX_THREAD];

CRITICAL_SECTION criticals[MAX_THREAD];

/* clientThread - Thread to receive the message from a client and response*/
unsigned __stdcall clientThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;
	int numThread = *(int*)param;
	int start = numThread * FD_SETSIZE;
	int end = start + FD_SETSIZE;

	fd_set initfds;
	FD_ZERO(&initfds);

	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 20000;
	int retry = 2;

	while (retry > 0) {
		//EnterCriticalSection(&criticals[numThread]);
		initfds = initfdsThread[numThread];
		//LeaveCriticalSection(&criticals[numThread]);

		int nEvents = select(0, &initfds, 0, 0, &timeout);
		if (nEvents < 0)
		{
			--retry;
			printf("Thread %d: %d\n", numThread, WSAGetLastError());
			continue;
		}
		else if (nEvents == 0)
		{
			retry = 2;
			continue;
		}

		retry = 2;
		//receives message from client
		for (int i = start; i < end; ++i)
		{
			if (clients[i].connSock == 0)
			{
				continue;
			}

			if (FD_ISSET(clients[i].connSock, &initfds))
			{
				ret = recv(clients[i].connSock, buff, BUFF_SIZE - 1, 0);
				if (ret <= 0)
				{
					if (ret == SOCKET_ERROR)
					{
						if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
						{
							clients[i].handleByeMessage(false, buff);
						}
						else
						{
							continue;
						}
					}
					else if (ret == 0)
					{
						clients[i].handleByeMessage(false, buff);
					}

					EnterCriticalSection(&criticals[numThread]);
					FD_CLR(clients[i].connSock, &initfdsThread[numThread]);
					LeaveCriticalSection(&criticals[numThread]);
					closesocket(clients[i].connSock);
					clients[i].reset();
				}
				else
				{
					buff[ret] = 0;
					clients[i].handleReceivedMessage(buff);
				}
			}
		}
	}

	EnterCriticalSection(&criticals[numThread]);
	for (int i = start; i < end; ++i)
	{
		if (clients[i].connSock != 0)
		{
			closesocket(clients[i].connSock);
			clients[i].reset();
		}
	}
	FD_ZERO(&initfdsThread[numThread]);
	numOfThread[numThread] = -1;
	LeaveCriticalSection(&criticals[numThread]);

	printf("Thread %d exit\n", numThread);
	return 0;
}

int main(int argc, char* argv[])
{
	int serverPort = Helper::checkCommandLineArgument(argc, argv);
	if (serverPort == -1)
		return 0;
	if (!AccountHandler::readAccountFromFile(FILE_ACCOUNT, MessageHandler::accountStatus))
	{
		getchar();
		return 0;
	}

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

	for (int i = 0; i < MAX_THREAD; ++i)
	{
		InitializeCriticalSection(&criticals[i]);
		FD_ZERO(&initfdsThread[i]);
	}
	memset(numOfThread, -1, sizeof(numOfThread));

	printf("Server started!\n");

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
			//printf("Accept incoming connection from [%s:%d]\n", clientIP, clientPort);

			int i;
			for (i = 0; i < MAX_CLIENT; ++i)
			{
				int threadPos = i / FD_SETSIZE;
				EnterCriticalSection(&criticals[threadPos]);
				if (clients[i].connSock == 0)
				{
					clients[i].connSock = connSocket;
					FD_SET(connSocket, &initfdsThread[threadPos]);
					if (numOfThread[threadPos] == -1)
					{
						numOfThread[threadPos] = threadPos;
						_beginthreadex(0, 0, clientThread, (void*)&numOfThread[threadPos], 0, 0); //start thread
					}
					LeaveCriticalSection(&criticals[threadPos]);
					break;
				}
				else
				{
					LeaveCriticalSection(&criticals[threadPos]);
				}
			}
			if (i == MAX_CLIENT)
			{
				printf("Max client reached\n");
				closesocket(connSocket);
			}
		}
	}

	closesocket(listenSock);

	for (int i = 0; i < MAX_THREAD; ++i)
	{
		DeleteCriticalSection(&criticals[i]);
	}

	WSACleanup();

	return 0;
}
