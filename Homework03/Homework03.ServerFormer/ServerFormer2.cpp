#define FD_SETSIZE 1000

#include <iostream>
#include <string>
#include <vector>
#include <queue>
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
#define MAX_CLIENT 3000


const int MAX_THREAD = MAX_CLIENT / FD_SETSIZE + 1;
AccountManager accountManager;
std::vector<MessageHandler> clients = std::vector<MessageHandler>(MAX_CLIENT, MessageHandler(&accountManager, 0, "", 0));
int numOfThread[MAX_THREAD];

CRITICAL_SECTION criticals[MAX_THREAD];
//CRITICAL_SECTION criticalForClientQueue;
CRITICAL_SECTION criticalForNumOfThread;


/* clientThread - Thread to receive the message from a client and response*/
unsigned __stdcall clientThread(void *param) {
	char sendBuff[BUFF_SIZE], recvBuff[BUFF_SIZE];
	int ret;
	//EnterCriticalSection(&criticalForNumOfThread);
	int numThread = *(int*)param;
	//LeaveCriticalSection(&criticalForNumOfThread);
	int start = numThread * FD_SETSIZE;
	int end = start + FD_SETSIZE;

	fd_set initfds;
	FD_ZERO(&initfds);

	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	while (1) {
		for (int i = start; i < end; ++i)
		{
			EnterCriticalSection(&criticals[numThread]);
			if (clients[i].connSock != 0 && !FD_ISSET(clients[i].connSock, &initfds))
				FD_SET(clients[i].connSock, &initfds);
			LeaveCriticalSection(&criticals[numThread]);
		}
		//readfds = initfds;

		int nEvents = select(0, &initfds, 0, 0, &timeout);
		if (nEvents < 0)
		{
			if (WSAGetLastError() == WSAEINVAL || WSAGetLastError() == WSAENOTSOCK)
			{
				//printf("Error! Cannot poll sockets: %d\n", WSAGetLastError());
				//continue;
			}
			printf("Error! Cannot poll sockets: %d\n", WSAGetLastError());
			break;
			//continue;
		}
		else if (nEvents == 0)
			continue;

		//receives message from client
		for (int i = start; i < end; ++i)
		{
			if (clients[i].connSock == 0)
			{
				continue;
			}

			if (FD_ISSET(clients[i].connSock, &initfds))
			{
				ret = recv(clients[i].connSock, recvBuff, BUFF_SIZE - 1, 0);
				if (ret <= 0)
				{
					if (ret == SOCKET_ERROR)
					{
						if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
						{
							printf("Client [%s:%d] closed the connection forcibly\n", &(clients[i].clientIP[0]), clients[i].clientPort);
							clients[i].handleByeMessage(false, sendBuff);
						}
						else
						{
							printf("Error %d: Cannot receive data from client [%s:%d]\n", WSAGetLastError(), &(clients[i].clientIP[0]), clients[i].clientPort);
							continue;
						}
					}
					else if (ret == 0)
					{
						clients[i].handleByeMessage(false, sendBuff);
						//printf("Client [%s:%d] disconnects.\n", &(clients[connSockPos].clientIP[0]), clients[connSockPos].clientPort);
					}

					FD_CLR(clients[i].connSock, &initfds);
					closesocket(clients[i].connSock);
					clients[i].reset();
				}
				else
				{
					recvBuff[ret] = 0;
					//printf("Receive from client %d[%s:%d] %s\n", nClient, &(managedClients[i].clientIP[0]), managedClients[i].clientPort, buff);
					clients[i].handleReceivedMessage(recvBuff);
				}
			}
			//LeaveCriticalSection(&criticalForClientQueue);
		}
	}

	printf("break %d\n", numThread);
	EnterCriticalSection(&criticals[numThread]);
	for (int i = start; i < end; ++i)
	{
		if (clients[i].connSock != 0 /*&& FD_ISSET(clients[i].connSock, &initfds)*/)
		{
			closesocket(clients[i].connSock);
			clients[i].reset();
		}
	}
	//EnterCriticalSection(&criticalForNumOfThread);
	numOfThread[numThread] = -1;
	//LeaveCriticalSection(&criticalForNumOfThread);
	LeaveCriticalSection(&criticals[numThread]);
	return 0;
}

int main(int argc, char* argv[])
{
	int serverPort = Helper::checkCommandLineArgument(argc, argv);
	if (serverPort == -1)
		return 0;
	if (!accountManager.readAccountFromFile(FILE_ACCOUNT))
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


	//InitializeCriticalSection(&criticalForClientQueue);
	InitializeCriticalSection(&criticalForNumOfThread);
	for (int i = 0; i < MAX_THREAD; ++i)
	{
		InitializeCriticalSection(&criticals[i]);
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
					clients[i].clientIP = std::string(clientIP);
					clients[i].clientPort = clientPort;
					LeaveCriticalSection(&criticals[threadPos]);

					EnterCriticalSection(&criticalForNumOfThread);
					if (numOfThread[threadPos] == -1)
					{
						numOfThread[threadPos] = threadPos;
						HANDLE hThread = (HANDLE)_beginthreadex(0, 0, clientThread, (void*)&numOfThread[threadPos], 0, 0); //start thread
					}
					LeaveCriticalSection(&criticalForNumOfThread);
					break;
				}
				else
					LeaveCriticalSection(&criticals[threadPos]);
			}
			if (i == MAX_CLIENT)
			{
				printf("Max client reached\n");
				closesocket(connSocket);
			}
		}
	}

	closesocket(listenSock);

	//DeleteCriticalSection(&criticalForClientQueue);
	DeleteCriticalSection(&criticalForNumOfThread);
	InitializeCriticalSection(&criticalForNumOfThread);
	for (int i = 0; i < MAX_THREAD; ++i)
	{
		DeleteCriticalSection(&criticals[i]);
	}

	WSACleanup();

	return 0;
}
