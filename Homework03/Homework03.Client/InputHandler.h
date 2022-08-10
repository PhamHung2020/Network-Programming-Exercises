#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <iostream>
#include <string>
#include "Helper.h"

namespace InputHandler
{
	/**
	* @function inputInt: ensures user enter an integer. If the input is not an integer, user is required to enter again.
	*
	* @param message: The message need to be displayed before user enter an input.
	*
	* @return: the integer user entered.
	**/
	int inputInt(const std::string& message);

	/**
	* @function inputIntInRange: ensures user enter an integer from startRange to endRange.
	*							 If the input is not an integer, or the entered integer is invalid, user is required to enter again.
	*
	* @param message: The message need to be displayed before user enter an input.
	* @param startRange: The start number of the range.
	* @param endRange: The end number of the range.
	*
	* @return: the integer user entered.
	**/
	int inputIntInRange(const std::string& message, int startRange, int endRange);

	/**
	* @function inputInt: ensures user enter an non-empty string. If the input is an empty string, user is required to enter again.
	*
	* @param message: The message need to be displayed before user enter an input.
	*
	* @return: the string user entered.
	**/
	std::string inputString(const std::string& message);
}

#endif // !INPUT_HANDLER_H
