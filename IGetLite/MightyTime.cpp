//#include "StdAfx.h"
#include ".\mightytime.h"


MightyTimeSpan::MightyTimeSpan() throw() :
m_timeSpan(0)
{
}

MightyTimeSpan::MightyTimeSpan( __time64_t time ) throw() :
m_timeSpan( time )
{
}

MightyTimeSpan::MightyTimeSpan(LONG lDays, int nHours, int nMins, int nSecs) throw()
{
	m_timeSpan = nSecs + 60* (nMins + 60* (nHours + __int64(24) * lDays));
}

LONGLONG MightyTimeSpan::GetDays() const throw()
{
	return( m_timeSpan/(24*3600) );
}

LONGLONG MightyTimeSpan::GetTotalHours() const throw()
{
	return( m_timeSpan/3600 );
}

LONG MightyTimeSpan::GetHours() const throw()
{
	return( LONG( GetTotalHours()-(GetDays()*24) ) );
}

LONGLONG MightyTimeSpan::GetTotalMinutes() const throw()
{
	return( m_timeSpan/60 );
}

LONG MightyTimeSpan::GetMinutes() const throw()
{
	return( LONG( GetTotalMinutes()-(GetTotalHours()*60) ) );
}

LONGLONG MightyTimeSpan::GetTotalSeconds() const throw()
{
	return( m_timeSpan );
}

LONG MightyTimeSpan::GetSeconds() const throw()
{
	return( LONG( GetTotalSeconds()-(GetTotalMinutes()*60) ) );
}

__time64_t MightyTimeSpan::GetTimeSpan() const throw()
{
	return( m_timeSpan );
}

MightyTimeSpan MightyTimeSpan::operator+( MightyTimeSpan span ) const throw()
{
	return( MightyTimeSpan( m_timeSpan+span.m_timeSpan ) );
}

MightyTimeSpan MightyTimeSpan::operator-( MightyTimeSpan span ) const throw()
{
	return( MightyTimeSpan( m_timeSpan-span.m_timeSpan ) );
}

MightyTimeSpan& MightyTimeSpan::operator+=( MightyTimeSpan span ) throw()
{
	m_timeSpan += span.m_timeSpan;
	return( *this );
}

MightyTimeSpan& MightyTimeSpan::operator-=( MightyTimeSpan span ) throw()
{
	m_timeSpan -= span.m_timeSpan;
	return( *this );
}

bool MightyTimeSpan::operator==( MightyTimeSpan span ) const throw()
{
	return( m_timeSpan == span.m_timeSpan );
}

bool MightyTimeSpan::operator!=( MightyTimeSpan span ) const throw()
{
	return( m_timeSpan != span.m_timeSpan );
}

bool MightyTimeSpan::operator<( MightyTimeSpan span ) const throw()
{
	return( m_timeSpan < span.m_timeSpan );
}

bool MightyTimeSpan::operator>( MightyTimeSpan span ) const throw()
{
	return( m_timeSpan > span.m_timeSpan );
}

bool MightyTimeSpan::operator<=( MightyTimeSpan span ) const throw()
{
	return( m_timeSpan <= span.m_timeSpan );
}

bool MightyTimeSpan::operator>=( MightyTimeSpan span ) const throw()
{
	return( m_timeSpan >= span.m_timeSpan );
}

/////////////////////////////////////////////////////////////////////////////
// MightyTime
/////////////////////////////////////////////////////////////////////////////

MightyTime WINAPI MightyTime::GetCurrentTime() throw()
{
	return( MightyTime( ::_time64( NULL ) ) );
}

MightyTime::MightyTime() throw() :
m_time(0)
{
}

MightyTime::MightyTime( __time64_t time )  throw():
m_time( time )
{
}

