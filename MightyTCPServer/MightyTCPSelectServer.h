#ifndef MIGHTY_TCP_SELECT_SERVER_INCLUDE_H
#define MIGHTY_TCP_SELECT_SERVER_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "BaseMightyServerInterface.h"
typedef struct _SelectIOObject
{
	SOCKET s;
	sockaddr_in addrRemote;
	string identityIDKey;
}SelectIOObject,*PSelectIOObject;
class HandleIOSelectServer: public MightyThread
{
public:
	HandleIOSelectServer(void);
	virtual ~HandleIOSelectServer(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

	PSelectIOObject findSocket(const SOCKET & sct);

	void RebuildEventArray();

	void SetSelectHandle(SOCKET sct);
	void RemoveSelectHandle(SOCKET sct);
	void RemoveSocketObj(PSelectIOObject pSocket);

	int InsertSocketObj(PSelectIOObject pSocket);
	int WorkProcess();
	//BOOL HandleIO(PSOCKET_OBJ pSocket);

	//void setNotifyEvent();

	void DestroyAllSocket();
	PSelectIOObject SearchSocketObj(const string & keyId);

public:
	fd_set fdClientRead;//FD_SETSIZE
	fd_set fdClientWrite;
	fd_set fdClientExcept;
	int nSocketCount;
	map<string,PSelectIOObject> m_SocketMap;
	int m_exitCode;
};

class SelectServerExecute : public MightyThread
{
public:
	SelectServerExecute(void);
	virtual ~SelectServerExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

};
class MightyTCPSelectServer : public ITcpServer
{
public:
	MightyTCPSelectServer(ITcpServerNotify * pNotify);
	virtual ~MightyTCPSelectServer(void);
public:
	virtual bool Create(const unsigned short & usPort);
	virtual void Destroy(void);
	virtual bool SendData(const char *pData, int nLen, const string & strLinkNo);
	virtual bool CloseLink(const string & strLinkNo);
public:
	int acceptSocketObject();
	PSelectIOObject GetSocketObj(SOCKET s);
	void AssignToFreeThread(PSelectIOObject pSocket);
	void startWork();
public:
	fd_set fdServer;
	SOCKET m_ListenSock;
	WORD m_ListenPort;
	DWORD m_Address;

	vector<MightyThread *> m_pThreadPool;
	SelectServerExecute * m_pAcceptThread;
	int m_tdPoolIndex;
	ITcpServerNotify * m_pNotifyCallBack;
};
#endif