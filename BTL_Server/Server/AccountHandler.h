#ifndef ACCOUNT_HANDLER_H
#define ACCOUNT_HANDLER_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

namespace AccountHandler
{
	/**
	* @function parseAccount: parse a string which contains information about an account and save into a hash table (username --> is locked?).
	*						  this function is called mostly by readAccountFromFile function.
	*
	* @param accountInfo: A string contains information abount an account.
	* @param accountStatus: A hash table to store information after parsing.
	*
	* @return: true if no error occurs.
	*		   false if an error occurs.
	**/
	bool parseAccount(std::string accountInfo, std::unordered_map<std::string, std::pair<std::string, bool>>& accountStatus);

	/**
	* @function readAccountFromFile: read information of all accounts stored in a file text and stores them in a hash table.
	*
	* @param filename: Name of file containing information of all accounts. This file and program must be in the same folder.
	* @param accountStatus: A hash table to store information of all accounts
	*
	* @return: true if no error occurs.
	*		   false if an error occurs.
	**/
	bool readAccountFromFile(const std::string& filename, std::unordered_map<std::string, std::pair<std::string, bool>>& accountStatus);
}

#endif // !ACCOUNT_HANDLER_H
