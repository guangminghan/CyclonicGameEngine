#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "basemightyclientinterface.h"
#pragma pack(1)



typedef struct ICE_BUF {

	ICE_BUF(int nlen) {

		this->buffer.buf = new char[nlen];

		if (this->buffer.buf == NULL) this->buffer.len = 0;

		else this->buffer.len = nlen;

		this->used = 0;

	}

	~ICE_BUF() {

		if (this->buffer.len) delete [] this->buffer.buf;

		this->buffer.buf = NULL;

		this->buffer.len = 0;

	}

	int addData(const char *buf, u_long datalen) {

		if ( datalen > this->buffer.len - this->used ) {

			datalen = this->buffer.len - this->used;

		}

		memcpy(this->buffer.buf + used, buf, datalen);

		this->used += datalen;

		return datalen;

	}

	void zero() {

		if ( this->buffer.buf ) ZeroMemory(this->buffer.buf, this->buffer.len);

		this->used = 0;

	}

	WSABUF buffer;

	u_long used;

}STRUCT_ICE_BUF,*P_STRUCT_ICE_BUF;

#define ICE_IO_MAX_READ_BUFFER 65536

#define ICE_IO_OPERATION_INIT 1
#define ICE_IO_OPERATION_READ 2
#define ICE_IO_OPERATION_READ_COMPLETE 3
#define ICE_IO_OPERATION_WRITE 4
#define ICE_IO_OPERATION_WRITE_COMPLETE 5

typedef struct ICE_OVERLAPPED_BUF {

	ICE_OVERLAPPED_BUF() { 

		ZeroMemory( &this->overLapped, sizeof(this->overLapped) );

		this->pBuffer = NULL; 

	}

	ICE_OVERLAPPED_BUF(int len) {

		ZeroMemory( &this->overLapped, sizeof(this->overLapped) );

		if ( len == 0 ) this->pBuffer = NULL;

		else {

			this->pBuffer = new ICE_BUF(len);

			if ( this->pBuffer == NULL ) return;

			if ( this->pBuffer->buffer.len == 0 ) {

				delete this->pBuffer;

				this->pBuffer = NULL;

			}

		}

	}

	~ICE_OVERLAPPED_BUF() {

		if ( this->pBuffer ) delete this->pBuffer;

		this->pBuffer = NULL;

	}

	void reInit() {

		ZeroMemory( &this->overLapped, sizeof( this->overLapped ) );

		if ( this->pBuffer ) {

			this->pBuffer->zero();

		}

	}

	void SetOperator(int operatorType) {

		this->nOperationType = operatorType;

	}

	//OVERLAPPED overLapped;//重叠结构

	WSAOVERLAPPED overLapped;//重叠结构
	int nOperationType;//缓冲区类型

	ICE_BUF *pBuffer;//缓冲区
	void * pMainCall;

}STRUCT_ICE_OVERLAPPED_BUF,*P_STRUCT_ICE_OVERLAPPED_BUF;

typedef struct CONN_CONTEXT {

	CONN_CONTEXT() {

		this->s = INVALID_SOCKET;

		//this->remoteIp = NULL;

		//this->remotePort = NULL;

		this->ptr	= NULL;

	}

	SOCKET	s;

	//DWORD	remoteIp;

	//WORD	remotePort;

	void	*ptr;

}STRUCT_CONN_CONTEXT,*P_STRUCT_CONN_CONTEXT;


#pragma pack()

class CBufferManage
{
public:
	CBufferManage(void);
	~CBufferManage(void);
	ICE_BUF *AllocBuffer(int len);
	void FreeBuffer(ICE_BUF *pBuffer);
	ICE_OVERLAPPED_BUF *AllocOverLappedBuf(int len);
	void FreeOverLappedBuf(ICE_OVERLAPPED_BUF *pOverLappedBuf);
private:
	volatile long m_count;
};
class HandleIOCompletionPortClient: public MightyThread
{
public:
	HandleIOCompletionPortClient(void);
	virtual ~HandleIOCompletionPortClient(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

};
class CompletionPortClientExecute: public MightyThread
{
public:
	CompletionPortClientExecute(void);
	virtual ~CompletionPortClientExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);

};
class MightyTCPCompletionPortClient :
	public ITcpClient
{
public:
	MightyTCPCompletionPortClient(ITcpClientNotify* pNotify);
	virtual ~MightyTCPCompletionPortClient(void);
public:
	virtual bool ConnectServer( const char* pServerAddr, unsigned short usPort);
	virtual bool SendData( const char* pData, int nLen);
	virtual void Disconnect(void);
	virtual void ReleaseConnection();
	bool bind();
	int init(const char * ip,WORD port);
	int init(DWORD ip,WORD port);
	int afreshConnect();
	int Process();
	int ExecuteWork();
	bool PostAsyncCompletePortMessage(P_STRUCT_ICE_OVERLAPPED_BUF pStatePack);
	void pushOverlappCacheQueue(P_STRUCT_ICE_OVERLAPPED_BUF pBackOverlapped);
	int PostIOOpt(P_STRUCT_ICE_OVERLAPPED_BUF pReUseOverlapped,int isWait=1);
	int PostRecvOpt();
	int PostSendOpt(const char * outBuf,int size);
	void processIoMessage(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize);
	void disconnectClient(CONN_CONTEXT *pConnContext);
	int PostConnect();


	
	bool send(CONN_CONTEXT *pConnContext, ICE_OVERLAPPED_BUF *pOverlapBuffer);
	bool read(CONN_CONTEXT *pConnContext);
	void onReadComplete(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize);
	void onWriteComplete(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize);
	void onWrite(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize);
	void onInit(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext, int nIoSize);
	void onRead(ICE_OVERLAPPED_BUF *pOverLapBuffer, CONN_CONTEXT *pConnContext);
public:
	HANDLE m_hCompletionPort;			// 完成端口句柄
	ITcpClientNotify * m_pNotifyCallBack;
	SOCKADDR_IN m_SockAddr;
	BOOL m_bRun;

	CONN_CONTEXT * m_SocketContext;
	//SOCKET m_ClientSock;
	WORD m_ConnectPort;
	DWORD m_dwAddress;
	string m_strAddress;
	volatile long	m_nCurrentThread;
	//LPFN_CONNECTEX  m_connectFuncPtr;
	//LPFN_WSARECVMSG m_wsaRecvMsgPtr;
	//LPFN_DISCONNECTEX m_disConnectExFuncPtr;
	//LPFN_TRANSMITPACKETS m_transmitPacketsFuncPtr;
	//LPFN_GETACCEPTEXSOCKADDRS m_getAcceptExSockAddrFuncPtr;
	//LPFN_ACCEPTEX m_acceptExFuncPtr;
	//LPFN_TRANSMITFILE m_transmitFileFuncPtr;
	
	list<P_STRUCT_ICE_OVERLAPPED_BUF> m_pWsaOverLappedList;
	list<P_STRUCT_ICE_OVERLAPPED_BUF> m_pCacheWsaOverLappedList;
	list<P_STRUCT_ICE_OVERLAPPED_BUF> m_pTaskWsaOverLappedList;
};
