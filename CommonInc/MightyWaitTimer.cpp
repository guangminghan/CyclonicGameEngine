//#include "StdAfx.h"
#include ".\MightyWaitTimer.h"

MightyWaitTimer::MightyWaitTimer(void)
{

}

MightyWaitTimer::~MightyWaitTimer(void)
{
}



void CALLBACK MightyWaitTimer::TimerApcFunction(LPVOID lpArgToCompletionRoutine,DWORD dwTimerLowValue,DWORD dwTimerHighValue)
{
	MightyWaitTimer * pWaiterTime=(MightyWaitTimer *)(lpArgToCompletionRoutine);
	if (TryEnterCriticalSection(&(pWaiterTime->TimerCritialSection))) 
	{

			//CancelWaitableTimer(pWaiterTime->WaitableTimerHandle);
			pWaiterTime->ExecuteLoop();
			LeaveCriticalSection(&(pWaiterTime->TimerCritialSection));
	}
}


DWORD WINAPI MightyWaitTimer::ClockThread(void *arg)
{
	//LARGE_INTEGER DueTime;
	MightyWaitTimer * pWaiterTime=(MightyWaitTimer *)(arg);
	pWaiterTime->WaitableTimerHandle = CreateWaitableTimer(NULL, FALSE, NULL);


	/* Reset the Clock to a reasonable increment */
	//SetSystemTimeAdjustment(PRESET_TICK, FALSE);
		/*���룺1/1000�� 
		1�������1���ʮ�ڷ�֮һ 

		1����=1ǧ������ 


		0.1����=100������*/

	DWORD Period = (1<<2) * 1000;
	LARGE_INTEGER DueTime;
	DueTime.QuadPart = -(Period * 10000i64);
	//HANDLE hTimer;

	LARGE_INTEGER li;

	//hTimer = CreateWaitableTimer(NULL,FALSE,NULL);

	const int nNanosecondsPerSecond = 10000000;

	__int64 qwTimeFromNowInNanoseconds = 5 * nNanosecondsPerSecond;

	qwTimeFromNowInNanoseconds = - qwTimeFromNowInNanoseconds;

	li.LowPart = (DWORD)( qwTimeFromNowInNanoseconds & 0xFFFFFFFF);

	li.HighPart = (LONG)( qwTimeFromNowInNanoseconds >> 32);

	//SetWaitableTimer(hTimer,&li,0,NULL,NULL,FALSE);

/////////////////////////////////////////
	//DWORD Period =1000;
	//DueTime.QuadPart =Period * 10000i64;
	if (pWaiterTime->WaitableTimerHandle != NULL) {
		//DueTime.QuadPart = 0i64;
		if (SetWaitableTimer(pWaiterTime->WaitableTimerHandle,&DueTime,30*1000, TimerApcFunction,arg, FALSE) != NO_ERROR) {
			for(;;) {
				if (WaitForSingleObjectEx(pWaiterTime->TimerThreadExitRequest, INFINITE, TRUE) == WAIT_OBJECT_0) {
					break; 
				}
			}
		}
		CloseHandle(pWaiterTime->WaitableTimerHandle);
		pWaiterTime->WaitableTimerHandle = NULL;
	}
	return 0;
}

void MightyWaitTimer::StartClockThread(void)
{
	DWORD tid;
	FILETIME StartTime;
	LARGE_INTEGER Freq = { 0, 0 };


	if (QueryPerformanceFrequency(&Freq)) { 
		PerfFrequency = Freq.QuadPart;
	}


	GetSystemTimeAsFileTime(&StartTime);
	LastTimerTime = (((ULONGLONG) StartTime.dwHighDateTime) << 32) + (ULONGLONG) StartTime.dwLowDateTime;


	InitializeCriticalSection(&TimerCritialSection);
	TimerThreadExitRequest = CreateEvent(NULL, FALSE, FALSE, NULL);
	ClockThreadHandle = CreateThread(NULL, 0, ClockThread,this, 0, &tid);
	if (ClockThreadHandle != NULL) {
	
		if (!SetThreadPriority(ClockThreadHandle, THREAD_PRIORITY_TIME_CRITICAL)) {
			
		}
	}
}

void MightyWaitTimer::StopClockThread(void)
{	
	if (SetEvent(TimerThreadExitRequest) &&
		WaitForSingleObject(ClockThreadHandle, 10000L) == 0) {
			CloseHandle(TimerThreadExitRequest);
			TimerThreadExitRequest = NULL;

			CloseHandle(ClockThreadHandle);
			ClockThreadHandle = NULL;

			DeleteCriticalSection(&TimerCritialSection);
		} 
}

int MightyWaitTimer::ExecuteLoop(void)
{
	ExecuteRun(NULL);
	return 0;
}