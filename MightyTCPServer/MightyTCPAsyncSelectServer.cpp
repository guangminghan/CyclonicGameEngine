#include ".\mightytcpasyncselectserver.h"
#include "../CommonInc/P2PUtilTools.h"
HandleIOAsyncSelectServer::HandleIOAsyncSelectServer(void)
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
HandleIOAsyncSelectServer::~HandleIOAsyncSelectServer(void)
{
	//Destroy window
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}
void HandleIOAsyncSelectServer::Init(void * pCallObj)
{
	m_pControlHandle=pCallObj;
}
LRESULT CALLBACK HandleIOAsyncSelectServer::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HandleIOAsyncSelectServer *pWnd = (HandleIOAsyncSelectServer *)GetWindowLong(hWnd, GWL_USERDATA);
	MightyTCPAsyncSelectServer * pSocket=(MightyTCPAsyncSelectServer *)pWnd->m_pControlHandle;
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
						PAsyncSelectIOObject ptrObj_=pWnd->findSocket(hSocket);
						if(nRecv > 0)
						{

							pSocket->m_pNotifyCallBack->OnRecvData(tcpPackage,nRecv,ptrObj_->identityIDKey);
						}
						else
						{
							pWnd->RemoveSocketObj(ptrObj_);
						}
					}
					break;
				}
				case FD_WRITE:
					break;
				case FD_CONNECT:
					break;
				case FD_ACCEPT:
				{
					struct sockaddr_in adrFrom;
					int iAddrLen = sizeof( adrFrom );
					SOCKET client = ::accept(hSocket, (struct sockaddr *)&adrFrom, &iAddrLen);
					::WSAAsyncSelect(client, hWnd, WM_SOCKETEX_NOTIFY, FD_READ|FD_WRITE|FD_CLOSE);
					PAsyncSelectIOObject asyncClient=pSocket->GetSocketObj(client);
					asyncClient->addrRemote =adrFrom;
					pSocket->AssignToFreeThread(asyncClient);
					break;
				}
				case FD_CLOSE:
				{
					PAsyncSelectIOObject ptrObj_=pWnd->findSocket(hSocket);
					pWnd->RemoveSocketObj(ptrObj_);
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
PAsyncSelectIOObject HandleIOAsyncSelectServer::findSocket(const SOCKET & sct)
{
	map<string,PAsyncSelectIOObject>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();

	while (itorIndex!=itorRight)
	{

		if(itorIndex->second->s==sct)
		{
			return itorIndex->second;
		}

		itorIndex++;
	}
	return NULL;
}

void HandleIOAsyncSelectServer::RemoveSocketObj(PAsyncSelectIOObject pSocket)
{
	((MightyTCPAsyncSelectServer *)(m_pControlHandle))->m_pNotifyCallBack->OnClosed(pSocket->identityIDKey);
	map<string,PAsyncSelectIOObject>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();

	while (itorIndex!=itorRight)
	{
		if(pSocket==(itorIndex->second))
		{
			m_SocketMap.erase(itorIndex++);

			if(pSocket->s != INVALID_SOCKET)
			{
				::closesocket(pSocket->s);
			}
			
			delete pSocket;
			nSocketCount--;

			break;
		}
		itorIndex++;
	}
}

int HandleIOAsyncSelectServer::InsertSocketObj(PAsyncSelectIOObject pSocket)
{
	if (m_SocketMap.size()>=PER_WND_MAX_SOCKET)
	{
		return -1;
	}
	else
	{
		//GUID guid_val;
		string guid_strkey=P2PUtilTools::GetStringUuid();
		pSocket->identityIDKey=guid_strkey;
		AsyncSelect(pSocket->s,0);
		m_SocketMap.insert(map<string,PAsyncSelectIOObject>::value_type(guid_strkey,pSocket));
		nSocketCount++;
		
		((MightyTCPAsyncSelectServer *)(m_pControlHandle))->m_pNotifyCallBack->OnAccept(guid_strkey);
		//return nSocketCount;
		return 0;
	}
	
}
int HandleIOAsyncSelectServer::InsertListenSocketObj(PAsyncSelectIOObject pSocket)
{
	if (m_SocketMap.size()>=PER_WND_MAX_SOCKET)
	{
		return -1;
	}
	else
	{
		//GUID guid_val;
		string guid_strkey=P2PUtilTools::GetStringUuid();
		pSocket->identityIDKey=guid_strkey;
		AsyncSelect(pSocket->s,0);
		m_SocketMap.insert(map<string,PAsyncSelectIOObject>::value_type(guid_strkey,pSocket));
		nSocketCount++;
		return 0;
		

	}
}
int HandleIOAsyncSelectServer::AsyncSelect(SOCKET hSocket,long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/)
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
PAsyncSelectIOObject HandleIOAsyncSelectServer::SearchSocketObj(const string & keyId)
{
	map<string,PAsyncSelectIOObject>::iterator leftPos,rightPos;
	rightPos=m_SocketMap.end();
	if ((leftPos=m_SocketMap.find(keyId))!=rightPos)
	{
		return leftPos->second;
	}
	else
	{
		return NULL;
	}
}
//----------------------------------------------------------
MightyTCPAsyncSelectServer::MightyTCPAsyncSelectServer(ITcpServerNotify * pNotify)
:m_tdPoolIndex(0)
,m_pNotifyCallBack(pNotify)
{
	m_pThreadPool.clear();
}
MightyTCPAsyncSelectServer::~MightyTCPAsyncSelectServer(void)
{
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		delete (HandleIOAsyncSelectServer *)m_pThreadPool[tdIndex];
		Sleep(10);
	}
}
bool MightyTCPAsyncSelectServer::Create(const unsigned short & usPort)
{

	m_ListenPort=usPort;
	m_ListenSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(usPort);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if(::bind(m_ListenSock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		return false;
	}
	PAsyncSelectIOObject asyncListen=GetSocketObj(m_ListenSock);
	AssignListenSocketToFreeThread(asyncListen);

	::listen(m_ListenSock,200);

}
void MightyTCPAsyncSelectServer::Destroy(void)
{

}
bool MightyTCPAsyncSelectServer::SendData(const char *pData, int nLen, const string & strLinkNo)
{
	PAsyncSelectIOObject sendHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		sendHandle=((HandleIOAsyncSelectServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			::send(sendHandle->s,pData,nLen,0);
			return true;
		}


	}
	return false;
}
bool MightyTCPAsyncSelectServer::CloseLink(const string & strLinkNo)
{
	PAsyncSelectIOObject sendHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		sendHandle=m_pThreadPool[tdIndex]->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			m_pThreadPool[tdIndex]->RemoveSocketObj(sendHandle);
			return true;
		}
	}
	return false;
}

