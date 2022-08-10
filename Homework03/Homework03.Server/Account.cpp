#include "Account.h"
Account::Account() {}

Account::Account(std::string username, bool isLocked)
	: username(username), isLocked(isLocked) {}

bool Account::parseAccount(std::string accountInfo, Account& account)
{
	int whitespaceLastPos = accountInfo.rfind(' ');
	if (whitespaceLastPos == std::string::npos)
	{
		printf("Account information is in invalid format\n");
		return false;
	}

	account.username = accountInfo.substr(0, whitespaceLastPos);
	int isLocked = accountInfo[accountInfo.length() - 1] - '0';
	if (isLocked != 1 && isLocked != 0)
	{
		printf("Account information is in invalid format\n");
		return false;
	}
	account.isLocked = isLocked == 1 ? true : false;
	return true;
}