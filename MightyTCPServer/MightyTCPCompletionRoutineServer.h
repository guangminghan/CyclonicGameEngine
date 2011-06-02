#ifndef MIGHTY_TCP_COMPLETION_ROUTINE_SERVER_INCLUDE_H
#define MIGHTY_TCP_COMPLETION_ROUTINE_SERVER_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "BaseMightyServerInterface.h"
typedef struct _CompletionRoutineIOObject
{
	SOCKET s;
	sockaddr_in addrRemote;
	int nOutstandingOps;			// 记录此套节字上的重叠I/O数量
	string identityIDKey;
}CompletionRoutineIOObject,*PCompletionRoutineIOObject;

typedef struct _CompletionRoutine_BufObj
{	
	WSAOVERLAPPED wsaol;
	WSABUF wsaDataBuf;
	char *buff;
	int nLen;
	int nOperation;			// 提交的操作类型
#define OP_RECV	1
#define OP_SEND	2
	PCompletionRoutineIOObject pSocket;
	void * pAgent;
	
}CompletionRoutine_BufObj, *PCompletionRoutine_BufObj;

class HandleIOCompletionRoutineServer: public MightyThread
{
public:
	HandleIOCompletionRoutineServer(void);
	virtual ~HandleIOCompletionRoutineServer(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

	//POverlappedEventIOObject findSocket(const SOCKET & sct);

	//void RebuildEventArray();


	//int AnalyzeProcess(PCompletionRoutine_BufObj pBuffer);
	int WorkProcess();
	void RemoveSocketObj(PCompletionRoutineIOObject pSocket);

	int InsertSocketObj(PCompletionRoutineIOObject pSocket);

	//BOOL HandleIO(PSOCKET_OBJ pSocket);

	//void setNotifyEvent();

	void DestroyAllSocket();
	PCompletionRoutineIOObject SearchSocketObj(const string & keyId);

	//----------------
	PCompletionRoutine_BufObj GetBufferObj(PCompletionRoutineIOObject pSocket, ULONG nLen);
	void FreeBufferObj(PCompletionRoutine_BufObj pBuffer);
	//PCompletionRoutine_BufObj FindBufferObj(HANDLE hEvent);
	
	int PostRecv(PCompletionRoutine_BufObj pBuffer);
	int PostSend(PCompletionRoutine_BufObj pBuffer);

	int PostCompletionRoutineRequest(PCompletionRoutine_BufObj pBuffer);

public:
	int nSocketCount;
	map<string,PCompletionRoutineIOObject> m_SocketMap;
	//list<PCompletionRoutineIOObject> m_HaveNeverPostRecvList;
	list<PCompletionRoutine_BufObj> m_overLappedBufList;
	//HANDLE m_Notify_Events[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT m_Notify_WsaEvents[WSA_MAXIMUM_WAIT_EVENTS];
	int m_nEffectiveEventCount;
	int m_exitCode;
};

class CompletionRoutineExecute : public MightyThread
{
public:
	CompletionRoutineExecute(void);
	virtual ~CompletionRoutineExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
public:



};
class MightyTCPCompletionRoutineServer: public ITcpServer
{
public:
	MightyTCPCompletionRoutineServer(ITcpServerNotify * pNotify);
	virtual ~MightyTCPCompletionRoutineServer(void);

public:
	virtual bool Create(const unsigned short & usPort);
	virtual void Destroy(void);
	virtual bool SendData(const char *pData, int nLen, const string & strLinkNo);
	virtual bool CloseLink(const string & strLinkNo);
public:
	void startWork();
	int acceptSocketObject();
	PCompletionRoutineIOObject GetSocketObj(SOCKET s);
	void AssignToFreeThread(PCompletionRoutineIOObject pSocket);
public:

	SOCKET m_ListenSock;
	WORD m_ListenPort;
	DWORD m_Address;

	vector<MightyThread *> m_pThreadPool;
	CompletionRoutineExecute * m_pAcceptThread;
	int m_tdPoolIndex;
	ITcpServerNotify * m_pNotifyCallBack;
	int m_exitNotify;
};
#endif