#include ".\MightyTCPCompletionRoutineServer.h"
#include "../CommonInc/P2PUtilTools.h"
//----------------------------------------------------------------------
void CALLBACK SendComplete(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK RecvComplete(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK SendComplete(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{


}
void CALLBACK RecvComplete(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	
	PCompletionRoutine_BufObj pOriginalCondition=(PCompletionRoutine_BufObj)lpOverlapped;
	HandleIOCompletionRoutineServer * pHandleIOThread=(HandleIOCompletionRoutineServer *)(pOriginalCondition->pAgent);
	
	((MightyTCPCompletionRoutineServer *)(pHandleIOThread->m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnRecvData(pOriginalCondition->buff,cbTransferred,pOriginalCondition->pSocket->identityIDKey);
	//((HandleIOCompletionRoutineServer *)pOriginalCondition->pAgent)->PostRecv(pOriginalCondition);
	pHandleIOThread->PostRecv(pOriginalCondition);

}
//----------------------------------------------------------------------
HandleIOCompletionRoutineServer::HandleIOCompletionRoutineServer(void)
{
	m_Notify_WsaEvents[0] = WSACreateEvent();
	

}
HandleIOCompletionRoutineServer::~HandleIOCompletionRoutineServer(void)
{

}
int HandleIOCompletionRoutineServer::threadrun(void * pBuf)
{
	
	MightyTCPCompletionRoutineServer * ptrUser=(MightyTCPCompletionRoutineServer *)pBuf;

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
				//((HandleIOOverlappedEventServer *)m_userInstance.threadThis_)->DestroyAllSocket();
				_endthreadex(0);
				return 0;
			}
		case WAIT_TIMEOUT:
		case WAIT_OBJECT_0 + 1:
			{
				((HandleIOCompletionRoutineServer *)m_userInstance.threadThis_)->WorkProcess();
				break;
			}
		default:
			break;
		}
	}
}

int HandleIOCompletionRoutineServer::ExecuteTask(void * pBuf)
{
	return 0;
}

int HandleIOCompletionRoutineServer::branchEvent(void * pBuf)
{
	return 0;
}

int HandleIOCompletionRoutineServer::WorkProcess()
{
	PCompletionRoutine_BufObj pLoopIndexBuf=NULL;
	int stateCode=0;
	while(m_exitCode==0)
	{
		pLoopIndexBuf=NULL;
		stateCode=0;

		stateCode=PostCompletionRoutineRequest(pLoopIndexBuf);
		if (stateCode==-1)
		{
			RemoveSocketObj(pLoopIndexBuf->pSocket);
			FreeBufferObj(pLoopIndexBuf);
		}
		// 等待重叠请求完成，自动回调完成例程函数
		DWORD dwIndex = WSAWaitForMultipleEvents(1,m_Notify_WsaEvents,FALSE,10,TRUE);

		// 返回WAIT_IO_COMPLETION表示一个重叠请求完成例程例结束
		if(dwIndex == WAIT_IO_COMPLETION)
		{
			continue;
		}
		else  
		{
			if(dwIndex == WAIT_TIMEOUT)                              
				continue;
			else                                                    
			{ 
				break;
			}
		}
	}
	return 0;

}
void HandleIOCompletionRoutineServer::RemoveSocketObj(PCompletionRoutineIOObject pSocket)
{

	((MightyTCPCompletionRoutineServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnClosed(pSocket->identityIDKey);
	map<string,PCompletionRoutineIOObject>::iterator itorRight,itorIndex;
	itorRight=m_SocketMap.end();
	itorIndex=m_SocketMap.begin();

	while (itorIndex!=itorRight)
	{
		if(pSocket==(itorIndex->second))
		{


			if(pSocket->s != INVALID_SOCKET)
			{
				::closesocket(pSocket->s);
				pSocket->s = INVALID_SOCKET;
			}
			if(pSocket->nOutstandingOps == 0)
			{
				delete pSocket;
				m_SocketMap.erase(itorIndex++);
				nSocketCount--;
			}

			break;
		}
		itorIndex++;
	}
}

int HandleIOCompletionRoutineServer::InsertSocketObj(PCompletionRoutineIOObject pSocket)
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
		m_SocketMap.insert(map<string,PCompletionRoutineIOObject>::value_type(guid_strkey,pSocket));
		nSocketCount++;
		PCompletionRoutine_BufObj pIOBuf=GetBufferObj(pSocket,4096);
		PostRecv(pIOBuf);
		//((MightyTCPOverlappedEventServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnAccept(guid_strkey);

	}
}
void HandleIOCompletionRoutineServer::DestroyAllSocket()
{
	map<string,PCompletionRoutineIOObject>::iterator itorRight,itorIndex;
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


			delete (itorIndex->second);
		}

		m_SocketMap.erase(itorIndex++);
		//itorIndex= m_SocketMap.erase(itorIndex);
		nSocketCount--;
		//itorIndex++;
	}
}
PCompletionRoutineIOObject HandleIOCompletionRoutineServer::SearchSocketObj(const string & keyId)
{
	map<string,PCompletionRoutineIOObject>::iterator leftPos,rightPos;
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


PCompletionRoutine_BufObj HandleIOCompletionRoutineServer::GetBufferObj(PCompletionRoutineIOObject pSocket, ULONG nLen)
{


	PCompletionRoutine_BufObj pBuffer = new CompletionRoutine_BufObj();
	if(pBuffer != NULL)
	{
		//pBuffer->buff = (char*)::GlobalAlloc(GPTR, nLen);
		pBuffer->buff =new char[nLen]; 
		pBuffer->nLen=nLen;
		ZeroMemory(&pBuffer->wsaol,sizeof(WSAOVERLAPPED));
		
		pBuffer->pSocket = pSocket;
		//pBuffer->pAgent=m_userInstance.InstanceUserData_;
		pBuffer->pAgent=m_userInstance.threadThis_;

	}
	return pBuffer;
}

int HandleIOCompletionRoutineServer::PostRecv(PCompletionRoutine_BufObj pBuffer)
{

	pBuffer->nOperation = OP_RECV;
	pBuffer->pSocket->nOutstandingOps++;
	pBuffer->wsaDataBuf.buf = pBuffer->buff;
	pBuffer->wsaDataBuf.len = pBuffer->nLen;
	m_overLappedBufList.push_back(pBuffer);
	return 0;
	// 投递此重叠I/O
	/*DWORD dwBytes=0;
	DWORD dwFlags = 0;
	

	if(::WSARecv(pBuffer->pSocket->s, &pBuffer->wsaDataBuf, 1, &dwBytes, &dwFlags, &pBuffer->wsaol, (LPWSAOVERLAPPED_COMPLETION_ROUTINE)RecvComplete) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}*/
}
int HandleIOCompletionRoutineServer::PostSend(PCompletionRoutine_BufObj pBuffer)
{

	pBuffer->nOperation = OP_SEND;
	pBuffer->pSocket->nOutstandingOps++;
	pBuffer->wsaDataBuf.buf = pBuffer->buff;
	pBuffer->wsaDataBuf.len = pBuffer->nLen;
	m_overLappedBufList.push_back(pBuffer);
	return 0;
	/*DWORD dwBytes=0;
	DWORD dwFlags = 0;
	WSABUF buf;
	buf.buf = pBuffer->buff;
	buf.len = pBuffer->nLen;
	if(::WSASend(pBuffer->pSocket->s,&pBuffer->wsaDataBuf,1,&dwBytes,dwFlags,&pBuffer->wsaol, (LPWSAOVERLAPPED_COMPLETION_ROUTINE)SendComplete) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}*/
}

int HandleIOCompletionRoutineServer::PostCompletionRoutineRequest(PCompletionRoutine_BufObj pBuffer)
{
	int retCode=-1;
	DWORD dwBytes=0;
	DWORD dwFlags = 0;
	if (m_overLappedBufList.empty())
	{
		return 1;
	}
	//PCompletionRoutine_BufObj pTaskIndexBuf=NULL;
	//pLoopIndexBuf=m_overLappedBufList.back();
	pBuffer=m_overLappedBufList.front();
	m_overLappedBufList.pop_front();
	switch (pBuffer->nOperation)
	{
		case OP_RECV:
		{
			if(::WSARecv(pBuffer->pSocket->s, &pBuffer->wsaDataBuf, 1, &dwBytes, &dwFlags, &pBuffer->wsaol, (LPWSAOVERLAPPED_COMPLETION_ROUTINE)RecvComplete) != NO_ERROR)
			{
				if(::WSAGetLastError() != WSA_IO_PENDING)
					return -1;
			}
			retCode=0;
			break;

		}
		case OP_SEND:
		{
			if(::WSASend(pBuffer->pSocket->s,&pBuffer->wsaDataBuf,1,&dwBytes,dwFlags,&pBuffer->wsaol, (LPWSAOVERLAPPED_COMPLETION_ROUTINE)SendComplete) != NO_ERROR)
			{
				if(::WSAGetLastError() != WSA_IO_PENDING)
					return -1;
			}
			retCode=0;
			break;
		}
		default:
		{
			retCode=-1;
			break;
		}
	}
	return retCode;
}
void HandleIOCompletionRoutineServer::FreeBufferObj(PCompletionRoutine_BufObj pBuffer)
{
	list<PCompletionRoutine_BufObj>::iterator leftIter,EndIter;
	if (!m_overLappedBufList.empty())
	{
		EndIter=m_overLappedBufList.end();
		leftIter=m_overLappedBufList.begin();
		while(leftIter!=EndIter)
		{
			if ((*leftIter)==pBuffer)
			{
				m_overLappedBufList.erase(leftIter);
				if (pBuffer->buff)
				{
					delete [] pBuffer->buff;
				}
				delete pBuffer;
				m_nEffectiveEventCount--;
				break;
			}
			leftIter++;

		}
	}
}
//PCompletionRoutine_BufObj FindBufferObj(HANDLE hEvent)
//{

//}
//----------------------------------------------------------------------
CompletionRoutineExecute::CompletionRoutineExecute(void)
{
}

CompletionRoutineExecute::~CompletionRoutineExecute(void)
{
	exit();
}
int CompletionRoutineExecute::threadrun(void * pBuf)
{

	MightyTCPCompletionRoutineServer * ptrUser=(MightyTCPCompletionRoutineServer *)pBuf;

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

int CompletionRoutineExecute::ExecuteTask(void * pBuf)
{
	return 1;
}

int CompletionRoutineExecute::branchEvent(void * pBuf)
{
	return 1;
}
//----------------------------------------------------------------------
MightyTCPCompletionRoutineServer::MightyTCPCompletionRoutineServer(ITcpServerNotify * pNotify)
:m_pNotifyCallBack(pNotify)
{
}

MightyTCPCompletionRoutineServer::~MightyTCPCompletionRoutineServer(void)
{
}

bool MightyTCPCompletionRoutineServer::Create(const unsigned short & usPort)
{
	m_ListenPort=usPort;
	m_ListenSock = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN si;
	si.sin_family = AF_INET;
	si.sin_port = ::ntohs(usPort);
	si.sin_addr.S_un.S_addr = INADDR_ANY;
	::bind(m_ListenSock, (sockaddr*)&si, sizeof(si));
	::listen(m_ListenSock, 200);


	startWork();
	
	return 0;
}
void MightyTCPCompletionRoutineServer::Destroy(void)
{

}
bool MightyTCPCompletionRoutineServer::SendData(const char *pData, int nLen, const string & strLinkNo)
{

	PCompletionRoutineIOObject sendHandle=NULL;
	HandleIOCompletionRoutineServer * pHandleIOSend=NULL;
	PCompletionRoutine_BufObj pSendBufRole=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{

		sendHandle=((HandleIOCompletionRoutineServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			pHandleIOSend=(HandleIOCompletionRoutineServer *)m_pThreadPool[tdIndex];
			
			pSendBufRole=pHandleIOSend->GetBufferObj(sendHandle,nLen);
			if (pSendBufRole)
			{
				
				pSendBufRole->nLen = nLen;
				memcpy(pSendBufRole->buff,pData, nLen);
				pHandleIOSend->PostSend(pSendBufRole);
			}


			return true;
		}


	}
	return false;
}
bool MightyTCPCompletionRoutineServer::CloseLink(const string & strLinkNo)
{

	PCompletionRoutineIOObject controlHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		controlHandle=((HandleIOCompletionRoutineServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (controlHandle!=NULL)
		{
			((HandleIOCompletionRoutineServer *)m_pThreadPool[tdIndex])->RemoveSocketObj(controlHandle);

			return true;
		}


	}
	return false;
}

void MightyTCPCompletionRoutineServer::startWork()
{
	m_pAcceptThread=new CompletionRoutineExecute();
	m_pAcceptThread->start(this);
	m_pAcceptThread->setExecuteSignal();

}

int MightyTCPCompletionRoutineServer::acceptSocketObject()
{
	while(m_exitNotify==0)
	{

				sockaddr_in si;
				int nLen = sizeof(si);
				SOCKET sNew = ::accept(m_ListenSock, (sockaddr*)&si, &nLen);
				if(sNew == SOCKET_ERROR)
					break;
				PCompletionRoutineIOObject pSocket = GetSocketObj(sNew);
				pSocket->addrRemote = si;
				
				AssignToFreeThread(pSocket);
			
		
	}
	return 0;

}

PCompletionRoutineIOObject MightyTCPCompletionRoutineServer::GetSocketObj(SOCKET s)
{
	PCompletionRoutineIOObject pSocket =NULL;
	pSocket=new CompletionRoutineIOObject();
	if(pSocket != NULL)
	{
		pSocket->s = s;
	}
	return pSocket;
}
void MightyTCPCompletionRoutineServer::AssignToFreeThread(PCompletionRoutineIOObject pSocket)
{
	if (m_pThreadPool.empty())
	{
		HandleIOCompletionRoutineServer * pIOThread=new HandleIOCompletionRoutineServer();
		pIOThread->InsertSocketObj(pSocket);
		pIOThread->start(this);
		pIOThread->setExecuteSignal();
		m_tdPoolIndex=0;

	}
	else
	{
		int rtCode=0;
		rtCode=((HandleIOCompletionRoutineServer *)m_pThreadPool[m_tdPoolIndex])->InsertSocketObj(pSocket);
		if (rtCode==-1)
		{
			HandleIOCompletionRoutineServer * pIOThread=new HandleIOCompletionRoutineServer();
			pIOThread->InsertSocketObj(pSocket);
			pIOThread->start(this);
			pIOThread->setExecuteSignal();
			m_tdPoolIndex++;

		}

	}
}