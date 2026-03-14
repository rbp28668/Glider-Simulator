#pragma once
#include <windows.h>
#include <cstdint>


class Timer {

	double period = 0.0;
public:

	Timer() {
		LARGE_INTEGER freq;
		::QueryPerformanceFrequency(&freq);
		period = 1.0 / (double)freq.QuadPart;
	}

	inline double now() {
		LARGE_INTEGER t;
		::QueryPerformanceCounter(&t);
		return t.QuadPart * period;
	}

	inline int64_t raw() {
		LARGE_INTEGER t;
		::QueryPerformanceCounter(&t);
		return t.QuadPart;
	}

	inline double since(int64_t s)	{
		LARGE_INTEGER t;
		::QueryPerformanceCounter(&t);
		int64_t ticks = int64_t(t.QuadPart) - s;
		return ticks * period;
	}
};