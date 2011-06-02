#ifndef MIGHTY_TCP_ASYNC_SELECT_SERVER_INCLUDE_H
#define MIGHTY_TCP_ASYNC_SELECT_SERVER_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "BaseMightyServerInterface.h"
#define PER_WND_MAX_SOCKET 1000

#define WM_SOCKETEX_TRIGGER		(WM_USER + 0x101 + 0)				// 0x0501
#define WM_SOCKETEX_GETHOST		(WM_USER + 0x101 + 1)				// 0x0502
#define WM_SOCKETEX_NOTIFY		(WM_USER + 0x101 + 2)				// 0x0503
#define MAX_SOCKETS				(0xBFFF - WM_SOCKETEX_NOTIFY + 1)	// 0xBAFD 47869d
typedef struct _AsyncSelectIOObject
{
	SOCKET s;
	sockaddr_in addrRemote;
	string identityIDKey;
}AsyncSelectIOObject,*PAsyncSelectIOObject;
class HandleIOAsyncSelectServer
{
public:
	HandleIOAsyncSelectServer(void);
	virtual ~HandleIOAsyncSelectServer(void);
public:
	void Init(void * pCallObj);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	PAsyncSelectIOObject findSocket(const SOCKET & sct);

	

	//void SetSelectHandle(SOCKET sct);
	//void RemoveSelectHandle(SOCKET sct);
	void RemoveSocketObj(PAsyncSelectIOObject pSocket);

	int InsertSocketObj(PAsyncSelectIOObject pSocket);
	int InsertListenSocketObj(PAsyncSelectIOObject pSocket);
	int AsyncSelect(SOCKET hSocket,long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/);
	//int WorkProcess();
	//void DestroyAllSocket();
	PAsyncSelectIOObject SearchSocketObj(const string & keyId);
	//HWND GetHwnd();
public:
	HWND m_hWnd;
	int nSocketCount;
	map<string,PAsyncSelectIOObject> m_SocketMap;
	int m_exitCode;
	string m_guid_wndClassName;
	void * m_pControlHandle;
};
class MightyTCPAsyncSelectServer : public ITcpServer
{
public:
	MightyTCPAsyncSelectServer(ITcpServerNotify * pNotify);
	virtual ~MightyTCPAsyncSelectServer(void);

public:
	virtual bool Create(const unsigned short & usPort);
	virtual void Destroy(void);
	virtual bool SendData(const char *pData, int nLen, const string & strLinkNo);
	virtual bool CloseLink(const string & strLinkNo);
public:
	PAsyncSelectIOObject GetSocketObj(SOCKET s);
	void AssignToFreeThread(PAsyncSelectIOObject pSocket);
	void AssignListenSocketToFreeThread(PAsyncSelectIOObject pSocket);
	int IOCtl(SOCKET & hSocket,long lCommand, DWORD* lpArgument);
public:
	SOCKET m_ListenSock;
	WORD m_ListenPort;
	DWORD m_Address;

	vector<HandleIOAsyncSelectServer *> m_pThreadPool;
	int m_tdPoolIndex;
	ITcpServerNotify * m_pNotifyCallBack;
};
#endif