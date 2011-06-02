#include ".\mightytcpselectclient.h"
#include "../CommonInc/P2PUtilTools.h"
HandleIOSelectClient::HandleIOSelectClient(void)
{

}
HandleIOSelectClient::~HandleIOSelectClient(void)
{
	exit();
}

int HandleIOSelectClient::threadrun(void * pBuf)
{
	MightyTCPSelectClient * ptrUser=(MightyTCPSelectClient *)pBuf;

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
int HandleIOSelectClient::ExecuteTask(void * pBuf)
{
	return 1;
}
int HandleIOSelectClient::branchEvent(void * pBuf)
{
	return 1;
}
//////////////////////////////////////////////////////////////////////
MightyTCPSelectClient::MightyTCPSelectClient(ITcpClientNotify* pNotify)
:m_pNotifyCallBack(pNotify)
{
}

MightyTCPSelectClient::~MightyTCPSelectClient(void)
{
}
bool MightyTCPSelectClient::ConnectServer( const char* pServerAddr, unsigned short usPort)
{
	init(pServerAddr,usPort);
	return true;
}
bool MightyTCPSelectClient::SendData( const char* pData, int nLen)
{
	return true;
}
void MightyTCPSelectClient::Disconnect(void)
{

}
void MightyTCPSelectClient::ReleaseConnection()
{

}

int MightyTCPSelectClient::init(const char * ip,WORD port)
{
	m_strAddress=ip;
	m_ConnectPort=port;
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr = inet_addr(ip);
	m_SockAddr.sin_port = htons(port);
	m_dwAddress=inet_addr(ip);

	return 0;

}
int MightyTCPSelectClient::init(DWORD ip,WORD port)
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
int MightyTCPSelectClient::ConnectRemote(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port)
{

	SOCKADDR_IN remoteAddr = {0};

	remoteAddr.sin_addr.S_un.S_addr = htonl(Ip);

	remoteAddr.sin_family = AF_INET;

	remoteAddr.sin_port = htons(Port);

	if ( localPort == 0 ) {

		if ( connect(sRemote, (sockaddr*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR ) {

			return 0;
		}

		return 1;

	}

	SOCKADDR_IN localAddr = {0};

	localAddr.sin_family = AF_INET;

	localAddr.sin_port = htons(localPort);

	if ( bind(sRemote, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR ) {

		printf("\nreBind Error! %d", GetLastError());

		return -1;

	}

	

	if ( connect(sRemote, (sockaddr*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR ) {

		return 0;

	}

	return 1;

}
int MightyTCPSelectClient::asyncConnectRemote(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port, int msecWait)
{

	SOCKADDR_IN remoteAddr = {0};

	remoteAddr.sin_addr.S_un.S_addr = htonl(Ip);

	remoteAddr.sin_family = AF_INET;

	remoteAddr.sin_port = htons(Port);

	if ( localPort == 0 ) {

		if ( connect(sRemote, (sockaddr*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR ) {

			return 0;
		}

		return 1;

	}

	SOCKADDR_IN localAddr = {0};

	localAddr.sin_family = AF_INET;

	localAddr.sin_port = htons(localPort);

	if ( bind(sRemote, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR ) {

		printf("\nreBind Error!");

		return -1;

	}

	unsigned long lArgp = 1;

	ioctlsocket(sRemote, FIONBIO, &lArgp);

	if ( connect(sRemote, (sockaddr*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR ) {

		if ( GetLastError() == 10035 ) {

			fd_set writeSet; 
			FD_ZERO(&writeSet);
			FD_SET(sRemote, &writeSet);

			timeval	timeout;
			timeout.tv_sec	= msecWait / 1000;
			timeout.tv_usec	= 1000 * (msecWait % 1000);

			int	rCode = select(0,NULL,&writeSet,NULL,&timeout);

			if ( rCode == 0 ) {

				return 0;

			}

			if (rCode == SOCKET_ERROR) 
			{

				return 0;

			}

			return 1;

		}

	}

	return 1;
}
int MightyTCPSelectClient::asyncConnectRemoteEx(SOCKET sRemote, WORD localPort, DWORD Ip, WORD Port, int msecWait)
{
	//readfds
	//1.有未决的连接请求2.数据可读3连接关闭重启中断
	//writefds
	//1.连接成功(如果一个非阻塞连接调用正在被处理那么连接已经成功)2.数据可写
	//exceptfds
	//1.连接失败(如果一个非阻塞连接调用正在被处理那么连接试图失败)2.OOB数据可读
	SOCKADDR_IN remoteAddr = {0};

	remoteAddr.sin_addr.S_un.S_addr = htonl(Ip);

	remoteAddr.sin_family = AF_INET;

	remoteAddr.sin_port = htons(Port);

	if ( localPort == 0 ) {

		if ( connect(sRemote, (sockaddr*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR ) {

			return 0;
		}

		return 1;

	}

	SOCKADDR_IN localAddr = {0};

	localAddr.sin_family = AF_INET;

	localAddr.sin_port = htons(localPort);

	if ( bind(sRemote, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR ) {

		printf("\nreBind Error!");

		return -1;

	}

	unsigned long lArgp = 1;

	ioctlsocket(sRemote, FIONBIO, &lArgp);

	if ( connect(sRemote, (sockaddr*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR ) {

		if ( GetLastError() == 10035 ) {

			fd_set writeSet;
			fd_set exceptSet;
			FD_ZERO(&writeSet);
			FD_SET(sRemote, &writeSet);
			FD_ZERO(&exceptSet);
			FD_SET(sRemote, &exceptSet);
			timeval	timeout;
			timeout.tv_sec	= msecWait / 1000;
			timeout.tv_usec	= 1000 * (msecWait % 1000);

			int	rCode = select(0,NULL,&writeSet,&exceptSet,&timeout);

			if ( rCode == 0 ) 
			{
				if (FD_ISSET(sRemote,&writeSet))
				{
					FD_CLR(sRemote,&writeSet);
					return 0;
				}
			}

			if (rCode == SOCKET_ERROR) 
			{

				return 0;

			}
			if (FD_ISSET(sRemote,&exceptSet))
			{
				FD_CLR(sRemote,&exceptSet);
				return 1;
			}
			
			return 1;

		}

	}

	return 1;
}
bool MightyTCPSelectClient::waitSocketRead(SOCKET s, DWORD waitTime/*毫秒*/)
{

	DWORD enterTime = GetTickCount();

	for (;;) {

		DWORD loseTime = GetTickCount() - enterTime;

		if ( loseTime >= waitTime ) break;

		DWORD leaveTime = waitTime - loseTime;

		fd_set	readfds; FD_ZERO(&readfds);

		FD_SET(s, &readfds);

		timeval	timeout;

		timeout.tv_sec	= leaveTime / 1000;

		timeout.tv_usec	= (leaveTime % 1000) * 1000;//微秒

		int	rCode = ::select(0,&readfds,NULL,NULL,&timeout);

		if ( (rCode == 0) ) return false;

		return true;

	}

	return false;
}
int MightyTCPSelectClient::afreshConnect()
{
	if (m_ClientSock==INVALID_SOCKET)
	{
		m_ClientSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		asyncConnectRemote(m_ClientSock,0,m_dwAddress,m_ConnectPort,40000);
		return 0;
	}
	else
	{
		return 1;
	}
	
}

int MightyTCPSelectClient::Process()
{
	afreshConnect();

	/*while ( ! *pEndState ) {

		MSG message = {0};

		if ( ! ::PeekMessage(&message, (HWND)INVALID_HANDLE_VALUE, MESSAGE_PEER_CONNECT - 1, MESSAGE_PEER_CONNECT + 1, PM_REMOVE) ) {

			Sleep(10); continue;
		}
		if(message.message==WM_QUIT) break;
		if(message.hwnd!=NULL) {
			TranslateMessage(&message);
			DispatchMessage(&message);
			continue;
		}

		tcpHandMessage(message.message, message.wParam, message.lParam);

	}*/
	while (true) 
	{

		/*DWORD currTime = GetTickCount();

		if ( (currTime - startTime) > timeOut ) {

			bError = true; break;

		}*/

		if ( ! waitSocketRead(m_ClientSock, 10) ) 
			continue;

		char buffer[10240];

		int nRecved = recv(m_ClientSock, buffer, sizeof(buffer), 0);

		if ( nRecved == 0 || nRecved == SOCKET_ERROR ) {

			//bError = true; 
			closesocket(m_ClientSock);
			m_ClientSock=m_ClientSock;
			break;

		}
		m_pNotifyCallBack->OnReceivedData(buffer,nRecved);


		//handRet = handRecvData(pDataBuffer, buffer, nRecved, handProc, outBuffer, outLen);

	}

	
	return 0;
}