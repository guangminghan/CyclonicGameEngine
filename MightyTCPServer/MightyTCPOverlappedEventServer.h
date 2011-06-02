#ifndef MIGHTY_TCP_OVERLAPPED_EVENT_SERVER_INCLUDE_H
#define MIGHTY_TCP_OVERLAPPED_EVENT_SERVER_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "BaseMightyServerInterface.h"
#define SINGLE_ORDER_MAX 5
#define BUFFER_SIZE 4096
typedef struct _OverlappedEventIOObject
{
	SOCKET s;
	sockaddr_in addrRemote;
	int nOutstandingOps;			// 记录此套节字上的重叠I/O数量

	LPFN_ACCEPTEX lpfnAcceptEx;		// 扩展函数AcceptEx的指针（仅对监听套节字而言）
	string identityIDKey;
}OverlappedEventIOObject,*POverlappedEventIOObject;


typedef struct _OverlappedEvent_BufObj
{	
	OVERLAPPED ol;			// 重叠结构
	char *buff;				// send/recv/AcceptEx所使用的缓冲区
	int nLen;				// buff的长度
	POverlappedEventIOObject pSocket;	// 此I/O所属的套节字对象

	int nOperation;			// 提交的操作类型
#define OP_ACCEPT	1
#define OP_READ		2
#define OP_WRITE	3
	SOCKET sAccept;			// 用来保存AcceptEx接受的客户套节字（仅对监听套节字而言）
}OverlappedEvent_BufObj, *POverlappedEvent_BufObj;
class HandleIOOverlappedEventServer: public MightyThread
{
public:
	HandleIOOverlappedEventServer(void);
	virtual ~HandleIOOverlappedEventServer(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

	//POverlappedEventIOObject findSocket(const SOCKET & sct);

	void RebuildEventArray();

	//void SetSelectHandle(SOCKET sct);
	//void RemoveSelectHandle(SOCKET sct);
	int AnalyzeProcess(POverlappedEvent_BufObj pBuffer);
	int WorkProcess();
	void RemoveSocketObj(POverlappedEventIOObject pSocket);

	int InsertSocketObj(POverlappedEventIOObject pSocket);
	
	//BOOL HandleIO(PSOCKET_OBJ pSocket);

	//void setNotifyEvent();

	void DestroyAllSocket();
	POverlappedEventIOObject SearchSocketObj(const string & keyId);

	//----------------
	POverlappedEvent_BufObj GetBufferObj(POverlappedEventIOObject pSocket, ULONG nLen);
	void FreeBufferObj(POverlappedEvent_BufObj pBuffer);
	POverlappedEvent_BufObj FindBufferObj(HANDLE hEvent);
	int PostAccept(POverlappedEvent_BufObj pBuffer);
	int PostRecv(POverlappedEvent_BufObj pBuffer);
	int PostSend(POverlappedEvent_BufObj pBuffer);

public:
	int nSocketCount;
	map<string,POverlappedEventIOObject> m_SocketMap;
	list<POverlappedEvent_BufObj> m_overLappedBufList;
	HANDLE m_Notify_Events[WSA_MAXIMUM_WAIT_EVENTS];
	int m_nEffectiveEventCount;				// m_Notify_Events数组中有效句柄数量
	int m_exitCode;
};

class OverlappedEventExecute : public MightyThread
{
public:
	OverlappedEventExecute(void);
	virtual ~OverlappedEventExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
public:
	
	

};
class MightyTCPOverlappedEventServer : public ITcpServer
{
public:
	MightyTCPOverlappedEventServer(ITcpServerNotify * pNotify);
	virtual ~MightyTCPOverlappedEventServer(void);

public:
	virtual bool Create(const unsigned short & usPort);
	virtual void Destroy(void);
	virtual bool SendData(const char *pData, int nLen, const string & strLinkNo);
	virtual bool CloseLink(const string & strLinkNo);
public:
	void startWork();
	POverlappedEventIOObject GetSocketObj(SOCKET s);
	int acceptSocketObject();
	void AssignToFreeThread(POverlappedEventIOObject pSocket);
	//---------------------------------------
	void FreeSocketObj(POverlappedEventIOObject pSocket);
	POverlappedEvent_BufObj GetBufferObj(POverlappedEventIOObject pSocket, ULONG nLen);
	void FreeBufferObj(POverlappedEvent_BufObj pBuffer);
	POverlappedEvent_BufObj FindBufferObj(HANDLE hEvent);
	int PostAccept(POverlappedEvent_BufObj pBuffer);
	int PostRecv(POverlappedEvent_BufObj pBuffer);
	int PostSend(POverlappedEvent_BufObj pBuffer);
	//---------------------------------------
	int ExecuteProcess(POverlappedEvent_BufObj pBuffer);


	void RecombinationArray();
public:
	SOCKET m_ListenSock;
	WORD m_ListenPort;
	DWORD m_Address;

	vector<MightyThread *> m_pThreadPool;
	OverlappedEventExecute * m_pAcceptThread;
	int m_tdPoolIndex;
	ITcpServerNotify * m_pNotifyCallBack;

	HANDLE m_Notify_Handles[WSA_MAXIMUM_WAIT_EVENTS];
	int m_nEffectiveHandleCount;				// m_Notify_Events数组中有效句柄数量
	list<POverlappedEvent_BufObj> m_overLappedBufList;
	POverlappedEventIOObject m_pListenObj;
	int m_exitNotify;
};
#endif