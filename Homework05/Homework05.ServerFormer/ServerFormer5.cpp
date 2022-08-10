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
#define MAX_CLIENT 500
#define MAX_CLIENT_PER_THREAD 60

//max thread can be created
const int MAX_THREAD = (MAX_CLIENT % MAX_CLIENT_PER_THREAD == 0) ? MAX_CLIENT / MAX_CLIENT_PER_THREAD : MAX_CLIENT / MAX_CLIENT_PER_THREAD + 1;

//an array holds information abount clients (connecting socket, username, is login yet?)
//each thread manages a segment of this array
//i-th thread manages from clients[i*FD_SETSIZE] to clients[i*FD_SETSIZE + FD_SETSIZE]
//std::vector<std::vector<MessageHandler*>> clientPtr(MAX_THREAD, std::vector<MessageHandler*>());
MessageHandler* clientPtr[MAX_THREAD][MAX_CLIENT_PER_THREAD];
//an array of initfds, each for one thread
//fd_set initfdsThread[MAX_THREAD];
//WSAEVENT events[MAX_CLIENT];
std::vector<std::vector<WSAEVENT>> events(MAX_THREAD, std::vector<WSAEVENT>());

//an array indicates number of a thread (0, 1, 2, ...)
//if i-th thread not created yet, numOfThread[i] = -1
//otherwise, numOfThread[i] = i
int numOfThread[MAX_THREAD];

//an array of criticals, each for a segment of clients array above
CRITICAL_SECTION criticals[MAX_THREAD];

void closeMessageHandler(MessageHandler** clients, int index, int numClient)
{
	MessageHandler* deleteClient = clients[index];
	for (int i = index; i < numClient - 1; ++i)
	{
		clients[i] = clients[i + 1];
	}
	clients[numClient - 1] = deleteClient;
	clients[numClient - 1]->reset();
}

/* clientThread - Thread to receive the messages from some clients and response to them*/
unsigned __stdcall clientThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;

	// number of thread, get from numOfThread[i], which is passed in main function
	int numThread = *(int*)param;

	int index, nEvents = 0;
	WSANETWORKEVENTS sockEvent;
	//std::vector<MessageHandler*>& clientsThread = clientPtr[numThread];
	MessageHandler** clientsThread = clientPtr[numThread];
	std::vector<WSAEVENT>& eventsThread = events[numThread];

	//number of retries before exitting thread
	int retry = 3;

	while (retry > 0)
	{
		//EnterCriticalSection(&criticals[numThread]);
		nEvents = eventsThread.size();
		if (nEvents == 0)
		{
			//LeaveCriticalSection(&criticals[numThread]);
			//printf("Thread %d: No events\n", numThread);
			Sleep(25);
			--retry;
			continue;
		}

		index = WSAWaitForMultipleEvents(nEvents, &eventsThread[0], FALSE, 2, FALSE);
		//LeaveCriticalSection(&criticals[numThread]);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());

			break;
			Sleep(25);
			--retry;
			continue;
		}
		else if (index == WSA_WAIT_TIMEOUT)
		{
			//printf("Time out\n");
			retry = 3;
			continue;
		}

		//printf("Thread %d: %d\n", numThread, nEvents);
		retry = 3;
		index = index - WSA_WAIT_EVENT_0;
		MessageHandler currentHandlingClient = *clientsThread[index];
		sockEvent.lNetworkEvents = 0;
		WSAEnumNetworkEvents(currentHandlingClient.connSock, eventsThread[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				continue;
			}

			ret = recv(currentHandlingClient.connSock, buff, BUFF_SIZE - 1, 0);

			//Release socket and event if an error occurs
			if (ret <= 0)
			{
				if (ret == SOCKET_ERROR)
				{
					//connection closed forcibly (e.g: client is closed by Control-C, or turn off window)
					if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
						currentHandlingClient.handleByeMessage(false, buff);
					else
						continue;
				}
				else if (ret == 0) //client disconnect
					currentHandlingClient.handleByeMessage(false, buff);

				EnterCriticalSection(&criticals[numThread]);

				closesocket(currentHandlingClient.connSock);
				WSACloseEvent(eventsThread[index]);
				nEvents = events.size();
				//closeMessageHandler(clientsThread, index, eventsThread.size());
				MessageHandler* deleteClient = clientsThread[index];
				for (int i = index; i < nEvents - 1; ++i)
				{
					clientsThread[i] = clientsThread[i + 1];
					eventsThread[i] = eventsThread[i + 1];
				}
				clientsThread[nEvents - 1] = deleteClient;
				clientsThread[nEvents - 1]->reset();
				//eventsThread.erase(eventsThread.begin() + index);
				eventsThread.pop_back();

				LeaveCriticalSection(&criticals[numThread]);
				continue;
			}
			else {
				buff[ret] = 0;
				currentHandlingClient.handleReceivedMessages(buff);
				//reset event
				WSAResetEvent(eventsThread[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				if (sockEvent.iErrorCode[FD_CLOSE_BIT] == WSAECONNRESET || sockEvent.iErrorCode[FD_CLOSE_BIT] == WSAECONNABORTED)
				{
					currentHandlingClient.handleByeMessage(false, buff);
				}
				else
					printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
				//continue;
			}
			//Release socket and event
			EnterCriticalSection(&criticals[numThread]);

			closesocket(currentHandlingClient.connSock);
			WSACloseEvent(eventsThread[index]);
			//closeMessageHandler(clientsThread, index, eventsThread.size());
			MessageHandler* deleteClient = clientsThread[index];
			for (int i = index; i < nEvents - 1; ++i)
			{
				clientsThread[i] = clientsThread[i + 1];
				eventsThread[i] = eventsThread[i + 1];
			}
			clientsThread[nEvents - 1] = deleteClient;
			clientsThread[nEvents - 1]->reset();
			//eventsThread.erase(eventsThread.begin() + index);
			eventsThread.pop_back();
			LeaveCriticalSection(&criticals[numThread]);
		}

	}
	//printf("Thread %d: %d\n", numThread, eventsThread.size());
	//clear before exitting thread
	EnterCriticalSection(&criticals[numThread]);
	nEvents = eventsThread.size();
	for (int i = 0; i < nEvents; ++i)
	{
		closesocket(clientsThread[i]->connSock);
		clientsThread[i]->reset();
		WSACloseEvent(eventsThread[i]);
	}
	eventsThread.clear();
	numOfThread[numThread] = -1;
	LeaveCriticalSection(&criticals[numThread]);

	printf("Thread %d exit: %d\n", numThread, nEvents);
	return 0;
}

