
#pragma once
//#include "windows.h"
#include "./common.h"
#pragma pack(push, 1)
class CMultipleLock
{
public:
	CMultipleLock(void);
	~CMultipleLock(void);
	void readEnter(void);
	void writeEnter(void);
	void leave(void);
private:
	CRITICAL_SECTION m_cs;
	HANDLE			 m_hsemReaders;
	HANDLE			 m_hsemWriters;
	int				 m_nWaitingReaders;
	int				 m_nWaitingWriters;
	int				 m_nActive;
};
#pragma pack(pop)
