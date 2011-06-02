#pragma once

#include "../CommonInc/common.h"
#include "XMLControlInterface.h"
#include "PacketID.h"
#include "RakClient.h"
#include "CustomPacketLogger.h"
#include "UDPClientOnRakNet.h"

using raknet_wrapper::RakClient;
using raknet_wrapper::CustomPacketLogger;
class CDispatchControl
{
public:
	CDispatchControl(void);
	~CDispatchControl(void);
	int mf_main(void *pService);
public:
	bool doConnectServer();
private:
	 void mf_checkTimeOut();
	 static void WINAPI onTimerHandleProc(PVOID, BOOLEAN );

public:
	DWORD m_dwLastUpdateTime;
	DWORD m_dwUpdateInterval;
	XMLControlInterface m_xml_serverData;

	UDPClientOnRakNet * m_client;

	unsigned short m_TcpPort;			
	unsigned short m_UdpPort;
	string m_userID;			
	string m_server_passwd;
	DWORD m_serverIpAddr;
	unsigned short m_server_tcpport;
	unsigned short m_server_udpport;
	unsigned long m_findServer_Pos;
	TRACKERIP_ m_ipValVect;
};
