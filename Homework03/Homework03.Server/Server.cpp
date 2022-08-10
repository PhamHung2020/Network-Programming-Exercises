//max clients for each thread
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

//max clients server can handle in the same time
#define MAX_CLIENT 20000

//max thread can be created
const int MAX_THREAD = (MAX_CLIENT % FD_SETSIZE == 0) ? MAX_CLIENT / FD_SETSIZE : MAX_CLIENT / FD_SETSIZE + 1;

//an array holds information abount clients (connecting socket, username, is login yet?)
//each thread manages a segment of this array
//i-th thread manages from clients[i*FD_SETSIZE] to clients[i*FD_SETSIZE + FD_SETSIZE]
std::vector<MessageHandler> clients = std::vector<MessageHandler>(MAX_CLIENT, MessageHandler(0));

//an array of initfds, each for one thread
fd_set initfdsThread[MAX_THREAD];

//an array indicates number of a thread (0, 1, 2, ...)
//if i-th thread not created yet, numOfThread[i] = -1
//otherwise, numOfThread[i] = i
int numOfThread[MAX_THREAD];

//an array of criticals, each for a segment of clients array above
CRITICAL_SECTION criticals[MAX_THREAD];

/* clientThread - Thread to receive the messages from some clients and response to them*/
unsigned __stdcall clientThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;

	// number of thread, get from numOfThread[i], which is passed in main function
	int numThread = *(int*)param;

	//the starting point and end point of segment of clients array this thread manages
	int start = numThread * FD_SETSIZE;
	int end = start + FD_SETSIZE;

	//fd_set to pass to select function
	fd_set initfds;
	FD_ZERO(&initfds);

	//timeout for select function
	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 20000;

	//number of retries before exitting thread
	int retry = 2;

	while (retry > 0) {
		//EnterCriticalSection(&criticals[numThread]);
		initfds = initfdsThread[numThread];
		//LeaveCriticalSection(&criticals[numThread]);

		int nEvents = select(0, &initfds, 0, 0, &timeout);

		//error, maybe all clients disconnect
		if (nEvents < 0)
		{
			--retry;
			printf("Thread %d: %d\n", numThread, WSAGetLastError());
			continue;
		}
		//timeout
		else if (nEvents == 0)
		{
			retry = 2;
			continue;
		}

		retry = 2;

		//receives messages from clients
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
						//connection closed forcibly (e.g: client is closed by Control-C, or turn off window)
						if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
						{
							clients[i].handleByeMessage(false, buff);
						}
						else
						{
							continue;
						}
					}
					else if (ret == 0) //client disconnect
					{
						clients[i].handleByeMessage(false, buff);
					}

					//clear information about this client
					EnterCriticalSection(&criticals[numThread]);
					FD_CLR(clients[i].connSock, &initfdsThread[numThread]);
					LeaveCriticalSection(&criticals[numThread]);
					closesocket(clients[i].connSock);
					clients[i].reset();
				}
				else //handle message from this client
				{
					buff[ret] = 0;
					clients[i].handleReceivedMessages(buff);
				}
			}
		}
	}

	//clear before exitting thread
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

	//read information of all accounts in file to a hash table named accountStatus
	if (!AccountHandler::readAccountFromFile(FILE_ACCOUNT, MessageHandler::accountStatus))
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

	//initialize criticals, initfds and numOfThread
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
		//try to accept a new connection
		connSocket = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		if (connSocket == SOCKET_ERROR)
			printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
		else
		{
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);

			//try to find a client whose connSock = 0
			int threadPos = 0;
			for (threadPos = 0; threadPos < MAX_THREAD; ++threadPos) //try to find in each thread
			{
				//if this thread is full --> find in another thread
				EnterCriticalSection(&criticals[threadPos]);
				if (initfdsThread[threadPos].fd_count == FD_SETSIZE)
				{
					LeaveCriticalSection(&criticals[threadPos]);
					continue;
				}
				LeaveCriticalSection(&criticals[threadPos]);

				//find a free client in this thread
				int start = threadPos * FD_SETSIZE;
				int end = start + FD_SETSIZE;
				bool freeClientFound = false;

				for (int clientPos = start; clientPos < end; ++clientPos)
				{
					EnterCriticalSection(&criticals[threadPos]);
					if (clients[clientPos].connSock == 0)
					{
						clients[clientPos].connSock = connSocket;
						FD_SET(connSocket, &initfdsThread[threadPos]);

						//if the thread managing segment that contains this client has not been created yet --> create
						if (numOfThread[threadPos] == -1)
						{
							numOfThread[threadPos] = threadPos; //number of this thread
							_beginthreadex(0, 0, clientThread, (void*)&numOfThread[threadPos], 0, 0); //start thread
						}
						LeaveCriticalSection(&criticals[threadPos]);
						freeClientFound = true;
						break;
					}
					else
					{
						LeaveCriticalSection(&criticals[threadPos]);
					}
				}

				if (freeClientFound)
					break;
			}

			if (threadPos == MAX_THREAD)
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
