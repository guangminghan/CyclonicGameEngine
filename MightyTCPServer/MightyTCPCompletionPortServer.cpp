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
	// Accept����
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
		// ������ɶ˿ڶ���
		m_hCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

		// ������չ����AcceptEx
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

		// ������չ����GetAcceptExSockaddrs
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


		// �������׽��ֹ�������ɶ˿ڣ�ע�⣬����Ϊ�����ݵ�CompletionKeyΪ0
		::CreateIoCompletionPort((HANDLE)m_ListenSock, m_hCompletionPort, (DWORD)0, 0);

		// ע��FD_ACCEPT�¼���
		// ���Ͷ�ݵ�AcceptEx I/O�������̻߳���յ�FD_ACCEPT�����¼���˵��Ӧ��Ͷ�ݸ����AcceptEx I/O
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
	// �����¼��������飬�Ա����������WSAWaitForMultipleEvents����
	HANDLE hWaitEvents[2 + MAX_THREAD];
	int nEventCount = 0;
	hWaitEvents[nEventCount ++] =m_hAcceptEvent;
	hWaitEvents[nEventCount ++] =m_hRepostEvent;

	// ����ָ�������Ĺ����߳�����ɶ˿��ϴ���I/O
	for(i=0; i<MAX_THREAD; i++)
	{
		HandleIOCompletionPortServer * pIOThread=new HandleIOCompletionPortServer();
		pIOThread->start(this);
		pIOThread->setExecuteSignal();
		m_tdPoolIndex++;
		hWaitEvents[nEventCount ++] =pIOThread->GetThreadHandle();
	}

	// �����������ѭ���������¼����������е��¼�
	while(TRUE)
	{
		int nIndex = ::WSAWaitForMultipleEvents(nEventCount, hWaitEvents, FALSE, 60*1000, FALSE);

		// ���ȼ���Ƿ�Ҫֹͣ����
		if((m_exitNotify==1) || (nIndex == WSA_WAIT_FAILED))
		{
			// �ر���������
			//CloseAllConnections();
			::Sleep(0);		// ��I/O�����߳�һ��ִ�еĻ���
			// �رռ����׽���
			::closesocket(m_ListenSock);
			m_ListenSock= INVALID_SOCKET;
			::Sleep(0);		// ��I/O�����߳�һ��ִ�еĻ���

			// ֪ͨ����I/O�����߳��˳�
			for(int i=2; i<MAX_THREAD + 2; i++)
			{	
				::PostQueuedCompletionStatus(m_hCompletionPort, -1, 0, NULL);
			}

			// �ȴ�I/O�����߳��˳�
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

		//��ʱ�������δ���ص�AcceptEx I/O�����ӽ����˶೤ʱ��
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
					nLimit = 50;  // ���ӵĸ�����������Ϊ50��
				}
			}
			else if(nIndex == 1)
			{
				nLimit = InterlockedExchange(&m_nRepostCount, 0);
			}
			else if(nIndex > 1)		// I/O�����߳��˳���˵���д��������رշ�����
			{
				m_exitNotify=1;
				continue;
			}

			// Ͷ��nLimit��AcceptEx I/O����
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
		// �ڹ���������ɶ˿ڵ������׽����ϵȴ�I/O���
		BOOL bOK = ::GetQueuedCompletionStatus(m_hCompletionPort,&dwTrans, (LPDWORD)&dwKey, (LPOVERLAPPED*)&lpol, WSA_INFINITE);

		if(dwTrans == -1) // �û�֪ͨ�˳�
		{
			::ExitThread(0);
		}
		pBuffer = CONTAINING_RECORD(lpol, CompletionPort_BufObj, wsaol);
		int nError = NO_ERROR;
		if(!bOK)						// �ڴ��׽������д�����
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
	else			// �Ѿ��ﵽ���ֵ���������ͷ��ڴ�
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
	// ����I/O����
	pBuffer->nOperation = OP_ACCEPT;

	// Ͷ�ݴ��ص�I/O  
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
	// ����I/O����
	pBuffer->nOperation = OP_READ;	

	

	// �������к�
	pBuffer->nSequenceNumber = pBuffer->pSocket->nReadSequence;

	// Ͷ�ݴ��ص�I/O
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

	// �����׽����ϵ��ص�I/O�����Ͷ����кż���

	pBuffer->pSocket->nOutstandingRecv ++;
	pBuffer->pSocket->nReadSequence ++;
	return TRUE;
}

