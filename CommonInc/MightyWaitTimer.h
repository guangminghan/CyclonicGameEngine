#ifndef MIGHTYWAITTIMER_INCLUDE_H
#define MIGHTYWAITTIMER_INCLUDE_H

#pragma once
//#include<process.h>
//#define FILETIME_1970     0x019db1ded53e8000
//#define HECTONANOSECONDS  10000000ui64
#include "./common.h"
class MightyWaitTimer
{
public:
	MightyWaitTimer(void);
	~MightyWaitTimer(void);
public:
	static void CALLBACK TimerApcFunction(LPVOID lpArgToCompletionRoutine,DWORD dwTimerLowValue,DWORD dwTimerHighValue);
	static DWORD WINAPI ClockThread(void *arg);
	void StartClockThread(void);
	void StopClockThread(void);
private:
	virtual void ExecuteRun(void * pBuf)=0;
	int ExecuteLoop(void);
public:
	__int64 PerfFrequency;
	__int64 LastTimerCount;
	unsigned __int64 LastTimerTime;
	CRITICAL_SECTION TimerCritialSection;
	HANDLE ClockThreadHandle;
	HANDLE TimerThreadExitRequest;
	LONGLONG sys_ticks;
	HANDLE WaitableTimerHandle;

};
#endif