MightyTime::MightyTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
									  int nDST)
{
	struct tm atm;
	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	//_ASSERTE(nDay >= 1 && nDay <= 31);
	atm.tm_mday = nDay;
	//_ASSERTE(nMonth >= 1 && nMonth <= 12);
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	//_ASSERTE(nYear >= 1900);
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = nDST;
	m_time = _mktime64(&atm);
	//_ASSERTE(m_time != -1);       // indicates an illegal input time

	if(m_time == -1)
	{
		//AtlThrow(E_INVALIDARG);
		;
	}
}

MightyTime::MightyTime(WORD wDosDate, WORD wDosTime, int nDST)
{
	struct tm atm;
	atm.tm_sec = (wDosTime & ~0xFFE0) << 1;
	atm.tm_min = (wDosTime & ~0xF800) >> 5;
	atm.tm_hour = wDosTime >> 11;

	atm.tm_mday = wDosDate & ~0xFFE0;
	atm.tm_mon = ((wDosDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (wDosDate >> 9) + 80;
	atm.tm_isdst = nDST;
	m_time = _mktime64(&atm);
	//_ASSERTE(m_time != -1);       // indicates an illegal input time

	if(m_time == -1)
	{
		//AtlThrow(E_INVALIDARG);
		;
	}

}

MightyTime::MightyTime(const SYSTEMTIME& sysTime, int nDST) throw()
{
	if (sysTime.wYear < 1900)
	{
		__time64_t time0 = 0L;
		MightyTime timeT(time0);
		*this = timeT;
	}
	else
	{
		MightyTime timeT(
			(int)sysTime.wYear, (int)sysTime.wMonth, (int)sysTime.wDay,
			(int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond,
			nDST);
		*this = timeT;
	}
}

MightyTime::MightyTime(const FILETIME& fileTime, int nDST)
{
	// first convert file time (UTC time) to local time
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		m_time = 0;
		//AtlThrow(E_INVALIDARG);
		return;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		m_time = 0;
		//AtlThrow(E_INVALIDARG);		
		return;
	}

	// then convert the system time to a time_t (C-runtime local time)
	MightyTime timeT(sysTime, nDST);
	*this = timeT;
}

MightyTime& MightyTime::operator=( __time64_t time ) throw()
{
	m_time = time;

	return( *this );
}

MightyTime& MightyTime::operator+=( MightyTimeSpan span ) throw()
{
	m_time += span.GetTimeSpan();

	return( *this );
}

MightyTime& MightyTime::operator-=( MightyTimeSpan span ) throw()
{
	m_time -= span.GetTimeSpan();

	return( *this );
}

MightyTimeSpan MightyTime::operator-( MightyTime time ) const throw()
{
	return( MightyTimeSpan( m_time-time.m_time ) );
}

MightyTime MightyTime::operator-( MightyTimeSpan span ) const throw()
{
	return( MightyTime( m_time-span.GetTimeSpan() ) );
}

MightyTime MightyTime::operator+( MightyTimeSpan span ) const throw()
{
	return( MightyTime( m_time+span.GetTimeSpan() ) );
}

bool MightyTime::operator==( MightyTime time ) const throw()
{
	return( m_time == time.m_time );
}

bool MightyTime::operator!=( MightyTime time ) const throw()
{
	return( m_time != time.m_time );
}

bool MightyTime::operator<( MightyTime time ) const throw()
{
	return( m_time < time.m_time );
}

bool MightyTime::operator>( MightyTime time ) const throw()
{
	return( m_time > time.m_time );
}

bool MightyTime::operator<=( MightyTime time ) const throw()
{
	return( m_time <= time.m_time );
}

bool MightyTime::operator>=( MightyTime time ) const throw()
{
	return( m_time >= time.m_time );
}

struct tm* MightyTime::GetGmtTm(struct tm* ptm) const throw()
{
	if (ptm != NULL)
	{
		struct tm * ptmTemp;
		ptmTemp = _gmtime64(&m_time);

		if(ptmTemp == NULL)
			return NULL;

		*ptm = *ptmTemp;

		return ptm;
	}
	else
		return _gmtime64(&m_time);
}

struct tm* MightyTime::GetLocalTm(struct tm* ptm) const throw()
{
	if (ptm != NULL)
	{
		struct tm* ptmTemp = _localtime64(&m_time);
		if (ptmTemp == NULL)
			return NULL;    // indicates the m_time was not initialized!

		*ptm = *ptmTemp;
		return ptm;
	}
	else
		return _localtime64(&m_time);
}

bool MightyTime::GetAsSystemTime(SYSTEMTIME& timeDest) const throw()
{
	struct tm* ptm = GetLocalTm(NULL);
	if (!ptm)
		return false;

	timeDest.wYear = (WORD) (1900 + ptm->tm_year);
	timeDest.wMonth = (WORD) (1 + ptm->tm_mon);
	timeDest.wDayOfWeek = (WORD) ptm->tm_wday;
	timeDest.wDay = (WORD) ptm->tm_mday;
	timeDest.wHour = (WORD) ptm->tm_hour;
	timeDest.wMinute = (WORD) ptm->tm_min;
	timeDest.wSecond = (WORD) ptm->tm_sec;
	timeDest.wMilliseconds = 0;

	return true;
}

__time64_t MightyTime::GetTime() const throw()
{
	return( m_time );
}

int MightyTime::GetYear() const
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? (ptm->tm_year) + 1900 : 0 ; 
}

int MightyTime::GetMonth() const
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_mon + 1 : 0;
}

