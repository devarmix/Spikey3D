#pragma once

namespace Spikey
{
	inline TimeSpan TimeSpan::operator+(const TimeSpan& other) const
	{
		return TimeSpan(m_Ticks + other.m_Ticks);
	}

	inline TimeSpan& TimeSpan::operator+=(const TimeSpan& other)
	{
		m_Ticks += other.m_Ticks;
		return *this;
	}

	inline TimeSpan TimeSpan::operator-() const
	{
		return TimeSpan(-m_Ticks);
	}

	inline TimeSpan TimeSpan::operator-(const TimeSpan& other) const
	{
		return TimeSpan(m_Ticks - other.m_Ticks);
	}

	inline TimeSpan& TimeSpan::operator-=(const TimeSpan& other)
	{
		m_Ticks -= other.m_Ticks;
		return *this;
	}

	inline TimeSpan  TimeSpan::operator*(float scalar) const
	{
		return TimeSpan((int64)((float)m_Ticks * scalar));
	}

	inline TimeSpan& TimeSpan::operator*=(float scalar)
	{
		m_Ticks = (int64)((float)m_Ticks * scalar);
		return *this;
	}

	inline bool TimeSpan::operator==(const TimeSpan& other) const
	{
		return m_Ticks == other.m_Ticks;
	}

	inline bool TimeSpan::operator!=(const TimeSpan& other) const
	{
		return m_Ticks != other.m_Ticks;
	}

	inline bool TimeSpan::operator>(const TimeSpan& other) const
	{
		return m_Ticks > other.m_Ticks;
	}

	inline bool TimeSpan::operator>=(const TimeSpan& other) const
	{
		return m_Ticks >= other.m_Ticks;
	}

	inline bool TimeSpan::operator<(const TimeSpan& other) const
	{
		return m_Ticks < other.m_Ticks;
	}

	inline bool TimeSpan::operator<=(const TimeSpan& other) const
	{
		return m_Ticks <= other.m_Ticks;
	}

	inline double TimeSpan::GetTotalDays() const
	{
		return (double)m_Ticks / TicksPerDay;
	}

	inline double TimeSpan::GetTotalHours() const
	{
		return (double)m_Ticks / TicksPerHour;
	}

	inline double TimeSpan::GetTotalMilliseconds() const
	{
		return (double)m_Ticks / TicksPerMillisecond;
	}

	inline double TimeSpan::GetTotalMinutes() const
	{
		return (double)m_Ticks / TicksPerMinute;
	}

	inline float TimeSpan::GetTotalSeconds() const
	{
		return static_cast<float>(m_Ticks) / TicksPerSecond;
	}

	inline TimeSpan TimeSpan::FromDays(double days)
	{
		assert((days >= MinValue().GetTotalDays()) && (days <= MaxValue().GetTotalDays()));
		return TimeSpan(static_cast<int64>(days * TicksPerDay));
	}

	inline TimeSpan TimeSpan::FromHours(double hours)
	{
		assert((hours >= MinValue().GetTotalHours()) && (hours <= MaxValue().GetTotalHours()));
		return TimeSpan(static_cast<int64>(hours * TicksPerHour));
	}

	inline TimeSpan TimeSpan::FromMilliseconds(double milliseconds)
	{
		assert((milliseconds >= MinValue().GetTotalMilliseconds()) && (milliseconds <= MaxValue().GetTotalMilliseconds()));
		return TimeSpan(static_cast<int64>(milliseconds * TicksPerMillisecond));
	}

	inline TimeSpan TimeSpan::FromMinutes(double minutes)
	{
		assert((minutes >= MinValue().GetTotalMinutes()) && (minutes <= MaxValue().GetTotalMinutes()));
		return TimeSpan(static_cast<int64>(minutes * TicksPerMinute));
	}

	inline TimeSpan TimeSpan::FromSeconds(double seconds)
	{
		assert((seconds >= MinValue().GetTotalSeconds()) && (seconds <= MaxValue().GetTotalSeconds()));
		return TimeSpan(static_cast<int64>(seconds * TicksPerSecond));
	}

	inline TimeSpan TimeSpan::MaxValue()
	{
		return TimeSpan(9223372036854775807);
	}

	inline TimeSpan TimeSpan::MinValue()
	{
		return TimeSpan(-9223372036854775807 - 1);
	}

