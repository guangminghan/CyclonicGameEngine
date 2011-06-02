#include ".\mightytcpeventselectclient.h"
#include "../CommonInc/P2PUtilTools.h"
HandleIOEventSelectClient::HandleIOEventSelectClient(void)
{

}
HandleIOEventSelectClient::~HandleIOEventSelectClient(void)
{
	exit();
}

int HandleIOEventSelectClient::threadrun(void * pBuf)
{
	MightyTCPEventSelectClient * ptrUser=(MightyTCPEventSelectClient *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;

	while(1)
	{
		DWORD Index = WaitForMultipleObjects(2,MightyEvent,FALSE,INFINITE);//m_dwTimeOut);	
		switch(Index) 
		{
		case WAIT_OBJECT_0 + 0:
		case WAIT_ABANDONED_0:
		case WAIT_ABANDONED_0+1:
			{
				_endthreadex(0);
				return 0;
			}
		case WAIT_TIMEOUT:
		case WAIT_OBJECT_0 + 1:
			{
				ptrUser->Process();
				break;
			}
		default:
			break;
		}
	}

}
int HandleIOEventSelectClient::ExecuteTask(void * pBuf)
{
	return 1;
}
int HandleIOEventSelectClient::branchEvent(void * pBuf)
{
	return 1;
}
//------------------------------------------------------------------
MightyTCPEventSelectClient::MightyTCPEventSelectClient(ITcpClientNotify* pNotify)
:m_pHandleIOProcess(NULL)
,m_pNotifyCallBack(pNotify)
{
}

MightyTCPEventSelectClient::~MightyTCPEventSelectClient(void)
{
	Disconnect();
	//DestroySocket();
	if(m_pHandleIOProcess)
	{
		delete m_pHandleIOProcess;
	}
}
bool MightyTCPEventSelectClient::ConnectServer( const char* pServerAddr, unsigned short usPort)
{
	init(pServerAddr,usPort);
	return true;
}
bool MightyTCPEventSelectClient::SendData( const char* pData, int nLen)
{
	return true;
}
void MightyTCPEventSelectClient::Disconnect(void)
{
	m_bRun=FALSE;
	WSASetEvent(m_EventArray[0]);
}
void MightyTCPEventSelectClient::ReleaseConnection()
{

}
int MightyTCPEventSelectClient::init(const char * ip,WORD port)
{
	m_strAddress=ip;
	m_ConnectPort=port;
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr = inet_addr(ip);
	m_SockAddr.sin_port = htons(port);
	m_dwAddress=inet_addr(ip);
	m_bRun=TRUE;
	if(NULL==m_pHandleIOProcess)
	{
		m_pHandleIOProcess=new HandleIOEventSelectClient();
		m_pHandleIOProcess->start(this);
	}
	m_pHandleIOProcess->setExecuteSignal();
	return 0;

}
int MightyTCPEventSelectClient::init(DWORD ip,WORD port)
{
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr =htonl(ip);
	m_SockAddr.sin_port = htons(port);
	char chip[40]={0};

	strcpy(chip,P2PUtilTools::IpToString(ip));
	m_strAddress=chip;
	m_dwAddress=ip;
	m_ConnectPort=port;
	m_bRun=TRUE;
	if(NULL==m_pHandleIOProcess)
	{
		m_pHandleIOProcess=new HandleIOEventSelectClient();
		m_pHandleIOProcess->start(this);

	}
	m_pHandleIOProcess->setExecuteSignal();
	return 0;
}

int MightyTCPEventSelectClient::afreshConnect()
{
	if (m_ClientSock==INVALID_SOCKET)
	{
		m_ClientSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		m_EventArray[0]=WSACreateEvent();
		WSAEventSelect(m_ClientSock,m_EventArray[0],FD_READ|FD_WRITE|FD_CLOSE|FD_ACCEPT|FD_CONNECT|FD_OOB);
		while(m_bRun)
		{
			if(SOCKET_ERROR==connect(m_ClientSock,(SOCKADDR*)(&m_SockAddr),sizeof(SOCKADDR_IN)))
			{
				Sleep(50);
				continue;
			}
			else
			{
				break;
			}
		}
	}

	
	return 0;

}

int MightyTCPEventSelectClient::Process()
{
	DWORD Index;
	if (m_ClientSock==INVALID_SOCKET)
	{
		afreshConnect();
	}
	while(m_bRun)
	{
		Index = WSAWaitForMultipleEvents(1,m_EventArray,FALSE,WSA_INFINITE,FALSE);
		if ((Index==WSA_WAIT_FAILED)||(Index==WSA_WAIT_TIMEOUT))
		{
			//continue;
			::CloseHandle(m_EventArray[0]);
			DestroySocket();
			break;
		}
		else
		{
			if(handleIO(m_ClientSock,m_EventArray[0]))
			{

				//WSASetEvent(m_EventArray[0]);
				::CloseHandle(m_EventArray[0]);
				DestroySocket();
				m_pNotifyCallBack->OnDisconnected();
				//afreshConnect();
				continue;
			}
		}
	}
	return 0;
}
int MightyTCPEventSelectClient::handleIO(SOCKET sct,HANDLE event)
{
	WSANETWORKEVENTS nevents;
	WSAEnumNetworkEvents(sct,event,&nevents);
	int len;
	char buf[10240];


	/*if(nevents.lNetworkEvents&FD_CONNECT)
	{
	if (nevents.iErrorCode[FD_CONNECT_BIT] != 0)
	{
	return 1;
	}
	return 0;
	}*/
	if (nevents.lNetworkEvents&FD_READ)
	{

		if (nevents.iErrorCode[FD_READ_BIT] != 0)
		{
			return 1;
		}
		else if((len=recv(sct,buf,10240,0)) == SOCKET_ERROR)
		{
			//closesocket(sct);
			//DestroySocket();
			return 1;
		}
		else
		{

			m_pNotifyCallBack->OnReceivedData(buf,len);
		}
	}
	if (nevents.lNetworkEvents&FD_WRITE)
	{
		if (nevents.iErrorCode[FD_WRITE_BIT] != 0)
		{
			return 1;
		}
	}
	if (nevents.lNetworkEvents & FD_CLOSE)
	{
		/*if (nevents.iErrorCode[FD_CLOSE_BIT]!= 0)
		{
		closesocket(sct);
		return 1;
		}
		else
		{
		closesocket(sct);
		return 1;
		}*/
		//closesocket(sct);
		//DestroySocket();
		return 1;
	}
}
void MightyTCPEventSelectClient::DestroySocket()
{
	closesocket(m_ClientSock);
	m_ClientSock=INVALID_SOCKET;
}