#include ".\mightytcpoverlappedeventserver.h"
#include "../CommonInc/P2PUtilTools.h"
//-------------------------------------------------------------------------
HandleIOOverlappedEventServer::HandleIOOverlappedEventServer(void)
{

}
HandleIOOverlappedEventServer::~HandleIOOverlappedEventServer(void)
{

}
int HandleIOOverlappedEventServer::threadrun(void * pBuf)
{
	MightyTCPOverlappedEventServer * ptrUser=(MightyTCPOverlappedEventServer *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;

	m_Notify_Events[0] = ::WSACreateEvent();
	::WSASetEvent(m_Notify_Events[0]);

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
				((HandleIOOverlappedEventServer *)m_userInstance.threadThis_)->WorkProcess();
				break;
			}
		default:
			break;
		}
	}
}

int HandleIOOverlappedEventServer::ExecuteTask(void * pBuf)
{
	return 0;

}

int HandleIOOverlappedEventServer::branchEvent(void * pBuf)
{
	return 0;

}
void HandleIOOverlappedEventServer::RebuildEventArray()
{
	list<POverlappedEvent_BufObj>::iterator leftIter,EndIter;
	int i =  1;
	if (!m_overLappedBufList.empty())
	{
		EndIter=m_overLappedBufList.end();
		leftIter=m_overLappedBufList.begin();
		while(leftIter!=EndIter)
		{
			m_Notify_Events[i++]=(*leftIter)->ol.hEvent;
			leftIter++;

		}
	}
}
int HandleIOOverlappedEventServer::WorkProcess()
{

	while(m_exitCode==0)
	{
		int nIndex = ::WSAWaitForMultipleEvents(m_nEffectiveEventCount + 1, m_Notify_Events, FALSE, WSA_INFINITE, FALSE);
		if(nIndex == WSA_WAIT_FAILED)
		{
			break;
		}
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		for(int i=0; i<=nIndex; i++)
		{
			int nRet = ::WSAWaitForMultipleEvents(1, &m_Notify_Events[i], TRUE, 0, FALSE);
			if(nRet == WSA_WAIT_TIMEOUT)
				continue;
			else
			{
				::WSAResetEvent(m_Notify_Events[i]);
				// 重新建立g_events数组
				if(i == 0)
				{
					RebuildEventArray();
					continue;
				}

				// 处理这个I/O
				POverlappedEvent_BufObj pBuffer = FindBufferObj(m_Notify_Events[i]);
				if(pBuffer != NULL)
				{
					if(AnalyzeProcess(pBuffer)==-1)
						RebuildEventArray();
				}
			}
		}
	}
	return 0;
}
int HandleIOOverlappedEventServer::AnalyzeProcess(POverlappedEvent_BufObj pBuffer)
{
	POverlappedEventIOObject pSocket = pBuffer->pSocket; // 从BUFFER_OBJ对象中提取SOCKET_OBJ对象指针，为的是方便引用
	pSocket->nOutstandingOps --;

	// 获取重叠操作结果
	DWORD dwTrans;
	DWORD dwFlags;
	BOOL bRet = ::WSAGetOverlappedResult(pSocket->s, &pBuffer->ol, &dwTrans, FALSE, &dwFlags);
	if(!bRet)
	{
		// 在此套节字上有错误发生，因此，关闭套节字，移除此缓冲区对象。
		// 如果没有其它抛出的I/O请求了，释放此缓冲区对象，否则，等待此套节字上的其它I/O也完成
		/*if(pSocket->s != INVALID_SOCKET)
		{
			::closesocket(pSocket->s);
			pSocket->s = INVALID_SOCKET;
		}

		if(pSocket->nOutstandingOps == 0)
			FreeSocketObj(pSocket);	*/
		RemoveSocketObj(pSocket);
		FreeBufferObj(pBuffer);
		return -1;
	}

	//----------------

	// 没有错误发生，处理已完成的I/O
	switch(pBuffer->nOperation)
	{

		case OP_READ:	// 接收数据完成
		{
				if(dwTrans > 0)
				{
					// 创建一个缓冲区，以发送数据。这里就使用原来的缓冲区
					POverlappedEvent_BufObj pNextRecv = pBuffer;
					((MightyTCPOverlappedEventServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnRecvData(pNextRecv->buff,dwTrans,pSocket->identityIDKey);
				}
				else
				{

					// 必须先关闭套节字，以便在此套节字上投递的其它I/O也返回
					/*if(pSocket->s != INVALID_SOCKET)
					{
						::closesocket(pSocket->s);
						pSocket->s = INVALID_SOCKET;
					}

					if(pSocket->nOutstandingOps == 0)
						FreeSocketObj(pSocket);		*/
					RemoveSocketObj(pSocket);
					FreeBufferObj(pBuffer);
					return -1;
				}
		}
		break;
		case OP_WRITE:		// 发送数据完成
		{
			if(dwTrans > 0)
			{
				// 继续使用这个缓冲区投递接收数据的请求
				pBuffer->nLen = BUFFER_SIZE;
				PostRecv(pBuffer);
			}
			else	// 套节字关闭
			{

				RemoveSocketObj(pSocket);
				FreeBufferObj(pBuffer);
				return -1;
			}
		}
		break;
	}
	return 0;

}
void HandleIOOverlappedEventServer::RemoveSocketObj(POverlappedEventIOObject pSocket)
{
	((MightyTCPOverlappedEventServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnClosed(pSocket->identityIDKey);
	map<string,POverlappedEventIOObject>::iterator itorRight,itorIndex;
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
int HandleIOOverlappedEventServer::InsertSocketObj(POverlappedEventIOObject pSocket)
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
		m_SocketMap.insert(map<string,POverlappedEventIOObject>::value_type(guid_strkey,pSocket));
		nSocketCount++;
		POverlappedEvent_BufObj pIOBuf=GetBufferObj(pSocket,4096);
		((MightyTCPOverlappedEventServer *)(m_userInstance.InstanceUserData_))->PostRecv(pIOBuf);
		((MightyTCPOverlappedEventServer *)(m_userInstance.InstanceUserData_))->m_pNotifyCallBack->OnAccept(guid_strkey);

	}
}

void HandleIOOverlappedEventServer::DestroyAllSocket()
{
	map<string,POverlappedEventIOObject>::iterator itorRight,itorIndex;
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
POverlappedEventIOObject HandleIOOverlappedEventServer::SearchSocketObj(const string & keyId)
{

	map<string,POverlappedEventIOObject>::iterator leftPos,rightPos;
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
//-------------------------------------------------------------------------
POverlappedEvent_BufObj HandleIOOverlappedEventServer::GetBufferObj(POverlappedEventIOObject pSocket, ULONG nLen)
{
	if(m_nEffectiveEventCount > WSA_MAXIMUM_WAIT_EVENTS - 1)
		return NULL;

	POverlappedEvent_BufObj pBuffer = new OverlappedEvent_BufObj();
	if(pBuffer != NULL)
	{
		//pBuffer->buff = (char*)::GlobalAlloc(GPTR, nLen);
		pBuffer->ol.hEvent = ::WSACreateEvent();
		pBuffer->pSocket = pSocket;
		pBuffer->sAccept = INVALID_SOCKET;

		m_overLappedBufList.push_back(pBuffer);
		m_Notify_Events[++m_nEffectiveEventCount] = pBuffer->ol.hEvent;
	}
	return pBuffer;
}

void HandleIOOverlappedEventServer::FreeBufferObj(POverlappedEvent_BufObj pBuffer)
{

	list<POverlappedEvent_BufObj>::iterator leftIter,EndIter;
	if (!m_overLappedBufList.empty())
	{
		EndIter=m_overLappedBufList.end();
		leftIter=m_overLappedBufList.begin();
		while(leftIter!=EndIter)
		{
			if ((*leftIter)==pBuffer)
			{
				m_overLappedBufList.erase(leftIter);
				::CloseHandle(pBuffer->ol.hEvent);
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

POverlappedEvent_BufObj HandleIOOverlappedEventServer::FindBufferObj(HANDLE hEvent)
{
	POverlappedEvent_BufObj pBuffer = NULL;
	list<POverlappedEvent_BufObj>::iterator leftIter,EndIter;
	if (!m_overLappedBufList.empty())
	{

		EndIter=m_overLappedBufList.end();
		leftIter=m_overLappedBufList.begin();
		while(leftIter!=EndIter)
		{
			if((*leftIter)->ol.hEvent == hEvent)
			{
				//m_overLappedBufList.erase(leftIter);
				return pBuffer=(*leftIter);
			}
			leftIter++;

		}
	}
	return pBuffer;
}

int HandleIOOverlappedEventServer::PostRecv(POverlappedEvent_BufObj pBuffer)
{	
	// 设置I/O类型，增加套节字上的重叠I/O计数
	pBuffer->nOperation = OP_READ;
	pBuffer->pSocket->nOutstandingOps ++;

	// 投递此重叠I/O
	DWORD dwBytes;
	DWORD dwFlags = 0;
	WSABUF buf;
	buf.buf = pBuffer->buff;
	buf.len = pBuffer->nLen;
	if(::WSARecv(pBuffer->pSocket->s, &buf, 1, &dwBytes, &dwFlags, &pBuffer->ol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return 0;
}

int HandleIOOverlappedEventServer::PostSend(POverlappedEvent_BufObj pBuffer)
{
	// 设置I/O类型，增加套节字上的重叠I/O计数
	pBuffer->nOperation = OP_WRITE;
	pBuffer->pSocket->nOutstandingOps ++;

	// 投递此重叠I/O
	DWORD dwBytes;
	DWORD dwFlags = 0;
	WSABUF buf;
	buf.buf = pBuffer->buff;
	buf.len = pBuffer->nLen;
	if(::WSASend(pBuffer->pSocket->s, 
		&buf, 1, &dwBytes, dwFlags, &pBuffer->ol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return 0;
}
//-------------------------------------------------------------------------
OverlappedEventExecute::OverlappedEventExecute(void)
{

}
OverlappedEventExecute::~OverlappedEventExecute(void)
{

}

int OverlappedEventExecute::threadrun(void * pBuf)
{
	return 0;
}
int OverlappedEventExecute::ExecuteTask(void * pBuf)
{

	return 0;
}
int OverlappedEventExecute::branchEvent(void * pBuf)
{
	return 0;
}
//------------------------------------------------------------------------
MightyTCPOverlappedEventServer::MightyTCPOverlappedEventServer(ITcpServerNotify * pNotify)
:m_pNotifyCallBack(pNotify)
{
}

MightyTCPOverlappedEventServer::~MightyTCPOverlappedEventServer(void)
{
}

bool MightyTCPOverlappedEventServer::Create(const unsigned short & usPort)
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
	// 为监听套节字创建一个SOCKET_OBJ对象
	m_pListenObj= GetSocketObj(m_ListenSock);

	// 加载扩展函数AcceptEx
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes;
	WSAIoctl(m_pListenObj->s, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx),
		&m_pListenObj->lpfnAcceptEx, 
		sizeof(m_pListenObj->lpfnAcceptEx), 
		&dwBytes, 
		NULL, 
		NULL);
	return 0;


}
void MightyTCPOverlappedEventServer::Destroy(void)
{

}
bool MightyTCPOverlappedEventServer::SendData(const char *pData, int nLen, const string & strLinkNo)
{
	POverlappedEventIOObject sendHandle=NULL;
	HandleIOOverlappedEventServer * pHandleIOSend=NULL;
	POverlappedEvent_BufObj pSendBufRole=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		
		sendHandle=((HandleIOOverlappedEventServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (sendHandle!=NULL)
		{
			pHandleIOSend=(HandleIOOverlappedEventServer *)m_pThreadPool[tdIndex];
			//::send(sendHandle->s,pData,nLen,0);
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
bool MightyTCPOverlappedEventServer::CloseLink(const string & strLinkNo)
{

	POverlappedEventIOObject controlHandle=NULL;
	unsigned int thdCount=m_pThreadPool.size();
	for (unsigned int tdIndex=0;tdIndex<thdCount;tdIndex++)
	{
		controlHandle=((HandleIOOverlappedEventServer *)m_pThreadPool[tdIndex])->SearchSocketObj(strLinkNo);
		if (controlHandle!=NULL)
		{
			((HandleIOOverlappedEventServer *)m_pThreadPool[tdIndex])->RemoveSocketObj(controlHandle);

			return true;
		}


	}
	return false;
}
void MightyTCPOverlappedEventServer::startWork()
{
	m_pAcceptThread=new OverlappedEventExecute();
	m_pAcceptThread->start(this);
	m_pAcceptThread->setExecuteSignal();
}

POverlappedEventIOObject MightyTCPOverlappedEventServer::GetSocketObj(SOCKET s)
{
	POverlappedEventIOObject pSocket =NULL;
	pSocket=new OverlappedEventIOObject();
	if(pSocket != NULL)
	{
		pSocket->s = s;
	}
	return pSocket;

}

int MightyTCPOverlappedEventServer::acceptSocketObject()
{
	// 创建用来重新建立g_events数组的事件对象
	m_Notify_Handles[0] = ::WSACreateEvent();

	// 在此可以投递多个接受I/O请求
	for(int i=0; i<SINGLE_ORDER_MAX; i++)
	{
		PostAccept(GetBufferObj(m_pListenObj, BUFFER_SIZE));
	}
	::WSASetEvent(m_Notify_Handles[0]);

	while(m_exitNotify==0)
	{
		int nIndex = ::WSAWaitForMultipleEvents(m_nEffectiveHandleCount+1,m_Notify_Handles,FALSE,WSA_INFINITE,FALSE);
		if(nIndex == WSA_WAIT_FAILED)
		{
			break;
		}
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		for(int i=0; i<=nIndex; i++)
		{
			int nRet = ::WSAWaitForMultipleEvents(1,&m_Notify_Handles[i],TRUE,0,FALSE);
			if(nRet == WSA_WAIT_TIMEOUT)
				continue;
			else
			{
				::WSAResetEvent(m_Notify_Handles[i]);
				// 重新建立g_events数组
				if(i == 0)
				{
					RecombinationArray();
					continue;
				}

				// 处理这个I/O
				POverlappedEvent_BufObj pBuffer = FindBufferObj(m_Notify_Handles[i]);
				if(pBuffer != NULL)
				{
					if(ExecuteProcess(pBuffer)==-1)
						RecombinationArray();
				}
			}
		}
	}
	return 0;

}
void MightyTCPOverlappedEventServer::AssignToFreeThread(POverlappedEventIOObject pSocket)
{
		if (m_pThreadPool.empty())
		{
			HandleIOOverlappedEventServer * pIOThread=new HandleIOOverlappedEventServer();
			pIOThread->InsertSocketObj(pSocket);
			pIOThread->start(this);
			pIOThread->setExecuteSignal();
			m_tdPoolIndex=0;

		}
		else
		{
			int rtCode=0;
			rtCode=((HandleIOOverlappedEventServer *)m_pThreadPool[m_tdPoolIndex])->InsertSocketObj(pSocket);
			if (rtCode==-1)
			{
				HandleIOOverlappedEventServer * pIOThread=new HandleIOOverlappedEventServer();
				pIOThread->InsertSocketObj(pSocket);
				pIOThread->start(this);
				pIOThread->setExecuteSignal();
				m_tdPoolIndex++;

			}

		}
	
}
void MightyTCPOverlappedEventServer::FreeSocketObj(POverlappedEventIOObject pSocket)
{
	if(pSocket->s != INVALID_SOCKET)
		::closesocket(pSocket->s);
	delete pSocket;
}

POverlappedEvent_BufObj MightyTCPOverlappedEventServer::GetBufferObj(POverlappedEventIOObject pSocket, ULONG nLen)
{
	if(m_nEffectiveHandleCount > WSA_MAXIMUM_WAIT_EVENTS - 1)
		return NULL;

	POverlappedEvent_BufObj pBuffer = new OverlappedEvent_BufObj();
	if(pBuffer != NULL)
	{
		//pBuffer->buff = (char*)::GlobalAlloc(GPTR, nLen);
		pBuffer->ol.hEvent = ::WSACreateEvent();
		pBuffer->pSocket = pSocket;
		pBuffer->sAccept = INVALID_SOCKET;

		m_overLappedBufList.push_back(pBuffer);
		m_Notify_Handles[++m_nEffectiveHandleCount] = pBuffer->ol.hEvent;
	}
	return pBuffer;
}

void MightyTCPOverlappedEventServer::FreeBufferObj(POverlappedEvent_BufObj pBuffer)
{

	list<POverlappedEvent_BufObj>::iterator leftIter,EndIter;
	if (!m_overLappedBufList.empty())
	{
		EndIter=m_overLappedBufList.end();
		leftIter=m_overLappedBufList.begin();
		while(leftIter!=EndIter)
		{
			if ((*leftIter)==pBuffer)
			{
				m_overLappedBufList.erase(leftIter);
				::CloseHandle(pBuffer->ol.hEvent);
				delete pBuffer;
				m_nEffectiveHandleCount--;
				break;
			}
			leftIter++;

		}
	}


}

POverlappedEvent_BufObj MightyTCPOverlappedEventServer::FindBufferObj(HANDLE hEvent)
{
	POverlappedEvent_BufObj pBuffer = NULL;
	list<POverlappedEvent_BufObj>::iterator leftIter,EndIter;
	if (!m_overLappedBufList.empty())
	{

		EndIter=m_overLappedBufList.end();
		leftIter=m_overLappedBufList.begin();
		while(leftIter!=EndIter)
		{
			if((*leftIter)->ol.hEvent == hEvent)
			{
				m_overLappedBufList.erase(leftIter);
				return pBuffer=(*leftIter);
			}
			leftIter++;

		}
	}
	return pBuffer;
}



int MightyTCPOverlappedEventServer::PostAccept(POverlappedEvent_BufObj pBuffer)
{
	POverlappedEventIOObject pSocket = pBuffer->pSocket;
	if(pSocket->lpfnAcceptEx != NULL)
	{	
		// 设置I/O类型，增加套节字上的重叠I/O计数
		pBuffer->nOperation = OP_ACCEPT;
		pSocket->nOutstandingOps ++;

		// 投递此重叠I/O  
		DWORD dwBytes;
		pBuffer->sAccept =::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		BOOL b = pSocket->lpfnAcceptEx(pSocket->s, 
			pBuffer->sAccept,
			pBuffer->buff, 
			BUFFER_SIZE - ((sizeof(sockaddr_in) + 16) * 2),
			sizeof(sockaddr_in) + 16, 
			sizeof(sockaddr_in) + 16, 
			&dwBytes, 
			&pBuffer->ol);
		if(!b)
		{
			if(::WSAGetLastError() != WSA_IO_PENDING)
				return -1;
		}
		return 0;
	}
	return -1;
};

int MightyTCPOverlappedEventServer::PostRecv(POverlappedEvent_BufObj pBuffer)
{	
	// 设置I/O类型，增加套节字上的重叠I/O计数
	pBuffer->nOperation = OP_READ;
	pBuffer->pSocket->nOutstandingOps ++;

	// 投递此重叠I/O
	DWORD dwBytes;
	DWORD dwFlags = 0;
	WSABUF buf;
	buf.buf = pBuffer->buff;
	buf.len = pBuffer->nLen;
	if(::WSARecv(pBuffer->pSocket->s, &buf, 1, &dwBytes, &dwFlags, &pBuffer->ol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return 0;
}

int MightyTCPOverlappedEventServer::PostSend(POverlappedEvent_BufObj pBuffer)
{
	// 设置I/O类型，增加套节字上的重叠I/O计数
	pBuffer->nOperation = OP_WRITE;
	pBuffer->pSocket->nOutstandingOps ++;

	// 投递此重叠I/O
	DWORD dwBytes;
	DWORD dwFlags = 0;
	WSABUF buf;
	buf.buf = pBuffer->buff;
	buf.len = pBuffer->nLen;
	if(::WSASend(pBuffer->pSocket->s, 
		&buf, 1, &dwBytes, dwFlags, &pBuffer->ol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
			return -1;
	}
	return 0;
}

int MightyTCPOverlappedEventServer::ExecuteProcess(POverlappedEvent_BufObj pBuffer)
{
	POverlappedEventIOObject pSocket = pBuffer->pSocket; // 从BUFFER_OBJ对象中提取SOCKET_OBJ对象指针，为的是方便引用
	pSocket->nOutstandingOps --;

	// 获取重叠操作结果
	DWORD dwTrans;
	DWORD dwFlags;
	BOOL bRet = ::WSAGetOverlappedResult(pSocket->s, &pBuffer->ol, &dwTrans, FALSE, &dwFlags);
	if(!bRet)
	{
		// 在此套节字上有错误发生，因此，关闭套节字，移除此缓冲区对象。
		// 如果没有其它抛出的I/O请求了，释放此缓冲区对象，否则，等待此套节字上的其它I/O也完成
		if(pSocket->s != INVALID_SOCKET)
		{
			::closesocket(pSocket->s);
			pSocket->s = INVALID_SOCKET;
		}

		if(pSocket->nOutstandingOps == 0)
			FreeSocketObj(pSocket);	

		FreeBufferObj(pBuffer);
		return -1;
	}

	// 没有错误发生，处理已完成的I/O
	switch(pBuffer->nOperation)
	{
		case OP_ACCEPT:	// 接收到一个新的连接，并接收到了对方发来的第一个封包
		{
			// 为新客户创建一个SOCKET_OBJ对象
			POverlappedEventIOObject pClient = GetSocketObj(pBuffer->sAccept);

			// 为发送数据创建一个BUFFER_OBJ对象，这个对象会在套节字出错或者关闭时释放
			/*POverlappedEvent_BufObj pSend = GetBufferObj(pClient, BUFFER_SIZE);	
			if(pSend == NULL)
			{
				
				FreeSocketObj(pClient);
				return -1;
			}
			RebuildArray();

			// 将数据复制到发送缓冲区
			pSend->nLen = dwTrans;
			memcpy(pSend->buff, pBuffer->buff, dwTrans);

			// 投递此发送I/O（将数据回显给客户）
			if(!PostSend(pSend))
			{
				// 万一出错的话，释放上面刚申请的两个对象
				FreeSocketObj(pSocket);	
				FreeBufferObj(pSend);
				return -1;
			}*/

			// 继续投递接受I/O
			PostAccept(pBuffer);
		}
		break;
		default:
			return -1;

	}
	return 0;
}

void MightyTCPOverlappedEventServer::RecombinationArray()
{
	list<POverlappedEvent_BufObj>::iterator leftIter,EndIter;
	int i =  1;
	if (!m_overLappedBufList.empty())
	{
		EndIter=m_overLappedBufList.end();
		leftIter=m_overLappedBufList.begin();
		while(leftIter!=EndIter)
		{
			m_Notify_Handles[i++]=(*leftIter)->ol.hEvent;
			leftIter++;

		}
	}
}