int main(int argc, char* argv[])
{
	int serverPort = Helper::checkCommandLineArgument(argc, argv);
	if (serverPort == -1)
		return 0;

	//read information of all accounts in file to a hash table named accountStatus
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

	//initialize criticals, initfds and numOfThread
	for (int i = 0; i < MAX_THREAD; ++i)
	{
		InitializeCriticalSection(&criticals[i]);
		//clients[i].reserve(MAX_CLIENT_PER_THREAD);
		for (int j = 0; j < MAX_CLIENT_PER_THREAD; ++j)
		{
			//clientPtr[i].push_back(new MessageHandler(0));
			clientPtr[i][j] = new MessageHandler(0);
			//events[i].push_back(0);
		}
		events[i].reserve(MAX_CLIENT_PER_THREAD);
		//FD_ZERO(&initfdsThread[i]);
	}
	memset(numOfThread, -1, sizeof(numOfThread));

	printf("Server started!\n");

	//Step 5: Communicate with client
	SOCKET connSocket;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr);
	while (1)
	{
		//try to accept a new connection
		connSocket = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		if (connSocket == SOCKET_ERROR)
			printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
		else
		{
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			//clientPort = ntohs(clientAddr.sin_port);

			//try to find a client whose connSock = 0
			int threadPos = 0;
			for (threadPos = 0; threadPos < MAX_THREAD; ++threadPos) //try to find in each thread
			{
				//if this thread is full --> find in another thread
				EnterCriticalSection(&criticals[threadPos]);
				int nEvents = events[threadPos].size();
				if (nEvents == MAX_CLIENT_PER_THREAD)
				{
					LeaveCriticalSection(&criticals[threadPos]);
					continue;
				}
				clientPtr[threadPos][nEvents]->connSock = connSocket;
				events[threadPos].push_back(WSACreateEvent());
				WSAEventSelect(clientPtr[threadPos][nEvents]->connSock, events[threadPos][nEvents], FD_READ | FD_CLOSE);
				//if the thread managing segment that contains this client has not been created yet --> create
				if (numOfThread[threadPos] == -1)
				{
					numOfThread[threadPos] = threadPos; //number of this thread
					_beginthreadex(0, 0, clientThread, (void*)&numOfThread[threadPos], 0, 0); //start thread
																							  //printf("Thread %d created\n", threadPos);
				}
				LeaveCriticalSection(&criticals[threadPos]);
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
		delete clientPtr[i];
	}

	WSACleanup();

	return 0;
}
