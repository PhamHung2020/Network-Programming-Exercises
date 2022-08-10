#include "Time.h"
#include <vector>
#include "Utility.h"

Time::Time()
{
	hour = 0;
	minute = 0;
	second = 0;
}

bool operator==(const Time &t1, const Time &t2)
{
	return t1.hour == t2.hour && t1.minute == t2.minute && t1.second == t2.second;
}

bool operator>(const Time & t1, const Time & t2)
{
	return t1.hour > t2.hour || (t1.hour == t2.hour && t1.minute > t2.minute) || (t1.hour == t2.hour && t1.minute == t2.minute && t1.second > t2.second);
}

bool operator<(const Time & t1, const Time & t2)
{
	return t1.hour < t2.hour || (t1.hour == t2.hour && t1.minute < t2.minute) || (t1.hour == t2.hour && t1.minute == t2.minute && t1.second < t2.second);
}

bool operator==(const TimeRange & tr1, const TimeRange & tr2)
{
	return tr1.start == tr2.start && tr1.end == tr2.end;
}

Time Time::build(std::string strTime)
{
	std::vector<std::string> splitStrs;
	int hour, minute, second;

	//split string time into substring by ":" e.g: 6:45:12 --> {6, 45, 12}
	if (splitString(strTime, ":", 1, splitStrs, -1) != 3)
		return INVALID_TIME;
	else {

		//convert substrings into int
		try {
			hour = std::stoi(splitStrs[0]);
			minute = std::stoi(splitStrs[1]);
			second = std::stoi(splitStrs[2]);
		}
		catch (const std::invalid_argument &e) {
			std::cout << e.what();
			return INVALID_TIME;
		}
		catch (const std::out_of_range &e) {
			std::cout << e.what();
			return INVALID_TIME;
		}

		//check if the value is valid
		if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59)
			return Time(hour, minute, second);
		else
			return INVALID_TIME;
	}

}

bool Time::compare(Time t1, Time t2)
{
	if (t1.hour < t2.hour || (t1.hour == t2.hour && t1.minute < t2.minute) || (t1.hour == t2.hour && t1.minute == t2.minute && t1.second < t2.second))
		return true;
	return false;
}

TimeRange::TimeRange()
{
	start = Time(0, 0, 0);
	end = Time(0, 0, 0);
}

TimeRange TimeRange::build(std::string strTimeRange)
{
	// HH:mm:ss HH:mm:ss
	if (strTimeRange.length() != 17)
		return INVALID_TIMERANGE;
	Time start, end;
	start = Time::build(strTimeRange.substr(0, 8));
	end = Time::build(strTimeRange.substr(9, 8));

	// start or end is invalid or start time is greater than end time
	if (start == INVALID_TIME || end == INVALID_TIME || !Time::compare(start, end))
	{
		return INVALID_TIMERANGE;
	}
	else
		return TimeRange(start, end);
}

TimeRange TimeRange::merge(TimeRange tr1, TimeRange tr2)
{
	return INVALID_TIMERANGE;
}

std::vector<TimeRange> addTimeRangeToArray(std::vector<TimeRange>& v, TimeRange tr)
{
	bool flag = false;
	for (int i = 0; i < v.size(); i++)
	{
		TimeRange TimeRangeMerge = TimeRange::merge(v[i], tr);
		if (v[i].start == TimeRangeMerge.start && v[i].end == TimeRangeMerge.end)
		{
			v.erase(v.begin() + i - 1);
			continue;
		}

		if (TimeRangeMerge.start == INVALID_TIME)
			continue;
		else
		{
			flag = true;
			if (Time::compare(v[i].end, TimeRangeMerge.end))
			{
				for (int j = i; j < v.size(); j++)
				{
					TimeRange tr1 = TimeRange::merge(TimeRangeMerge, v[j]);
					if (tr1.start == INVALID_TIME)
						continue;
					else
						v[j] = tr1;
				}
			}
			else v[i] = TimeRangeMerge;
		}
		v.erase(std::unique(v.begin(), v.end(), [](TimeRange a, TimeRange b)
		{
			return (a.start == b.start) && (a.end == b.end);
		}), v.end());
	}
	if (flag == false)
	{
		v.push_back(tr);
		std::sort(v.begin(), v.end(), [](TimeRange tr1, TimeRange tr2)
		{
			return Time::compare(tr1.start, tr2.start);
		});
	}

	return v;
}