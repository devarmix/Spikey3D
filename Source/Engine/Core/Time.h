#pragma once

#include <Engine/Core/Common.h>

namespace Spikey
{
	class TimeSpan
	{
	public:
		static constexpr int64 TicksPerDay = 864000000000;
		static constexpr int64 TicksPerHour = 36000000000;
		static constexpr int64 TicksPerMillisecond = 10000;
		static constexpr int64 TicksPerMinute = 600000000;
		static constexpr int64 TicksPerSecond = 10000000;
		static constexpr int64 TicksPerWeek = 6048000000000;

	private:
		int64 m_Ticks;

	public:
		TimeSpan()
			: m_Ticks(0)
		{
		}

		TimeSpan(int64 ticks)
			: m_Ticks(ticks)
		{
		}

		TimeSpan(int32 days, int32 hours = 0, int32 minutes = 0, int32 seconds = 0, int32 milliseconds = 0)
		{
			Set(days, hours, minutes, seconds, milliseconds);
		}

		int64 GetTicks() const
		{
			return m_Ticks;
		}

	public:
		TimeSpan  operator+(const TimeSpan& other) const;
		TimeSpan& operator+=(const TimeSpan& other);
		TimeSpan  operator-() const;
		TimeSpan  operator-(const TimeSpan& other) const;
		TimeSpan& operator-=(const TimeSpan& other);
		TimeSpan  operator*(float scalar) const;
		TimeSpan& operator*=(float scalar);

		bool operator==(const TimeSpan& other) const;
		bool operator!=(const TimeSpan& other) const;
		bool operator>(const TimeSpan& other) const;
		bool operator>=(const TimeSpan& other) const;
		bool operator<(const TimeSpan& other) const;
		bool operator<=(const TimeSpan& other) const;

	public:
		double GetTotalDays() const;
		double GetTotalHours() const;
		double GetTotalMilliseconds() const;
		double GetTotalMinutes() const;
		float  GetTotalSeconds() const;

	public:
		static TimeSpan FromDays(double days);
		static TimeSpan FromHours(double hours);
		static TimeSpan FromMilliseconds(double milliseconds);
		static TimeSpan FromMinutes(double minutes);
		static TimeSpan FromSeconds(double seconds);

		static TimeSpan MaxValue();
		static TimeSpan MinValue();
		static TimeSpan Zero();

	private:
		void Set(int32 days, int32 hours, int32 minutes, int32 seconds, int32 milliseconds);
	};

	class DateTime
	{
	private:
		int64 m_Ticks = 0;

	public:
		DateTime() = default;

		DateTime(int64 ticks)
			: m_Ticks(ticks)
		{
		}

		DateTime(int32 year, int32 month = 0, int32 day = 0, int32 hour = 0, int32 minute = 0, int32 second = 0, int32 millisecond = 0)
		{
			Set(year, month, day, hour, minute, second, millisecond);
		}

		int64 GetTicks() const
		{
			return m_Ticks;
		}

	public:
		DateTime  operator+(const TimeSpan& other) const;
		DateTime& operator+=(const TimeSpan& other);
		TimeSpan  operator-(const DateTime& other) const;
		DateTime  operator-(const TimeSpan& other) const;
		DateTime& operator-=(const TimeSpan& other);

		bool operator==(const DateTime& other) const;
		bool operator!=(const DateTime& other) const;
		bool operator>(const DateTime& other) const;
		bool operator>=(const DateTime& other) const;
		bool operator<(const DateTime& other) const;
		bool operator<=(const DateTime& other) const;

	public:
		static int32 DaysInMonth(int32 year, int32 month);
		static int32 DaysInYear(int32 year);
		static bool IsLeapYear(int32 year);

		static DateTime MaxValue();
		static DateTime MinValue();
		static DateTime NowLocal();
		static DateTime NowUTC();

		static bool Validate(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 millisecond);

	private:
		void Set(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 millisecond);
	};

	class Time
	{
	public:

		static TimeSpan DeltaTime;
		static TimeSpan UnscaledDeltaTime;
		// static TimeSpan Time;
		static TimeSpan UnscaledTime;
	};
}

#include <Engine/Core/Time_inl.h>