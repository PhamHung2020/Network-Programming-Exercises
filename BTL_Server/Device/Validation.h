#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>
#include "Time.h"

bool isInt(std::string value, int& out);
bool isDouble(std::string value, double& out);
bool isTime(std::string value, Time& out);
bool isTimeRange(std::string value, TimeRange& out);
std::string toString(int value);
std::string toString(double value);
std::string toString(Time value);
std::string toString(TimeRange value);

#endif // !VALIDATION_H
