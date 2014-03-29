#include "Timer.h"

#define NOMINMAX

#include <windows.h>

float RDTimer::cpuSpeed=0;
unsigned RDTimer::overhead=0;

void RDTimer::Init() {
	overhead = (unsigned)abs(GetCPUTicks()-GetCPUTicks());
	long timeStart = timeGetTime()+1;
	while (timeGetTime() < unsigned(timeStart));
	unsigned long long startTick = GetCPUTicks();
	timeStart = timeGetTime()+1000;
	while(timeGetTime() < unsigned(timeStart));
	unsigned long long endTick = GetCPUTicks();
	cpuSpeed=1.0f/((endTick - startTick) - overhead);
}
