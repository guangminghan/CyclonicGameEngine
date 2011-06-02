
#include ".\_initinfo.h"
#include "..\CommonInc\P2PUtilTools.h"
C_InitInfo::C_InitInfo(void)
{

}

C_InitInfo::~C_InitInfo(void)
{
}


bool C_InitInfo::mf_init(const char *szIniFileName)
{
	this->m_lock.writeEnter();

	this->m_IniFileName = szIniFileName;

	
//字符串类型的
	//1.swsHost.xml下载路径
	char szSwsHost[256]={0};
	GetPrivateProfileString("IGet", "HostUrl",szSwsHost,szSwsHost,sizeof(szSwsHost) - 1, szIniFileName);
	this->m_nameStringMap["HostUrl"] =szSwsHost;
	//2.GoMonitor
	char szMonitorPath[256]={0};
	GetPrivateProfileString("IGet","GoMonitor",szMonitorPath,szMonitorPath,sizeof(szMonitorPath)-1,szIniFileName);
	this->m_nameStringMap["GoMonitor"] =szMonitorPath;

	char szIp[2048]={0};
	GetPrivateProfileString("IGet", "IPAddr",szIp,szIp,sizeof(szIp) - 1, szIniFileName);
	this->m_nameStringMap["IPAddr"] =szIp;
	splipTrackerAddress(szIp);

	char szShareFilePath[256]={0};
	GetPrivateProfileString("IGet", "SHARE_PATH",szShareFilePath,szShareFilePath,sizeof(szShareFilePath) - 1, szIniFileName);
	this->m_nameStringMap["SHARE_PATH"] =szShareFilePath;

	char szIncomePath[256]={0};
	GetPrivateProfileString("IGet", "INCOME_PATH",szIncomePath,szIncomePath,sizeof(szIncomePath) - 1, szIniFileName);
	this->m_nameStringMap["INCOME_PATH"] =szIncomePath;

	char szTask[4096]={0};
	GetPrivateProfileString("IGet", "FILETASK",szTask,szTask,sizeof(szTask) - 1, szIniFileName);
	this->m_nameStringMap["FILETASK"] =szTask;
	slipFileTask(szTask);

//-----------------------------------------------------------------------------------------------------------------
//数值型的
	this->m_nameDwordMap["expireHours"] = GetPrivateProfileInt("IGet", "expireHours",72, szIniFileName);

	this->m_nameDwordMap["downloadRate"] = GetPrivateProfileInt("IGet", "downloadRate",20480, szIniFileName);

	this->m_nameDwordMap["notifyPhpCycle"] = GetPrivateProfileInt("IGet", "notifyPhpCycle",10, szIniFileName);

	this->m_nameDwordMap["maxThreadCount"] = GetPrivateProfileInt("IGet", "maxThreadCount",5, szIniFileName);

	this->m_nameDwordMap["minDiskSpace"] = GetPrivateProfileInt("IGet", "minDiskSpace",2, szIniFileName);

	this->m_nameDwordMap["MaxPeerNumber"] = GetPrivateProfileInt("IGet", "MaxPeerNumber",16, szIniFileName);

	this->m_nameDwordMap["MaxChunkSize"] = GetPrivateProfileInt("IGet", "MaxChunkSize",480, szIniFileName);

	this->m_nameDwordMap["MaxConrrentNumber"] = GetPrivateProfileInt("IGet", "MaxConrrentNumber",10, szIniFileName);

	this->m_nameDwordMap["MaxTaskchunkSize"] = GetPrivateProfileInt("IGet", "MaxTaskchunkSize",80, szIniFileName);

	this->m_nameDwordMap["TCPPORT"] = GetPrivateProfileInt("IGet", "TCPPORT",8090, szIniFileName);
	this->m_nameDwordMap["UDPPORT"] = GetPrivateProfileInt("IGet", "UDPPORT",9090, szIniFileName);
	this->m_lock.leave();

	return true;

}

DWORD C_InitInfo::mf_GetValueInt(const char *szVariantName, DWORD dwDefault)
{

	DWORD dwRetValue = dwDefault;

	bool isNoFind = false;

	this->m_lock.readEnter();

	if ( this->m_nameDwordMap.find(szVariantName) != this->m_nameDwordMap.end() ) dwRetValue = this->m_nameDwordMap[szVariantName];

	else isNoFind = true;

	this->m_lock.leave();

	if ( isNoFind ) {

		dwDefault = GetPrivateProfileInt("IGet", szVariantName, dwDefault, this->m_IniFileName.c_str());

		this->m_lock.writeEnter();

		this->m_nameDwordMap[szVariantName] = dwDefault;

		dwRetValue = dwDefault;

		this->m_lock.leave();

	}

	return dwRetValue;

}

