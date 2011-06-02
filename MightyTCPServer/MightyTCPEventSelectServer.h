#ifndef MIGHTY_TCP_EVENT_SELECT_SERVER_INCLUDE_H
#define MIGHTY_TCP_EVENT_SELECT_SERVER_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "BaseMightyServerInterface.h"
typedef struct _SOCKET_OBJ
{
	SOCKET s;					// 套节字句柄
	HANDLE event;				// 与此套节字相关联的事件对象句柄
	sockaddr_in addrRemote;		// 客户端地址信息
	string identityIDKey;
}SOCKET_OBJ, *PSOCKET_OBJ;

class HandleIOProcessTCPServer: public MightyThread
{
public:
	HandleIOProcessTCPServer(void);
	virtual ~HandleIOProcessTCPServer(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

	PSOCKET_OBJ findSocket(const HANDLE & Hdsct);

	void RebuildEventArray();


	void RemoveSocketObj(PSOCKET_OBJ pSocket);

	int InsertSocketObj(PSOCKET_OBJ pSocket);

	BOOL HandleIO(PSOCKET_OBJ pSocket);

	void setNotifyEvent();

	void DestroyAllSocket();
	PSOCKET_OBJ SearchSocketObj(const string & keyId);

public:
	HANDLE events[WSA_MAXIMUM_WAIT_EVENTS];	// 记录当前线程要等待的事件对象的句柄
	int nSocketCount;
	list<PSOCKET_OBJ> m_PerThreadSockList;
	map<string,PSOCKET_OBJ> m_SocketMap;
	int m_exitCode;
};

class TCPServerExecute : public MightyThread
{
public:
	TCPServerExecute(void);
	virtual ~TCPServerExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
};
class MightyTCPEventSelectServer :public ITcpServer
{
public:
	MightyTCPEventSelectServer(ITcpServerNotify * pNotify);
	virtual ~MightyTCPEventSelectServer(void);
public:
	int init(const char * ip,WORD port);
	//int init(DWORD ip,WORD port);
	int acceptSocketObject();
	PSOCKET_OBJ GetSocketObj(SOCKET s);
	void AssignToFreeThread(PSOCKET_OBJ pSocket);
	void startWork();
public:
	virtual bool Create(const unsigned short & usPort);
	virtual void Destroy(void);
	virtual bool SendData(const char *pData, int nLen, const string & strLinkNo);
	virtual bool CloseLink(const string & strLinkNo);
//private:
	//WSAEVENT m_wsaEvent[WSA_EVENT_MAX_THREAD_COUNT][WSA_MAXIMUM_WAIT_EVENTS];
public:
	int m_currentThreadIndex;
	int m_currentEventIndex;
	SOCKET m_ListenSock;
	WORD m_ListenPort;
	DWORD m_Address;
	WSAEVENT m_ListenEvent;
	vector<MightyThread *> m_pThreadPool;
	TCPServerExecute * m_pAcceptThread;
	int m_tdPoolIndex;
	ITcpServerNotify * m_pNotifyCallBack;
};
#endif