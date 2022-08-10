#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <iostream>
struct Account
{
	std::string username;
	bool isLogin;
	bool isLocked;

	Account();

	Account(std::string username, bool isLogin, bool isLocked);

	/**
	* @function parseAccount: parse a string which contains information about an account to an Account entity.
	*
	* @param accountInfo: A string contains information abount an account.
	* @param account: An Account entity to store information after parsing.
	*
	* @return: true if no error occurs.
	*		   false if an error occurs.
	**/
	static bool parseAccount(std::string accountInfo, Account& account);
};


#endif // !ACCOUNT_H
