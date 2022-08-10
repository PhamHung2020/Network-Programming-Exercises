#include "InputHandler.h"


int InputHandler::inputInt(const std::string& message)
{
	//the return value
	int input = 0;
	//used to store the input user entered
	std::string inputStr;
	do
	{
		inputStr = inputString(message);
		try
		{
			input = std::stoi(inputStr);
			break;
		}
		catch (const std::invalid_argument&)
		{
			printf("Your input must be a number\n");
			continue;
		}
		catch (const std::out_of_range&)
		{
			printf("Your input is too big or too small\n");
			continue;
		}
	} while (true);
	return input;
}





int InputHandler::inputIntInRange(const std::string& message, int startRange, int endRange)
{
	//swap startRange and endRange if needed
	if (startRange > endRange)
	{
		int temp = startRange;
		startRange = endRange;
		endRange = temp;
	}
	//the return value
	int input;
	do
	{
		input = inputInt(message);
		if (input < startRange || input > endRange)
		{
			printf("Your input must be in range [%d, %d]\n", startRange, endRange);
			continue;
		}
		break;
	} while (true);
	return input;
}

std::string InputHandler::inputString(const std::string& message)
{
	//the return value
	std::string input = "";
	do
	{
		std::cout << message;
		getline(std::cin, input);
		if (input.empty())
		{
			printf("Your input must not be empty\n");
		}
		else if (input.length() > BUFF_SIZE)
		{
			printf("Your message is too long\n");
		}
		else
			break;
	} while (true);
	return input;
}