#include ".\mightytcpselectserver.h"
#include "../CommonInc/P2PUtilTools.h"
HandleIOSelectServer::HandleIOSelectServer(void)
:nSocketCount(0)
,m_exitCode(0)
{
	m_SocketMap.clear();
	FD_ZERO( &fdClientRead );
	FD_ZERO( &fdClientWrite );
	FD_ZERO( &fdClientExcept );
}
HandleIOSelectServer:: ~HandleIOSelectServer(void)
{
	exit();
}
int HandleIOSelectServer::threadrun(void * pBuf)
{

	MightyTCPSelectServer * ptrUser=(MightyTCPSelectServer *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;

	while(1)
	{
		DWORD Index = WaitForMultipleObjects(2,MightyEvent,FALSE,m_dwTimeOut);//m_dwTimeOut);	INFINITE
		switch(Index) 
		{
		case WAIT_OBJECT_0 + 0:
		case WAIT_ABANDONED_0:
		case WAIT_ABANDONED_0+1:
			{
				((HandleIOSelectServer *)m_userInstance.threadThis_)->DestroyAllSocket();
				_endthreadex(0);
				return 0;
			}
		case WAIT_TIMEOUT:
		case WAIT_OBJECT_0 + 1:
			{
				((HandleIOSelectServer *)m_userInstance.threadThis_)->WorkProcess();
				break;
			}
		default:
			break;
		}
	}
}

int HandleIOSelectServer::ExecuteTask(void * pBuf)
{
	return 1;
}

int HandleIOSelectServer::branchEvent(void * pBuf)
{
	return 1;
}
PSelectIOObject HandleIOSelectServer::findSocket(const SOCKET & sct)
{
	map<string,PSelectIOObject>::iterator itorRight,itorIndex;
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
void HandleIOSelectServer::SetSelectHandle(SOCKET sct)
{
	FD_SET(sct, &fdClientRead);
	FD_SET(sct, &fdClientWrite);
	FD_SET(sct, &fdClientExcept);
}
void HandleIOSelectServer::RemoveSelectHandle(SOCKET sct)
{
	FD_CLR(sct, &fdClientRead);
	FD_CLR(sct, &fdClientWrite);
	FD_CLR(sct, &fdClientExcept);
}
void HandleIOSelectServer::RemoveSocketObj(PSelectIOObject pSocket)
{
	((MightyTCPSelectServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnClosed(pSocket->identityIDKey);
	map<string,PSelectIOObject>::iterator itorRight,itorIndex;
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
			RemoveSelectHandle(pSocket->s);
			delete pSocket;
			nSocketCount--;
			
			break;
		}
		itorIndex++;
	}
}
int HandleIOSelectServer::InsertSocketObj(PSelectIOObject pSocket)
{
	if (m_SocketMap.size()>=FD_SETSIZE)
	{
		return -1;
	}
	else
	{
		//GUID guid_val;
		string guid_strkey=P2PUtilTools::GetStringUuid();
		pSocket->identityIDKey=guid_strkey;
		m_SocketMap.insert(map<string,PSelectIOObject>::value_type(guid_strkey,pSocket));
		nSocketCount++;
		SetSelectHandle(pSocket->s);
		((MightyTCPSelectServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnAccept(guid_strkey);

	}
}
int HandleIOSelectServer::WorkProcess()
{
	fd_set fdTempRead = fdClientRead;
	fd_set fdTempWrite = fdClientWrite;
	fd_set fdTempExcept = fdClientExcept;


	int nRet = ::select(0, &fdTempRead, NULL, NULL, NULL);

	if(nRet > 0)
	{
		//通过将原来fdSocket集合与select处理过的fdRead集合比较，
		//确定都有哪些套节字有未决I/O,并进一步处理这些I/O
		for(int i=0; i<(int)fdClientRead.fd_count; i++)
		{
			if(FD_ISSET(fdClientRead.fd_array[i], &fdTempRead))
			{
	
					char tcpPackage[4096]={0};

					int nRecv = ::recv(fdClientRead.fd_array[i], tcpPackage, 4096, 0);
					PSelectIOObject ptrObj_=findSocket(fdClientRead.fd_array[i]);
					if(nRecv > 0)
					{
						
						((MightyTCPSelectServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnRecvData(tcpPackage,nRecv,ptrObj_->identityIDKey);
					}
					else
					{
						//::closesocket(fdClientRead.fd_array[i]);
						//FD_CLR(fdClientRead.fd_array[i], &fdClientRead);
						RemoveSocketObj(ptrObj_);
					}

			}
		}
	}
	else
	{
		return -1;
	}
}

void HandleIOSelectServer::DestroyAllSocket()
{
	map<string,PSelectIOObject>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();
	while (itorIndex!=itorRight)
	{
		if ((itorIndex->second))
		{
			if(itorIndex->second->s != INVALID_SOCKET)
			{
				::closesocket(itorIndex->second->s);
			}

			RemoveSelectHandle(itorIndex->second->s);
			delete (itorIndex->second);
		}

		m_SocketMap.erase(itorIndex++);
		//itorIndex= m_SocketMap.erase(itorIndex);
		nSocketCount--;
		//itorIndex++;
	}
}
PSelectIOObject HandleIOSelectServer::SearchSocketObj(const string & keyId)
{
	map<string,PSelectIOObject>::iterator leftPos,rightPos;
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
//-------------------------------------------
SelectServerExecute::SelectServerExecute(void)
{
}

SelectServerExecute::~SelectServerExecute(void)
{
	exit();
}
int SelectServerExecute::threadrun(void * pBuf)
{

	MightyTCPSelectServer * ptrUser=(MightyTCPSelectServer *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;

	while(1)
	{
		DWORD Index = WaitForMultipleObjects(2,MightyEvent,FALSE,m_dwTimeOut);//m_dwTimeOut);	INFINITE
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
				ptrUser->acceptSocketObject();
				break;
			}
		default:
			break;
		}
	}
}

int SelectServerExecute::ExecuteTask(void * pBuf)
{
	return 1;
}

int SelectServerExecute::branchEvent(void * pBuf)
{
	return 1;
}
//----------------------------------------
MightyTCPSelectServer::MightyTCPSelectServer(ITcpServerNotify * pNotify)
:m_tdPoolIndex(0)
,m_pNotifyCallBack(pNotify)
{
	m_pThreadPool.clear();
}

MightyTCPSelectServer::~MightyTCPSelectServer(void)
{
	if (m_pAcceptThread)
	{
		delete m_pAcceptThread;
	}

	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		delete (HandleIOSelectServer *)m_pThreadPool[tdIndex];
		Sleep(10);
	}
}

bool MightyTCPSelectServer::Create(const unsigned short & usPort)
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
	::listen(m_ListenSock,200);

	startWork();
	return true;
}
void MightyTCPSelectServer::Destroy(void)
{

}
bool MightyTCPSelectServer::SendData(const char *pData, int nLen, const string & strLinkNo)
{
	PSelectIOObject sendHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		sendHandle=((HandleIOSelectServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			::send(sendHandle->s,pData,nLen,0);
			return true;
		}


	}
	return false;
}
bool MightyTCPSelectServer::CloseLink(const string & strLinkNo)
{
	PSelectIOObject sendHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		sendHandle=((HandleIOSelectServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			((HandleIOSelectServer *)m_pThreadPool[tdIndex])->RemoveSocketObj(sendHandle);

			return true;
		}


	}
	return false;
}
int MightyTCPSelectServer::acceptSocketObject()
{
	FD_ZERO( &fdServer );
	FD_SET(m_ListenSock,&fdServer);

	struct timeval tv;
	int nRet;
	//if( bBlock )
	//{
		// block for 100 ms to keep from eating up all cpu time

		//tv.tv_sec = 0;
		//tv.tv_usec = 100000;
	//}
	//else
	//{
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	//}

	if((nRet=select( 1, &fdServer, NULL, NULL, &tv ))>0)
	{
		if(FD_ISSET(m_ListenSock,&fdServer))
		{
			struct sockaddr_in adrFrom;

			int iAddrLen = sizeof( adrFrom );

			SOCKET sckClient;
			if( ( sckClient = accept( m_ListenSock, (struct sockaddr *)&adrFrom, &iAddrLen ) ) == INVALID_SOCKET )
			{
				return -1;
			}
			else
			{
				PSelectIOObject pSocket = GetSocketObj(sckClient);
				pSocket->addrRemote =adrFrom;
				AssignToFreeThread(pSocket);
			}

		}
	}
	else
	{
		return -1;
	}


}
PSelectIOObject MightyTCPSelectServer::GetSocketObj(SOCKET s)
{
	PSelectIOObject pSocket =NULL;
	pSocket=new SelectIOObject();
	if(pSocket != NULL)
	{
		pSocket->s = s;
	}
	return pSocket;
}
void MightyTCPSelectServer::AssignToFreeThread(PSelectIOObject pSocket)
{
	if (m_pThreadPool.empty())
	{
		HandleIOSelectServer * pIOThread=new HandleIOSelectServer();
		pIOThread->InsertSocketObj(pSocket);
		pIOThread->start(this);
		pIOThread->setExecuteSignal();
		m_tdPoolIndex=0;

	}
	else
	{
		int rtCode=0;
		rtCode=((HandleIOSelectServer *)m_pThreadPool[m_tdPoolIndex])->InsertSocketObj(pSocket);
		if (rtCode==-1)
		{
			HandleIOSelectServer * pIOThread=new HandleIOSelectServer();
			pIOThread->InsertSocketObj(pSocket);
			pIOThread->start(this);
			pIOThread->setExecuteSignal();
			m_tdPoolIndex++;

		}

	}
}
void MightyTCPSelectServer::startWork()
{
	m_pAcceptThread=new SelectServerExecute();
	m_pAcceptThread->start(this);
	m_pAcceptThread->setExecuteSignal();
}