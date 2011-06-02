//#include "stdafx.h"
//#include <windows.h>
#include ".\multiplelock.h"

CMultipleLock::CMultipleLock(void)
{
	::InitializeCriticalSection(&this->m_cs);
	this->m_hsemReaders	= ::CreateSemaphore( NULL, 0, MAXLONG, NULL );
	this->m_hsemWriters = ::CreateSemaphore( NULL, 0, MAXLONG, NULL );
	this->m_nActive		= 0;
	this->m_nWaitingReaders = 0;
	this->m_nWaitingWriters	= 0;
}

CMultipleLock::~CMultipleLock(void)
{
	this->m_nActive		= 0;
	this->m_nWaitingReaders = 0;
	this->m_nWaitingWriters	= 0;
	::DeleteCriticalSection(&this->m_cs);
	::CloseHandle(this->m_hsemReaders);
	::CloseHandle(this->m_hsemWriters);
}

void CMultipleLock::readEnter(void)
{

	::EnterCriticalSection(&this->m_cs);
	
	bool fResourceWritePending = ( this->m_nWaitingWriters || (this->m_nActive < 0) );

	if ( fResourceWritePending ) {
		
		this->m_nWaitingReaders++;
	}

	else {

		this->m_nActive++;
	}

	::LeaveCriticalSection(&this->m_cs);

	if ( fResourceWritePending ) {

		::WaitForSingleObject( this->m_hsemReaders, INFINITE );
	}
}

void CMultipleLock::writeEnter(void)
{
	::EnterCriticalSection(&this->m_cs);

	bool fResourceOwned = ( this->m_nActive != 0 );

	if ( fResourceOwned ) {

		this->m_nWaitingWriters++;
	}

	else {

		this->m_nActive = -1;
	}

	::LeaveCriticalSection(&this->m_cs);

	if ( fResourceOwned ) {

		::WaitForSingleObject( this->m_hsemWriters, INFINITE );
	}
}

void CMultipleLock::leave(void)
{
	::EnterCriticalSection(&this->m_cs);

	if ( this->m_nActive > 0 ) {

		this->m_nActive--;
	}
	
	else {

		this->m_nActive++;
	}

	HANDLE hsem		= NULL;
	long   lCount	= 1;

	if ( m_nActive == 0 ) {

		if ( this->m_nWaitingWriters > 0 ) {

			this->m_nActive = -1;
			this->m_nWaitingWriters--;
			hsem = this->m_hsemWriters;
		}

		else if ( this->m_nWaitingReaders > 0 ) {

			this->m_nActive = this->m_nWaitingReaders;
			this->m_nWaitingReaders = 0;
			lCount = this->m_nActive;
			hsem = this->m_hsemReaders;
		}
		else {

		}
	}
	::LeaveCriticalSection(&this->m_cs);

	if ( hsem != NULL ) {

		::ReleaseSemaphore( hsem, lCount, NULL );
	}
}
