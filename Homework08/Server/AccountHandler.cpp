#include "AccountHandler.h"

bool AccountHandler::parseAccount(std::string accountInfo, std::unordered_map<std::string, bool>& accountStatus)
{
	//find the space between username and status
	int whitespaceLastPos = accountInfo.rfind(' ');
	if (whitespaceLastPos == std::string::npos) //if not found that space
	{
		printf("Account information is in invalid format\n");
		return false;
	}

	//status is the last character (1 or 0) in input string
	int isLocked = accountInfo[accountInfo.length() - 1] - '0';
	if (isLocked != 1 && isLocked != 0)
	{
		printf("Account information is in invalid format\n");
		return false;
	}
	std::string username = accountInfo.substr(0, whitespaceLastPos);
	//store infor to hash table
	accountStatus[username] = isLocked == 1 ? true : false;
	return true;
}

bool AccountHandler::readAccountFromFile(const std::string& filename, std::unordered_map<std::string, bool>& accountStatus)
{
	std::ifstream accountFile(filename);
	if (accountFile.fail())
	{
		std::cout << "Open file failed\n";
		return false;
	}

	std::string line = "";
	//read file line by line, each line is an account
	while (getline(accountFile, line))
	{
		if (!parseAccount(line, accountStatus))
			return false;
	}

	accountFile.close();
	return true;
}