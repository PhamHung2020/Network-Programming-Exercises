#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <iostream>
#include <string>
#include "Helper.h"

namespace InputHandler
{
	int inputInt(const std::string& message);
	int inputIntInRange(const std::string& message, int startRange, int endRange);
	std::string inputString(const std::string& message);
}
#endif // !INPUT_HANDLER_H
