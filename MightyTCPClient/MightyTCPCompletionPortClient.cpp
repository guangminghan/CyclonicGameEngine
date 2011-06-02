#include ".\mightytcpcompletionportclient.h"
#include "../CommonInc/P2PUtilTools.h"
//---------------------------------------------------------------
CBufferManage::CBufferManage(void)
{
	this->m_count = 0;
}

CBufferManage::~CBufferManage(void)
{
}
ICE_BUF *CBufferManage::AllocBuffer(int len)
{
	ICE_BUF *pRetValue = new ICE_BUF(len);

	if ( pRetValue->buffer.len != len ) {

		delete pRetValue;

		pRetValue = NULL;
	}
	return pRetValue;
}
void CBufferManage::FreeBuffer(ICE_BUF *pBuffer)
{
	if ( pBuffer != NULL ) delete pBuffer;
}

ICE_OVERLAPPED_BUF *CBufferManage::AllocOverLappedBuf(int len)
{
	ICE_OVERLAPPED_BUF *pRetValue = new ICE_OVERLAPPED_BUF(len);

	InterlockedIncrement(&this->m_count);

	if ( pRetValue->pBuffer == NULL ) {

		if ( len == 0 ) return pRetValue;

		delete pRetValue; return NULL;

	}

	if ( pRetValue->pBuffer->buffer.len != len ) {

		delete pRetValue; return NULL;

	}
	return pRetValue;
}
void CBufferManage::FreeOverLappedBuf(ICE_OVERLAPPED_BUF *pOverLappedBuf)
{

	InterlockedDecrement(&this->m_count);

	//K_PRINT("\n缓冲区个数为%d", this->m_count);

	if ( pOverLappedBuf != NULL ) delete pOverLappedBuf;

}

//---------------------------------------------------------------
HandleIOCompletionPortClient::HandleIOCompletionPortClient(void)
{

}
HandleIOCompletionPortClient::~HandleIOCompletionPortClient(void)
{
	exit();
}

int HandleIOCompletionPortClient::threadrun(void * pBuf)
{
	MightyTCPCompletionPortClient * ptrUser=(MightyTCPCompletionPortClient *)pBuf;

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
int HandleIOCompletionPortClient::ExecuteTask(void * pBuf)
{
	return 1;
}
int HandleIOCompletionPortClient::branchEvent(void * pBuf)
{
	return 1;
}
//--------------------------------------------------------------
CompletionPortClientExecute::CompletionPortClientExecute(void)
{

}
CompletionPortClientExecute::~CompletionPortClientExecute(void)
{
	exit();
}

int CompletionPortClientExecute::threadrun(void * pBuf)
{
	MightyTCPCompletionPortClient * ptrUser=(MightyTCPCompletionPortClient *)pBuf;

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
				ptrUser->ExecuteWork();
				break;
			}
		default:
			break;
		}
	}

}
int CompletionPortClientExecute::ExecuteTask(void * pBuf)
{
	return 1;
}
int CompletionPortClientExecute::branchEvent(void * pBuf)
{
	return 1;
}
//---------------------------------------------------------------
MightyTCPCompletionPortClient::MightyTCPCompletionPortClient(ITcpClientNotify* pNotify)
:m_pNotifyCallBack(pNotify)
{
	m_SocketContext=new CONN_CONTEXT();
	m_hCompletionPort=NULL;

}

MightyTCPCompletionPortClient::~MightyTCPCompletionPortClient(void)
{
}
bool MightyTCPCompletionPortClient::ConnectServer( const char* pServerAddr, unsigned short usPort)
{
	return true;
}
bool MightyTCPCompletionPortClient::SendData( const char* pData, int nLen)
{
	return true;
}
void MightyTCPCompletionPortClient::Disconnect(void)
{

}
void MightyTCPCompletionPortClient::ReleaseConnection()
{

}
bool MightyTCPCompletionPortClient::bind()
{

	this->m_hCompletionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE , NULL, 0, 0 );

	if ( this->m_hCompletionPort == NULL ) {
		return false;
	}
	return true;
}
int MightyTCPCompletionPortClient::init(const char * ip,WORD port)
{
	m_strAddress=ip;
	m_ConnectPort=port;
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr = inet_addr(ip);
	m_SockAddr.sin_port = htons(port);
	m_dwAddress=inet_addr(ip);

	return 0;

}
int MightyTCPCompletionPortClient::init(DWORD ip,WORD port)
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

