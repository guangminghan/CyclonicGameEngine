#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "basemightyclientinterface.h"
enum PostOperatorType
{
	iorecv_id,
	iosend_id
};
typedef struct stWSAOVERLAPPED_ 
{
	WSAOVERLAPPED m_PostOverLapped;
	WSAEVENT m_EventArray[1];
	WSABUF m_WsaIOBuf;
	int iotype;
	int useSize;
	char iobuf[4096];
}STCT_WSAOVERLAPPED, *P_STCT_WSAOVERLAPPED;

class HandleIOOverlappedEventClient: public MightyThread
{
public:
	HandleIOOverlappedEventClient(void);
	virtual ~HandleIOOverlappedEventClient(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

};
class MightyTCPOverlappedEventClient :
	public ITcpClient
{
public:
	MightyTCPOverlappedEventClient(ITcpClientNotify* pNotify);
	virtual ~MightyTCPOverlappedEventClient(void);
public:
	virtual bool ConnectServer( const char* pServerAddr, unsigned short usPort);
	virtual bool SendData( const char* pData, int nLen);
	virtual void Disconnect(void);
	virtual void ReleaseConnection();

	int init(const char * ip,WORD port);
	int init(DWORD ip,WORD port);
	int afreshConnect();
	int Process();
	//int handleIO(SOCKET sct,HANDLE event);
	void DestroySocket();
	void pushOverlappCacheQueue(P_STCT_WSAOVERLAPPED pBackOverlapped);
	int PostIOOpt(P_STCT_WSAOVERLAPPED pReUseOverlapped,int iostyle,int isWait=1);
	int PostRecvOpt();
	int PostSendOpt(const char * outBuf,int size);
public:
	ITcpClientNotify * m_pNotifyCallBack;
	SOCKADDR_IN m_SockAddr;
	BOOL m_bRun;
	SOCKET m_ClientSock;
	WORD m_ConnectPort;
	DWORD m_dwAddress;
	string m_strAddress;
	//vector<P_STCT_WSAOVERLAPPED> m_pWsaOverLappedList;
	list<P_STCT_WSAOVERLAPPED> m_pWsaOverLappedList;
	list<P_STCT_WSAOVERLAPPED> m_pCacheWsaOverLappedList;
	list<P_STCT_WSAOVERLAPPED> m_pTaskWsaOverLappedList;
};