int MightyTime::GetDay() const
{
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_mday : 0 ; 
}

int MightyTime::GetHour() const
{
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_hour : -1 ; 
}

int MightyTime::GetMinute() const
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_min : -1 ; 
}

int MightyTime::GetSecond() const
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_sec : -1 ;
}

int MightyTime::GetDayOfWeek() const
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_wday + 1 : 0 ;
}

/////////////////////////////////////////////////////////////////////////////
// CFileTimeSpan
/////////////////////////////////////////////////////////////////////////////

/*ATLTIME_INLINE CFileTimeSpan::CFileTimeSpan() throw() :
m_nSpan( 0 )
{
}

ATLTIME_INLINE CFileTimeSpan::CFileTimeSpan( const CFileTimeSpan& span ) throw() :
m_nSpan( span.m_nSpan )
{
}

ATLTIME_INLINE CFileTimeSpan::CFileTimeSpan( LONGLONG nSpan ) throw() :
m_nSpan( nSpan )
{
}

ATLTIME_INLINE CFileTimeSpan& CFileTimeSpan::operator=( const CFileTimeSpan& span ) throw()
{
	m_nSpan = span.m_nSpan;

	return( *this );
}

ATLTIME_INLINE CFileTimeSpan& CFileTimeSpan::operator+=( CFileTimeSpan span ) throw()
{
	m_nSpan += span.m_nSpan;

	return( *this );
}

ATLTIME_INLINE CFileTimeSpan& CFileTimeSpan::operator-=( CFileTimeSpan span ) throw()
{
	m_nSpan -= span.m_nSpan;

	return( *this );
}

ATLTIME_INLINE CFileTimeSpan CFileTimeSpan::operator+( CFileTimeSpan span ) const throw()
{
	return( CFileTimeSpan( m_nSpan+span.m_nSpan ) );
}

ATLTIME_INLINE CFileTimeSpan CFileTimeSpan::operator-( CFileTimeSpan span ) const throw()
{
	return( CFileTimeSpan( m_nSpan-span.m_nSpan ) );
}

ATLTIME_INLINE bool CFileTimeSpan::operator==( CFileTimeSpan span ) const throw()
{
	return( m_nSpan == span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator!=( CFileTimeSpan span ) const throw()
{
	return( m_nSpan != span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator<( CFileTimeSpan span ) const throw()
{
	return( m_nSpan < span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator>( CFileTimeSpan span ) const throw()
{
	return( m_nSpan > span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator<=( CFileTimeSpan span ) const throw()
{
	return( m_nSpan <= span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator>=( CFileTimeSpan span ) const throw()
{
	return( m_nSpan >= span.m_nSpan );
}

ATLTIME_INLINE LONGLONG CFileTimeSpan::GetTimeSpan() const throw()
{
	return( m_nSpan );
}

ATLTIME_INLINE void CFileTimeSpan::SetTimeSpan( LONGLONG nSpan ) throw()
{
	m_nSpan = nSpan;
}


/////////////////////////////////////////////////////////////////////////////
// CFileTime
/////////////////////////////////////////////////////////////////////////////

ATLTIME_INLINE CFileTime::CFileTime() throw()
{
	dwLowDateTime = 0;
	dwHighDateTime = 0;
}

ATLTIME_INLINE CFileTime::CFileTime( const FILETIME& ft ) throw()
{
	dwLowDateTime = ft.dwLowDateTime;
	dwHighDateTime = ft.dwHighDateTime;
}

ATLTIME_INLINE CFileTime::CFileTime( ULONGLONG nTime ) throw()
{
	dwLowDateTime = DWORD( nTime );
	dwHighDateTime = DWORD( nTime>>32 );
}

ATLTIME_INLINE CFileTime& CFileTime::operator=( const FILETIME& ft ) throw()
{
	dwLowDateTime = ft.dwLowDateTime;
	dwHighDateTime = ft.dwHighDateTime;

	return( *this );
}

ATLTIME_INLINE CFileTime WINAPI CFileTime::GetCurrentTime() throw()
{
	CFileTime ft;
	GetSystemTimeAsFileTime(&ft);
	return ft;
}

ATLTIME_INLINE CFileTime& CFileTime::operator+=( CFileTimeSpan span ) throw()
{
	SetTime( GetTime()+span.GetTimeSpan() );

	return( *this );
}

ATLTIME_INLINE CFileTime& CFileTime::operator-=( CFileTimeSpan span ) throw()
{
	SetTime( GetTime()-span.GetTimeSpan() );

	return( *this );
}

ATLTIME_INLINE CFileTime CFileTime::operator+( CFileTimeSpan span ) const throw()
{
	return( CFileTime( GetTime()+span.GetTimeSpan() ) );
}

ATLTIME_INLINE CFileTime CFileTime::operator-( CFileTimeSpan span ) const throw()
{
	return( CFileTime( GetTime()-span.GetTimeSpan() ) );
}

ATLTIME_INLINE CFileTimeSpan CFileTime::operator-( CFileTime ft ) const throw()
{
	return( CFileTimeSpan( GetTime()-ft.GetTime() ) );
}

ATLTIME_INLINE bool CFileTime::operator==( CFileTime ft ) const throw()
{
	return( GetTime() == ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator!=( CFileTime ft ) const throw()
{
	return( GetTime() != ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator<( CFileTime ft ) const throw()
{
	return( GetTime() < ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator>( CFileTime ft ) const throw()
{
	return( GetTime() > ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator<=( CFileTime ft ) const throw()
{
	return( GetTime() <= ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator>=( CFileTime ft ) const throw()
{
	return( GetTime() >= ft.GetTime() );
}

ATLTIME_INLINE ULONGLONG CFileTime::GetTime() const throw()
{
	return( (ULONGLONG( dwHighDateTime )<<32)|dwLowDateTime );
}

ATLTIME_INLINE void CFileTime::SetTime( ULONGLONG nTime ) throw()
{
	dwLowDateTime = DWORD( nTime );
	dwHighDateTime = DWORD( nTime>>32 );
}

ATLTIME_INLINE CFileTime CFileTime::UTCToLocal() const throw()
{
	CFileTime ftLocal;

	::FileTimeToLocalFileTime( this, &ftLocal );

	return( ftLocal );
}

ATLTIME_INLINE CFileTime CFileTime::LocalToUTC() const throw()
{
	CFileTime ftUTC;

	::LocalFileTimeToFileTime( this, &ftUTC );

	return( ftUTC );
}*/


