#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <string>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 65537
#define FAILURE_MESSAGE_PREFIX '-'
#define LAST_SUCCESS_MESSAGE_PREFIX '+'
#define SUCCESS_MESSAGE_PREFIX '!'
#define ENDING_DELIMITER '\n'
#define NOT_FOUND_MESSAGE "-Not found information"
#define ERROR_MESSAGE "-Something wrong happened. Please try again"

#pragma comment(lib, "Ws2_32.lib")

/**
* @function doesContainNumberOnly: check if a string only contains number characters
*
* @param str: An input string
*
* @ return: true, if input string only contains number characters
			false, otherwise
**/
bool doesContainNumberOnly(char* str)
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
	if (argc != 2)
	{
		printf("This program only accepts 1 argument, which is a port number.");
		exit(-1);
	}

	if (!doesContainNumberOnly(argv[1]))
	{
		printf("The only argument must be a number, which is a port number");
		exit(-1);
	}

	int portNumber = atoi(argv[1]);
	if (portNumber < 0 || portNumber > 65536)
	{
		printf("Port number must be in range [0, 65536]");
		exit(-1);
	}

	return portNumber;
}

/*
* @function bindAddressToSocket: bind address to a socket
*
* @param serverAddr: A pointer to server address
* @param portNumber: Server's port number
* @param server: A pointer to a socket
*
* @return: 0 if no error occurs
		   -1 if an error occurs
*/
int bindAddressToSocket(sockaddr_in* serverAddr, int portNumber, SOCKET* server)
{
	serverAddr->sin_family = AF_INET;
	serverAddr->sin_port = htons(portNumber);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr->sin_addr);
	if (bind(*server, (sockaddr*)serverAddr, sizeof(*serverAddr)))
	{
		printf("Error %d: Cannot bind address to socket", WSAGetLastError());
		return -1;
	}
	return 0;
}

/*
* @function sendIPAddressesToClient: send all resolved ip addresses to client
*
* @param addressList: A pointer to a linked list that contains all resolved ip addresses
* @param serverSock: A pointer to the socket that associates with server address
* @param clientAddr: A pointer to client address
*
*/
void sendIPAddressesToClient(addrinfo* addressList, SOCKET* serverSock, sockaddr_in* clientAddr)
{
	//used to store resolved ip addresses
	std::string addressStr = "";
	addressStr += SUCCESS_MESSAGE_PREFIX;
	sockaddr_in *address;
	char ipStr[INET_ADDRSTRLEN];
	int ret = 0;
	//traverse address list and add to addressStr
	while (addressList != NULL)
	{
		address = (sockaddr_in*)addressList->ai_addr;
		inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));
		printf("%s\n", ipStr);
		addressStr.append(ipStr);
		addressStr += ENDING_DELIMITER;
		addressList = addressList->ai_next;

		// if addressStr length is nearly equal to BUFF_SIZE, send resloved addresses to client
		if (addressList != NULL && BUFF_SIZE - addressStr.length() < INET_ADDRSTRLEN)
		{
			const char* c_addresses = addressStr.c_str();
			ret = sendto(*serverSock, c_addresses, strlen(c_addresses), 0, (sockaddr*)clientAddr, sizeof(*clientAddr));
			if (ret == SOCKET_ERROR)
			{
				printf("Error %d: Cannot send data\n", WSAGetLastError());
			}
			else
			{
				printf("%s\n", c_addresses);
				addressStr.clear();
				addressStr += SUCCESS_MESSAGE_PREFIX;
			}
		}
	}

	if (addressStr.length() == 1)
		return;
	addressStr[0] = LAST_SUCCESS_MESSAGE_PREFIX;
	const char* c_addresses = addressStr.c_str();
	ret = sendto(*serverSock, c_addresses, strlen(c_addresses), 0, (sockaddr*)clientAddr, sizeof(*clientAddr));
	if (ret == SOCKET_ERROR)
	{
		printf("Error %d: Cannot send data\n", WSAGetLastError());
	}
	/*else
	{
		printf("%s\n", c_addresses);
	}*/
}

/*
* @function resolveDomainName: resolve a domain name to ip addresses
*
* @param domainName: A pointer to string stores domain name
* @param result: A pointer to a linked list that contains all ip addresses after resolved
*
* @return: 0 if success
		   WINSOCK SOCKETS ERROR CODE if an error occurs
*/
int resolveDomainName(char* domainName, addrinfo **result)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	return getaddrinfo(domainName, NULL, &hints, result);
}

int main(int argc, char* argv[])
{
	int portNumber = checkCommandLineArgument(argc, argv);

	// Step 1: Initiate Winsock
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata))
	{
		printf("Version 2.2 is not supported");
		return 0;
	}

	// Step 2: Construct socket
	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create server", WSAGetLastError());
		return 0;
	}

	// Step 3: Bind address to socket
	sockaddr_in serverAddr;
	if (bindAddressToSocket(&serverAddr, portNumber, &server))
	{
		printf("Error %d: Cannot bind address to socket", WSAGetLastError());
		closesocket(server);
		return 0;
	}
	
	printf("Server is started\n");

	// Step 4: Communicate with client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr);

	while (1)
	{
		//Receive message
		ret = recvfrom(server, buff, BUFF_SIZE - 1, 0, (sockaddr*)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR)
		{
			printf("Error %d: Cannot receive data\n", WSAGetLastError());
		}
		else if (strlen(buff) > 0)
		{
			buff[ret] = 0;
			printf("\nReceived: %s\n", buff);

			addrinfo *result;
			int rc = resolveDomainName(buff, &result);
			//handle result after resolved
			if (rc == 0)
			{
				sendIPAddressesToClient(result, &server, &clientAddr);
			}
			else if (rc == WSAHOST_NOT_FOUND || rc == WSATRY_AGAIN)
			{
				printf("Not found information\n");
				ret = sendto(server, NOT_FOUND_MESSAGE, strlen(NOT_FOUND_MESSAGE), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
			}
			else
			{
				printf("Error: %d", WSAGetLastError());
				ret = sendto(server, ERROR_MESSAGE, strlen(ERROR_MESSAGE), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
			}

			if (ret == SOCKET_ERROR)
			{
				printf("Error %d: Cannot send data\n", WSAGetLastError());
			}
			freeaddrinfo(result);
		}
	}

	//Step 5: Close socket
	closesocket(server);

	//Step 6: Clean up Winsock
	WSACleanup();

	getchar();
	return 0;
}