int MightyTCPCompletionPortClient::afreshConnect()
{
	
	//if (m_SocketContext==NULL)
	//{
	//}
	if (m_SocketContext->s==INVALID_SOCKET)
	{
		m_SocketContext->s=WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);

		//WSAEventSelect(m_ClientSock,m_EventArray[0],FD_READ|FD_WRITE|FD_CLOSE|FD_ACCEPT|FD_CONNECT|FD_OOB);
		while(m_bRun)
		{
			if(SOCKET_ERROR==connect(m_SocketContext->s,(SOCKADDR*)(&m_SockAddr),sizeof(SOCKADDR_IN)))
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
int MightyTCPCompletionPortClient::Process()
{
	DWORD dwIoSize = 0;

	CONN_CONTEXT *lpClientContext = NULL;

	ICE_OVERLAPPED_BUF *pOverlapBuff = NULL;

	//HANDLE hCompletionPort = this->m_hCompletePort;

	if ( m_hCompletionPort == NULL ) 
		return false;

	while(m_bRun)
	{
		BOOL bIORet = GetQueuedCompletionStatus( m_hCompletionPort, &dwIoSize,(LPDWORD) &lpClientContext,	(LPOVERLAPPED*)&pOverlapBuff, 1000);

		if ( ! bIORet ) {

			DWORD errCode = GetLastError();

			if ( errCode != WAIT_TIMEOUT ) {

				//K_LOG("\nGetQueuedCompletionStatus Error %d ", errCode );

				if ( lpClientContext ) {

					this->disconnectClient( lpClientContext );

					//K_PRINT("\nlpClientContext = %.8X", (int)lpClientContext);

				}

				if ( pOverlapBuff ) {

					//this->m_bufferManager.FreeOverLappedBuf(pOverlapBuff);

					//K_PRINT("\npOverlapBuff = %.8X", (int)pOverlapBuff);
					pushOverlappCacheQueue(pOverlapBuff);
				}

			}

			//return 0;

		}

		if ( lpClientContext && pOverlapBuff ) {

			this->processIoMessage(pOverlapBuff, lpClientContext, dwIoSize);

		}

	}

	return 0;
}
int MightyTCPCompletionPortClient::ExecuteWork()
{
	list<P_STRUCT_ICE_OVERLAPPED_BUF>::iterator indexIter,endIter;

	m_pWsaOverLappedList.insert(m_pWsaOverLappedList.end(),m_pTaskWsaOverLappedList.begin(),m_pTaskWsaOverLappedList.end());
	m_pTaskWsaOverLappedList.clear();
	if (m_pWsaOverLappedList.empty())
	{
		PostRecvOpt();
		return 0;
	}
	indexIter=m_pWsaOverLappedList.begin();
	endIter=m_pWsaOverLappedList.end();
	while(indexIter!=endIter)
	{
		PostAsyncCompletePortMessage((*indexIter));
		indexIter++;
	}
	//m_pCacheWsaOverLappedList.insert(m_pCacheWsaOverLappedList.end(),m_pWsaOverLappedList.begin(),m_pWsaOverLappedList.end());
	m_pWsaOverLappedList.clear();
	return 0;
}
bool MightyTCPCompletionPortClient::PostAsyncCompletePortMessage(P_STRUCT_ICE_OVERLAPPED_BUF pStatePack)
{
	if ( ! PostQueuedCompletionStatus( this->m_hCompletionPort, 0, (ULONG_PTR)m_SocketContext, &pStatePack->overLapped) ){

		DWORD errCode = GetLastError();

		if ( errCode != ERROR_IO_PENDING ) {

			//K_LOG("\n PostQueuedCompletionStatus Error = %d", errCode);

			//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
			pushOverlappCacheQueue(pStatePack);
			return false;

		}

	}

	return true;
}
void MightyTCPCompletionPortClient::pushOverlappCacheQueue(P_STRUCT_ICE_OVERLAPPED_BUF pBackOverlapped)
{
	m_pCacheWsaOverLappedList.push_back(pBackOverlapped);
}
int MightyTCPCompletionPortClient::PostIOOpt(P_STRUCT_ICE_OVERLAPPED_BUF pReUseOverlapped,int isWait)
{
	DWORD flag=0;
	DWORD sendSize=0;
	int iState;
	//ZeroMemory(&pReUseOverlapped->m_PostOverLapped,sizeof(WSAOVERLAPPED));
	//pReUseOverlapped->m_EventArray[0]=WSACreateEvent();
	//pReUseOverlapped->m_PostOverLapped.hEvent=pReUseOverlapped->m_EventArray[0];
	//pReUseOverlapped->m_WsaIOBuf.len=sizeof(pReUseOverlapped->iobuf);
	//pReUseOverlapped->m_WsaIOBuf.buf=pReUseOverlapped->iobuf;
	//pReUseOverlapped->iotype=iostyle;
	if (isWait==1)
	{
		m_pTaskWsaOverLappedList.push_back(pReUseOverlapped);
		return 0;
	}
	else
	{
		if (pReUseOverlapped->nOperationType==ICE_IO_OPERATION_READ)
		{
			//pReUseOverlapped->SetOperator(ICE_IO_OPERATION_READ_COMPLETE);
			if ((WSARecv(m_SocketContext->s,&(pReUseOverlapped->pBuffer->buffer),1,&sendSize,&flag,(LPWSAOVERLAPPED)(pReUseOverlapped),NULL))==SOCKET_ERROR)
			{
				iState=WSAGetLastError();
				if (iState!=WSA_IO_PENDING)
				{

					return 1; 
				}

			}
			return 0;
		}
		else if (pReUseOverlapped->nOperationType==ICE_IO_OPERATION_WRITE)
		{
			//pReUseOverlapped->SetOperator(ICE_IO_OPERATION_WRITE_COMPLETE);
			if ((WSASend(m_SocketContext->s,&(pReUseOverlapped->pBuffer->buffer),1,&sendSize,flag,(LPWSAOVERLAPPED)(pReUseOverlapped),NULL))==SOCKET_ERROR)
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
	return 1;

}
int MightyTCPCompletionPortClient::PostRecvOpt()
{
	if (m_pCacheWsaOverLappedList.empty())
	{
		P_STRUCT_ICE_OVERLAPPED_BUF pOverLapStruct=new STRUCT_ICE_OVERLAPPED_BUF(4096);
		pOverLapStruct->pMainCall=this;
		//ZeroMemory(&pOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pOverLapStruct->m_PostOverLapped.hEvent=pOverLapStruct->m_EventArray[0];
		//pOverLapStruct->m_WsaIOBuf.len=sizeof(pOverLapStruct->iobuf);
		//pOverLapStruct->m_WsaIOBuf.buf=pOverLapStruct->iobuf;
		//pOverLapStruct->iotype=iorecv_id;
		//pOverLapStruct->useSize=sizeof(pOverLapStruct->iobuf);
		//pOverLapStruct->pBuffer->zero();
		pOverLapStruct->reInit();
		pOverLapStruct->SetOperator(ICE_IO_OPERATION_READ);
	
		PostIOOpt(pOverLapStruct,ICE_IO_OPERATION_READ);

	}
	else
	{
		P_STRUCT_ICE_OVERLAPPED_BUF pExistOverLapStruct=m_pCacheWsaOverLappedList.front();
		m_pCacheWsaOverLappedList.pop_front();
		//ZeroMemory(&pExistOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pExistOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pExistOverLapStruct->m_PostOverLapped.hEvent=pExistOverLapStruct->m_EventArray[0];
		//pExistOverLapStruct->m_WsaIOBuf.len=sizeof(pExistOverLapStruct->iobuf);
		//pExistOverLapStruct->m_WsaIOBuf.buf=pExistOverLapStruct->iobuf;
		//pExistOverLapStruct->iotype=iorecv_id;
		//pExistOverLapStruct->useSize=sizeof(pExistOverLapStruct->iobuf);
		pExistOverLapStruct->reInit();
		pExistOverLapStruct->SetOperator(ICE_IO_OPERATION_READ);
		PostIOOpt(pExistOverLapStruct,ICE_IO_OPERATION_READ);
	}
	return 0;

}
int MightyTCPCompletionPortClient::PostSendOpt(const char * outBuf,int size)
{
	if (m_pCacheWsaOverLappedList.empty())
	{
		P_STRUCT_ICE_OVERLAPPED_BUF pOverLapStruct=new STRUCT_ICE_OVERLAPPED_BUF(size);
		pOverLapStruct->pMainCall=this;
		//ZeroMemory(&pOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pOverLapStruct->m_PostOverLapped.hEvent=pOverLapStruct->m_EventArray[0];
		//memset(pOverLapStruct->iobuf,0,sizeof(pOverLapStruct->iobuf));
		//memcpy(pOverLapStruct->iobuf,outBuf,size);
		//pOverLapStruct->m_WsaIOBuf.len=sizeof(pOverLapStruct->iobuf);
		//pOverLapStruct->m_WsaIOBuf.buf=pOverLapStruct->iobuf;
		//pOverLapStruct->iotype=iosend_id;
		//pOverLapStruct->useSize=size;
		pOverLapStruct->pBuffer->addData(outBuf,size);
		pOverLapStruct->SetOperator(ICE_IO_OPERATION_WRITE);
		PostIOOpt(pOverLapStruct,ICE_IO_OPERATION_WRITE);
	}
	else
	{
		P_STRUCT_ICE_OVERLAPPED_BUF pExistOverLapStruct=m_pCacheWsaOverLappedList.front();
		m_pCacheWsaOverLappedList.pop_front();
		//ZeroMemory(&pExistOverLapStruct->m_PostOverLapped,sizeof(WSAOVERLAPPED));
		//pExistOverLapStruct->m_EventArray[0]=WSACreateEvent();
		//pExistOverLapStruct->m_PostOverLapped.hEvent=pExistOverLapStruct->m_EventArray[0];
		//memset(pExistOverLapStruct->iobuf,0,sizeof(pExistOverLapStruct->iobuf));
		//memcpy(pExistOverLapStruct->iobuf,outBuf,size);
		//pExistOverLapStruct->m_WsaIOBuf.len=sizeof(pExistOverLapStruct->iobuf);
		//pExistOverLapStruct->m_WsaIOBuf.buf=pExistOverLapStruct->iobuf;
		//pExistOverLapStruct->iotype=iosend_id;
		//pExistOverLapStruct->useSize=size;
		pExistOverLapStruct->reInit();
		pExistOverLapStruct->pBuffer->addData(outBuf,size);
		pExistOverLapStruct->SetOperator(ICE_IO_OPERATION_WRITE);
		PostIOOpt(pExistOverLapStruct,ICE_IO_OPERATION_WRITE);

	}
	return 0;
}

void MightyTCPCompletionPortClient::processIoMessage(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize)
{



	if ( pOverLapBuffer == NULL ) {

		if ( pConnContext ) 
			//this->disconnectClient(pConnContext);
		return;

	}

	switch(pOverLapBuffer->nOperationType) {

	case ICE_IO_OPERATION_INIT:

		this->onInit(pOverLapBuffer, pConnContext, nIoSize);

		break;

	case ICE_IO_OPERATION_READ:

		this->onRead(pOverLapBuffer, pConnContext);

		break;

	case ICE_IO_OPERATION_READ_COMPLETE:

		this->onReadComplete(pOverLapBuffer, pConnContext, nIoSize);

		break;

	case ICE_IO_OPERATION_WRITE:

		this->onWrite(pOverLapBuffer, pConnContext, nIoSize);

		break;

	case ICE_IO_OPERATION_WRITE_COMPLETE:

		this->onWriteComplete(pOverLapBuffer, pConnContext, nIoSize);

		break;

	default:

		//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
		pushOverlappCacheQueue(pOverLapBuffer);
		this->disconnectClient(pConnContext);

		break;

	}

}

void MightyTCPCompletionPortClient::disconnectClient(CONN_CONTEXT *pConnContext)
{

	/*this->m_sockSetLock.writeEnter();

	if ( this->m_ConnSet.find(pConnContext) == this->m_ConnSet.end() ) {

		this->m_sockSetLock.leave(); return;

	}

	this->m_socketSet.erase((void*)pConnContext->s);

	this->m_ConnSet.erase(pConnContext);

	this->m_sockSetLock.leave();

	InterlockedDecrement(&this->m_nCurrentConn);*/

	if ( pConnContext->s == INVALID_SOCKET || pConnContext->s == 0 ) return;

	LINGER lingerStruct = {1,0};

	setsockopt( pConnContext->s, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct) ); //强制关闭

	/*if ( this->m_routineBreakHandle ) {

		this->m_routineBreakHandle(this->m_pPerHandleData, (HANDLE) pConnContext->s, pConnContext->ptr);

	}*/


	CancelIo((HANDLE)pConnContext->s);

	closesocket(pConnContext->s);

	pConnContext->s = INVALID_SOCKET;

	delete pConnContext;//?

}


int MightyTCPCompletionPortClient::PostConnect()
{
	const char chOpt = 0;

	setsockopt(m_SocketContext->s, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(chOpt) ); //允许Nagle算法

	//ICE_OVERLAPPED_BUF *pOverLappedBuf = this->m_bufferManager.AllocOverLappedBuf(0);
	ICE_OVERLAPPED_BUF * pOverLappedBuf =new ICE_OVERLAPPED_BUF(0);
	if ( pOverLappedBuf == NULL ) {

		//K_LOG("\n内存分配失败！");

		//InterlockedDecrement(&this->m_nCurrentConn); 
		return 1;

	}

	pOverLappedBuf->SetOperator(ICE_IO_OPERATION_INIT);

	HANDLE hIocp = ::CreateIoCompletionPort((HANDLE) m_SocketContext->s, this->m_hCompletionPort, (ULONG_PTR)m_SocketContext, 0);
	if ( this->m_hCompletionPort != hIocp ) {

		//delete pConnContext; 

		//this->m_bufferManager.FreeOverLappedBuf(pOverLappedBuf);

		//K_LOG("\nCreateIoCompletionPort Error %d", GetLastError());

		//InterlockedDecrement(&this->m_nCurrentConn); 
		if ( pOverLappedBuf != NULL ) 
			delete pOverLappedBuf;
		return 1;

	}

	//this->m_sockSetLock.writeEnter();

	//this->m_socketSet[(void*)pConnContext->s] = pConnContext;

	//this->m_ConnSet[pConnContext] = pConnContext;

	//this->m_sockSetLock.leave();

	BOOL bRet = PostQueuedCompletionStatus(this->m_hCompletionPort, 0, (ULONG_PTR)m_SocketContext, &pOverLappedBuf->overLapped);

	if ( ! bRet ) 
		if ( GetLastError() != ERROR_IO_PENDING ) {

		//K_LOG("\nPostQueuedCompletionStatus Error %d", GetLastError());

		//this->m_sockSetLock.writeEnter();

		//this->m_socketSet.erase((void*)pConnContext->s);

		//this->m_ConnSet.erase(pConnContext);

		//this->m_sockSetLock.leave();

		//delete pConnContext; 

		//this->m_bufferManager.FreeOverLappedBuf(pOverLappedBuf);

		//InterlockedDecrement(&this->m_nCurrentConn); return NULL;
		if ( pOverLappedBuf != NULL ) 
			delete pOverLappedBuf;
		return 1;
	}

	//K_LOG("\nInsert Conn OK this = %.8X", (int)this);
	//return (HANDLE)sockConn;
	return 0;

}
bool MightyTCPCompletionPortClient::send(CONN_CONTEXT *pConnContext, ICE_OVERLAPPED_BUF *pOverlapBuffer)
{
	/*if ( pConnContext == NULL || pOverlapBuffer == NULL ) return false;

	if ( pOverlapBuffer->pBuffer == NULL ) return false;

	if ( pOverlapBuffer->pBuffer->used == 0 ) return false;

	pOverlapBuffer->SetOperator(ICE_IO_OPERATION_WRITE);

	if ( ! PostQueuedCompletionStatus(this->m_hCompletePort, pOverlapBuffer->pBuffer->used, (ULONG_PTR) pConnContext, &pOverlapBuffer->overLapped) ) {

		if ( GetLastError() != ERROR_IO_PENDING ) return false;

	}*/

	return true;

}

bool MightyTCPCompletionPortClient::read(CONN_CONTEXT *pConnContext)
{

	/*if ( pConnContext == NULL ) return false;

	SOCKET sConn = pConnContext->s;

	if ( sConn == INVALID_SOCKET ) return false;

	ICE_OVERLAPPED_BUF *pOverLapBuffer = this->m_bufferManager.AllocOverLappedBuf(ICE_IO_MAX_READ_BUFFER);

	if ( pOverLapBuffer == NULL ) return false;

	pOverLapBuffer->SetOperator(ICE_IO_OPERATION_READ);

	if ( ! PostQueuedCompletionStatus( this->m_hCompletePort, 0, (ULONG_PTR)pConnContext, &pOverLapBuffer->overLapped) ){

		DWORD errCode = GetLastError();

		if ( errCode != ERROR_IO_PENDING ) {

			K_LOG("\n PostQueuedCompletionStatus Error = %d", errCode);

			this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);

			return false;

		}

	}*/
	PostRecvOpt();
	return true;

}

void MightyTCPCompletionPortClient::onReadComplete(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize)
{

	if ( nIoSize == 0 ) {

		if ( pConnContext ) 
			this->disconnectClient(pConnContext);

		//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
		pushOverlappCacheQueue(pOverLapBuffer);
		return;

	}

	if ( pOverLapBuffer->pBuffer == NULL ) {

		this->disconnectClient(pConnContext);

		//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
		//pushOverlappCacheQueue(pOverLapBuffer);
		delete pOverLapBuffer;

	}

	/*if ( this->m_routineDataInHandle ) {

		this->m_routineDataInHandle( this->m_pPerHandleData, (HANDLE)pConnContext->s, pOverLapBuffer->pBuffer->buffer.buf, nIoSize, pConnContext->ptr );

		pOverLapBuffer->reInit();

		this->onRead(pOverLapBuffer, pConnContext);

	}*/
	m_pNotifyCallBack->OnReceivedData(pOverLapBuffer->pBuffer->buffer.buf, nIoSize);
	pOverLapBuffer->reInit();

	this->onRead(pOverLapBuffer, pConnContext);
}

void MightyTCPCompletionPortClient::onWriteComplete(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize)
{

	if ( nIoSize != pOverLapBuffer->pBuffer->used ) 
		;//K_LOG("\noWriteComplete Write size = %d, data size = %d", nIoSize, pOverLapBuffer->pBuffer->used);

	//if ( this->m_routineDataSendOkHandle ) {

		//this->m_routineDataSendOkHandle( this->m_pPerHandleData, (HANDLE)pConnContext->s, pConnContext->ptr );

	//}
	
	//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
	pushOverlappCacheQueue(pOverLapBuffer);
}

void MightyTCPCompletionPortClient::onWrite(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize)
{

	if ( nIoSize <= 0 ) {

		//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
		pushOverlappCacheQueue(pOverLapBuffer);
		return;

	}

	pOverLapBuffer->SetOperator(ICE_IO_OPERATION_WRITE_COMPLETE);

	DWORD dwSendNumBytes = 0;

	if ( WSASend(pConnContext->s, &pOverLapBuffer->pBuffer->buffer, 1, &dwSendNumBytes, MSG_PARTIAL, &pOverLapBuffer->overLapped, NULL) == SOCKET_ERROR ) {

		DWORD errCode = WSAGetLastError();

		if ( errCode != WSA_IO_PENDING ) {

			if ( errCode != WSAENOTSOCK ) {

				//K_LOG("\n WSASend Error %d", errCode);

			}

			//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
			pushOverlappCacheQueue(pOverLapBuffer);
			this->disconnectClient(pConnContext);

		}

	}

}

void MightyTCPCompletionPortClient::onInit(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize)
{

	//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
	if (pOverLapBuffer)
	{
		delete pOverLapBuffer;
	}
	this->read( pConnContext );

}

void MightyTCPCompletionPortClient::onRead(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext)
{

	pOverLapBuffer->SetOperator(ICE_IO_OPERATION_READ_COMPLETE);

	DWORD dwIoSize=0;

	ULONG ulFlags = MSG_PARTIAL;

	if ( WSARecv(pConnContext->s, &pOverLapBuffer->pBuffer->buffer, 1, &dwIoSize, &ulFlags,&pOverLapBuffer->overLapped, NULL) == SOCKET_ERROR ) {

		DWORD errCode = WSAGetLastError();

		if ( errCode != WSA_IO_PENDING ) {

			if ( errCode != WSAENOTSOCK ) {

				//K_LOG("\nWSARecv Error %d", errCode);
				;
			}

			this->disconnectClient(pConnContext);

			//this->m_bufferManager.FreeOverLappedBuf(pOverLapBuffer);
			pushOverlappCacheQueue(pOverLapBuffer);

		}

	}

}