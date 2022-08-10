#include "AccountHandler.h"

bool AccountHandler::parseAccount(std::string accountInfo, std::unordered_map<std::string, std::pair<std::string, bool>>& accountStatus)
{
	//find the space between username and status
	int whitespaceLastPos = accountInfo.rfind(' ');
	if (whitespaceLastPos == std::string::npos) //if not found that space
	{
		printf("Account information is in invalid format\n");
		return false;
	}

	std::string username = accountInfo.substr(0, whitespaceLastPos);
	std::string password = accountInfo.substr(whitespaceLastPos + 1);
	//store infor to hash table
	accountStatus[username] = std::make_pair(password, false);
	return true;
}

bool AccountHandler::readAccountFromFile(const std::string& filename, std::unordered_map<std::string, std::pair<std::string, bool>>& accountStatus)
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