#ifndef MIGHTYTHREAD_INCLUDE_H
#define MIGHTYTHREAD_INCLUDE_H

#pragma once
//#include <tchar.h>
//#include <list>
//#include <string>
//#include <iomanip>
//#include <fstream>
//#include <sstream>
//#include <strstream>
//#include <time.h>
//#include <utility>
//using namespace std;
#include "./common.h"
#ifdef _UNICODE
typedef wstring tstring;
typedef wostringstream  tostringstream;
typedef wistringstream tistringstream;
typedef wfstream tfstream;
typedef wofstream tofstream;
typedef wifstream tifstream;
#else
typedef string  tstring;
typedef ostringstream tostringstream;
typedef istringstream tistringstream;
typedef fstream tfstream;
typedef ofstream tofstream;
typedef ifstream tifstream;
#endif
//#include<process.h>
#include "./MultipleLock.h"
#include "./ReadWriteLock.h"
#define IGT_WAITFOR_MULTIPLEOBJECTS 1
//#define IGT_MSG_WAITFOR_MULTIPLEOBJECTSEx
#define EVENT_TIMEOUT 0
#define IGT_COMPLETEIO 1
#define USE_WIN_CREATETHREAD 0
enum thread_style
{
	COMMPLETE_PORT=1,
	MULTI_EVENT,
	WAITABLE_TIMER

};
typedef struct ThreadUserData
{
	void * threadThis_;
	void * InstanceUserData_;
}TUD,*PTUD;
class MightyThread
{
public:
	MightyThread(void);
	virtual ~MightyThread(void);
public:

	int start(void * userInfo)
	{
		m_userInstance.threadThis_=this;
		m_userInstance.InstanceUserData_=userInfo;
		//m_ThreadHandle=(HANDLE)_beginthreadex(NULL,0,threadfunc,this,0,&m_nThreadID);
		
		//m_ThreadHandle = (HANDLE)_beginthreadex(NULL,0,threadfunc,this,CREATE_SUSPENDED,&m_nThreadID);
		#ifndef IGT_WAITFOR_MULTIPLEOBJECTS
		m_ExecuteSignal = CreateWaitableTimer(NULL, FALSE, NULL);//采用该方法需要在预编译头中定义#define _WIN32_WINNT 0x0501
		if (m_ExecuteSignal == NULL) 
		{
			return 1;
		}
		else 
		{
			DWORD Period = (1<<EVENT_TIMEOUT) * 1000;
			LARGE_INTEGER DueTime;
			DueTime.QuadPart = -(Period * 10000i64);
			if (!SetWaitableTimer(m_ExecuteSignal, &DueTime, Period, NULL, NULL, FALSE) != NO_ERROR) 
			{
				return 1;
			}
		}
		m_nRunType=WAITABLE_TIMER;
		
		#else
		m_ExecuteSignal=CreateEvent(NULL,FALSE,FALSE,NULL);
		m_nRunType=MULTI_EVENT;
		#endif
		m_ExitSignal=CreateEvent(NULL,FALSE,FALSE,NULL);


		m_ThreadHandle=(HANDLE)_beginthreadex(NULL,0,threadfunc,(void *)&m_userInstance,0,&m_nThreadID);

		return 0;
	}
	int init(void * userInfo)
	{
		m_userInstance.threadThis_=this;
		m_userInstance.InstanceUserData_=userInfo;
		m_hQueueCompleteIO=CreateIoCompletionPort((HANDLE)INVALID_HANDLE_VALUE, NULL, 0, 0);
		m_ExecuteHandle_=(HANDLE)_beginthreadex(NULL,0,ExecuteTaskThread,(void *)&m_userInstance,0,&m_nExecuteID_);
		m_nRunType=COMMPLETE_PORT;
		return 0;
	}
	bool setExecuteSignal() 
	{
		if (m_nRunType==COMMPLETE_PORT)
		{
			return true;
		}
		#ifndef IGT_WAITFOR_MULTIPLEOBJECTS
			return true;
		#endif
		return (FALSE != ::SetEvent(m_ExecuteSignal));
	}
	bool resetExecuteSignal() 
	{
		if (m_nRunType==COMMPLETE_PORT)
		{
			return true;
		}
		#ifndef IGT_WAITFOR_MULTIPLEOBJECTS
				return true;
		#endif
		return (FALSE != ::ResetEvent(m_ExecuteSignal));
	}
	bool setExitSignal() 
	{
		if (m_nRunType==COMMPLETE_PORT)
		{
			return true;
		}
		return (FALSE != ::SetEvent(m_ExitSignal));
	}
	bool resetExitSignal() 
	{
		if (m_nRunType==COMMPLETE_PORT)
		{
			return true;
		}
		return (FALSE != ::ResetEvent(m_ExitSignal));
	}

