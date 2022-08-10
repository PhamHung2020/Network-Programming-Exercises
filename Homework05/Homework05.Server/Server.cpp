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
#define MAX_CLIENT 1600

//max thread can be created
const int MAX_THREAD = (MAX_CLIENT % WSA_MAXIMUM_WAIT_EVENTS == 0) ? MAX_CLIENT / WSA_MAXIMUM_WAIT_EVENTS : MAX_CLIENT / WSA_MAXIMUM_WAIT_EVENTS + 1;

//a vector contains multiple inner vectors, each inner vector holds information abount clients (connecting socket, username, is login yet?)
//each thread manages an inner vector
std::vector<std::vector<MessageHandler>> clients(MAX_THREAD, std::vector<MessageHandler>());

//a vector contains multiple inner vectors, each inner vector contains events for a thread
std::vector<std::vector<WSAEVENT>> events(MAX_THREAD, std::vector<WSAEVENT>());

//an array indicates number of a thread (0, 1, 2, ...)
//if i-th thread not created yet, numOfThread[i] = -1
//otherwise, numOfThread[i] = i
int numOfThread[MAX_THREAD];

//an array of criticals, each for an inner events vector and clients vector above
CRITICAL_SECTION criticals[MAX_THREAD];


/* clientThread - Thread to receive the messages from some clients and response to them*/
unsigned __stdcall clientThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;

	// number of thread, get from numOfThread[i], which is passed in main function
	int numThread = *(int*)param;

	int index, nEvents = 0;
	WSANETWORKEVENTS sockEvent;

	//get clients vector and events vector corresponding with this thread for quick access later
	std::vector<MessageHandler>& clientsThread = clients[numThread];
	std::vector<WSAEVENT>& eventsThread = events[numThread];

	//number of retries before exitting thread
	int retry = 2;
	
	while (retry > 0)
	{
		EnterCriticalSection(&criticals[numThread]);
		nEvents = eventsThread.size();
		if (nEvents == 0) //no events
		{
			LeaveCriticalSection(&criticals[numThread]);
			Sleep(30); //prevent the thread from exitting too fast, so main thread doesnt need to create a new thread
			--retry;
			continue;
		}
		LeaveCriticalSection(&criticals[numThread]);

		index = WSAWaitForMultipleEvents(nEvents, &eventsThread[0], FALSE, 2, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			--retry;
			continue;
		}
		else if (index == WSA_WAIT_TIMEOUT) //time out
		{
			retry = 2;
			continue;
		}

		
		retry = 2;
		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(clientsThread[index].connSock, eventsThread[index], &sockEvent);

		//WSAEnumNetworkEvents already reset the event, so we dont need to reset it manually (source: stackoverflow)
		//WSAResetEvent(eventsThread[index]);

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				continue;
			}

			ret = recv(clientsThread[index].connSock, buff, BUFF_SIZE - 1, 0);

			//release socket and event if an error occurs
			if (ret <= 0)
			{
				if (ret == SOCKET_ERROR)
				{
					//connection closed forcibly (e.g: client is closed by Control-C, or turn off window)
					if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAECONNABORTED)
						clientsThread[index].handleByeMessage(false, buff);
					else //WSAWOULDBLOCK
						continue;
				}
				else if (ret == 0) //client disconnect
					clientsThread[index].handleByeMessage(false, buff);

				EnterCriticalSection(&criticals[numThread]);
				//close event
				WSACloseEvent(eventsThread[index]);
				eventsThread.erase(eventsThread.begin() + index);
				//close socket
				closesocket(clientsThread[index].connSock);
				clientsThread.erase(clientsThread.begin() + index);
				LeaveCriticalSection(&criticals[numThread]);
			}
			else {
				buff[ret] = 0;
				clientsThread[index].handleReceivedMessages(buff);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			int errorCode = sockEvent.iErrorCode[FD_CLOSE_BIT];
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				if (errorCode != WSAECONNABORTED && errorCode != WSAECONNRESET)
				{
					printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
					continue;
				}
				clientsThread[index].handleByeMessage(false, buff);
			}
			//release socket and event
			EnterCriticalSection(&criticals[numThread]);
			//close event
			WSACloseEvent(eventsThread[index]);
			eventsThread.erase(eventsThread.begin() + index);
			//close socket
			closesocket(clientsThread[index].connSock);
			clientsThread.erase(clientsThread.begin() + index);
			LeaveCriticalSection(&criticals[numThread]);
		}
	}

	
	//clear before exitting thread
	EnterCriticalSection(&criticals[numThread]);
	nEvents = eventsThread.size();
	for (int i = 0; i < nEvents; ++i)
	{
		closesocket(clientsThread[i].connSock);
		WSACloseEvent(eventsThread[i]);
	}
	clientsThread.clear();
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

	//initialize criticals, clients, events and numOfThread
	for (int i = 0; i < MAX_THREAD; ++i)
	{
		InitializeCriticalSection(&criticals[i]);
		clients[i].reserve(WSA_MAXIMUM_WAIT_EVENTS);
		events[i].reserve(WSA_MAXIMUM_WAIT_EVENTS);
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

			//try to find a thread which still can handle more clients
			int threadPos = 0;
			for (threadPos = 0; threadPos < MAX_THREAD; ++threadPos) //try to find in each thread
			{
				//if this thread is full --> find another thread
				EnterCriticalSection(&criticals[threadPos]);
				if (clients[threadPos].size() == WSA_MAXIMUM_WAIT_EVENTS)
				{
					LeaveCriticalSection(&criticals[threadPos]);
					continue;
				}

				clients[threadPos].push_back(MessageHandler(connSocket));
				events[threadPos].push_back(WSACreateEvent());
				WSAEventSelect(clients[threadPos].back().connSock, events[threadPos].back(), FD_READ | FD_CLOSE);

				//if the thread has not been created yet --> create
				if (numOfThread[threadPos] == -1)
				{
					numOfThread[threadPos] = threadPos; //number of this thread
					_beginthreadex(0, 0, clientThread, (void*)&numOfThread[threadPos], 0, 0); //start thread
					printf("Thread %d created\n", threadPos);
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
	}

	WSACleanup();

	return 0;
}
