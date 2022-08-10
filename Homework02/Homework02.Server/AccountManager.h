#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <winsock2.h>
#include "Account.h"

class AccountManager
{
private:
	std::vector<Account> accounts;
	int numOfAccount;

public:
	AccountManager();

	/**
	* @function readAccountFromFile: reads information of all accounts stored in a file text and stores them in accounts vector.
	*
	* @param filename: Name of file containing information of all accounts. This file and program must be in the same folder.
	*
	* @return: true if no error occurs.
	*		   false if an error occurs.
	**/
	bool readAccountFromFile(const std::string& filename);

	/**
	* @function getAccount: finds in vector the first account that has the same username as username input.
	*
	* @param username: Username of the account that need to be found.
	* @param account: An account entity which is used to store information about found account.
	*
	* @return: position of found account in the vector.
	*		   -1 if not found.
	**/
	int getAccount(const std::string& username, Account& account);

	/**
	* @function changeLoginStatus: change login status of a specific account.
	*
	* @param status: Login status input.
	* @param posOfAccount: position of account in vector.
	*
	**/
	void changeLoginStatus(bool status, int posOfAccount);
};

#endif // !ACCOUNT_MANAGER_H
