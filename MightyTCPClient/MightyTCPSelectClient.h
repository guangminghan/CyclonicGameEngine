#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "basemightyclientinterface.h"
class HandleIOSelectClient: public MightyThread
{
public:
	HandleIOSelectClient(void);
	virtual ~HandleIOSelectClient(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
public:
	fd_set fdClientRead;//FD_SETSIZE
	fd_set fdClientWrite;
	fd_set fdClientExcept;

};
class MightyTCPSelectClient :
	public ITcpClient
{
public:
	MightyTCPSelectClient(ITcpClientNotify* pNotify);
	virtual ~MightyTCPSelectClient(void);
public:
	virtual bool ConnectServer( const char* pServerAddr, unsigned short usPort);
	virtual bool SendData( const char* pData, int nLen);
	virtual void Disconnect(void);
	virtual void ReleaseConnection();
public:
	int init(const char * ip,WORD port);
	int init(DWORD ip,WORD port);
	int ConnectRemote(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port);
	int asyncConnectRemote(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port, int msecWait);
	int asyncConnectRemoteEx(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port, int msecWait);
	bool waitSocketRead(SOCKET s, DWORD waitTime/*∫¡√Î*/);
	int afreshConnect();
	int Process();
	
public:
	ITcpClientNotify * m_pNotifyCallBack;
	SOCKADDR_IN m_SockAddr;
	SOCKET m_ClientSock;
	WORD m_ConnectPort;
	DWORD m_dwAddress;
	string m_strAddress;
};
