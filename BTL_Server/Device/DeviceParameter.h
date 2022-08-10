#ifndef DEVICE_PARAMETER_H
#define DEVICE_PARAMETER_H

#include <string>
#include <vector>
#include <typeinfo>
#include "Validation.h"
#include <utility>

enum DeviceParameterType
{
	Single,
	Multiple
};

class DeviceParameter
{
private:
	std::string name;
	std::string fullName;
	DeviceParameterType parameterType;
public:
	DeviceParameter(std::string name, std::string fullName, DeviceParameterType parameterType);
	std::string getName();
	std::string getFullName();
	DeviceParameterType getParameterType();
	virtual bool setValue(std::string value) = 0;
	virtual std::string getValue() = 0;
	virtual std::string getType() = 0;
};


template <class T>
class SingleDeviceParameter : public DeviceParameter
{
private:
	T value;

public:
	SingleDeviceParameter(std::string name, std::string fullname, T value) : DeviceParameter(name, fullname, DeviceParameterType::Single), value(value) {};
	bool setValue(std::string value)
	{
		return false;
	}

	std::string getValue()
	{
		return toString(value);
	}

	std::string getType()
	{
		return typeid(T).name();
	}

	//get parameter (real value in memory)
	T getParameter()
	{
		return this->value;
	}
};

template<>
bool SingleDeviceParameter<int>::setValue(std::string value)
{
	int tmp;
	if (!isInt(value, tmp))
		return false;
	this->value = tmp;
	return true;
}

template<>
bool SingleDeviceParameter<double>::setValue(std::string value)
{
	double tmp;
	if (!isDouble(value, tmp))
		return false;
	this->value = tmp;
	return true;
}

template<>
bool SingleDeviceParameter<Time>::setValue(std::string value)
{
	Time tmp;
	if (!isTime(value, tmp))
		return false;
	this->value = tmp;
	return true;
}

template<>
bool SingleDeviceParameter<TimeRange>::setValue(std::string value)
{
	TimeRange tmp;
	if (!isTimeRange(value, tmp))
		return false;
	this->value = tmp;
	return true;
}


template <class T>
class MultipleDeviceParameter : public DeviceParameter
{
private:
	std::vector<std::pair<int, T>> values;
public:
	MultipleDeviceParameter(std::string name, std::string fullname, std::vector<T> values) : DeviceParameter(name, fullname, DeviceParameterType::Multiple)
	{
		for (size_t i = 0; i < values.size(); ++i)
		{
			this->values.push_back({ i, values[i] });
		}
	};

	bool setValue(std::string value)
	{
		return false;
	}

	std::string getValue()
	{
		// id#value id#value id#value
		std::string strVals = "";
		if (values.size() == 0)
			return strVals;
		else
		{
			for (std::pair<int, T>& v : values)
			{
				strVals += toString(v.first) + '#' + toString(v.second);
				strVals += 6;
			}
			strVals.pop_back();
			return strVals;
		}

	}

	std::string getType()
	{
		return typeid(T).name();
	}

	bool removeValue(int id)
	{
		for (size_t i = 0; i < values.size(); i++)
		{
			if (values[i].first == id)
			{
				values.erase(values.begin() + i);
				return true;
			}
		}
		return false;
	}

	bool isValueExist(T value)
	{
		for (std::pair<int, T>& val : values)
		{
			if (val.second == value)
			{
				return true;
			}
		}
		return false;
	}

	//get parameter (real value in memory)
	std::vector<std::pair<int, T>> getParameters()
	{
		return this->values;
	}
};

template<>
bool MultipleDeviceParameter<int>::setValue(std::string value)
{
	int id;
	if (values.size() == 0)
		id = 1;
	else
		id = values.back().first + 1;

	int tmp;
	if (isInt(value, tmp))
	{
		values.push_back({ id, tmp });
		return true;
	}
	else
		return false;
}

template<>
bool MultipleDeviceParameter<double>::setValue(std::string value)
{
	int id;
	if (values.size() == 0)
		id = 1;
	else
		id = values.back().first + 1;

	double tmp;
	if (isDouble(value, tmp))
	{
		values.push_back({ id, tmp });
		return true;
	}
	else
		return false;
}

template<>
bool MultipleDeviceParameter<Time>::setValue(std::string value)
{
	int id;
	if (values.size() == 0)
		id = 1;
	else
		id = values.back().first + 1;

	Time tmp;
	if (isTime(value, tmp))
	{
		values.push_back({ id, tmp });
		std::sort(values.begin(), values.end(), [](std::pair<int, Time> timeA, std::pair<int, Time> timeB)
		{
			return timeA.second < timeB.second;
		});
		return true;
	}
	else
		return false;
}

template<>
bool MultipleDeviceParameter<TimeRange>::setValue(std::string value)
{
	TimeRange tmp;
	if (isTimeRange(value, tmp))
	{
		std::vector<TimeRange> tmpRange;
		for (auto& tr : values)
		{
			tmpRange.push_back(tr.second);
		}
		tmpRange = addTimeRangeToArray(tmpRange, tmp);
		values.clear();
		for (size_t i = 0; i < tmpRange.size(); ++i)
		{
			values.push_back({ i, tmp });
		}
		return true;
	}
	else
		return false;
}
#endif // !DEVICE_PARAMETER_H