BOOL MightyTCPCompletionPortServer::PostSend(PCompletionPort_BufObj pBuffer)
{	
	// ����Ͷ�ݵķ��͵���������ֹ�û����������ݶ������գ����·������׳��������Ͳ���
	//if(pContext->nOutstandingSend > m_nMaxSends)
		//return FALSE;

	// ����I/O���ͣ������׽����ϵ��ص�I/O����
	pBuffer->nOperation = OP_WRITE;

	// Ͷ�ݴ��ص�I/O
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

	// �����׽����ϵ��ص�I/O����
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
		
		// ȡ�����ӽ�����ʱ��
		::getsockopt(pBufProcess->sAccept,SOL_SOCKET, SO_CONNECT_TIME, (char *)&nSeconds, &nLen);	
		// �������2���ӿͻ��������ͳ�ʼ���ݣ���������ͻ�go away
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

	//���ȼ����׽����ϵ�δ��I/O����
	if(pContext != NULL)
	{
		if(pBuffer->nOperation == OP_READ)
			pContext->nOutstandingRecv --;
		else if(pBuffer->nOperation == OP_WRITE)
			pContext->nOutstandingSend --;



		// ����׽����Ƿ��Ѿ������ǹر�
		if(pContext->nCondition) 
		{

			if(pContext->nOutstandingRecv == 0 && pContext->nOutstandingSend == 0)
			{		
				RemoveSocketObj(pContext);
			}
			// �ͷ��ѹر��׽��ֵ�δ��I/O
			ReleaseBuffer(pBuffer);	
			return;
		}
	}
	else
	{
		RemovePendingAccept(pBuffer);
	}

	//����׽����Ϸ����Ĵ�������еĻ���֪ͨ�û���Ȼ��ر��׽���
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
		else // �ڼ����׽����Ϸ�������Ҳ���Ǽ����׽��ִ���Ŀͻ�������
		{
			// �ͻ��˳����ͷ�I/O������
			if(pBuffer->sAccept!= INVALID_SOCKET)
			{
				::closesocket(pBuffer->sAccept);
				pBuffer->sAccept= INVALID_SOCKET;
			}		
		}

		ReleaseBuffer(pBuffer);
		return;
	}


	// ��ʼ����
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
			// Ϊ�½��ܵ���������ͻ������Ķ���
			PCompletionPortIOObject pClient =GetSocketObj(pBuffer->sAccept);
			if(pClient != NULL)
			{
				if(InsertSocketObj(pClient)==0)
				{	
					// ȡ�ÿͻ���ַ
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

					// ���������ӵ���ɶ˿ڶ���
					::CreateIoCompletionPort((HANDLE)pClient->s, m_hCompletionPort, (DWORD)pClient, 0);

					// ֪ͨ�û�
					pBuffer->nLen = dwTrans;
					

					m_pNotifyCallBack->OnRecvData(pBuffer->buff,dwTrans,pClient->identityIDKey);

					// ��������Ͷ�ݼ���Read������Щ�ռ����׽��ֹرջ����ʱ�ͷ�
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
				else	// ���������������ر�����
				{
					RemoveSocketObj(pClient);
					
				}
			}
			else
			{
				// ��Դ���㣬�ر���ͻ������Ӽ���
				::closesocket(pBuffer->sAccept);
				pBuffer->sAccept= INVALID_SOCKET;
			}
		}

		// Accept������ɣ��ͷ�I/O������
		ReleaseBuffer(pBuffer);	

		// ֪ͨ�����̼߳�����Ͷ��һ��Accept����
		::InterlockedIncrement(&m_nRepostCount);
		::SetEvent(m_hRepostEvent);
	}
	else if(pBuffer->nOperation == OP_READ)
	{
		if(dwTrans == 0)	// �Է��ر��׽���
		{
			// ��֪ͨ�û�
			pBuffer->nLen = 0;
			RemoveSocketObj(pContext);
			// �ͷſͻ������ĺͻ���������
			if(pContext->nOutstandingRecv == 0 && pContext->nOutstandingSend == 0)
			{		
				//ReleaseContext(pContext);
			}
			//ReleaseBuffer(pBuffer);	
		}
		else
		{
			pBuffer->nLen = dwTrans;
			// ����I/OͶ�ݵ�˳���ȡ���յ�������
			PCompletionPort_BufObj pNextIO = GetNextReadBuffer(pContext, pBuffer);
			while(pNextIO != NULL)
			{
				

				m_pNotifyCallBack->OnRecvData(pNextIO->buff,dwTrans,pContext->identityIDKey);
				// ����Ҫ�������кŵ�ֵ
				::InterlockedIncrement((LONG*)&pContext->nCurrentReadSequence);
				// �ͷ��������ɵ�I/O
				ReleaseBuffer(pNextIO);
				pNextIO= GetNextReadBuffer(pContext, NULL);
			}

			// ����Ͷ��һ���µĽ�������
			pBuffer = AllocateBuffer(pContext,4096);
			if(pBuffer == NULL || !PostRecv(pBuffer))
			{
				//closeAConnection(pContext);
			}
		}
	}
	else if(pBuffer->nOperation == OP_WRITE)
	{

		if(dwTrans == 0)	// �Է��ر��׽���
		{
			// ��֪ͨ�û�
			pBuffer->nLen = 0;
			RemoveSocketObj(pContext);
			// �ٹر�����
			//CloseAConnection(pContext);

			// �ͷſͻ������ĺͻ���������
			if(pContext->nOutstandingRecv == 0 && pContext->nOutstandingSend == 0)
			{		
				//ReleaseContext(pContext);
			}
			//ReleaseBuffer(pBuffer);	
		}
		else
		{
			// д������ɣ�֪ͨ�û�
			pBuffer->nLen = dwTrans;
			//OnWriteCompleted(pContext, pBuffer);
			// �ͷ�SendText��������Ļ�����
			ReleaseBuffer(pBuffer);
		}
	}
}

PCompletionPort_BufObj MightyTCPCompletionPortServer::GetNextReadBuffer(PCompletionPortIOObject pContext,PCompletionPort_BufObj pBuffer)
{
	PCompletionPort_BufObj currentBufIO=NULL;
	if(pBuffer != NULL)
	{
		// �����Ҫ������һ�����к���ȣ������黺����
		if(pBuffer->nSequenceNumber == pContext->nCurrentReadSequence)
		{
			return pBuffer;
		}
		// �������ȣ���˵��û�а�˳��������ݣ�����黺�������浽���ӵ�pOutOfOrderReads�б���

		// �б��еĻ������ǰ��������кŴ�С�����˳�����е�
		pContext->pBufQueue->push_back(pBuffer);

		pContext->pBufQueue->sort(SortList());
	}

	//list<PCompletionPort_BufObj>::iterator loopPos,endPos;
	//loopPos=pContext->pBufQueue->begin();
	//endPos=pContext->pBufQueue->end();
	// ����ͷԪ�ص����кţ������Ҫ�������к�һ�£��ͽ����ӱ����Ƴ������ظ��û�
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