PAsyncSelectIOObject MightyTCPAsyncSelectServer::GetSocketObj(SOCKET s)
{
	PAsyncSelectIOObject pSocket =NULL;
	pSocket=new AsyncSelectIOObject();
	if(pSocket != NULL)
	{
		pSocket->s = s;
	}
	return pSocket;
}
void MightyTCPAsyncSelectServer::AssignToFreeThread(PAsyncSelectIOObject pSocket)
{
	if (m_pThreadPool.empty())
	{
		HandleIOAsyncSelectServer * pIOThread=new HandleIOAsyncSelectServer();
		pIOThread->Init(this);
		m_pThreadPool.push_back(pIOThread);
		
		pIOThread->InsertSocketObj(pSocket);
		
		m_tdPoolIndex=0;

	}
	else
	{
		int rtCode=0;
		rtCode=((HandleIOAsyncSelectServer *)m_pThreadPool[m_tdPoolIndex])->InsertSocketObj(pSocket);
		if (rtCode==-1)
		{
			HandleIOAsyncSelectServer * pIOThread=new HandleIOAsyncSelectServer();
			pIOThread->Init(this);
			m_pThreadPool.push_back(pIOThread);
			pIOThread->InsertSocketObj(pSocket);
			
			m_tdPoolIndex++;

		}

	}

}
void MightyTCPAsyncSelectServer::AssignListenSocketToFreeThread(PAsyncSelectIOObject pSocket)
{
	if (m_pThreadPool.empty())
	{
		HandleIOAsyncSelectServer * pIOThread=new HandleIOAsyncSelectServer();
		pIOThread->Init(this);
		m_pThreadPool.push_back(pIOThread);

		pIOThread->InsertListenSocketObj(pSocket);

		m_tdPoolIndex=0;

	}
	else
	{
		int rtCode=0;
		rtCode=((HandleIOAsyncSelectServer *)m_pThreadPool[m_tdPoolIndex])->InsertSocketObj(pSocket);
		if (rtCode==-1)
		{
			HandleIOAsyncSelectServer * pIOThread=new HandleIOAsyncSelectServer();
			pIOThread->Init(this);
			m_pThreadPool.push_back(pIOThread);
			pIOThread->InsertListenSocketObj(pSocket);

			m_tdPoolIndex++;

		}

	}
}
int MightyTCPAsyncSelectServer::IOCtl(SOCKET & hSocket,long lCommand, DWORD* lpArgument)
{
	return ioctlsocket(hSocket, lCommand, lpArgument);
}