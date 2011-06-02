#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "basemightyclientinterface.h"
enum OverlappedOperatorType
{
	RECV_ID,
	SEND_ID
};
typedef struct stWSAOVERLAPPED_EX 
{
	WSAOVERLAPPED m_PostOverLapped;
	//WSAEVENT m_EventArray[1];
	WSABUF m_WsaIOBuf;
	int iotype;
	int useSize;
	void * pMainCall;
	char iobuf[4096];
}Routine_WSAOVERLAPPED, *P_Routine_WSAOVERLAPPED;

class HandleIOCompletionRoutineClient: public MightyThread
{
public:
	HandleIOCompletionRoutineClient(void);
	virtual ~HandleIOCompletionRoutineClient(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

};
class MightyTCPCompletionRoutineClient :
	public ITcpClient
{
public:
	MightyTCPCompletionRoutineClient(ITcpClientNotify* pNotify);
	virtual ~MightyTCPCompletionRoutineClient(void);
public:
	virtual bool ConnectServer( const char* pServerAddr, unsigned short usPort);
	virtual bool SendData( const char* pData, int nLen);
	virtual void Disconnect(void);
	virtual void ReleaseConnection();

	int init(const char * ip,WORD port);
	int init(DWORD ip,WORD port);
	int afreshConnect();
	int Process();
	
	void DestroySocket();
	void pushOverlappCacheQueue(P_Routine_WSAOVERLAPPED pBackOverlapped);
	int PostIOOpt(P_Routine_WSAOVERLAPPED pReUseOverlapped,int iostyle,int isWait=1);
	int PostRecvOpt();
	int PostSendOpt(const char * outBuf,int size);
	static void CALLBACK WorkerRoutine(DWORD Error,DWORD BytesTranferred,LPWSAOVERLAPPED overlapped,DWORD Inflag);
public:
	ITcpClientNotify * m_pNotifyCallBack;
	SOCKADDR_IN m_SockAddr;
	BOOL m_bRun;
	SOCKET m_ClientSock;
	WORD m_ConnectPort;
	DWORD m_dwAddress;
	string m_strAddress;

	list<P_Routine_WSAOVERLAPPED> m_pWsaOverLappedList;
	list<P_Routine_WSAOVERLAPPED> m_pCacheWsaOverLappedList;
	list<P_Routine_WSAOVERLAPPED> m_pTaskWsaOverLappedList;
	WSAEVENT m_AlertableWaitStateArray[1];
};
