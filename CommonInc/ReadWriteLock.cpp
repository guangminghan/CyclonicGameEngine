
#include "ReadWriteLock.h"
CReadWriteLock::CReadWriteLock()
{
	m_nReadLocks = 0;
	m_nWriteLocks = 0;
	m_sState = 0;
	m_hAccessLock = CreateMutex(NULL, FALSE, NULL);
	m_hCanWrite = CreateMutex(NULL, FALSE, NULL);
	m_hCanRead = CreateEvent(NULL, TRUE, TRUE, NULL);
	m_other = NULL;
}

CReadWriteLock::CReadWriteLock(CReadWriteLock* other)
{
	m_nReadLocks = 0;
	m_nWriteLocks = 0;
	m_other = other;
}

CReadWriteLock::~CReadWriteLock()
{
	if (!m_other) {
		CloseHandle(m_hAccessLock);
		CloseHandle(m_hCanRead);
		CloseHandle(m_hCanWrite);
	}
	else {
		while (m_nReadLocks) {
			m_nReadLocks--;
			m_other->ReadUnlock();
		}
		while (m_nWriteLocks) {
			m_nWriteLocks--;
			m_other->WriteUnlock();
		}
	}
}

bool CReadWriteLock::ReadLock(DWORD dwMilliseconds)
{
	if (m_other) {
		if (m_other->ReadLock(dwMilliseconds)) {
			m_nReadLocks++;
			return true;
		}
		else
			return false;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nReadLocks++;
	if (m_sState == 2) {
		ReleaseMutex(m_hAccessLock);
		if (WaitForSingleObject(m_hCanRead, dwMilliseconds) == WAIT_TIMEOUT) {
			ReadUnlock();
			return false;
		}
	}
	else {
		if (m_sState == 0) {
			m_sState = 1;
			WaitForSingleObject(m_hCanWrite, 0);
		}
		ReleaseMutex(m_hAccessLock);
	}
	return true;
}

void CReadWriteLock::ReadUnlock()
{
	if (m_other) {
		m_nReadLocks--;
		m_other->ReadUnlock();
		return;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nReadLocks--;
	if (!m_nReadLocks) {
		if (m_nWriteLocks) {
			m_sState = 2;
			ResetEvent(m_hCanRead);
		}
		else
			m_sState = 0;
		ReleaseMutex(m_hCanWrite);
	}
	ReleaseMutex(m_hAccessLock);
}

bool CReadWriteLock::WriteLock(DWORD dwMilliseconds)
{
	if (m_other) {
		if (m_other->WriteLock(dwMilliseconds)) {
			m_nWriteLocks++;
			return true;
		}
		else
			return false;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nWriteLocks++;
	if (m_sState == 1) {
		ReleaseMutex(m_hAccessLock);
		if (WaitForSingleObject(m_hCanWrite, dwMilliseconds) == WAIT_TIMEOUT) {
			WriteUnlock();
			return false;
		}
	}
	else {
		if (m_sState == 0) {
			m_sState = 2;
			ResetEvent(m_hCanRead);
		}
		ReleaseMutex(m_hAccessLock);
		// Now, wait for any other write threads to finish
		if (WaitForSingleObject(m_hCanWrite, dwMilliseconds) == WAIT_TIMEOUT) {
			WriteUnlock();
			return false;
		}
	}
	return true;
}

void CReadWriteLock::WriteUnlock()
{
	if (m_other) {
		m_nWriteLocks--;
		m_other->WriteUnlock();
		return;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nWriteLocks--;
	ReleaseMutex(m_hCanWrite);	// Allow other write threads to run
	if (!m_nWriteLocks) {
		if (m_nReadLocks) {
			m_sState = 1;
			WaitForSingleObject(m_hCanWrite, 0);	// Just reset it, should always be signalled
		}
		else
			m_sState = 0;
		SetEvent(m_hCanRead);
	}
	ReleaseMutex(m_hAccessLock);
}
