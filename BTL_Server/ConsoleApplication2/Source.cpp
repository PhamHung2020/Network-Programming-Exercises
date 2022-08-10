#include <iostream>
#include <typeinfo>
#include <string>

struct Time
{
	int hour;
	int minute;
	int second;

	Time(int hour, int minute, int second) : hour(hour), minute(minute), second(second) {};
	Time();
};

std::string toString(int value)
{
	return std::to_string(value);
}

std::string toString(Time time)
{
	return std::to_string(time.hour) + ':' + std::to_string(time.minute) + ':' + std::to_string(time.second);
}

template <typename T>
class A
{
public:
	T value;
	A(T value)
	{
		this->value = value;
	}
	void str()
	{
		std::cout << toString(value);
	}
};
int main()
{
	A<int> a(5);
	a.str();
	getchar();
	return 0;
}