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
//#define FD_SETSIZE 1024

//struct ClientInfo
//{
//	SOCKET connectedSocket;
//	std::string clientIP;
//	int clientPort;
//
//	ClientInfo() {}
//
//	ClientInfo(SOCKET connectedSocket, std::string clientIP, int clientPort)
//		: connectedSocket(connectedSocket), clientIP(clientIP), clientPort(clientPort) {}
//};

AccountManager accountManager;
CRITICAL_SECTION criticalForClientQueue;
CRITICAL_SECTION criticalForTotalClient;
CRITICAL_SECTION criticalForNumThread;
int totalClient = 0;
int numThread = 0;
std::queue<MessageHandler> clientQueue;
int a = 0;

/* clientThread - Thread to receive the message from a client and response*/
unsigned __stdcall clientThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;
	int nClient = 0;
	/*std::vector<MessageHandler> managedClients = std::vector<MessageHandler>(FD_SETSIZE, MessageHandler(&accountManager, 0, "", 0));
	std::queue<int> emptyConnSock;
	for (int i = 0; i < FD_SETSIZE; ++i)
	emptyConnSock.push(i);*/

	std::vector<MessageHandler> managedClients = std::vector<MessageHandler>();
	fd_set initfds, readfds;
	FD_ZERO(&initfds);
	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	while (1) {
		while (nClient < FD_SETSIZE)
		{
			EnterCriticalSection(&criticalForClientQueue);
			if (!clientQueue.empty())
			{
				/*MessageHandler messHdl = clientQueue.front();
				clientQueue.pop();
				LeaveCriticalSection(&criticalForClientQueue);
				int emptyConnSockPos = emptyConnSock.front();
				managedClients[emptyConnSockPos] = messHdl;
				emptyConnSock.pop();*/
				managedClients.push_back(clientQueue.front());
				clientQueue.pop();
				LeaveCriticalSection(&criticalForClientQueue);
				++nClient;
				FD_SET(managedClients.back().connSock, &initfds);
				//FD_SET(managedClients[emptyConnSockPos].connSock, &initfds);
			}
			else
			{
				LeaveCriticalSection(&criticalForClientQueue);
				break;
			}
		}

		//printf("A3 %d\n", nClient);
		if (nClient == 0)
		{
			printf("break\n");
			break;
		}
		//printf("A2 %d\n", nClient);
		readfds = initfds;
		int nEvents = select(0, &readfds, 0, 0, &timeout);
		if (nEvents < 0)
		{
			//printf("A1 %d\n", nClient);
			if (WSAGetLastError() == WSAEINVAL || WSAGetLastError() == WSAENOTSOCK)
			{
				//printf("A %d\n", nClient);
				EnterCriticalSection(&criticalForTotalClient);
				totalClient -= nClient;
				LeaveCriticalSection(&criticalForTotalClient);
				nClient = 0;
				FD_ZERO(&initfds);
				for (int i = 0; i < nClient; ++i)
				{
					closesocket(managedClients[i].connSock);
				}
				managedClients.clear();
				continue;
			}
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}
		else if (nEvents == 0)
			continue;

		//receives message from client
		for (int i = 0; i < nClient; ++i)
		{
			if (FD_ISSET(managedClients[i].connSock, &readfds))
			{
				ret = recv(managedClients[i].connSock, buff, BUFF_SIZE - 1, 0);
				if (ret <= 0)
				{
					if (ret == SOCKET_ERROR)
					{
						if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
						{
							//printf("Client [%s:%d] closed the connection forcibly\n", &(managedClients[i].clientIP[0]), managedClients[i].clientPort);
							managedClients[i].handleByeMessage(false, buff);
						}
						else
						{
							printf("Error %d: Cannot receive data from client [%s:%d]\n", WSAGetLastError(), &(managedClients[i].clientIP[0]), managedClients[i].clientPort);
							continue;
						}
					}
					else if (ret == 0)
					{
						managedClients[i].handleByeMessage(false, buff);
						//printf("Client [%s:%d] disconnects.\n", &(managedClients[i].clientIP[0]), managedClients[i].clientPort);
					}
					EnterCriticalSection(&criticalForTotalClient);
					--totalClient;
					LeaveCriticalSection(&criticalForTotalClient);
					//printf("%d\n", nClient);
					FD_CLR(managedClients[i].connSock, &initfds);
					closesocket(managedClients[i].connSock);
					managedClients.erase(managedClients.begin() + i);
					--i;
					--nClient;
				}
				else
				{
					buff[ret] = 0;
					//printf("Receive from client %d[%s:%d] %s\n", nClient, &(managedClients[i].clientIP[0]), managedClients[i].clientPort, buff);
					managedClients[i].handleReceivedMessage(buff);
				}
			}
		}
	}

	EnterCriticalSection(&criticalForNumThread);
	--numThread;
	LeaveCriticalSection(&criticalForNumThread);
	managedClients.clear();

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

	printf("Server started!\n");
	InitializeCriticalSection(&criticalForClientQueue);
	InitializeCriticalSection(&criticalForTotalClient);
	InitializeCriticalSection(&criticalForNumThread);

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
			/*unsigned long ul = 1;
			ioctlsocket(connSocket, FIONBIO, (unsigned long *)&ul);*/
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			//printf("Accept incoming connection from [%s:%d] %d\n", clientIP, clientPort, totalClient);

			if (numThread == 0 || (totalClient % FD_SETSIZE == 0 && totalClient / FD_SETSIZE + 1 > numThread))
			{
				HANDLE hThread = (HANDLE)_beginthreadex(0, 0, clientThread, 0, 0, 0); //start thread
				if (!hThread)
				{
					printf("Error: Cannot serve more client. Thread creation failed\n");
					closesocket(connSocket);
					continue;
				}
				else
				{
					EnterCriticalSection(&criticalForNumThread);
					++numThread;
					LeaveCriticalSection(&criticalForNumThread);
				}
			}

			EnterCriticalSection(&criticalForClientQueue);
			clientQueue.push(MessageHandler(&accountManager, connSocket, std::string(clientIP), clientPort));
			LeaveCriticalSection(&criticalForClientQueue);
			EnterCriticalSection(&criticalForTotalClient);
			++totalClient;
			LeaveCriticalSection(&criticalForTotalClient);
		}
	}

	closesocket(listenSock);

	DeleteCriticalSection(&criticalForClientQueue);
	DeleteCriticalSection(&criticalForTotalClient);
	DeleteCriticalSection(&criticalForNumThread);

	WSACleanup();

	return 0;
}
