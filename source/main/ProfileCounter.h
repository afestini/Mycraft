#ifndef PROFILECOUNTER_INCLUDED
#define PROFILECOUNTER_INCLUDED

#include "timer.h"
#include "OpenGL.h"
#include "glfont.h"

#include <string>
#include <vector>
#include <algorithm>


class ProfileCounter {
	static unsigned Overhead;
	
	/*static volatile inline __int64 Ticks() {
			__asm cpuid
			__asm rdtsc
	}*/
	static inline __int64 Ticks()
	{
		return (__int64)(glfwGetTime()*1000000);
	}
	static void Init() {Overhead=(unsigned)-(Ticks()-Ticks());}
	
	static unsigned FrameTime;
	static std::vector<ProfileCounter*> Counters;

	RDTimer UpdateTimer;
	std::vector<unsigned> Values;
	float Interval;
	unsigned curPos, Samples, numValues;
	float Update;
	std::string Name;
	float ScaleX, ScaleY;
	float Seconds;
	__int64 StartTicks, SumTicks;
	unsigned Min, Max, Avg, Sum;

public:
	static bool Profiling;

	ProfileCounter(const char* name, float interval, float secs);
	~ProfileCounter();
	void Print(float x, float y, float scale);
	static ProfileCounter* NewCounter(const char* name, float inter, float secs);
	static void DrawCounters();

	inline void Start() {if (Profiling) StartTicks=Ticks();}

	inline void Stop() {
		if (!Profiling) return;
		SumTicks+=Ticks()-(StartTicks+Overhead);
		Update+=UpdateTimer.Reset();
		++Samples;
		if (Update>=Interval) {
			unsigned val = unsigned(SumTicks/Samples);
			if (val >= Max) Max=val; else if (Values[curPos]==Max) Max=0;
			if (val <= Min) Min=val; else if (Values[curPos]==Min) Min=0;
			Sum+=val-Values[curPos];
			Avg=Sum/numValues;
			Values[curPos++]=val;

			if (!Min) Min = *std::min_element(Values.begin(), Values.end());
			if (!Max) Max = *std::max_element(Values.begin(), Values.end());
			if (curPos>=numValues) curPos=0;

			Update-=Interval;
			SumTicks=0;
			Samples=0;			
			
			ScaleY=1.0f;
			while (Max>ScaleY) ScaleY*=10;
			ScaleY=1.0f/ScaleY;
		}
	}
};

#endif
