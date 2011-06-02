// Control_Mutex.h: interface for the DB_Mutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROL_MUTEX_H__D638F590_56FA_43DB_97FF_BA98FDFE1CB7__INCLUDED_)
#define AFX_CONTROL_MUTEX_H__D638F590_56FA_43DB_97FF_BA98FDFE1CB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN32
#include <windows.h>
#define CONTROL_MUTEX CRITICAL_SECTION
#define CONTROL_MUTEX_INIT(m) InitializeCriticalSection(m)
#define CONTROL_MUTEX_LOCK(m) EnterCriticalSection(m)
#define CONTROL_MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#define CONTROL_MUTEX_RELEASE(m) DeleteCriticalSection(m)
#else
#include <pthread.h>
#define CONTROL_MUTEX pthread_mutex_t
#define CONTROL_MUTEX_INIT(m) pthread_mutex_init(m,NULL)
#define CONTROL_MUTEX_LOCK(m) pthread_mutex_lock(m)
#define CONTROL_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#define CONTROL_MUTEX_RELEASE(m) pthread_mutex_destroy(m)
#endif

class CONTROL_Mutex  
{
public:
	CONTROL_Mutex();
	virtual ~CONTROL_Mutex();
	void lock();
	void unlock();
private:
	CONTROL_MUTEX mutex_;
};

#endif // !defined(AFX_DB_MUTEX_H__D638F590_56FA_43DB_97FF_BA98FDFE1CB7__INCLUDED_)

