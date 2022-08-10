#include "Helper.h"

int Helper::checkCommandLineArgument(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("This program only accepts 1 arguments: port number");
		return -1;
	}

	int portNumber;
	try
	{
		portNumber = std::stoi(argv[1]);
	}
	catch (const std::invalid_argument&)
	{
		printf("The only argument must be a number, which is a port number");
		return -1;
	}
	catch (const std::out_of_range)
	{
		printf("Port number must be in range [0, 65536]");
		return -1;
	}

	if (portNumber < 0 || portNumber > 65536)
	{
		printf("Port number must be in range [0, 65536]");
		return -1;
	}
	return portNumber;
}
int Helper::sendMessage(SOCKET connSock, std::string message, char* buff)
{
	std::cout << "Sending: " << message;
	// this code is inspired by Mr.Tung's lecture
	message += ENDING_DELIMITER;
	int leftBytes = message.length();
	int index = 0, ret = 0;
	while (leftBytes > 0)
	{
		int numCopiedChar = message.copy(buff, BUFF_SIZE, index);
		buff[numCopiedChar] = 0;
		ret = send(connSock, buff, numCopiedChar, 0);
		if (ret == SOCKET_ERROR)
		{
			index = ret;
			break;
		}
		leftBytes -= ret;
		index += ret;
	}
	std::cout << " - Done!\n\n";
	return index;
}
int Helper::splitString(std::string& str, const char* delimiter, int delimiterLength, std::vector<std::string>& arrStr, int count)
{
	//used to store first occurrence of ending delimiter in str
	int posOfDelimiter = std::string::npos;
	//used to store number of substrings after splitting
	int numSubString = 0;

	while (count != 1 && (posOfDelimiter = str.find(delimiter)) != std::string::npos)
	{
		std::string substring = str.substr(0, posOfDelimiter);
		arrStr.push_back(substring);
		++numSubString;
		--count;
		str.erase(0, posOfDelimiter + delimiterLength);
	}
	//the last substring in str after splitting (e.g: 123<delimiter>46 --> 46 is the last substring)
	// or the initial value of input string doesn't contain any delimiter
	if (count != 0 && !str.empty())
	{
		++numSubString;
		arrStr.push_back(str);
	}
	return numSubString;
}