#include ".\MightyTCPEventSelectServer.h"
#include "../CommonInc/P2PUtilTools.h"
HandleIOProcessTCPServer::HandleIOProcessTCPServer(void)
:m_exitCode(0)
,nSocketCount(0)
{

}
HandleIOProcessTCPServer:: ~HandleIOProcessTCPServer(void)
{
	exit();
}
PSOCKET_OBJ HandleIOProcessTCPServer::findSocket(const HANDLE & Hdsct)
{
	/*list<PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_PerThreadSockList.end();
	itorIndex=m_PerThreadSockList.begin();
	while (itorIndex!=itorRight)
	{
		if ((*itorIndex)->event==Hdsct)
		{
			return (*itorIndex);
		}
		itorIndex++;
	}
	return NULL;*/

	map<string,PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();

	while (itorIndex!=itorRight)
	{

		if(itorIndex->second->event==Hdsct)
		{
			return itorIndex->second;
		}

		itorIndex++;
	}
	return NULL;


}
void HandleIOProcessTCPServer::RebuildEventArray()
{
	/*list<PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_PerThreadSockList.end();
	itorIndex=m_PerThreadSockList.begin();
	int evtPlace=1;
	
	while (itorIndex!=itorRight)
	{
		events[evtPlace]=(*itorIndex)->event;
		evtPlace++;
		itorIndex++;
	}*/

	map<string,PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();
	int evtPlace=1;
	while (itorIndex!=itorRight)
	{
		events[evtPlace]=itorIndex->second->event;
		evtPlace++;
		itorIndex++;
	}
}
void HandleIOProcessTCPServer::RemoveSocketObj(PSOCKET_OBJ pSocket)
{
	/*list<PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_PerThreadSockList.end();
	itorIndex=m_PerThreadSockList.begin();
	while (itorIndex!=itorRight)
	{
		if(pSocket==(*itorIndex))
		{
			m_PerThreadSockList.erase(itorIndex);
			delete pSocket;
			nSocketCount--;
			::WSASetEvent(events[0]);
			break;
		}
		itorIndex++;
	}*/
	((MightyTCPEventSelectServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnClosed(pSocket->identityIDKey);
	map<string,PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();
	
	while (itorIndex!=itorRight)
	{
		if(pSocket==(itorIndex->second))
		{
			m_SocketMap.erase(itorIndex++);
			::CloseHandle(pSocket->event);
			if(pSocket->s != INVALID_SOCKET)
			{
				::closesocket(pSocket->s);
			}
			delete pSocket;
			nSocketCount--;
			::WSASetEvent(events[0]);
			break;
		}
		itorIndex++;
	}
	
}
int HandleIOProcessTCPServer::InsertSocketObj(PSOCKET_OBJ pSocket)
{
	/*if (m_PerThreadSockList.size()>=(WSA_MAXIMUM_WAIT_EVENTS - 1))
	{
			return -1;
	}
	else
	{
		m_PerThreadSockList.push_back(pSocket);
		nSocketCount++;
	}*/
	if (m_SocketMap.size()>=(WSA_MAXIMUM_WAIT_EVENTS - 1))
	{
		return -1;
	}
	else
	{
		//GUID guid_val;
		string guid_strkey=P2PUtilTools::GetStringUuid();
		pSocket->identityIDKey=guid_strkey;
		m_SocketMap.insert(map<string,PSOCKET_OBJ>::value_type(guid_strkey,pSocket));
		nSocketCount++;

		((MightyTCPEventSelectServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnAccept(guid_strkey);
	
	}
	return nSocketCount;
}
BOOL HandleIOProcessTCPServer::HandleIO(PSOCKET_OBJ pSocket)
{
	// 获取具体发生的网络事件
	WSANETWORKEVENTS event;
	::WSAEnumNetworkEvents(pSocket->s, pSocket->event, &event);
	do
	{
		if(event.lNetworkEvents & FD_READ)			// 套节字可读
		{
			if(event.iErrorCode[FD_READ_BIT] == 0)
			{
				char tcpPackage[4096]={0};
				int nRecv = ::recv(pSocket->s,tcpPackage,4096,0);
				/*if(nRecv > 0)				
				{
					szText[nRecv] = '\0';
					printf("接收到数据：%s \n", szText);
				}*/
				((MightyTCPEventSelectServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnRecvData(tcpPackage,nRecv,pSocket->identityIDKey);
			}
			else
				break;
		}
		else if(event.lNetworkEvents & FD_CLOSE)	// 套节字关闭
		{
			break;
		}
		else if(event.lNetworkEvents & FD_WRITE)	// 套节字可写
		{
			if(event.iErrorCode[FD_WRITE_BIT] == 0)
			{	
			}
			else
				break;
		}
		return TRUE;
	}
	while(FALSE);

	// 套节字关闭，或者有错误发生，程序都会转到这里来执行
	RemoveSocketObj(pSocket);
	return FALSE;
}
void HandleIOProcessTCPServer::setNotifyEvent()
{
	m_exitCode=1;
	::WSASetEvent(events[0]);
}
void HandleIOProcessTCPServer::DestroyAllSocket()
{
	/*list<PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_PerThreadSockList.end();
	itorIndex=m_PerThreadSockList.begin();
	while (itorIndex!=itorRight)
	{
			if ((*itorIndex))
			{
				delete (*itorIndex);
			}
			
			m_PerThreadSockList.erase(itorIndex);
			
			nSocketCount--;
			itorIndex++;
	}*/

	map<string,PSOCKET_OBJ>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();
	while (itorIndex!=itorRight)
	{
		if ((itorIndex->second))
		{
			::CloseHandle(itorIndex->second->event);
			if(itorIndex->second->s != INVALID_SOCKET)
			{
				::closesocket(itorIndex->second->s);
			}
			delete (itorIndex->second);
		}

		m_SocketMap.erase(itorIndex++);
		//itorIndex= m_SocketMap.erase(itorIndex);
		nSocketCount--;
		//itorIndex++;
	}
}
int HandleIOProcessTCPServer::threadrun(void * pBuf)
{
	while(TRUE)
	{
		//	等待网络事件
		int nIndex = ::WSAWaitForMultipleEvents(nSocketCount+1,events, FALSE, WSA_INFINITE, FALSE);
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		// 查看受信的事件对象
		for(int i=nIndex; i<nSocketCount + 1; i++)
		{
			nIndex = ::WSAWaitForMultipleEvents(1, &events[i], TRUE, 1000, FALSE);
			if(nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT)
			{
				continue;
			}
			else
			{
				if(i == 0)	// events[0]受信，重建数组
				{
					RebuildEventArray();
					// 如果没有客户I/O要处理了，则本线程退出
					if(nSocketCount == 0||m_exitCode==1)
					{
						DestroyAllSocket();
						_endthreadex(0);
						return 0;
					}
					::WSAResetEvent(events[0]);
				}
				else// 处理网络事件
				{
					// 查找对应的套节字对象指针，调用HandleIO处理网络事件
					//nIndex-WSA_WAIT_EVENT_0
					//WSA_MAXIMUM_WAIT_EVENTS
					PSOCKET_OBJ pSocket = (PSOCKET_OBJ)findSocket(events[i]);
					if(pSocket != NULL)
					{
						if(!HandleIO(pSocket))
							RebuildEventArray();
					}
					else
						printf(" Unable to find socket object \n ");
				}
			}
		}
	}
}

int HandleIOProcessTCPServer::ExecuteTask(void * pBuf)
{
	return 1;
}

int HandleIOProcessTCPServer::branchEvent(void * pBuf)
{
	return 1;
}
PSOCKET_OBJ HandleIOProcessTCPServer::SearchSocketObj(const string & keyId)
{
	map<string,PSOCKET_OBJ>::iterator leftPos,rightPos;
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
TCPServerExecute::TCPServerExecute(void)
{
}

TCPServerExecute::~TCPServerExecute(void)
{
	exit();
}
int TCPServerExecute::threadrun(void * pBuf)
{

	MightyTCPEventSelectServer * ptrUser=(MightyTCPEventSelectServer *)pBuf;

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
				ptrUser->acceptSocketObject();
				break;
			}
		default:
			break;
		}
	}
}

int TCPServerExecute::ExecuteTask(void * pBuf)
{
	return 1;
}

int TCPServerExecute::branchEvent(void * pBuf)
{
	return 1;
}
MightyTCPEventSelectServer::MightyTCPEventSelectServer(ITcpServerNotify * pNotify)
:m_tdPoolIndex(0)
,m_pNotifyCallBack(pNotify)
{
	m_pThreadPool.clear();
}

MightyTCPEventSelectServer::~MightyTCPEventSelectServer(void)
{
	if (m_pAcceptThread)
	{
		delete m_pAcceptThread;
	}

	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		((HandleIOProcessTCPServer *)m_pThreadPool[tdIndex])->setNotifyEvent();
		Sleep(10);
		delete (HandleIOProcessTCPServer *)m_pThreadPool[tdIndex];

	}
}
int MightyTCPEventSelectServer::init(const char * ip,WORD port)
{

	m_ListenPort=port;
	m_ListenSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if(::bind(m_ListenSock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		return -1;
	}
	::listen(m_ListenSock,200);

	// 创建事件对象，并关联到监听的套节字
	m_ListenEvent= ::WSACreateEvent();
	::WSAEventSelect(m_ListenSock,m_ListenEvent,FD_ACCEPT|FD_CLOSE);

	startWork();

}
int MightyTCPEventSelectServer::acceptSocketObject()
{
	while(TRUE)
	{
		int nRet = ::WaitForSingleObject(m_ListenEvent,5*1000);
		if(nRet == WAIT_FAILED)
		{
			return -1;
		}
		else if(nRet == WSA_WAIT_TIMEOUT)	// 定时显式状态信息
		{
			
			continue;
		}
		else								// 有新的连接未决
		{
			::ResetEvent(m_ListenEvent);
			// 循环处理所有未决的连接请求
			while(TRUE)
			{
				sockaddr_in si;
				int nLen = sizeof(si);
				SOCKET sNew = ::accept(m_ListenSock, (sockaddr*)&si, &nLen);
				if(sNew == SOCKET_ERROR)
					break;
				PSOCKET_OBJ pSocket = GetSocketObj(sNew);
				pSocket->addrRemote = si;
				::WSAEventSelect(pSocket->s, pSocket->event, FD_READ|FD_CLOSE|FD_WRITE);
				AssignToFreeThread(pSocket);
			}
		}
	}
}
PSOCKET_OBJ MightyTCPEventSelectServer::GetSocketObj(SOCKET s)
{
	PSOCKET_OBJ pSocket = new SOCKET_OBJ();
	if(pSocket != NULL)
	{
		pSocket->s = s;
		pSocket->event = ::WSACreateEvent();
	}
	return pSocket;
}
void MightyTCPEventSelectServer::AssignToFreeThread(PSOCKET_OBJ pSocket)
{
	if (m_pThreadPool.empty())
	{
		HandleIOProcessTCPServer * pIOThread=new HandleIOProcessTCPServer();
		pIOThread->InsertSocketObj(pSocket);
		pIOThread->start(this);
		pIOThread->setExecuteSignal();
		m_tdPoolIndex=0;

	}
	else
	{
		int rtCode=0;
		rtCode=((HandleIOProcessTCPServer *)m_pThreadPool[m_tdPoolIndex])->InsertSocketObj(pSocket);
		if (rtCode==-1)
		{
			HandleIOProcessTCPServer * pIOThread=new HandleIOProcessTCPServer();
			pIOThread->InsertSocketObj(pSocket);
			pIOThread->start(this);
			pIOThread->setExecuteSignal();
			m_tdPoolIndex++;
			
		}

	}

}

void MightyTCPEventSelectServer::startWork()
{
	m_pAcceptThread=new TCPServerExecute();
	m_pAcceptThread->start(this);
	m_pAcceptThread->setExecuteSignal();
}

bool MightyTCPEventSelectServer::Create(const unsigned short & usPort)
{
	m_ListenPort=usPort;
	m_ListenSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(usPort);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if(::bind(m_ListenSock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		return -1;
	}
	::listen(m_ListenSock,200);

	// 创建事件对象，并关联到监听的套节字
	m_ListenEvent= ::WSACreateEvent();
	::WSAEventSelect(m_ListenSock,m_ListenEvent,FD_ACCEPT|FD_CLOSE);

	startWork();
}
void MightyTCPEventSelectServer::Destroy(void)
{

}
bool MightyTCPEventSelectServer::SendData(const char *pData, int nLen, const string & strLinkNo)
{
	PSOCKET_OBJ sendHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		sendHandle=((HandleIOProcessTCPServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			::send(sendHandle->s,pData,nLen,0);
			return true;
		}
		

	}
	return false;
}
bool MightyTCPEventSelectServer::CloseLink(const string & strLinkNo)
{
	PSOCKET_OBJ sendHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		sendHandle=((HandleIOProcessTCPServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			((HandleIOProcessTCPServer *)m_pThreadPool[tdIndex])->RemoveSocketObj(sendHandle);

			return true;
		}


	}
	return false;

}