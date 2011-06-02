#ifndef MIGHTY_TCP_COMPLETION_PORT_INCLUDE_H
#define MIGHTY_TCP_COMPLETION_PORT_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "BaseMightyServerInterface.h"
#define MAX_THREAD 4
struct _CompletionPort_BufObj;
typedef struct _CompletionPortIOObject
{
	SOCKET s;
	sockaddr_in addrRemote;
	sockaddr_in addrLocal;			// 连接的本地地址
	//int nOutstandingOps;			// 记录此套节字上的重叠I/O数量
	int nOutstandingRecv;	// 此套节字上抛出的读重叠操作的数量
	int nOutstandingSend;//此套节字上抛出的写重叠操作的数量

	ULONG nReadSequence;			// 安排给接收的下一个序列号
	ULONG nCurrentReadSequence;		// 当前要读的序列号
	int nCondition;
	list<struct _CompletionPort_BufObj *> * pBufQueue;
	string identityIDKey;
}CompletionPortIOObject,*PCompletionPortIOObject;

typedef struct _CompletionPort_BufObj
{	
	WSAOVERLAPPED wsaol;
	WSABUF wsaDataBuf;
	char *buff;
	int nLen;
	int nOperation;			// 提交的操作类型
	SOCKET sAccept;		// AcceptEx接收的客户方套节字
	ULONG nSequenceNumber;	// 此I/O的序列号
#define OP_ACCEPT	1
#define OP_WRITE	2
#define OP_READ		3
	PCompletionPortIOObject pSocket;
	void * pAgent;

}CompletionPort_BufObj, *PCompletionPort_BufObj;
//--------------------------------------------------------
class SortList : less<PCompletionPort_BufObj>
{

public:

	bool operator()(const PCompletionPort_BufObj& _Left,const PCompletionPort_BufObj& _Right) const
	{

		

		//const char* f=strrchr(_Left.csIp,'.')+1;

		//const char* n=strrchr(_Right.csIp,'.')+1;

		//if(atoi(f)<atoi(n))

			//return true;
		if(_Left->nSequenceNumber < _Right->nSequenceNumber)
		{
			return true;
		}
		
		return false;

	}

};
//--------------------------------------------------------
class HandleIOCompletionPortServer: public MightyThread
{
public:
	HandleIOCompletionPortServer(void);
	virtual ~HandleIOCompletionPortServer(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

};

class CompletionPortExecute : public MightyThread
{
public:
	CompletionPortExecute(void);
	virtual ~CompletionPortExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
};
class MightyTCPCompletionPortServer: public ITcpServer
{
public:
	MightyTCPCompletionPortServer(ITcpServerNotify * pNotify);
	virtual ~MightyTCPCompletionPortServer(void);
public:
	virtual bool Create(const unsigned short & usPort);
	virtual void Destroy(void);
	virtual bool SendData(const char *pData, int nLen, const string & strLinkNo);
	virtual bool CloseLink(const string & strLinkNo);

public:
	void startWork();
	int acceptSocketObject();
	PCompletionPortIOObject GetSocketObj(SOCKET s);
	int InsertSocketObj(PCompletionPortIOObject pSocket);
	void RemoveSocketObj(PCompletionPortIOObject pSocket);
	PCompletionPortIOObject SearchSocketObj(const string & keyId);
	//void AssignToFreeThread(PCompletionRoutineIOObject pSocket);
	void DestroyAllSocket();
	int WorkProcess();
	// 申请和释放缓冲区对象
	PCompletionPort_BufObj AllocateBuffer(PCompletionPortIOObject pSocket,int nLen);
	void ReleaseBuffer(PCompletionPort_BufObj pBuffer);
	void CloseAllConnections();


	// 释放空闲缓冲区对象列表和空闲上下文对象列表
	void FreeBuffers();


	// 向连接列表中添加一个连接
	BOOL AddAConnection(PCompletionPortIOObject pContext);

	// 插入和移除未决的接受请求
	BOOL InsertPendingAccept(PCompletionPort_BufObj pBuffer);
	BOOL RemovePendingAccept(PCompletionPort_BufObj pBuffer);

	// 取得下一个要读取的
	PCompletionPort_BufObj GetNextReadBuffer(PCompletionPortIOObject pContext, PCompletionPort_BufObj pBuffer);


	// 投递接受I/O、发送I/O、接收I/O
	BOOL PostAccept(PCompletionPort_BufObj pBuffer);
	//BOOL PostSend(PCompletionPortIOObject pContext, PCompletionPort_BufObj pBuffer);
	//BOOL PostRecv(PCompletionPortIOObject pContext, PCompletionPort_BufObj pBuffer);
	BOOL PostSend(PCompletionPort_BufObj pBuffer);
	BOOL PostRecv(PCompletionPort_BufObj pBuffer);
	void HandleIO(DWORD dwKey, PCompletionPort_BufObj pBuffer, DWORD dwTrans, int nError);



	void CheckPostAcceptTimeOut();

	void DisconnectActiveSocket(PCompletionPortIOObject pContext);

	void setSocketNoDelay(PCompletionPortIOObject pContext);

public:
	SOCKET m_ListenSock;
	//-------------------------------------------------------
	HANDLE m_hCompletionPort;			// 完成端口句柄
	LPFN_ACCEPTEX m_lpfnAcceptEx;	// AcceptEx函数地址
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockaddrs; // GetAcceptExSockaddrs函数地址
	// 用于投递Accept请求
	HANDLE m_hAcceptEvent;
	HANDLE m_hRepostEvent;
	LONG m_nRepostCount;
	long m_nPendingAcceptCount;

	int m_nFreeBufferCount;
	int m_nFreeContextCount;	

	int m_nInitialAccepts;
	int m_nInitialReads;
	int m_nMaxAccepts;
	int m_nMaxSends;
	int m_nMaxFreeBuffers;
	int m_nMaxFreeContexts;
	int m_nMaxConnections;
	//-------------------------------------------------------
	WORD m_ListenPort;
	DWORD m_Address;

	vector<MightyThread *> m_pThreadPool;
	CompletionPortExecute * m_pAcceptThread;
	int m_tdPoolIndex;
	ITcpServerNotify * m_pNotifyCallBack;
	int m_exitNotify;

	list<PCompletionPort_BufObj> m_postAcceptBufList;
	list<PCompletionPort_BufObj> m_pPendingAcceptList;
	PCompletionPortIOObject m_pListenObj;

	int nActiveSocketCount;
	map<string,PCompletionPortIOObject> m_ActiveSocketMap;
};
#endif
