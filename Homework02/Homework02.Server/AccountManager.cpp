#include "AccountManager.h"

AccountManager::AccountManager()
{
	accounts = std::vector<Account>();
	numOfAccount = 0;
}

bool AccountManager::readAccountFromFile(const std::string& filename)
{
	accounts.clear();
	std::ifstream accountFile(filename);
	if (accountFile.fail())
	{
		std::cout << "Open file failed\n";
		return false;
	}

	std::string line = "";
	//read file line by line and parse each line to an Account then add it to account array
	while (getline(accountFile, line))
	{
		Account newAccount;
		if (!Account::parseAccount(line, newAccount))
			return false;
		accounts.push_back(newAccount);
	}
	numOfAccount = accounts.size();
	accountFile.close();
	return true;
}

int AccountManager::getAccount(const std::string& username, Account& account)
{
	for (int i = 0; i < numOfAccount; ++i)
	{
		if (accounts[i].username == username)
		{
			account = accounts[i];
			return i;
		}
	}
	return -1;
}

void AccountManager::changeLoginStatus(bool status, int posOfAccount)
{
	if (posOfAccount >= numOfAccount || posOfAccount < 0)
		return;
	accounts[posOfAccount].isLogin = status;
}