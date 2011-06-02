#pragma once
#include "../CommonInc/common.h"
typedef struct tagDBTIMESTAMP
{
	SHORT year;
	USHORT month;
	USHORT day;
	USHORT hour;
	USHORT minute;
	USHORT second;
	ULONG fraction;
} 	DBTIMESTAMP;
class MightyTimeSpan
{
public:
	MightyTimeSpan() throw();
	MightyTimeSpan( __time64_t time ) throw();
	MightyTimeSpan( LONG lDays, int nHours, int nMins, int nSecs ) throw();

	LONGLONG GetDays() const throw();
	LONGLONG GetTotalHours() const throw();
	LONG GetHours() const throw();
	LONGLONG GetTotalMinutes() const throw();
	LONG GetMinutes() const throw();
	LONGLONG GetTotalSeconds() const throw();
	LONG GetSeconds() const throw();

	__time64_t GetTimeSpan() const throw();

	MightyTimeSpan operator+( MightyTimeSpan span ) const throw();
	MightyTimeSpan operator-( MightyTimeSpan span ) const throw();
	MightyTimeSpan& operator+=( MightyTimeSpan span ) throw();
	MightyTimeSpan& operator-=( MightyTimeSpan span ) throw();
	bool operator==( MightyTimeSpan span ) const throw();
	bool operator!=( MightyTimeSpan span ) const throw();
	bool operator<( MightyTimeSpan span ) const throw();
	bool operator>( MightyTimeSpan span ) const throw();
	bool operator<=( MightyTimeSpan span ) const throw();
	bool operator>=( MightyTimeSpan span ) const throw();
/*
#ifndef _ATL_MIN_CRT
public:
	CString Format( LPCTSTR pszFormat ) const;
	CString Format( UINT nID ) const;
#endif
#if defined(_AFX) && defined(_UNICODE)
	// for compatibility with MFC 3.x
	CString Format(LPCSTR pFormat) const;
#endif

#ifdef _AFX
	CArchive& Serialize64(CArchive& ar);
#endif
	*/

private:
	__time64_t m_timeSpan;
};
class MightyTime
{
public:
	static MightyTime WINAPI GetCurrentTime() throw();

	MightyTime() throw();
	MightyTime( __time64_t time ) throw();
	MightyTime( int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1 );
	MightyTime( WORD wDosDate, WORD wDosTime, int nDST = -1 );
	MightyTime( const SYSTEMTIME& st, int nDST = -1 ) throw();
	MightyTime( const FILETIME& ft, int nDST = -1 );
#ifdef __oledb_h__
	MightyTime( const DBTIMESTAMP& dbts, int nDST = -1 ) throw();
#endif

	MightyTime& operator=( __time64_t time ) throw();

	MightyTime& operator+=( MightyTimeSpan span ) throw();
	MightyTime& operator-=( MightyTimeSpan span ) throw();

	MightyTimeSpan operator-( MightyTime time ) const throw();
	MightyTime operator-( MightyTimeSpan span ) const throw();
	MightyTime operator+( MightyTimeSpan span ) const throw();

	bool operator==( MightyTime time ) const throw();
	bool operator!=( MightyTime time ) const throw();
	bool operator<( MightyTime time ) const throw();
	bool operator>( MightyTime time ) const throw();
	bool operator<=( MightyTime time ) const throw();
	bool operator>=( MightyTime time ) const throw();

	struct tm* GetGmtTm( struct tm* ptm = NULL ) const throw();
	struct tm* GetLocalTm( struct tm* ptm = NULL ) const throw();
	bool GetAsSystemTime( SYSTEMTIME& st ) const throw();
	bool GetAsDBTIMESTAMP( DBTIMESTAMP& dbts ) const throw();

	__time64_t GetTime() const throw();

	int GetYear() const throw();
	int GetMonth() const throw();
	int GetDay() const throw();
	int GetHour() const throw();
	int GetMinute() const throw();
	int GetSecond() const throw();
	int GetDayOfWeek() const throw();

/*#ifndef _ATL_MIN_CRT
	// formatting using "C" strftime
	CString Format( LPCTSTR pszFormat ) const;
	CString FormatGmt( LPCTSTR pszFormat ) const;
	CString Format( UINT nFormatID ) const;
	CString FormatGmt( UINT nFormatID ) const;
#endif*/

/*#if defined(_AFX) && defined(_UNICODE)
	// for compatibility with MFC 3.x
	CString Format(LPCSTR pFormat) const;
	CString FormatGmt(LPCSTR pFormat) const;
#endif

#ifdef _AFX
	CArchive& Serialize64(CArchive& ar);
#endif*/

private:
	__time64_t m_time;
};