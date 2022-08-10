#ifndef UI_HANDLER_H
#define UI_HANDLER_H

#include <iostream>
#include <string>
#include "MessageSender.h"

namespace UIHandler
{
	void displayMenu(bool isLogin);
	void displayResponse(const std::string& responseCode);
}

#endif // !UI_HANDLER_H