bool C_InitInfo::mf_GetValueString(const char *szVariantName, OUT char *pszOut, int len)
{

	bool bFind = false;

	this->m_lock.readEnter();

	bFind = this->m_nameStringMap.find(szVariantName) != this->m_nameStringMap.end();

	if ( bFind ) 
		strncpy(pszOut, this->m_nameStringMap[szVariantName].c_str(), len);

	this->m_lock.leave();

	if ( ! bFind ) {

		int nCopyed = GetPrivateProfileString("IGet", szVariantName, "", pszOut, len, this->m_IniFileName.c_str());

		if ( nCopyed > 0 ) {

			this->m_lock.writeEnter();

			this->m_nameStringMap[szVariantName] = pszOut;

			this->m_lock.leave();

			bFind = true;

		}

	}

	return bFind;

}
void C_InitInfo::splipTrackerAddress(const char * netIp)
{
	std::string::size_type leftPos;
	std::string::size_type loopPos;
	string slipStr;
	string subStrIP,subStrTcpPort,subStrUdpPort;
	//vector<string> tempRoom;
	string rawStr;
	rawStr=netIp;
	int checkSum=0;
	loopPos=0;
	while ((leftPos=rawStr.find_first_of('$',loopPos))!=string::npos)
	{
		slipStr=rawStr.substr(loopPos,leftPos-loopPos);
		loopPos=leftPos+1;

			//tempRoom.push_back(slipStr);
			std::string::size_type Pos1;
			std::string::size_type Pos2;
			Pos2=0;
			checkSum=0;
			TRACKERIP_ loopVal;
			while ((Pos1=slipStr.find_first_of(':',Pos2))!=string::npos)
			{
				if (checkSum==0)
				{
					subStrIP=slipStr.substr(Pos2,Pos1-Pos2);
					loopVal.ip_=inet_addr(subStrIP.c_str());
				}
				else if (checkSum==1)
				{
					subStrUdpPort=slipStr.substr(Pos2,Pos1-Pos2);
					loopVal.udpPort_=atoi(subStrUdpPort.c_str());
				}
				checkSum++;
				Pos2=Pos1+1;
			}
			if ((Pos2<slipStr.size())&&(checkSum==2))
			{

				subStrTcpPort=slipStr.substr(Pos2,slipStr.size()-Pos2);
				loopVal.tcpPort_=atoi(subStrTcpPort.c_str());
			}

			m_configIPVect.push_back(loopVal);
	}

}
bool C_InitInfo::GetServerData(unsigned long & index,TRACKERIP_ & tckIP)
{
	this->m_lock.readEnter();
	if (m_configIPVect.empty())
	{
		this->m_lock.leave();
		return false;
	}
	index=index<m_configIPVect.size()?index:0;
	tckIP.ip_=m_configIPVect[index].ip_;
	tckIP.tcpPort_=m_configIPVect[index].tcpPort_;
	tckIP.udpPort_=m_configIPVect[index].udpPort_;
	this->m_lock.leave();
	return true;

}
bool C_InitInfo::slipFileTask(const char * pFiles)
{
	
	std::string::size_type leftPos;
	std::string::size_type loopPos;
	string slipStr;
	string subStrID,subStrFileName;

	string rawStr;
	rawStr=pFiles;
	int checkSum=0;
	loopPos=0;
	while ((leftPos=rawStr.find_first_of('$',loopPos))!=string::npos)
	{
		slipStr=rawStr.substr(loopPos,leftPos-loopPos);
		loopPos=leftPos+1;

		std::string::size_type Pos1;
		std::string::size_type Pos2;
		Pos2=0;
		checkSum=0;
		
		while ((Pos1=slipStr.find_first_of(':',Pos2))!=string::npos)
		{
			if (checkSum==0)
			{
				subStrID=slipStr.substr(Pos2,Pos1-Pos2);
				
			}
			checkSum++;
			Pos2=Pos1+1;
		}
		if ((Pos2<slipStr.size())&&(checkSum==1))
		{
			subStrFileName=slipStr.substr(Pos2,slipStr.size()-Pos2);
		}
		m_fileTaskMap.insert(map<QWORD,string>::value_type(_atoi64(subStrID.c_str()),subStrFileName));
		
	}
	return true;
}

bool C_InitInfo::GetTaskByFileID(const QWORD & id,string & fileName)
{
	map<QWORD,string>::iterator findIndex,LoopIndex;
	if ((findIndex=m_fileTaskMap.find(id))!=m_fileTaskMap.end())
	{
		fileName=(*findIndex).second;
		return true;
	}
	return false;

}