	void setTimeOutVal(DWORD dwVal) {m_dwTimeOut=(dwVal>0) ?dwVal:100;}
	void exit()
	{
		if (m_nRunType==COMMPLETE_PORT)
		{
			if (m_hQueueCompleteIO)
			{
				
				::PostQueuedCompletionStatus(m_hQueueCompleteIO,0,0,(OVERLAPPED*)0xFFFFFFFF);
				if (m_ExecuteHandle_)
				{
					DWORD ExitCode;
					int ReDoCount = 0;
					int nThreadCode;
					while(1)
					{
						if(ReDoCount++ >= 5)
						{
							TerminateThread(m_ExecuteHandle_,0);
							break;
						}
						nThreadCode=GetExitCodeThread(m_ExecuteHandle_,&ExitCode);
						if(ExitCode == STILL_ACTIVE)
						{
							if (WaitForSingleObject(m_ExecuteHandle_,10000L) ==WAIT_OBJECT_0)
							{
								break;
							}
							continue;
						}
						else
							break;
					}
					CloseHandle(m_ExecuteHandle_);
					m_ExecuteHandle_ = NULL;
				}
			}
			return;
		}
		resetExecuteSignal();
		setExitSignal();
		if (m_ThreadHandle)
		{
			DWORD code;
			int count = 0;
			while(1)
			{
				if(count++ >= 5)
				{
					TerminateThread(m_ThreadHandle,0);
					break;
				}
				GetExitCodeThread(m_ThreadHandle,&code);
				if(code == STILL_ACTIVE)
				{
					if (WaitForSingleObject(m_ThreadHandle,10000L) ==WAIT_OBJECT_0)
					{
						break;
					}
					continue;
				}
				else
					break;
			}
			CloseHandle(m_ThreadHandle);
			m_ThreadHandle = NULL;
			if(NULL != m_ExecuteSignal)
			{
				::CloseHandle(m_ExecuteSignal);
				m_ExecuteSignal=NULL;
			}
			if(NULL != m_ExitSignal)
			{
				::CloseHandle(m_ExitSignal);
				m_ExitSignal=NULL;
			}
			
		}

	}
	void ReadWait()
	{
		//m_Dispatch_Lock.readEnter();
		m_controlLock_.ReadLock();
	}
	void WriteWait()
	{
		//m_Dispatch_Lock.writeEnter();
		m_controlLock_.WriteLock();
	}
	/*void CancelWait()
	{
		m_Dispatch_Lock.leave();
	}*/
	void CancelReadWait()
	{
		m_controlLock_.ReadUnlock();
	}
	void CancelWriteWait()
	{
		m_controlLock_.WriteUnlock();
	}
	void PostWorkMessage(DWORD dwCommand,void * comptKey,void * pOverLp);
	HANDLE GetThreadHandle();
private:

	virtual int threadrun(void * pBuf)=0;

	virtual int ExecuteTask(void * pBuf)=0;

	virtual int branchEvent(void * pBuf)=0;

	int eventLoop(void);

	int ExecuteLoop(void);

	static unsigned int _stdcall threadfunc(void* ptr)
	{
		//MightyThread*	p = (MightyThread*)ptr ;
		PTUD p = (PTUD)ptr ;
		if ( p )
		{
			//((MightyThread*)p->threadThis_)->eventLoop();
			((MightyThread*)p->threadThis_)->threadrun(p->InstanceUserData_);
			//p->eventLoop();
		}
		return 0;
	}
	static unsigned __stdcall ExecuteTaskThread(void* lpParam)
	{
		MightyThread*	p = (MightyThread*)lpParam;
		if ( p )
		{
			p->ExecuteLoop();
		}
		return 0;
	}
	//static unsigned __stdcall QueryEventThread(void* lpParam);
public:
	HANDLE m_ExecuteSignal;//WaitForMultipleObjects模式
	HANDLE m_ExitSignal;
	DWORD m_dwTimeOut;
	TUD m_userInstance;
private:

	HANDLE m_ThreadHandle;//线程句柄
	//DWORD m_ThreadID;
	unsigned int  m_nThreadID;

	HANDLE m_ExecuteHandle_;
	unsigned int m_nExecuteID_;

	HANDLE	m_hQueueCompleteIO;//完成端口模式
	


	//CMultipleLock m_Dispatch_Lock;
	CReadWriteLock m_controlLock_;


	int m_nRunType;



};


#endif