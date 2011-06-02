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
	sockaddr_in addrLocal;			// ���ӵı��ص�ַ
	//int nOutstandingOps;			// ��¼���׽����ϵ��ص�I/O����
	int nOutstandingRecv;	// ���׽������׳��Ķ��ص�����������
	int nOutstandingSend;//���׽������׳���д�ص�����������

	ULONG nReadSequence;			// ���Ÿ����յ���һ�����к�
	ULONG nCurrentReadSequence;		// ��ǰҪ�������к�
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
	int nOperation;			// �ύ�Ĳ�������
	SOCKET sAccept;		// AcceptEx���յĿͻ����׽���
	ULONG nSequenceNumber;	// ��I/O�����к�
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
	// ������ͷŻ���������
	PCompletionPort_BufObj AllocateBuffer(PCompletionPortIOObject pSocket,int nLen);
	void ReleaseBuffer(PCompletionPort_BufObj pBuffer);
	void CloseAllConnections();


	// �ͷſ��л����������б�Ϳ��������Ķ����б�
	void FreeBuffers();


	// �������б������һ������
	BOOL AddAConnection(PCompletionPortIOObject pContext);

	// ������Ƴ�δ���Ľ�������
	BOOL InsertPendingAccept(PCompletionPort_BufObj pBuffer);
	BOOL RemovePendingAccept(PCompletionPort_BufObj pBuffer);

	// ȡ����һ��Ҫ��ȡ��
	PCompletionPort_BufObj GetNextReadBuffer(PCompletionPortIOObject pContext, PCompletionPort_BufObj pBuffer);


	// Ͷ�ݽ���I/O������I/O������I/O
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
	HANDLE m_hCompletionPort;			// ��ɶ˿ھ��
	LPFN_ACCEPTEX m_lpfnAcceptEx;	// AcceptEx������ַ
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockaddrs; // GetAcceptExSockaddrs������ַ
	// ����Ͷ��Accept����
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
