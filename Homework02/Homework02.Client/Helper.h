#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>

namespace Helper
{
	#define ENDING_DELIMITER "\r\n"
	#define ENDING_DELIMITER_LENGTH 2
	#define BUFF_SIZE 32769

	
	int checkCommandLineArgument(int argc, char* argv[]);


	int sendMessage(SOCKET connSock, std::string message, char* buff);

	int splitString(std::string& str, const char* delimiter, int delimiterLength, std::vector<std::string>& arrStr, int count);

}
#endif // !HELPER_H
