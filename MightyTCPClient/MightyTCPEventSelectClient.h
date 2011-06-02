#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "basemightyclientinterface.h"
class HandleIOEventSelectClient: public MightyThread
{
public:
	HandleIOEventSelectClient(void);
	virtual ~HandleIOEventSelectClient(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

};
class MightyTCPEventSelectClient :
	public ITcpClient
{
public:
	MightyTCPEventSelectClient(ITcpClientNotify* pNotify);
	virtual ~MightyTCPEventSelectClient(void);
public:
	virtual bool ConnectServer( const char* pServerAddr, unsigned short usPort);
	virtual bool SendData( const char* pData, int nLen);
	virtual void Disconnect(void);
	virtual void ReleaseConnection();

	int init(const char * ip,WORD port);
	int init(DWORD ip,WORD port);
	int afreshConnect();
	int Process();
	int handleIO(SOCKET sct,HANDLE event);
	void DestroySocket();
public:
	ITcpClientNotify * m_pNotifyCallBack;
	WSAEVENT m_EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	BOOL m_bRun;
	SOCKADDR_IN m_SockAddr;
	SOCKET m_ClientSock;
	WORD m_ConnectPort;
	DWORD m_dwAddress;
	string m_strAddress;
	HandleIOEventSelectClient * m_pHandleIOProcess;
};
