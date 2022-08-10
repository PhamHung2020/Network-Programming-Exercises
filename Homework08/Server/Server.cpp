#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <string.h>
#include "Helper.h"
#include "AccountHandler.h"
#include "MessageHandler.h"

#define SERVER_ADDR "127.0.0.1"
#define DATA_BUFSIZE 8192
#define RECEIVE 0
#define SEND 1
#define FILE_ACCOUNT "account.txt"

#pragma comment(lib, "Ws2_32.lib")

// Structure definition
typedef struct {
	WSAOVERLAPPED overlapped;
	WSABUF dataBuff;
	CHAR buffer[DATA_BUFSIZE];
	int bufLen;
	int recvBytes;
	int sentBytes;
	int operation;
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

unsigned __stdcall serverWorkerThread(LPVOID CompletionPortID);

int main(int argc, char* argv[])
{
	int serverPort = Helper::checkCommandLineArgument(argc, argv);
	if (serverPort == -1)
		return 0;

	//read information of all accounts in file to a hash table named accountStatus
	if (!AccountHandler::readAccountFromFile(FILE_ACCOUNT, MessageHandler::accountStatus))
		return 0;

	SOCKADDR_IN serverAddr;
	SOCKET listenSock, acceptSock;
	HANDLE completionPort;
	SYSTEM_INFO systemInfo;
	LPPER_IO_OPERATION_DATA perIoData;
	DWORD transferredBytes;
	DWORD flags;
	WSADATA wsaData;
	MessageHandler* messageHandlerPtr;

	if (WSAStartup((2, 2), &wsaData) != 0) {
		printf("WSAStartup() failed with error %d\n", GetLastError());
		return 1;
	}

	// Step 1: Setup an I/O completion port
	if ((completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) {
		printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
		return 1;
	}

	// Step 2: Determine how many processors are on the system
	GetSystemInfo(&systemInfo);

	// Step 3: Create worker threads based on the number of processors available on the
	// system. Create two worker threads for each processor	
	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
		// Create a server worker thread and pass the completion port to the thread
		if (_beginthreadex(0, 0, serverWorkerThread, (void*)completionPort, 0, 0) == 0) {
			printf("Create thread failed with error %d\n", GetLastError());
			return 1;
		}
	}

	// Step 4: Create a listening socket
	if ((listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		printf("WSASocket() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSock, (PSOCKADDR)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// Prepare socket for listening
	if (listen(listenSock, 20) == SOCKET_ERROR) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	printf("Server started!\n");

	while (1) {
		// Step 5: Accept connections
		if ((acceptSock = WSAAccept(listenSock, NULL, NULL, NULL, 0)) == SOCKET_ERROR) {
			printf("WSAAccept() failed with error %d\n", WSAGetLastError());
			continue;
		}

		// Step 6: Create a socket information structure to associate with the socket
		if ((messageHandlerPtr = (MessageHandler*)GlobalAlloc(GPTR, sizeof(MessageHandler))) == NULL) {
			closesocket(acceptSock);
			printf("GlobalAlloc() to MessageHandler Pointer failed with error %d\n", GetLastError());
			continue;
		}

		// Step 7: Associate the accepted socket with the original completion port
		printf("Socket number %d got connected...\n", acceptSock);
		messageHandlerPtr->connSock = acceptSock;
		if (CreateIoCompletionPort((HANDLE)acceptSock, completionPort, (DWORD)messageHandlerPtr, 0) == NULL) {
			closesocket(acceptSock);
			GlobalFree(messageHandlerPtr);
			printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
			continue;
		}

		// Step 8: Create per I/O socket information structure to associate with the WSARecv call
		if ((perIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATION_DATA))) == NULL) {
			closesocket(acceptSock);
			GlobalFree(messageHandlerPtr);
			printf("GlobalAlloc() to PER_IO_OPERATION_DATA failed with error %d\n", GetLastError());
			continue;
		}

		ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
		perIoData->sentBytes = 0;
		perIoData->recvBytes = 0;
		perIoData->dataBuff.len = DATA_BUFSIZE - 1;
		perIoData->dataBuff.buf = perIoData->buffer;
		perIoData->operation = RECEIVE;
		flags = 0;

		if (WSARecv(acceptSock, &(perIoData->dataBuff), 1, &transferredBytes, &flags, &(perIoData->overlapped), NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				continue;
			}
		}
		printf("Receving %d\n", acceptSock);
	}

	closesocket(listenSock);
	WSACleanup();
	getchar();
	return 0;
}

unsigned __stdcall serverWorkerThread(LPVOID completionPortID)
{
	HANDLE completionPort = (HANDLE)completionPortID;
	DWORD transferredBytes;
	LPPER_IO_OPERATION_DATA perIoData;
	MessageHandler* messageHandlerPtr;
	DWORD flags;

	while (TRUE) {
		if (GetQueuedCompletionStatus(completionPort, &transferredBytes,
			(LPDWORD)&messageHandlerPtr, (LPOVERLAPPED *)&perIoData, INFINITE) == 0) {
			if (WSAGetLastError() == 64) {
				printf("Connection is closed forcibly\n");
			}
			else {
				printf("GetQueuedCompletionStatus() failed with error %d", WSAGetLastError());
			}
			closesocket(messageHandlerPtr->connSock);
			GlobalFree(messageHandlerPtr);
			GlobalFree(perIoData);
			continue;
		}
		// Check to see if an error has occurred on the socket and if so
		// then close the socket and cleanup the SOCKET_INFORMATION structure
		// associated with the socket
		if (transferredBytes == 0) {
			printf("Closing socket %d\n", messageHandlerPtr->connSock);
			if (closesocket(messageHandlerPtr->connSock) == SOCKET_ERROR) {
				// client disconnected by Control+C, or turning off window
				printf("closesocket() failed with error %d\n", WSAGetLastError());
				continue;
			}
			printf("Connection closed\n");
			GlobalFree(messageHandlerPtr);
			GlobalFree(perIoData);
			continue;
		}
		// Check to see if the operation field equals RECEIVE. If this is so, then
		// this means a WSARecv call just completed so update the recvBytes field
		// with the transferredBytes value from the completed WSARecv() call
		if (perIoData->operation == RECEIVE) {
			perIoData->recvBytes = transferredBytes;
			perIoData->sentBytes = 0;
			perIoData->operation = SEND;
			perIoData->buffer[transferredBytes] = 0;
			messageHandlerPtr->handleReceivedMessages(perIoData->buffer);
			perIoData->buffer[0] = 0;
		}
		else if (perIoData->operation == SEND) {
			perIoData->sentBytes += transferredBytes;
		}

		int sendingBytes = strlen(perIoData->buffer);
		int countMessage = 0;
		if (perIoData->sentBytes < sendingBytes || (countMessage = messageHandlerPtr->getMessageNumber()) > 0) {
			// Post another WSASend() request.
			// Since WSASend() is not guaranteed to send all of the bytes requested or message queue is not empty,
			// continue posting WSASend() calls until all bytes are sent.
			if (countMessage > 0) {
				perIoData->sentBytes = 0;
				perIoData->operation = SEND;
				std::string response = messageHandlerPtr->handleMessage() + ENDING_DELIMITER;
				int numCopiedChar = response.copy(perIoData->buffer, DATA_BUFSIZE, 0);
				perIoData->buffer[numCopiedChar] = 0;
				sendingBytes = strlen(perIoData->buffer);
			}

			ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
			perIoData->dataBuff.buf = perIoData->buffer + perIoData->sentBytes;
			perIoData->dataBuff.len = sendingBytes - perIoData->sentBytes;
			perIoData->operation = SEND;

			if (WSASend(messageHandlerPtr->connSock,
				&(perIoData->dataBuff),
				1,
				&transferredBytes,
				0,
				&(perIoData->overlapped),
				NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != ERROR_IO_PENDING) {
					printf("WSASend() failed with error %d\n", WSAGetLastError());
					continue;
				}
			}
		}
		else {
			// No more bytes to send, post another WSARecv() request
			perIoData->recvBytes = 0;
			perIoData->operation = RECEIVE;
			flags = 0;
			ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
			perIoData->dataBuff.len = DATA_BUFSIZE - 1;
			perIoData->dataBuff.buf = perIoData->buffer;
			if (WSARecv(messageHandlerPtr->connSock,
				&(perIoData->dataBuff),
				1,
				&transferredBytes,
				&flags,
				&(perIoData->overlapped), NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != ERROR_IO_PENDING) {
					printf("WSARecv() failed with error %d\n", WSAGetLastError());
					return 0;
				}
			}
		}
	}
}