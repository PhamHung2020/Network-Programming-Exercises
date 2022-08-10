#include "Validation.h"

bool isInt(std::string value, int & out)
{
	int parsedValue;
	try
	{
		parsedValue = std::stoi(value);
		out = parsedValue;
		return true;
	}
	catch (const std::invalid_argument&)
	{
		return false;
	}
	catch (const std::out_of_range&)
	{
		return false;
	}
}

bool isDouble(std::string value, double & out)
{
	double parsedValue;
	try
	{
		parsedValue = std::stod(value);
		out = parsedValue;
		return true;
	}
	catch (const std::invalid_argument&)
	{
		return false;
	}
	catch (const std::out_of_range&)
	{
		return false;
	}
}
