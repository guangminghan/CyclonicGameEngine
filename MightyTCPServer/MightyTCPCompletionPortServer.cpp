#include ".\mightytcpcompletionportserver.h"
#include "../CommonInc/P2PUtilTools.h"
//------------------------------------------------------------------------------------
HandleIOCompletionPortServer::HandleIOCompletionPortServer(void)
{

}
HandleIOCompletionPortServer::~HandleIOCompletionPortServer(void)
{

}

int HandleIOCompletionPortServer::threadrun(void * pBuf)
{
	MightyTCPCompletionPortServer * ptrUser=(MightyTCPCompletionPortServer *)pBuf;

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
				//((HandleIOCompletionPortServer *)m_userInstance.threadThis_)->WorkProcess();
				ptrUser->WorkProcess();
				break;
			}
		default:
			break;
		}
	}
	return 0;
}

int HandleIOCompletionPortServer::ExecuteTask(void * pBuf)
{
	return 0;
}

int HandleIOCompletionPortServer::branchEvent(void * pBuf)
{
	return 0;
}


//-------------------------------------------------------------------------------------
CompletionPortExecute::CompletionPortExecute(void)
{

}
CompletionPortExecute::~CompletionPortExecute(void)
{

}

int CompletionPortExecute::threadrun(void * pBuf)
{
	MightyTCPCompletionPortServer * ptrUser=(MightyTCPCompletionPortServer *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;

	while(1)
	{
		DWORD Index = WaitForMultipleObjects(2,MightyEvent,FALSE,m_dwTimeOut);
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
	return 0;

}
int CompletionPortExecute::ExecuteTask(void * pBuf)
{
	return 0;
}

int CompletionPortExecute::branchEvent(void * pBuf)
{
	return 0;
}
//-------------------------------------------------------------------------------------
MightyTCPCompletionPortServer::MightyTCPCompletionPortServer(ITcpServerNotify * pNotify)
:m_pNotifyCallBack(pNotify)
{
	// Accept请求
	m_hAcceptEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hRepostEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_nRepostCount = 0;
}

MightyTCPCompletionPortServer::~MightyTCPCompletionPortServer(void)
{
}
bool MightyTCPCompletionPortServer::Create(const unsigned short & usPort)
{
		/*if (!(P2PUtilTools::is2KOS())) 
		{
			return false;
		}*/

		m_ListenPort=usPort;
		m_ListenSock = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		SOCKADDR_IN si;
		si.sin_family = AF_INET;
		si.sin_port = ::ntohs(usPort);
		si.sin_addr.S_un.S_addr = INADDR_ANY;

		if(::bind(m_ListenSock, (sockaddr*)&si, sizeof(si)) == SOCKET_ERROR)
		{
			return false;
		}
		::listen(m_ListenSock, 200);

		m_pListenObj= GetSocketObj(m_ListenSock);
		// 创建完成端口对象
		m_hCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

		// 加载扩展函数AcceptEx
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		DWORD dwBytes;
		::WSAIoctl(m_ListenSock, 
			SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&GuidAcceptEx, 
			sizeof(GuidAcceptEx),
			&m_lpfnAcceptEx, 
			sizeof(m_lpfnAcceptEx), 
			&dwBytes, 
			NULL, 
			NULL);

		// 加载扩展函数GetAcceptExSockaddrs
		GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
		::WSAIoctl(m_ListenSock,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidGetAcceptExSockaddrs,
			sizeof(GuidGetAcceptExSockaddrs),
			&m_lpfnGetAcceptExSockaddrs,
			sizeof(m_lpfnGetAcceptExSockaddrs),
			&dwBytes,
			NULL,
			NULL
			);


		// 将监听套节字关联到完成端口，注意，这里为它传递的CompletionKey为0
		::CreateIoCompletionPort((HANDLE)m_ListenSock, m_hCompletionPort, (DWORD)0, 0);

		// 注册FD_ACCEPT事件。
		// 如果投递的AcceptEx I/O不够，线程会接收到FD_ACCEPT网络事件，说明应该投递更多的AcceptEx I/O
		WSAEventSelect(m_ListenSock, m_hAcceptEvent, FD_ACCEPT);

		startWork();

		return 0;
}
void MightyTCPCompletionPortServer::Destroy(void)
{

}
bool MightyTCPCompletionPortServer::SendData(const char *pData, int nLen, const string & strLinkNo)
{
	
	PCompletionPortIOObject sendHandle=NULL;
	
	PCompletionPort_BufObj pSendBufRole=NULL;


	//map<string,PCompletionPortIOObject>::iterator leftPos,rightPos;
	//leftPos=m_ActiveSocketMap.begin();
	//rightPos=m_ActiveSocketMap.end();
	sendHandle=SearchSocketObj(strLinkNo);

	if (sendHandle!=NULL)
	{

		pSendBufRole=AllocateBuffer(sendHandle,nLen);
		if (pSendBufRole)
		{
			pSendBufRole->nLen = nLen;
			memcpy(pSendBufRole->buff,pData, nLen);
			PostSend(pSendBufRole);
			return true;
		}
	}

	return false;
}
bool MightyTCPCompletionPortServer::CloseLink(const string & strLinkNo)
{
	PCompletionPortIOObject controlHandle=NULL;
	controlHandle=SearchSocketObj(strLinkNo);
	if (controlHandle!=NULL)
	{
		RemoveSocketObj(controlHandle);

		return true;
	}

	return false;
}

void MightyTCPCompletionPortServer::startWork()
{
	m_pAcceptThread=new CompletionPortExecute();
	m_pAcceptThread->start(this);
	m_pAcceptThread->setExecuteSignal();
}
int MightyTCPCompletionPortServer::acceptSocketObject()
{
	int i=0;
	for(i=0; i<5; i++)
	{
		PCompletionPort_BufObj pUseBuf=AllocateBuffer(m_pListenObj,4096);
		InsertPendingAccept(pUseBuf);
		PostAccept(pUseBuf);
	}
	// 构建事件对象数组，以便在上面调用WSAWaitForMultipleEvents函数
	HANDLE hWaitEvents[2 + MAX_THREAD];
	int nEventCount = 0;
	hWaitEvents[nEventCount ++] =m_hAcceptEvent;
	hWaitEvents[nEventCount ++] =m_hRepostEvent;

	// 创建指定数量的工作线程在完成端口上处理I/O
	for(i=0; i<MAX_THREAD; i++)
	{
		HandleIOCompletionPortServer * pIOThread=new HandleIOCompletionPortServer();
		pIOThread->start(this);
		pIOThread->setExecuteSignal();
		m_tdPoolIndex++;
		hWaitEvents[nEventCount ++] =pIOThread->GetThreadHandle();
	}

	// 下面进入无限循环，处理事件对象数组中的事件
	while(TRUE)
	{
		int nIndex = ::WSAWaitForMultipleEvents(nEventCount, hWaitEvents, FALSE, 60*1000, FALSE);

		// 首先检查是否要停止服务
		if((m_exitNotify==1) || (nIndex == WSA_WAIT_FAILED))
		{
			// 关闭所有连接
			//CloseAllConnections();
			::Sleep(0);		// 给I/O工作线程一个执行的机会
			// 关闭监听套节字
			::closesocket(m_ListenSock);
			m_ListenSock= INVALID_SOCKET;
			::Sleep(0);		// 给I/O工作线程一个执行的机会

			// 通知所有I/O处理线程退出
			for(int i=2; i<MAX_THREAD + 2; i++)
			{	
				::PostQueuedCompletionStatus(m_hCompletionPort, -1, 0, NULL);
			}

			// 等待I/O处理线程退出
			/*::WaitForMultipleObjects(MAX_THREAD, &hWaitEvents[2], TRUE, 5*1000);

			for(i=2; i<MAX_THREAD + 2; i++)
			{	
				::CloseHandle(hWaitEvents[i]);
			}*/
			unsigned long leftIndex,RightIndex;
			RightIndex=m_pThreadPool.size();
			for (leftIndex=0;leftIndex<RightIndex;leftIndex++)
			{
				delete ((HandleIOCompletionPortServer *)m_pThreadPool[leftIndex]);
				Sleep(10);
			}
				

			::CloseHandle(m_hCompletionPort);


			::ExitThread(0);
		}	

		//定时检查所有未返回的AcceptEx I/O的连接建立了多长时间
		if(nIndex == WSA_WAIT_TIMEOUT)
		{
			CheckPostAcceptTimeOut();
		}
		else
		{
			nIndex = nIndex - WAIT_OBJECT_0;
			WSANETWORKEVENTS ne;
			int nLimit=0;
			if(nIndex == 0)
			{
				::WSAEnumNetworkEvents(m_ListenSock,hWaitEvents[nIndex], &ne);
				if(ne.lNetworkEvents & FD_ACCEPT)
				{
					nLimit = 50;  // 增加的个数，这里设为50个
				}
			}
			else if(nIndex == 1)
			{
				nLimit = InterlockedExchange(&m_nRepostCount, 0);
			}
			else if(nIndex > 1)		// I/O服务线程退出，说明有错误发生，关闭服务器
			{
				m_exitNotify=1;
				continue;
			}

			// 投递nLimit个AcceptEx I/O请求
			int i = 0;
			while(i++<nLimit)
			{
				PCompletionPort_BufObj pBeforeHandBuf=AllocateBuffer(m_pListenObj,4096);
				if (pBeforeHandBuf!=NULL)
				{
					InsertPendingAccept(pBeforeHandBuf);
					PostAccept(pBeforeHandBuf);
				}

			}
		}
	}
	return 0;
}
PCompletionPortIOObject MightyTCPCompletionPortServer::GetSocketObj(SOCKET s)
{
	PCompletionPortIOObject pSocket =NULL;

	pSocket=new CompletionPortIOObject();
	if(pSocket != NULL)
	{
		pSocket->s = s;
	}
	return pSocket;

}
int MightyTCPCompletionPortServer::InsertSocketObj(PCompletionPortIOObject pSocket)
{

		//GUID guid_val;
		string guid_strkey=P2PUtilTools::GetStringUuid();
		pSocket->identityIDKey=guid_strkey;
		m_ActiveSocketMap.insert(map<string,PCompletionPortIOObject>::value_type(guid_strkey,pSocket));
		//nSocketCount++;
		PCompletionPort_BufObj pIOBuf=AllocateBuffer(pSocket,4096);
		PostRecv(pIOBuf);
		m_pNotifyCallBack->OnAccept(guid_strkey);
		return 0;

	
}
void MightyTCPCompletionPortServer::RemoveSocketObj(PCompletionPortIOObject pSocket)
{
	m_pNotifyCallBack->OnClosed(pSocket->identityIDKey);
    map<string,PCompletionPortIOObject>::iterator itorRight,itorIndex;

	itorRight=m_ActiveSocketMap.end();
	itorIndex=m_ActiveSocketMap.begin();

	while (itorIndex!=itorRight)
	{
		if(pSocket==(itorIndex->second))
		{
			if(pSocket->s != INVALID_SOCKET)
			{
				::closesocket(pSocket->s);
				pSocket->s = INVALID_SOCKET;
			}
			if((pSocket->nOutstandingRecv == 0)&&(pSocket->nOutstandingSend == 0))
			{
				delete pSocket;
				m_ActiveSocketMap.erase(itorIndex++);
				
			}

			break;
		}
		itorIndex++;
	}
}
PCompletionPortIOObject MightyTCPCompletionPortServer::SearchSocketObj(const string & keyId)
{
	map<string,PCompletionPortIOObject>::iterator leftPos,rightPos;
	rightPos=m_ActiveSocketMap.end();
	if ((leftPos=m_ActiveSocketMap.find(keyId))!=rightPos)
	{
		return leftPos->second;
	}
	else
	{
		return NULL;
	}
}
void MightyTCPCompletionPortServer::DestroyAllSocket()
{
	map<string,PCompletionPortIOObject>::iterator itorRight,itorIndex;
	itorRight=m_ActiveSocketMap.end();
	itorIndex=m_ActiveSocketMap.begin();
	while (itorIndex!=itorRight)
	{
		if ((itorIndex->second))
		{
			if(itorIndex->second->s != INVALID_SOCKET)
			{
				::closesocket(itorIndex->second->s);
				itorIndex->second->s=NULL;
			}


			delete (itorIndex->second);
		}

		m_ActiveSocketMap.erase(itorIndex++);
		//itorIndex= m_SocketMap.erase(itorIndex);
		//nSocketCount--;
		//itorIndex++;
	}

}

int MightyTCPCompletionPortServer::WorkProcess()
{
	PCompletionPort_BufObj pBuffer;
	DWORD dwKey;
	DWORD dwTrans;
	LPOVERLAPPED lpol;
	while(m_exitNotify==0)
	{
		// 在关联到此完成端口的所有套节字上等待I/O完成
		BOOL bOK = ::GetQueuedCompletionStatus(m_hCompletionPort,&dwTrans, (LPDWORD)&dwKey, (LPOVERLAPPED*)&lpol, WSA_INFINITE);

		if(dwTrans == -1) // 用户通知退出
		{
			::ExitThread(0);
		}
		pBuffer = CONTAINING_RECORD(lpol, CompletionPort_BufObj, wsaol);
		int nError = NO_ERROR;
		if(!bOK)						// 在此套节字上有错误发生
		{
			SOCKET s;
			if(pBuffer->nOperation == OP_ACCEPT)
			{
				s =m_ListenSock;
			}
			else
			{
				if(dwKey == 0)
					break;
				s = ((PCompletionPortIOObject)dwKey)->s;
			}
			DWORD dwFlags = 0;
			if(!::WSAGetOverlappedResult(s, &pBuffer->wsaol, &dwTrans, FALSE, &dwFlags))
			{
				nError = ::WSAGetLastError();
			}
		}
		HandleIO(dwKey, pBuffer, dwTrans, nError);
	}
	return 0;
}

PCompletionPort_BufObj MightyTCPCompletionPortServer::AllocateBuffer(PCompletionPortIOObject pSocket,int nLen)
{
	PCompletionPort_BufObj pBuffer =NULL;
	if (m_postAcceptBufList.empty())
	{
		pBuffer = new CompletionPort_BufObj();
		pBuffer->buff=new char[nLen];
		pBuffer->nLen=nLen;
	}
	else
	{
		pBuffer=m_postAcceptBufList.front();
		m_postAcceptBufList.pop_front();
	}
	if(pBuffer != NULL)
	{
		pBuffer->pSocket = pSocket;
		pBuffer->sAccept = INVALID_SOCKET;
	}
	return pBuffer;
}

void MightyTCPCompletionPortServer::ReleaseBuffer(PCompletionPort_BufObj pBuffer)
{

	if(m_nFreeBufferCount <= m_nMaxFreeBuffers)
	{
		m_postAcceptBufList.push_back(pBuffer);
		m_nFreeBufferCount ++ ;
	}
	else			// 已经达到最大值，真正的释放内存
	{
		if (pBuffer)
		{
			if (pBuffer->buff)
			{
				delete [] pBuffer->buff;

			}
			delete pBuffer;
		}
	}
}

void MightyTCPCompletionPortServer::CloseAllConnections()
{

	map<string,PCompletionPortIOObject>::iterator itorRight,itorIndex;
	itorRight=m_ActiveSocketMap.end();
	itorIndex=m_ActiveSocketMap.begin();
	while (itorIndex!=itorRight)
	{
		if ((itorIndex->second))
		{
			if(itorIndex->second->s != INVALID_SOCKET)
			{
				::closesocket(itorIndex->second->s);
				itorIndex->second->s=INVALID_SOCKET;
				itorIndex->second->nCondition=1;

			}
			delete (itorIndex->second);
		}
		m_ActiveSocketMap.erase(itorIndex++);
	}
}
//-----------------------------------
BOOL MightyTCPCompletionPortServer::PostAccept(PCompletionPort_BufObj pBuffer)
{
	// 设置I/O类型
	pBuffer->nOperation = OP_ACCEPT;

	// 投递此重叠I/O  
	DWORD dwBytes;
	pBuffer->sAccept = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	BOOL b = m_lpfnAcceptEx(pBuffer->pSocket->s, 
		pBuffer->sAccept,
		pBuffer->buff, 
		pBuffer->nLen - ((sizeof(sockaddr_in) + 16) * 2),
		sizeof(sockaddr_in) + 16, 
		sizeof(sockaddr_in) + 16, 
		&dwBytes, 
		&pBuffer->wsaol);
	if(!b && ::WSAGetLastError() != WSA_IO_PENDING)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL MightyTCPCompletionPortServer::PostRecv(PCompletionPort_BufObj pBuffer)
{
	// 设置I/O类型
	pBuffer->nOperation = OP_READ;	

	

	// 设置序列号
	pBuffer->nSequenceNumber = pBuffer->pSocket->nReadSequence;

	// 投递此重叠I/O
	DWORD dwBytes;
	DWORD dwFlags = 0;
	//WSABUF buf;
	//buf.buf = pBuffer->buff;
	//buf.len = pBuffer->nLen;
	
	if(::WSARecv(pBuffer->pSocket->s, &pBuffer->wsaDataBuf, 1, &dwBytes, &dwFlags, &pBuffer->wsaol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
		{
			
			return FALSE;
		}
	}

	// 增加套节字上的重叠I/O计数和读序列号计数

	pBuffer->pSocket->nOutstandingRecv ++;
	pBuffer->pSocket->nReadSequence ++;
	return TRUE;
}

BOOL MightyTCPCompletionPortServer::PostSend(PCompletionPort_BufObj pBuffer)
{	
	// 跟踪投递的发送的数量，防止用户仅发送数据而不接收，导致服务器抛出大量发送操作
	//if(pContext->nOutstandingSend > m_nMaxSends)
		//return FALSE;

	// 设置I/O类型，增加套节字上的重叠I/O计数
	pBuffer->nOperation = OP_WRITE;

	// 投递此重叠I/O
	DWORD dwBytes;
	DWORD dwFlags = 0;
	//WSABUF buf;
	//buf.buf = pBuffer->buff;
	//buf.len = pBuffer->nLen;
	if(::WSASend(pBuffer->pSocket->s, 
		&pBuffer->wsaDataBuf, 1, &dwBytes, dwFlags, &pBuffer->wsaol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
			return FALSE;
	}	

	// 增加套节字上的重叠I/O计数
	//::EnterCriticalSection(&pContext->Lock);
	pBuffer->pSocket->nOutstandingSend ++;
	//::LeaveCriticalSection(&pContext->Lock);

	return TRUE;
}


BOOL MightyTCPCompletionPortServer::InsertPendingAccept(PCompletionPort_BufObj pBuffer)
{

	m_pPendingAcceptList.push_back(pBuffer);
	return TRUE;
}
BOOL MightyTCPCompletionPortServer::RemovePendingAccept(PCompletionPort_BufObj pBuffer)
{
	list<PCompletionPort_BufObj>::iterator leftPos,EndPos;
	leftPos=m_pPendingAcceptList.begin();
	EndPos=m_pPendingAcceptList.end();
	while (leftPos!=EndPos)
	{
		if ((*leftPos)==pBuffer)
		{
			m_pPendingAcceptList.erase(leftPos++);
			break;
		}
		leftPos++;

	}
	return TRUE;


}
void MightyTCPCompletionPortServer::CheckPostAcceptTimeOut()
{
	PCompletionPort_BufObj pBufProcess;
	PCompletionPortIOObject pClintSocketIO;
	list<PCompletionPort_BufObj>::iterator leftPos,EndPos;
	leftPos=m_pPendingAcceptList.begin();
	EndPos=m_pPendingAcceptList.end();
	while (leftPos!=EndPos)
	{
		pBufProcess=(*leftPos);
		pClintSocketIO=pBufProcess->pSocket;
		int nSeconds;
		int nLen = sizeof(nSeconds);
		
		// 取得连接建立的时间
		::getsockopt(pBufProcess->sAccept,SOL_SOCKET, SO_CONNECT_TIME, (char *)&nSeconds, &nLen);	
		// 如果超过2分钟客户还不发送初始数据，就让这个客户go away
		if(nSeconds != -1 && nSeconds > 2*60)
		{   
			closesocket(pBufProcess->sAccept);
			pBufProcess->sAccept = INVALID_SOCKET;
		}

		leftPos++;
	}
}

void MightyTCPCompletionPortServer::HandleIO(DWORD dwKey, PCompletionPort_BufObj pBuffer, DWORD dwTrans, int nError)
{
	PCompletionPortIOObject pContext = (PCompletionPortIOObject)dwKey;

	//首先减少套节字上的未决I/O计数
	if(pContext != NULL)
	{
		if(pBuffer->nOperation == OP_READ)
			pContext->nOutstandingRecv --;
		else if(pBuffer->nOperation == OP_WRITE)
			pContext->nOutstandingSend --;



		// 检查套节字是否已经被我们关闭
		if(pContext->nCondition) 
		{

			if(pContext->nOutstandingRecv == 0 && pContext->nOutstandingSend == 0)
			{		
				RemoveSocketObj(pContext);
			}
			// 释放已关闭套节字的未决I/O
			ReleaseBuffer(pBuffer);	
			return;
		}
	}
	else
	{
		RemovePendingAccept(pBuffer);
	}

	//检查套节字上发生的错误，如果有的话，通知用户，然后关闭套节字
	if(nError != NO_ERROR)
	{
		if(pBuffer->nOperation != OP_ACCEPT)
		{
			//if(nError==ERROR_NETNAME_DELETED)
			RemoveSocketObj(pContext);
			if(pContext->nOutstandingRecv == 0 && pContext->nOutstandingSend == 0)
			{		
				//ReleaseContext(pContext);
			}

		}
		else // 在监听套节字上发生错误，也就是监听套节字处理的客户出错了
		{
			// 客户端出错，释放I/O缓冲区
			if(pBuffer->sAccept!= INVALID_SOCKET)
			{
				::closesocket(pBuffer->sAccept);
				pBuffer->sAccept= INVALID_SOCKET;
			}		
		}

		ReleaseBuffer(pBuffer);
		return;
	}


	// 开始处理
	if(pBuffer->nOperation == OP_ACCEPT)
	{
		if(dwTrans == 0)
		{
			if(pBuffer->sAccept!= INVALID_SOCKET)
			{
				::closesocket(pBuffer->sAccept);
				pBuffer->sAccept= INVALID_SOCKET;
			}
		}
		else
		{
			// 为新接受的连接申请客户上下文对象
			PCompletionPortIOObject pClient =GetSocketObj(pBuffer->sAccept);
			if(pClient != NULL)
			{
				if(InsertSocketObj(pClient)==0)
				{	
					// 取得客户地址
					int nLocalLen, nRmoteLen;
					LPSOCKADDR pLocalAddr, pRemoteAddr;
					m_lpfnGetAcceptExSockaddrs(
						pBuffer->buff,
						pBuffer->nLen - ((sizeof(sockaddr_in) + 16) * 2),
						sizeof(sockaddr_in) + 16,
						sizeof(sockaddr_in) + 16,
						(SOCKADDR **)&pLocalAddr,
						&nLocalLen,
						(SOCKADDR **)&pRemoteAddr,
						&nRmoteLen);
					memcpy(&pClient->addrLocal, pLocalAddr, nLocalLen);
					memcpy(&pClient->addrRemote, pRemoteAddr, nRmoteLen);

					// 关联新连接到完成端口对象
					::CreateIoCompletionPort((HANDLE)pClient->s, m_hCompletionPort, (DWORD)pClient, 0);

					// 通知用户
					pBuffer->nLen = dwTrans;
					

					m_pNotifyCallBack->OnRecvData(pBuffer->buff,dwTrans,pClient->identityIDKey);

					// 向新连接投递几个Read请求，这些空间在套节字关闭或出错时释放
					for(int i=0; i<5; i++)
					{
						PCompletionPort_BufObj p = AllocateBuffer(pClient,2048);
						if(p != NULL)
						{
							if(!PostRecv(p))
							{
								RemoveSocketObj(pClient);
								break;
							}
						}
					}
				}
				else	// 连接数量已满，关闭连接
				{
					RemoveSocketObj(pClient);
					
				}
			}
			else
			{
				// 资源不足，关闭与客户的连接即可
				::closesocket(pBuffer->sAccept);
				pBuffer->sAccept= INVALID_SOCKET;
			}
		}

		// Accept请求完成，释放I/O缓冲区
		ReleaseBuffer(pBuffer);	

		// 通知监听线程继续再投递一个Accept请求
		::InterlockedIncrement(&m_nRepostCount);
		::SetEvent(m_hRepostEvent);
	}
	else if(pBuffer->nOperation == OP_READ)
	{
		if(dwTrans == 0)	// 对方关闭套节字
		{
			// 先通知用户
			pBuffer->nLen = 0;
			RemoveSocketObj(pContext);
			// 释放客户上下文和缓冲区对象
			if(pContext->nOutstandingRecv == 0 && pContext->nOutstandingSend == 0)
			{		
				//ReleaseContext(pContext);
			}
			//ReleaseBuffer(pBuffer);	
		}
		else
		{
			pBuffer->nLen = dwTrans;
			// 按照I/O投递的顺序读取接收到的数据
			PCompletionPort_BufObj pNextIO = GetNextReadBuffer(pContext, pBuffer);
			while(pNextIO != NULL)
			{
				

				m_pNotifyCallBack->OnRecvData(pNextIO->buff,dwTrans,pContext->identityIDKey);
				// 增加要读的序列号的值
				::InterlockedIncrement((LONG*)&pContext->nCurrentReadSequence);
				// 释放这个已完成的I/O
				ReleaseBuffer(pNextIO);
				pNextIO= GetNextReadBuffer(pContext, NULL);
			}

			// 继续投递一个新的接收请求
			pBuffer = AllocateBuffer(pContext,4096);
			if(pBuffer == NULL || !PostRecv(pBuffer))
			{
				//closeAConnection(pContext);
			}
		}
	}
	else if(pBuffer->nOperation == OP_WRITE)
	{

		if(dwTrans == 0)	// 对方关闭套节字
		{
			// 先通知用户
			pBuffer->nLen = 0;
			RemoveSocketObj(pContext);
			// 再关闭连接
			//CloseAConnection(pContext);

			// 释放客户上下文和缓冲区对象
			if(pContext->nOutstandingRecv == 0 && pContext->nOutstandingSend == 0)
			{		
				//ReleaseContext(pContext);
			}
			//ReleaseBuffer(pBuffer);	
		}
		else
		{
			// 写操作完成，通知用户
			pBuffer->nLen = dwTrans;
			//OnWriteCompleted(pContext, pBuffer);
			// 释放SendText函数申请的缓冲区
			ReleaseBuffer(pBuffer);
		}
	}
}

PCompletionPort_BufObj MightyTCPCompletionPortServer::GetNextReadBuffer(PCompletionPortIOObject pContext,PCompletionPort_BufObj pBuffer)
{
	PCompletionPort_BufObj currentBufIO=NULL;
	if(pBuffer != NULL)
	{
		// 如果与要读的下一个序列号相等，则读这块缓冲区
		if(pBuffer->nSequenceNumber == pContext->nCurrentReadSequence)
		{
			return pBuffer;
		}
		// 如果不相等，则说明没有按顺序接收数据，将这块缓冲区保存到连接的pOutOfOrderReads列表中

		// 列表中的缓冲区是按照其序列号从小到大的顺序排列的
		pContext->pBufQueue->push_back(pBuffer);

		pContext->pBufQueue->sort(SortList());
	}

	//list<PCompletionPort_BufObj>::iterator loopPos,endPos;
	//loopPos=pContext->pBufQueue->begin();
	//endPos=pContext->pBufQueue->end();
	// 检查表头元素的序列号，如果与要读的序列号一致，就将它从表中移除，返回给用户
	/*while(loopPos!=endPos)
	{
		if ((*loopPos)->nSequenceNumber==pContext->nCurrentReadSequence)
		{
			currentBufIO=(*loopPos);
			pContext->pBufQueue->erase(loopPos++);
		}
		loopPos++

	}*/
	//currentBufIO=(*loopPos);
	if (!pContext->pBufQueue->empty())
	{
		currentBufIO=pContext->pBufQueue->front();
		if ((currentBufIO!=NULL)&&(currentBufIO->nSequenceNumber==pContext->nCurrentReadSequence))
		{
			pContext->pBufQueue->pop_front();
			return currentBufIO;
		}
	}
	return NULL;
}

void MightyTCPCompletionPortServer::DisconnectActiveSocket(PCompletionPortIOObject pContext)
{
	LINGER lingerStruct;
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	setsockopt(pContext->s, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct) );

	// Now close the socket handle. This will do an abortive or graceful close, as requested. 
	CancelIo((HANDLE)pContext->s);

	closesocket(pContext->s);
	pContext->s= INVALID_SOCKET;

}
void setSocketNoDelay(PCompletionPortIOObject pContext)
{
	/* 
	* TCP_NODELAY	BOOL=TRUE Disables the "nagle algorithm for send coalescing" which delays
	* short packets in the hope that the application will send more data and allow
	* it to combine them into a single one to improve network efficiency.
	*/
	const char chOpt = 1;
	int nErr = setsockopt(pContext->s, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

}