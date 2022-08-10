#ifndef UI_HANDLER_H
#define UI_HANDLER_H

#include <iostream>
#include <string>
#include "MessageSender.h"

namespace UIHandler
{
	/**
	* @function displayMenu: displays menu to user depends on login status.
	*
	* @param isLogin: User's login status.
	*
	**/
	void displayMenu(bool isLogin);

	/**
	* @function displayMenu: displays response to user bases on response code from server.
	*
	* @param responseCode: Response code from server.
	*
	**/
	void displayResponse(const std::string& responseCode);
}

#endif // !UI_HANDLER_H
