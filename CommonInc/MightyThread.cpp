//#include "StdAfx.h"
#include ".\MightyThread.h"

MightyThread::MightyThread(void)
:m_ThreadHandle(NULL)
,m_nThreadID(0)
,m_hQueueCompleteIO(NULL)
,m_ExecuteSignal(NULL)
,m_ExitSignal(NULL)
,m_dwTimeOut(INFINITE)
,m_ExecuteHandle_(NULL)
,m_nExecuteID_(0)
,m_nRunType(0)
{

}

MightyThread::~MightyThread(void)
{
}

int MightyThread::eventLoop(void)
{
	HANDLE IGT_Event[2];
	IGT_Event[0] = m_ExitSignal;
	IGT_Event[1] = m_ExecuteSignal;
	int retCode=0;
#ifdef IGT_WAITFOR_MULTIPLEOBJECTS
	while(1)
	{
		DWORD Index = WaitForMultipleObjects(2,IGT_Event,FALSE,m_dwTimeOut);	
		switch(Index) 
		{
			case WAIT_OBJECT_0 + 0:
			case WAIT_ABANDONED_0:
			case WAIT_ABANDONED_0+1:
				{
					_endthreadex(0);
					return 0;
				}
			case WAIT_TIMEOUT:
			case WAIT_OBJECT_0 + 1:
				{
					retCode=threadrun(m_userInstance.InstanceUserData_);
					switch(retCode) 
					{
						case 1:

							break;
						default:
							break;
					}

					break;
				}
			default:
				break;
		}
	}
#else
	for (;;) 
	{
		DWORD Index = MsgWaitForMultipleObjectsEx(sizeof(IGT_Event)/sizeof(IGT_Event[0]), IGT_Event, INFINITE, QS_ALLEVENTS, MWMO_ALERTABLE);
		switch (Index) 
		{
			case WAIT_OBJECT_0 + 0 : /* exit request */
				return 0;
				break;
			case WAIT_OBJECT_0 + 1 : /* 1 second timer */
				threadrun();
				break;
			case WAIT_OBJECT_0 + 2 : 
				{ /* Windows message */
					MSG msg;
					while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
					{
						if (msg.message == WM_QUIT) 
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
							return 0;
						}
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				break;
			case WAIT_IO_COMPLETION : /* loop */
			case WAIT_TIMEOUT :
				break;

		}
	}

#endif

}
int MightyThread::ExecuteLoop(void)
{
	unsigned long	pN1, pN2; 
	OVERLAPPED*	pOverLapped;
	while(::GetQueuedCompletionStatus(m_hQueueCompleteIO, &pN1, &pN2,(LPOVERLAPPED* )&pOverLapped,INFINITE))
	{
		if(pOverLapped==(OVERLAPPED*)0xFFFFFFFF)
			break;
		else
		{
			ExecuteTask((void *)pN2);
		}
	}
	_endthreadex(0);
	return 0;
}
void MightyThread::PostWorkMessage(DWORD dwCommand,void * comptKey,void * pOverLp)
{
	::PostQueuedCompletionStatus(m_hQueueCompleteIO,dwCommand,reinterpret_cast<DWORD>(comptKey),(LPOVERLAPPED)(pOverLp));
}
HANDLE MightyThread::GetThreadHandle()
{
	return m_ThreadHandle;
}