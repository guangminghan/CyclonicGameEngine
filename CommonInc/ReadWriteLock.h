#ifndef CREAD_WRITE_LOCK_INCLUDE_H
#define CREAD_WRITE_LOCK_INCLUDE_H
//#include "windows.h"
#pragma once
#include "./common.h"
class CReadWriteLock
{
public:
	CReadWriteLock();
	CReadWriteLock(CReadWriteLock* other);
	~CReadWriteLock();
	bool ReadLock(DWORD dwMilliseconds = INFINITE);
	void ReadUnlock();
	bool WriteLock(DWORD dwMilliseconds = INFINITE);
	void WriteUnlock();
private:
	HANDLE m_hAccessLock;
	HANDLE m_hCanRead;
	HANDLE m_hCanWrite;
	int m_nReadLocks;
	int m_nWriteLocks;
	int m_sState;
	CReadWriteLock* m_other;
};
#endif