#ifndef TIMER_INCLUDED
#define TIMER_INCLUDED

#include <stdio.h>

class RDTimer {
public:
	static unsigned overhead;
	RDTimer() {Start();}
	void Start() {
		startTime = GetCPUTicks();
	}
	float Reset() {
		long long curtime = GetCPUTicks();
		float seconds=cpuSpeed*(curtime-startTime);
		startTime=curtime;
		return seconds;
	}
	float Peek() const {
		long long curtime = GetCPUTicks();
		return cpuSpeed*(curtime-startTime);
	}

	static void Init();
	static volatile inline long long GetCPUTicks() {
                __asm cpuid
                __asm rdtsc
	}
	static float cpuSpeed;
private:
	long long startTime;
};

#endif
