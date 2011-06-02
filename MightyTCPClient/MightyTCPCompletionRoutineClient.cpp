#include ".\mightytcpcompletionroutineclient.h"
#include "../CommonInc/P2PUtilTools.h"
HandleIOCompletionRoutineClient::HandleIOCompletionRoutineClient(void)
{

}
HandleIOCompletionRoutineClient::~HandleIOCompletionRoutineClient(void)
{
	exit();
}

int HandleIOCompletionRoutineClient::threadrun(void * pBuf)
{
	MightyTCPCompletionRoutineClient * ptrUser=(MightyTCPCompletionRoutineClient *)pBuf;

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
int HandleIOCompletionRoutineClient::ExecuteTask(void * pBuf)
{
	return 1;
}
int HandleIOCompletionRoutineClient::branchEvent(void * pBuf)
{
	return 1;
}

//-----------------------------------------------------------------------------
MightyTCPCompletionRoutineClient::MightyTCPCompletionRoutineClient(ITcpClientNotify* pNotify)
:m_pNotifyCallBack(pNotify)
{
}

MightyTCPCompletionRoutineClient::~MightyTCPCompletionRoutineClient(void)
{
}
bool MightyTCPCompletionRoutineClient::ConnectServer( const char* pServerAddr, unsigned short usPort)
{
	return true;
}
bool MightyTCPCompletionRoutineClient::SendData( const char* pData, int nLen)
{
	return true;
}
void MightyTCPCompletionRoutineClient::Disconnect(void)
{

}
void MightyTCPCompletionRoutineClient::ReleaseConnection()
{

}

int MightyTCPCompletionRoutineClient::init(const char * ip,WORD port)
{
	m_strAddress=ip;
	m_ConnectPort=port;
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr = inet_addr(ip);
	m_SockAddr.sin_port = htons(port);
	m_dwAddress=inet_addr(ip);

	return 0;

}
int MightyTCPCompletionRoutineClient::init(DWORD ip,WORD port)
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
int MightyTCPCompletionRoutineClient::afreshConnect()
{
	if (m_ClientSock==INVALID_SOCKET)
	{
		m_ClientSock=WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);

		//WSAEventSelect(m_ClientSock,m_EventArray[0],FD_READ|FD_WRITE|FD_CLOSE|FD_ACCEPT|FD_CONNECT|FD_OOB);
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

int MightyTCPCompletionRoutineClient::Process()
{
	list<P_Routine_WSAOVERLAPPED>::iterator indexIter,endIter;
	DWORD Index;
	DWORD flag=0;
	BOOL boverRest;
	int iState;

	DWORD sendSize;
	afreshConnect();
	while (m_bRun)
	{

		m_pWsaOverLappedList.insert(m_pWsaOverLappedList.end(),m_pTaskWsaOverLappedList.begin(),m_pTaskWsaOverLappedList.end());
		m_pTaskWsaOverLappedList.clear();
		if (m_pWsaOverLappedList.empty())
		{
			PostRecvOpt();
			continue;
		}
		indexIter=m_pWsaOverLappedList.begin();
		endIter=m_pWsaOverLappedList.end();
		
		while(indexIter!=endIter)
		{
			flag=0;
			sendSize=0;

			Index = WSAWaitForMultipleEvents(1,m_AlertableWaitStateArray,FALSE,WSA_INFINITE,TRUE);
			//if (Index==WSA_WAIT_EVENT_0)
			if (Index==WAIT_IO_COMPLETION)
			{
				indexIter++;
				continue;
			}
			/*{
				/*WSAResetEvent((*indexIter)->m_EventArray[Index-WSA_WAIT_EVENT_0]);
				boverRest=WSAGetOverlappedResult(m_ClientSock,&(*indexIter)->m_PostOverLapped,&sendSize,FALSE,&flag);
				if (sendSize==0)
				{
					DestroySocket();
					return 1;
				}
				if ((*indexIter)->iotype==iorecv_id)
				{
					m_pNotifyCallBack->OnReceivedData((*indexIter)->iobuf,sendSize);
					PostIOOpt((*indexIter),iorecv_id,0);
				}
				else if ((*indexIter)->iotype==iosend_id)
				{
					pushOverlappCacheQueue((*indexIter));
					m_pWsaOverLappedList.erase(indexIter++);
					continue;
				}
				indexIter++;
				continue;
			}*/
			else
			{

				DestroySocket();
				return 1;
			}
		}
		/*if (!m_pWsaOverLappedList.empty())
		{

		}*/
		m_pCacheWsaOverLappedList.insert(m_pCacheWsaOverLappedList.end(),m_pWsaOverLappedList.begin(),m_pWsaOverLappedList.end());
		m_pWsaOverLappedList.clear();
		

	}

}

void MightyTCPCompletionRoutineClient::DestroySocket()
{
	closesocket(m_ClientSock);
	m_ClientSock=INVALID_SOCKET;
}

void MightyTCPCompletionRoutineClient::pushOverlappCacheQueue(P_Routine_WSAOVERLAPPED pBackOverlapped)
{
	m_pCacheWsaOverLappedList.push_back(pBackOverlapped);
}
int MightyTCPCompletionRoutineClient::PostIOOpt(P_Routine_WSAOVERLAPPED pReUseOverlapped,int iostyle,int isWait)
{
	DWORD flag=0;
	DWORD sendSize=0;
	int iState;
	ZeroMemory(&pReUseOverlapped->m_PostOverLapped,sizeof(WSAOVERLAPPED));
	//pReUseOverlapped->m_EventArray[0]=WSACreateEvent();
	//pReUseOverlapped->m_PostOverLapped.hEvent=pReUseOverlapped->m_EventArray[0];
	//pReUseOverlapped->m_WsaIOBuf.len=sizeof(pReUseOverlapped->iobuf);
	pReUseOverlapped->m_WsaIOBuf.buf=pReUseOverlapped->iobuf;
	pReUseOverlapped->iotype=iostyle;
	if (isWait==1)
	{
		m_pTaskWsaOverLappedList.push_back(pReUseOverlapped);
		return 0;
	}
	else
	{
		if (iostyle==RECV_ID)
		{
			if ((WSARecv(m_ClientSock,&pReUseOverlapped->m_WsaIOBuf,1,&sendSize,&flag,(LPWSAOVERLAPPED)(pReUseOverlapped),MightyTCPCompletionRoutineClient::WorkerRoutine))==SOCKET_ERROR)
			{
				iState=WSAGetLastError();
				if (iState!=WSA_IO_PENDING)
				{

					return 1; 
				}

			}
			return 0;
		}
		/*else if (iostyle==SEND_ID)
		{
		}*/
		else
		{
			if ((WSASend(m_ClientSock,&pReUseOverlapped->m_WsaIOBuf,1,&sendSize,flag,(LPWSAOVERLAPPED)(pReUseOverlapped),MightyTCPCompletionRoutineClient::WorkerRoutine))==SOCKET_ERROR)
			{
				iState=WSAGetLastError();
				if (iState!=WSA_IO_PENDING)
				{

					return 1; 
				}
			}
			return 0;
		}
	}

}
int MightyTCPCompletionRoutineClient::PostRecvOpt()
{
	if (m_pCacheWsaOverLappedList.empty())
	{
		P_Routine_WSAOVERLAPPED pOverLapStruct=new Routine_WSAOVERLAPPED();
		pOverLapStruct->pMainCall=this;
		//ZeroMemory(&pOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pOverLapStruct->m_PostOverLapped.hEvent=pOverLapStruct->m_EventArray[0];
		//pOverLapStruct->m_WsaIOBuf.len=sizeof(pOverLapStruct->iobuf);
		//pOverLapStruct->m_WsaIOBuf.buf=pOverLapStruct->iobuf;
		//pOverLapStruct->iotype=iorecv_id;
		pOverLapStruct->useSize=sizeof(pOverLapStruct->iobuf);
		PostIOOpt(pOverLapStruct,RECV_ID);

	}
	else
	{
		P_Routine_WSAOVERLAPPED pExistOverLapStruct=m_pCacheWsaOverLappedList.front();
		m_pCacheWsaOverLappedList.pop_front();
		//ZeroMemory(&pExistOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pExistOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pExistOverLapStruct->m_PostOverLapped.hEvent=pExistOverLapStruct->m_EventArray[0];
		//pExistOverLapStruct->m_WsaIOBuf.len=sizeof(pExistOverLapStruct->iobuf);
		//pExistOverLapStruct->m_WsaIOBuf.buf=pExistOverLapStruct->iobuf;
		//pExistOverLapStruct->iotype=iorecv_id;
		pExistOverLapStruct->useSize=sizeof(pExistOverLapStruct->iobuf);
		PostIOOpt(pExistOverLapStruct,RECV_ID);
	}
	return 0;

}
int MightyTCPCompletionRoutineClient::PostSendOpt(const char * outBuf,int size)
{
	if (m_pCacheWsaOverLappedList.empty())
	{
		P_Routine_WSAOVERLAPPED pOverLapStruct=new Routine_WSAOVERLAPPED();
		pOverLapStruct->pMainCall=this;
		//ZeroMemory(&pOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pOverLapStruct->m_PostOverLapped.hEvent=pOverLapStruct->m_EventArray[0];
		memset(pOverLapStruct->iobuf,0,sizeof(pOverLapStruct->iobuf));
		memcpy(pOverLapStruct->iobuf,outBuf,size);
		//pOverLapStruct->m_WsaIOBuf.len=sizeof(pOverLapStruct->iobuf);
		//pOverLapStruct->m_WsaIOBuf.buf=pOverLapStruct->iobuf;
		//pOverLapStruct->iotype=iosend_id;
		pOverLapStruct->useSize=size;
		PostIOOpt(pOverLapStruct,SEND_ID);
	}
	else
	{
		P_Routine_WSAOVERLAPPED pExistOverLapStruct=m_pCacheWsaOverLappedList.front();
		m_pCacheWsaOverLappedList.pop_front();
		//ZeroMemory(&pExistOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pExistOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pExistOverLapStruct->m_PostOverLapped.hEvent=pExistOverLapStruct->m_EventArray[0];
		memset(pExistOverLapStruct->iobuf,0,sizeof(pExistOverLapStruct->iobuf));
		memcpy(pExistOverLapStruct->iobuf,outBuf,size);
		//pExistOverLapStruct->m_WsaIOBuf.len=sizeof(pExistOverLapStruct->iobuf);
		//pExistOverLapStruct->m_WsaIOBuf.buf=pExistOverLapStruct->iobuf;
		//pExistOverLapStruct->iotype=iosend_id;
		pExistOverLapStruct->useSize=size;
		PostIOOpt(pExistOverLapStruct,SEND_ID);

	}
	return 0;
}

void CALLBACK MightyTCPCompletionRoutineClient::WorkerRoutine(DWORD Error,DWORD BytesTranferred,LPWSAOVERLAPPED overlapped,DWORD Inflag)
{
	P_Routine_WSAOVERLAPPED pioHd=(P_Routine_WSAOVERLAPPED)overlapped;
	MightyTCPCompletionRoutineClient * pManageRoutine=(MightyTCPCompletionRoutineClient *)(pioHd->pMainCall);
	//if (Error!=0||BytesTranferred==0)
	//{
	//	return;
	//}
	if (pioHd->iotype==RECV_ID)
	{
		
		pManageRoutine->m_pNotifyCallBack->OnReceivedData(pioHd->iobuf,BytesTranferred);
		pManageRoutine->PostIOOpt(pioHd,RECV_ID);
	}
	else
	{
		;//sendComplete
	}
	
	
}