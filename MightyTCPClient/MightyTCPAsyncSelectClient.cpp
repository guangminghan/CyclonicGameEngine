#include ".\mightytcpasyncselectclient.h"
#include "../CommonInc/P2PUtilTools.h"
HandleIOAsyncSelectClient::HandleIOAsyncSelectClient(void)
{
	//Create window
	//GUID guid_val;
	m_guid_wndClassName=P2PUtilTools::GetStringUuid();
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof wndclass;
	wndclass.style = 0;
	wndclass.lpfnWndProc = WindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(0);
	wndclass.hIcon = 0;
	wndclass.hCursor = 0;
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = _T(m_guid_wndClassName.c_str());
	wndclass.hIconSm = 0;
	RegisterClassEx(&wndclass);

	m_hWnd = CreateWindow(_T(m_guid_wndClassName.c_str()), _T(m_guid_wndClassName.c_str()), 0, 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(0));

	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
}
HandleIOAsyncSelectClient::~HandleIOAsyncSelectClient(void)
{
	//Destroy window
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}
void HandleIOAsyncSelectClient::Init(void * pCallObj)
{
	m_pControlHandle=pCallObj;
}
LRESULT CALLBACK HandleIOAsyncSelectClient::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HandleIOAsyncSelectClient *pWnd = (HandleIOAsyncSelectClient *)GetWindowLong(hWnd, GWL_USERDATA);
	MightyTCPAsyncSelectClient * pSocket=(MightyTCPAsyncSelectClient *)pWnd->m_pControlHandle;
	int nEvent = lParam & 0xFFFF;
	int nErrorCode = lParam >> 16;
	SOCKET hSocket = wParam;
	switch (message)
	{
	case WM_SOCKETEX_NOTIFY :
		{
			switch (nEvent)
			{
			case FD_READ:
				{
					DWORD nBytes;
					if (pSocket->IOCtl(hSocket,FIONREAD, &nBytes)==SOCKET_ERROR)
						nErrorCode = WSAGetLastError();
					if (nBytes != 0 || nErrorCode != 0)
					{
						char tcpPackage[4096]={0};

						int nRecv = ::recv(hSocket,tcpPackage,4096,0);
						
						if(nRecv > 0)
						{

							pSocket->m_pNotifyCallBack->OnReceivedData(tcpPackage,nRecv);
						}
						else
						{
							pSocket->DestroySocket();
							pSocket->afreshConnect();
						}
					}
					break;
				}
			case FD_WRITE:
				break;
			case FD_CONNECT:
				pSocket->m_pNotifyCallBack->OnConnected();
				break;
			case FD_ACCEPT:
				{
					break;
				}
			case FD_CLOSE:
				{
					pSocket->DestroySocket();
					pSocket->afreshConnect();
					break;
				}
			}
		}
	case WM_DESTROY:
		::PostQuitMessage(0) ;
		return 0 ;

	}
	return ::DefWindowProc(hWnd, message, wParam, lParam);

}
int HandleIOAsyncSelectClient::AsyncSelect(SOCKET hSocket,long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/)
{
	if (lEvent==0)
	{
		if (WSAAsyncSelect(hSocket,m_hWnd,WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE)) 
		{
			return -1;
		}
	}
	else
	{
		if (!WSAAsyncSelect(hSocket,m_hWnd,WM_SOCKETEX_NOTIFY, lEvent))
		{
			return -1;
		}
	}
	return 0;
}

//------------------------------------------------------------------
MightyTCPAsyncSelectClient::MightyTCPAsyncSelectClient(ITcpClientNotify* pNotify)
:m_pNotifyCallBack(pNotify)
{
}

MightyTCPAsyncSelectClient::~MightyTCPAsyncSelectClient(void)
{
}
bool MightyTCPAsyncSelectClient::ConnectServer( const char* pServerAddr, unsigned short usPort)
{
	return true;
}
bool MightyTCPAsyncSelectClient::SendData( const char* pData, int nLen)
{
		return true;
}
void MightyTCPAsyncSelectClient::Disconnect(void)
{

}
void MightyTCPAsyncSelectClient::ReleaseConnection()
{

}
int MightyTCPAsyncSelectClient::IOCtl(SOCKET & hSocket,long lCommand, DWORD* lpArgument)
{
	return ioctlsocket(hSocket, lCommand, lpArgument);
}
int MightyTCPAsyncSelectClient::init(const char * ip,WORD port)
{
	m_strAddress=ip;
	m_ConnectPort=port;
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr = inet_addr(ip);
	m_SockAddr.sin_port = htons(port);
	m_dwAddress=inet_addr(ip);

	return 0;

}
int MightyTCPAsyncSelectClient::init(DWORD ip,WORD port)
{
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr =htonl(ip);
	m_SockAddr.sin_port = htons(port);
	char chip[40]={0};

	strcpy(chip,P2PUtilTools::IpToString(ip));
	m_strAddress=chip;
	m_dwAddress=ip;
	m_ConnectPort=port;

	return 0;
}
int MightyTCPAsyncSelectClient::AsyncSelect(long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/)
{
	//m_lEvent = lEvent;

	if (!WSAAsyncSelect(m_ClientSock, m_pHandleIOProcess->m_hWnd, WM_SOCKETEX_NOTIFY, lEvent))
		return 0;
	else
		return 1;
}
int MightyTCPAsyncSelectClient::Connect(const char * lpszHostAddress, unsigned short nHostPort)
{

		SOCKADDR_IN sockAddr = {0};
		sockAddr.sin_addr.s_addr = inet_addr(lpszHostAddress);
		if (sockAddr.sin_addr.s_addr == INADDR_NONE)
		{
			//m_pAsyncGetHostByNameBuffer = new char[MAXGETHOSTSTRUCT];
			//m_nAsyncGetHostByNamePort = nHostPort;
			//m_hAsyncGetHostByNameHandle = WSAAsyncGetHostByName(GetHelperWindowHandle(), WM_SOCKETEX_GETHOST, lpszHostAddress, m_pAsyncGetHostByNameBuffer, MAXGETHOSTSTRUCT);
			//if (!m_hAsyncGetHostByNameHandle)
				//return FALSE;
			//WSASetLastError(WSAEWOULDBLOCK);
			return TRUE;
		}
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons((u_short)nHostPort);
		return MightyTCPAsyncSelectClient::Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
}

int MightyTCPAsyncSelectClient::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	return connect(m_ClientSock, lpSockAddr, nSockAddrLen) != SOCKET_ERROR;
}
int MightyTCPAsyncSelectClient::afreshConnect()
{
	if (m_ClientSock==INVALID_SOCKET)
	{
		m_ClientSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		m_lEvent=FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE;
		AsyncSelect(m_lEvent);
		Connect(m_strAddress.c_str(),m_ConnectPort);
		return 0;
	}
	else
	{
		return 1;
	}
}
int MightyTCPAsyncSelectClient::Process()
{
	return 0;
}
void MightyTCPAsyncSelectClient::DestroySocket()
{
	closesocket(m_ClientSock);
	m_ClientSock=INVALID_SOCKET;
}
