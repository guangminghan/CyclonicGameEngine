#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "basemightyclientinterface.h"
#define PER_WND_MAX_SOCKET 1000

#define WM_SOCKETEX_TRIGGER		(WM_USER + 0x101 + 0)				// 0x0501
#define WM_SOCKETEX_GETHOST		(WM_USER + 0x101 + 1)				// 0x0502
#define WM_SOCKETEX_NOTIFY		(WM_USER + 0x101 + 2)				// 0x0503
#define MAX_SOCKETS				(0xBFFF - WM_SOCKETEX_NOTIFY + 1)	// 0xBAFD 47869d
class HandleIOAsyncSelectClient
{
public:
	HandleIOAsyncSelectClient(void);
	virtual ~HandleIOAsyncSelectClient(void);
public:
	void Init(void * pCallObj);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	int AsyncSelect(SOCKET hSocket,long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/);
public:
	HWND m_hWnd;
	
	int m_exitCode;
	string m_guid_wndClassName;
	void * m_pControlHandle;
};
class MightyTCPAsyncSelectClient :
	public ITcpClient
{
public:
	MightyTCPAsyncSelectClient(ITcpClientNotify* pNotify);
	virtual ~MightyTCPAsyncSelectClient(void);
public:
	virtual bool ConnectServer( const char* pServerAddr, unsigned short usPort);
	virtual bool SendData( const char* pData, int nLen);
	virtual void Disconnect(void);
	virtual void ReleaseConnection();

	int IOCtl(SOCKET & hSocket,long lCommand, DWORD* lpArgument);
	int init(const char * ip,WORD port);
	int init(DWORD ip,WORD port);
	int AsyncSelect(long lEvent = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
	int Connect(const char * lpszHostAddress, unsigned short nHostPort);
	int Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	//int ConnectRemote(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port);
	//int asyncConnectRemote(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port, int msecWait);
	//int asyncConnectRemoteEx(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port, int msecWait);
	//bool waitSocketRead(SOCKET s, DWORD waitTime/*∫¡√Î*/);
	int afreshConnect();
	int Process();
	void DestroySocket();

public:
	ITcpClientNotify * m_pNotifyCallBack;
	SOCKADDR_IN m_SockAddr;
	//If using layers, only the events specified with m_lEvent will send to the event handlers.
	long m_lEvent;
	SOCKET m_ClientSock;
	WORD m_ConnectPort;
	DWORD m_dwAddress;
	string m_strAddress;

	HandleIOAsyncSelectClient * m_pHandleIOProcess;
};
