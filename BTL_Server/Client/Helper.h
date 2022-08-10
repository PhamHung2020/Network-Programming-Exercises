#ifndef HELPER_H
#define HELPER_H


#include <stdio.h>
#include <string>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "ResponseCode.h"
#include "Notification.h"
#include "MessageSender.h"

namespace Helper {
#define ENDING_DELIMITER "\r\n"
#define ENDING_DELIMITER_LENGTH 2
#define BUFF_SIZE 32769
#define DEVICE_DELIMITER "#"
#define DEVICE_DELIMITER_LENGTH 1

	/**
	* @function checkCommandLineArgument: check if arguments passed in command line are valid.
	*
	* @param argc: Number of arguments.
	* @param argv: An array of pointers to arguments.
	*
	* @ return: port number passed in command line if arguments are valid.
	*			-1, otherwise.
	**/
	int checkCommandLineArgument(int argc, char* argv[]);

	/*
	* @function sendMessage: handle TCP streaming when sending message.
	*
	* @param connSock: A socket that is connecting with server.
	* @param message: A message that need to be sent.
	* @param buff: Application's buffer.
	*
	* @return: number of bytes sent if no error occurs.
	*		   SOCKET_ERROR if an error occurs.
	*/
	int sendMessage(std::string message);


	/**
	* @function splitString: splits a string into two or more substrings depending on delimiter.
	*
	* @param str: An input string. If str ends with delimiter, it will be empty after splitting.
	*			  Otherwise, str will contain the substring which stands behind the last delimiter in str.
	* @param delimiter: A constant pointer to a delimiter string.
	* @param delimiterLength: Length of delimiter.
	* @param arrStr: An empty array of strings that would contain all substrings after splitting.
	* @param count: Max number of substrings for splitting. If count < 0, the input string will be splitted as many times as possible.
	*
	* @ return: number of substrings after splitting
	**/
	int splitString(std::string& str, const char* delimiter, int delimiterLength, std::vector<std::string>& arrStr, int count);


	/**
	* @function handleMessage: handles a message from server
	*
	* @param message: A message from server.
	* @param userChoice: User's choice from menu
	* @param isLogin: User's login status
	*
	**/
	/*void handleMessage(const std::string& message, bool& isLogin, std::string id = "");


	std::string receiveData(SOCKET client, char* buff);*/
	std::string convertToString(char* a, int size);

	bool isTime(std::string value);

	void getDataOnFile(std::string fileName);

	void writeDataOnFile(std::string oxi, std::string ph, std::string salt, std::string fileName);
}





#endif