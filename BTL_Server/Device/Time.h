#ifndef TIME_H
#define TIME_H

#include <string>
#include <algorithm>
#include <vector>
struct Time
{
	int hour;
	int minute;
	int second;

	Time(int hour, int minute, int second) : hour(hour), minute(minute), second(second) {};
	Time();
	
	static Time build(std::string);
	static bool compare(Time, Time);
};

const Time INVALID_TIME = Time(-1, -1, -1);
bool operator==(const Time &t1, const Time &t2);
bool operator>(const Time &t1, const Time &t2);
bool operator<(const Time &t1, const Time &t2);

struct TimeRange
{
	Time start;
	Time end;

	TimeRange(Time start, Time end) : start(start), end(end) {};
	TimeRange();
	static TimeRange build(std::string);
	static TimeRange merge(TimeRange, TimeRange);
};

const TimeRange INVALID_TIMERANGE = TimeRange(INVALID_TIME, INVALID_TIME);
bool operator==(const TimeRange &tr1, const TimeRange &tr2);
std::vector<TimeRange> addTimeRangeToArray(std::vector<TimeRange>&, TimeRange);
#endif // !TIME_H
