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

bool isTime(std::string value, Time & out)
{
	// Time HH:mm:ss --> length = 8
	if (value.length() != 8)
		return false;

	if (value[2] != ':' || value[5] != ':')
		return false;

	int hour = (value[0] - '0') * 10 + (value[1] - '0');
	if (hour < 0 || hour > 23)
		return false;

	int minute = (value[3] - '0') * 10 + (value[4] - '0');
	if (minute < 0 || minute > 59)
		return false;

	int second = (value[6] - '0') * 10 + (value[7] - '0');
	if (0 < second || second > 59)
		return false;

	out.hour = hour;
	out.minute = minute;
	out.second = second;
	return true;
}

bool isTimeRange(std::string value, TimeRange & out)
{
	// Time range HH:mm:ss#HH:mm:ss --> length = 17
	if (value.length() != 17)
		return false;

	if (value[8] != '#')
		return false;

	std::string startStrTime = value.substr(0, 8);
	Time startTime;
	if (!isTime(startStrTime, startTime))
		return false;

	std::string endStrTime = value.substr(9, 8);
	Time endTime;
	if (!isTime(endStrTime, endTime))
		return false;

	if (!Time::compare(startTime, endTime))
		return false;

	out.start = startTime;
	out.end = endTime;
	return true;
}

std::string toString(int value)
{
	return std::to_string(value);
}

std::string toString(double value)
{
	return std::to_string(value);
}

std::string toString(Time time)
{
	std::string hour = time.hour > 0 ? std::to_string(time.hour) : '0' + std::to_string(time.hour);
	std::string minute = time.minute > 0 ? std::to_string(time.minute) : '0' + std::to_string(time.minute);
	std::string second = time.second > 0 ? std::to_string(time.second) : '0' + std::to_string(time.second);
	return  hour + ':' + minute + ':' + second;
}

std::string toString(TimeRange timerange)
{
	return toString(timerange.start) + '#' + toString(timerange.end);
}