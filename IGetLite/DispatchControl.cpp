#include "DispatchControl.h"
#include "..\CommonInc\Log.h"
#include ".\_initinfo.h"
#include "..\CommonInc\P2PUtilTools.h"



CDispatchControl::CDispatchControl(void)
{
	this->m_dwUpdateInterval =10*1000;
	this->m_dwLastUpdateTime = GetTickCount() - this->m_dwUpdateInterval;
}

CDispatchControl::~CDispatchControl(void)
{

}
int CDispatchControl::mf_main(void *pService)
{
	char szProFileName[1024] = {0};
	//string runPath_=P2PUtilTools::getRunPath(szProFileName);
	//runPath_.append("swsHost.xml");
	//m_xml_serverData.Load(runPath_.c_str());

	


	C_InitInfo *pIniInfo = C_InitInfo::GetInstancePtr();
	
	//HANDLE hTimer  = NULL;


	pIniInfo->mf_init(P2PUtilTools::getIniPath(szProFileName));
	string config_path=P2PUtilTools::getRunPath(szProFileName);
	config_path.append("config\\sysparm.ini");
	pIniInfo->mf_init(config_path.c_str());

	//////////////////////////////////////////////////////////////////////////
	// get these values through TCP from server
	m_findServer_Pos=0;
	m_userID = "test1";			
	m_server_passwd = "server";
	//m_server_udpport = 60000;

	
	//int charID = rand()%1000000;
	//string charName = userID;
	//int charType = rand()%10;
	//////////////////////////////////////////////////////////////////////////


	//TcpPort=pIniInfo->mf_GetValueInt("TCPPORT",8800);
	m_UdpPort=pIniInfo->mf_GetValueInt("UDPPORT",9800);

	m_client = new UDPClientOnRakNet(m_userID,this);

	CustomPacketLogger* pLogger = dynamic_cast<CustomPacketLogger*>(m_client->GetPacketLogger());
	pLogger->SetConsoleTrace(true);

	m_client->Init(m_UdpPort);
	//assert( client->GetIncomingPasswd() == userID );





	//SystemAddress usingAddr=m_client->GetSystemAddress();

	doConnectServer();
	while (1)
	{
		m_client->Update();
		Sleep(50);
	}








	//-------------------------------------------------------------------------------

	//list<string> testIPlist;
	//bool sucFil=P2PUtilTools::GetIpList(testIPlist);
	//this->m_wListenPort = (WORD)pIniInfo->mf_GetValueInt("LISTENPORT", 8089);
	
	//ostringstream runLogStream;

	//m_SyncData_ComPort.StartServer(this->m_wListenPort);
	//CreateTimerQueueTimer(&hTimer, NULL, CDispatchControl::onTimerHandleProc, this, 5000, 30000, WT_EXECUTEDEFAULT);
	
	/*while (!pService->isStoped()) 
	{
		Sleep(10);
	}*/
	//DeleteTimerQueueTimer(NULL, hTimer, NULL);
	//m_server_peer.handle_IO_loop();
	//m_network_peer.initNetWork();
	
	return 0;
}
bool CDispatchControl::doConnectServer()
{
	C_InitInfo *pIniInfo = C_InitInfo::GetInstancePtr();
	if (pIniInfo->GetServerData(m_findServer_Pos,m_ipValVect))
	{
		m_findServer_Pos++;
		m_serverIpAddr=m_ipValVect.ip_;
		m_server_tcpport=m_ipValVect.tcpPort_;
		m_server_udpport=m_ipValVect.udpPort_;
		string tempAddrVal=P2PUtilTools::IpToString(m_serverIpAddr);
		//return m_client->ConnectServer(m_client->GetLocalIP(0), m_server_udpport, m_server_passwd);
		return m_client->ConnectServer(tempAddrVal, m_server_udpport, m_server_passwd);
	}
	return false;

}
void CDispatchControl::mf_checkTimeOut()
{
	//ostringstream runLogStream;
	//runLogStream.str("");
	//runLogStream<<"定时器队列检查是否超时\n";
	//LOG(Log_Debug,runLogStream.str().c_str());
	DWORD dwCurrTime = GetTickCount();
	if ( dwCurrTime - this->m_dwLastUpdateTime > this->m_dwUpdateInterval ) 
	{
		this->m_dwLastUpdateTime = dwCurrTime;
	}
}
void WINAPI CDispatchControl::onTimerHandleProc(PVOID pObj, BOOLEAN )
{

	CDispatchControl *This = (CDispatchControl*)pObj;
	This->mf_checkTimeOut();

}
