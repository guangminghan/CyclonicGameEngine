#if !defined(INITINFO__INCLUDED_)
#define INITINFO__INCLUDED_

#pragma once
//#include <map>
//#include <string>
//#include <vector>
//#include <list>
#include "../CommonInc/common.h"
#include "../CommonInc/MultipleLock.h"
using namespace std;

inline bool operator <(const string &_Left, const string &_Right) {

	const char *szLeft = _Left.c_str();

	const char *szRight = _Right.c_str();

	for ( ; (*szLeft == *szRight) && (*szLeft != '\0') && (*szRight != '\0'); szLeft++, szRight++ );

	return *szLeft < *szRight;

}


#pragma pack(push, 1)



class C_InitInfo
{

private:

	C_InitInfo(void);
	C_InitInfo(const C_InitInfo &);//禁止复制构造
	C_InitInfo & operator = (const C_InitInfo &);//禁止=操作

public:

	~C_InitInfo(void);

	static C_InitInfo& GetInstance() { static C_InitInfo _instance;return _instance; }

	static C_InitInfo* GetInstancePtr() { return &C_InitInfo::GetInstance(); }

	bool mf_init(const char *szIniFileName);

	DWORD mf_GetValueInt(const char *szVariantName, DWORD dwDefault);

	bool mf_GetValueString(const char *szVariantName, OUT char *pszOut, int len);

	void splipTrackerAddress(const char * netIp);

	bool GetServerData(unsigned long & index,TRACKERIP_ & tckIP);

	bool slipFileTask(const char * pFiles);

	bool GetTaskByFileID(const QWORD & id,string & fileName);

public:
	string m_IniFileName;

private:

	typedef map<string, string> MAP_NAME_STRING;

	typedef map<string, DWORD> MAP_NAME_DWORD;
	vector<TRACKERIP_> m_configIPVect;
	
	CMultipleLock m_lock;

	MAP_NAME_STRING m_nameStringMap;

	MAP_NAME_DWORD m_nameDwordMap;
	map<QWORD,string> m_fileTaskMap;
	


};

#pragma pack(pop)

#endif	//!defined(INITINFO__INCLUDED_)