	inline TimeSpan TimeSpan::Zero()
	{
		return TimeSpan(0);
	}

	inline void TimeSpan::Set(int32 days, int32 hours, int32 minutes, int32 seconds, int32 milliseconds)
	{
		const int64 totalMs = 1000 * (60 * 60 * 24 * (int64)days + 60 * 60 * (int64)hours + 60 * (int64)minutes + (int64)seconds) + (int64)milliseconds;
		assert((totalMs >= MinValue().GetTotalMilliseconds()) && (totalMs <= MaxValue().GetTotalMilliseconds()));
		m_Ticks = totalMs * TicksPerMillisecond;
	}

	inline DateTime DateTime::operator+(const TimeSpan& other) const
	{
		return DateTime(m_Ticks + other.GetTicks());
	}

	inline DateTime& DateTime::operator+=(const TimeSpan& other)
	{
		m_Ticks += other.GetTicks();
		return *this;
	}

	inline TimeSpan DateTime::operator-(const DateTime& other) const
	{
		return TimeSpan(m_Ticks - other.m_Ticks);
	}

	inline DateTime DateTime::operator-(const TimeSpan& other) const
	{
		return DateTime(m_Ticks - other.GetTicks());
	}

	inline DateTime& DateTime::operator-=(const TimeSpan& other)
	{
		m_Ticks -= other.GetTicks();
		return *this;
	}

	inline bool DateTime::operator==(const DateTime& other) const
	{
		return m_Ticks == other.m_Ticks;
	}

	inline bool DateTime::operator!=(const DateTime& other) const
	{
		return m_Ticks != other.m_Ticks;
	}

	inline bool DateTime::operator>(const DateTime& other) const
	{
		return m_Ticks > other.m_Ticks;
	}

	inline bool DateTime::operator>=(const DateTime& other) const
	{
		return m_Ticks >= other.m_Ticks;
	}

	inline bool DateTime::operator<(const DateTime& other) const
	{
		return m_Ticks < other.m_Ticks;
	}

	inline bool DateTime::operator<=(const DateTime& other) const
	{
		return m_Ticks <= other.m_Ticks;
	}

	inline int32 DateTime::DaysInMonth(int32 year, int32 month)
	{
		static const int32 s_DaysPerMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

		assert(month >= 1 && month <= 12);
		if (month == 2 && IsLeapYear(year))
		{
			return 29;
		}

		return s_DaysPerMonth[month];
	}

	inline int32 DateTime::DaysInYear(int32 year)
	{
		if (IsLeapYear(year))
		{
			return 366;
		}
		return 365;
	}

	inline bool DateTime::IsLeapYear(int32 year)
	{
		if ((year % 4) == 0)
		{
			return (((year % 100) != 0) || ((year % 400) == 0));
		}

		return false;
	}

	inline DateTime DateTime::MaxValue()
	{
		return DateTime(3652059 * TimeSpan::TicksPerDay - 1);
	}

	inline DateTime DateTime::MinValue()
	{
		return DateTime(1, 1, 1, 0, 0, 0, 0);
	}

	inline DateTime DateTime::NowLocal()
	{
		static_assert(false);
	}

	inline DateTime DateTime::NowUTC()
	{
		static_assert(false);
	}

	inline bool DateTime::Validate(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 millisecond)
	{
		return (year >= 1) && (year <= 9999) &&
			(month >= 1) && (month <= 12) &&
			(day >= 1) && (day <= DaysInMonth(year, month)) &&
			(hour >= 0) && (hour <= 23) &&
			(minute >= 0) && (minute <= 59) &&
			(second >= 0) && (second <= 59) &&
			(millisecond >= 0) && (millisecond <= 999);
	}

	inline void DateTime::Set(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 millisecond)
	{
		static const int32 s_DaysToMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

		assert(Validate(year, month, day, hour, minute, second, millisecond));
		int32 daysSum = 0;
		if (month > 2 && IsLeapYear(year))
			daysSum++;
		year--;
		month--;
		daysSum += year * 365 + year / 4 - year / 100 + year / 400 + s_DaysToMonth[month] + day - 1;
		m_Ticks = daysSum * TimeSpan::TicksPerDay
			+ hour * TimeSpan::TicksPerHour
			+ minute * TimeSpan::TicksPerMinute
			+ second * TimeSpan::TicksPerSecond
			+ millisecond * TimeSpan::TicksPerMillisecond;
	}
}