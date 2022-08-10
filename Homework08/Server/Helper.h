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
#define BUFF_SIZE 2049

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

}
#endif // !HELPER_H
