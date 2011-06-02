#include ".\udpclientonraknet.h"
#include ".\_initinfo.h"
#include "..\CommonInc\P2PUtilTools.h"
#include "DispatchControl.h"
#include "..\CommonInc\Log.h"
int completeFlag=0;
int sendEndFlag=0;
TaskQueueExecute::TaskQueueExecute(void)
{

}
TaskQueueExecute::~TaskQueueExecute(void)
{
	exit();
}

int TaskQueueExecute::threadrun(void * pBuf)
{
	UDPClientOnRakNet * ptrUser=(UDPClientOnRakNet *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;
	CBaseCommand * pCmdPack=NULL;
	tostringstream Log_Info;
	tstring strlog;

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
				pCmdPack=NULL;
				WriteWait();

				if (!m_taskQueue.empty())
				{
					pCmdPack=m_taskQueue.front();
					m_taskQueue.pop_front();
				}
				CancelWriteWait();
				if (pCmdPack)
				{
					strlog=_T("ȡ������UDP����");
					Log_Info.str(_T(""));
					Log_Info<<strlog<<_T("\r\n");
					LOG(Log_Debug,Log_Info.str().c_str());
					ptrUser->TransUdpCommand(pCmdPack);
					delete pCmdPack;
				}

				/*if (ptrUser->m_peersMap.empty()&&ptrUser->isLogin)
				{
					strlog=_T("���»�ȡ�ڵ�");
					Log_Info.str(_T(""));
					Log_Info<<strlog<<_T("\r\n");
					LOG(Log_Debug,Log_Info.str().c_str());
					ptrUser->RepeatRequestPeer();
					
				}*/
				break;
			}
		default:
			break;
		}
	}

}
int TaskQueueExecute::ExecuteTask(void * pBuf)
{
	return 1;
}
int TaskQueueExecute::branchEvent(void * pBuf)
{
	return 1;
}
void TaskQueueExecute::pushIn(CBaseCommand* pCmd )
{
	WriteWait();
	m_taskQueue.push_back(pCmd);
	CancelWriteWait();
}
//------------------------------------------------------------------------------------------------------

FileIOQueueExecute::FileIOQueueExecute(void)
{

}
FileIOQueueExecute::~FileIOQueueExecute(void)
{
	exit();
}

int FileIOQueueExecute::threadrun(void * pBuf)
{
	UDPClientOnRakNet * ptrUser=(UDPClientOnRakNet *)pBuf;

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
				//ptrUser->Process();
				break;
			}
		default:
			break;
		}
	}

}
int FileIOQueueExecute::ExecuteTask(void * pBuf)
{
	return 1;
}
int FileIOQueueExecute::branchEvent(void * pBuf)
{
	return 1;
}

//-----------------------------------------------------------------------------------------------------------------------------
PeerQueueMaintain::PeerQueueMaintain(void)
{

}
PeerQueueMaintain::~PeerQueueMaintain(void)
{
	exit();
}
int PeerQueueMaintain::threadrun(void * pBuf)
{

	UDPClientOnRakNet * ptrUser=(UDPClientOnRakNet *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;
	CBaseCommand * pCmdPack=NULL;
	tostringstream Log_Info;
	tstring strlog;

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
				ptrUser->PeerManageTask();
				break;
			}
		default:
			break;
		}
	}
}

int PeerQueueMaintain::ExecuteTask(void * pBuf)
{
	return 0;
}

int PeerQueueMaintain::branchEvent(void * pBuf)
{
	return 0;
}
//-----------------------------------------------------------------------------------------------------------------------------
UDPClientOnRakNet::UDPClientOnRakNet(const string & passWord,void * pPtrCall)
:RakClient(MAX_PEER,passWord)
,m_MainCallBack(pPtrCall)
,isLogin(false)
,m_pSendBufMemory(NULL)
{
	char szProFileName[1024] = {0};
	m_recvFileHandle=new ST_FILE_OPERATION();
	m_sendFileHandle=new ST_FILE_OPERATION();
	string exe_path=P2PUtilTools::getRunPath(szProFileName);
	string share_path,income_path;
	share_path=exe_path;
	income_path=exe_path;
	share_path.append("uploadshare\\QQ2008IIBeta1.exe");
	income_path.append("income\\QQ2008IIBeta1.exe");

	UQWORD sendFileTotalLen=0;
	UQWORD recvFileTotalLen=0;
	int lenSize;
	try
	{
		
		m_task=new TaskQueueExecute();
		//m_task->setTimeOutVal(1000);
		m_task->setTimeOutVal(5);
		m_task->start(this);
		m_task->setExecuteSignal();
		m_memoryPool.clear();
		
		for (int d=0;d<80;d++)
		{
			char * bufIndex=new char[perPackLen];
			if (bufIndex!=NULL)
			{
				m_memoryPool.push_back(bufIndex);
			}
		}
		m_PeerQueueControl=new PeerQueueMaintain();
		m_PeerQueueControl->setTimeOutVal(5000);
		m_PeerQueueControl->start(this);
		m_PeerQueueControl->setExecuteSignal();

		//m_recvBuf=new char[128*1024];
		//m_sendBuf=new char[163848];
		m_sendBuf=new char[204808];
		/*for (int CountSeed=0;CountSeed<10;CountSeed++)
		{

			m_sendDataBufferPool.push_back(pSendBuf);

		}*/
		
		m_pSendBufMemory=new P2P_COMMAND::CCmdSendPiece();

		m_pSendBufMemory->m_szBuffer=new char[perPackLen];	

		
		if (!(m_sendFileHandle->m_fd.Open(share_path.c_str(),MightyFile::modeCreate|MightyFile::modeRead|MightyFile::modeNoTruncate|MightyFile::shareDenyWrite)))
		{
			m_sendFileHandle->m_isopen=false;
		}
		else
		{
			m_sendFileHandle->m_isopen=true;
			sendFileTotalLen=m_sendFileHandle->m_fd.GetLength();
		}

		if(!(m_recvFileHandle->m_fd.Open(income_path.c_str(),MightyFile::modeCreate|MightyFile::modeReadWrite|MightyFile::modeNoTruncate|MightyFile::shareDenyWrite)))
		{

			m_recvFileHandle->m_isopen=false;
		}
		else
		{
			m_recvFileHandle->m_isopen=true;
			m_recvFileHandle->m_fd.SetLength(sendFileTotalLen);
			m_recvFileHandle->m_fd.Close();
			GetPieceQueue(income_path);
			m_recvFileHandle->m_fd.Open(income_path.c_str(),MightyFile::modeCreate|MightyFile::modeReadWrite|MightyFile::modeNoTruncate|MightyFile::shareDenyWrite);

		}
		/*map<QWORD,unsigned long>::iterator posLeftIter,posRightIter;

		posLeftIter=m_PosIndexBitMap.begin();
		posRightIter=m_PosIndexBitMap.end();
		QWORD wd1;
		int nn2;
		while (posLeftIter!=posRightIter)
		{
			wd1=posLeftIter->first;
			nn2=posLeftIter->second;
			m_sendFileHandle->m_fd.Seek(posLeftIter->first,MightyFile::begin);

			lenSize=m_sendFileHandle->m_fd.Read(m_sendBuf,posLeftIter->second);

			
			m_recvFileHandle->m_fd.Seek(posLeftIter->first,MightyFile::begin);
			m_recvFileHandle->m_fd.Write(m_sendBuf,lenSize);
			posLeftIter++;
		}
		lenSize=0;*/
	}

	catch (...)
	{
		m_sendFileHandle->m_isopen=false;
		m_recvFileHandle->m_isopen=false;
	}


}

UDPClientOnRakNet::~UDPClientOnRakNet(void)
{
	if (m_task)
	{
		delete m_task;
	}
	if (m_PeerQueueControl)
	{
		delete m_PeerQueueControl;
	}
	if (m_sendBuf)
	{
		delete [] m_sendBuf;
	}
	if (m_recvFileHandle->m_isopen)
	{
		m_recvFileHandle->m_fd.Close();
	}
	if (m_sendFileHandle->m_isopen)
	{
		m_sendFileHandle->m_fd.Close();
	}
	if (m_pSendBufMemory)
	{
		delete m_pSendBufMemory;
	}
	list<char *>::iterator loopPos,endPos;
	loopPos=m_memoryPool.begin();
	endPos=m_memoryPool.end();
	while(loopPos!=endPos)
	{
		if ((*loopPos)!=NULL)
		{
			delete [] (*loopPos);
		}

	}
}
int UDPClientOnRakNet::Update()
{
	return RakClient::Update();
}

int UDPClientOnRakNet::OnPeerConnected(Packet* p)
{
	//SendCharJoin(p->systemAddress);
	ResponsePeerConnect(p);
	return 0;
}

int UDPClientOnRakNet::OnUserPacket(Packet* p)
{
	tostringstream Log_Info;
	tstring strlog;
	switch(p->data[0])
	{
		case (raknet_wrapper::ID_IGT_CLIENT) :
			return OnHandleP2PProcess(p);
		case (raknet_wrapper::ID_IGT_SERVER) :
			return OnHandleP2SProcess(p);
		default:
			break;
			
	}
	strlog=_T("δ֪��������");
	Log_Info.str(_T(""));
	Log_Info<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	return RakClient::OnUserPacket(p);
}
int UDPClientOnRakNet::OnServerConnected(Packet* p)
{
	tostringstream Log_Info;
	tstring strlog;
	strlog=_T("���ӵ�������");
	Log_Info.str(_T(""));
	Log_Info<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());

	m_client_sessionID=GetRandSession();
	P2S_COMMAND::CCmdRegister* pCmdPeerLogin = new P2S_COMMAND::CCmdRegister();
	int nSendLength = 0;
	SystemAddress usingAddr=GetSystemAddress();
	pCmdPeerLogin->m_nSessionID=m_client_sessionID;
	
	pCmdPeerLogin->m_nLocalIP=usingAddr.binaryAddress;
	pCmdPeerLogin->m_sLocalUdpPort=usingAddr.port;
	pCmdPeerLogin->m_sLocalTcpPort=9988;
	string strAddress=P2PUtilTools::IpToString(usingAddr.binaryAddress);
	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));

	if ( pCmdPeerLogin->Create(buf+1,nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+1,HIGH_PRIORITY, RELIABLE_ORDERED,0,p->systemAddress,false);
	}

	delete pCmdPeerLogin;
	return 0;
	/*char szSendBuffer[TCP_MAX_BUFFER_LENGTH];
	int nSendLength = 0;

	while ( !m_bWantStop )
	{
		CBaseCommand* pCmd = m_clientMgr.m_listSendCmd.Pop();
		while( pCmd )
		{
			nSendLength = 0;
			if ( pCmd->Create( szSendBuffer, nSendLength ) > 0 )
			{
				assert( pCmd->m_ulSocketID >= 0 );
				m_pTcpServer->SendData( szSendBuffer, nSendLength, pCmd->m_ulSocketID );
			}

			delete pCmd;
			pCmd = m_clientMgr.m_listSendCmd.Pop();
		}

		m_clientMgr.m_evSendCmd.Wait( 1000 );

		m_clientMgr.UpdateChannelTick();
	}*/
}
int UDPClientOnRakNet::OnConnectionAttemptFailed( Packet* p )
{
	tostringstream Log_Info;
	/*tstring strlog;
	strlog=_T("���ӷ�����ʧ��");
	Log_Info.str(_T(""));
	Log_Info<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	((CDispatchControl *)m_MainCallBack)->doConnectServer(); */


	tstring strlog;
	//strlog=_T("���ӷ�����ʧ��");
	Log_Info.str(_T(""));
	strlog=p->systemAddress.ToString();
	Log_Info<<"��������ʧ��"<<"���������ⲿIP : "<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());

	if ( p->systemAddress ==GetServerIPAddr())
	{
		((CDispatchControl *)m_MainCallBack)->doConnectServer();
	}
	else
	{
		;
	}
	return 0;
}
int UDPClientOnRakNet::OnConnectionLost(Packet* p)
{
	tostringstream Log_Info;
	/*tstring strlog;
	strlog=_T("�������ʧȥ����");
	Log_Info.str(_T(""));
	Log_Info<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	((CDispatchControl *)m_MainCallBack)->doConnectServer(); */

	tstring strlog;
	//strlog=_T("�������ʧȥ����");

	Log_Info.str(_T(""));
	strlog=p->systemAddress.ToString();
	Log_Info<<"���Ӷ�ʧ"<<"���������ⲿIP : "<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	//((CDispatchControl *)m_MainCallBack)->doConnectServer();
	if ( p->systemAddress ==GetServerIPAddr())
	{
		((CDispatchControl *)m_MainCallBack)->doConnectServer();
	}
	else
	{
		;
	}
	return 0;
}
int UDPClientOnRakNet::OnHandleP2PProcess(Packet* p)
{
	tostringstream Log_Info;
	tstring strlog;
	char * pIndexBuf=(char *)(p->data+1);
	P2P_COMMAND::CP2PBaseCommand* pCmd = NULL;
	strlog=_T("�յ�P2P����");
	Log_Info.str(_T(""));
	Log_Info<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	switch( CBaseCommand::getID( pIndexBuf ) ) 
	{
	case P2P_CMD_HANDSHAKE:
		strlog=_T("P2P���ְ�");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdHandshake();
		break;

	case P2P_CMD_GETBLOCK_AREA:
		strlog=_T("P2Pȡ�ļ����");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdGetBlockArea();
		break;

	case P2P_CMD_SENDBLOCK:
		strlog=_T("P2P���ͷ�Ƭ��");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdSendPiece();
		
		m_LockMemoryPool.writeEnter();
		if (!m_memoryPool.empty())
		{
			((P2P_COMMAND::CCmdSendPiece *)pCmd)->m_szBuffer=m_memoryPool.front();
			m_memoryPool.pop_front();

		}
		else
		{
			((P2P_COMMAND::CCmdSendPiece *)pCmd)->m_szBuffer=new char[perPackLen];
		}
		m_LockMemoryPool.leave();
		break;
	case P2P_CMD_ECHO_SHOOTHOLE:
		strlog=_T("P2P��Ӧ�򶴰�");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdEchoShootHole();
		break;
	case P2P_CMD_ACCEPT_REVERSECONNECT:
		strlog=_T("P2P���ܷ������Ӱ�");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdAcceptReverseConnect();
		break;

	//------------------------------------------------------------------------
	case P2P_CMD_HANDSHAKE_RET:
		strlog=_T("P2P���ֻ�Ӧ��");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdHandshakeRet();
		break;

	case P2P_CMD_GETBLOCK_AREA_RET:
		strlog=_T("P2Pȡ���ݿ��Ӧ��");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdGetBlockAreaRet();
		break;

	case P2P_CMD_SENDBLOCK_RET:
		strlog=_T("P2P���ͷ�Ƭ��Ӧ��");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdSendPieceRet();
		break;
	case P2P_CMD_ECHO_SHOOTHOLE_RET:
		strlog=_T("P2P��Ӧ�򶴻�Ӧ��");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdEchoShootHoleRet();
		break;
	case P2P_CMD_ACCEPT_REVERSECONNECT_RET :
		strlog=_T("P2P��Ӧ���ܷ������ӻ�Ӧ��");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		pCmd = new P2P_COMMAND::CCmdAcceptReverseConnectRet();
		break;
	//-------------------------------------------------------------------------

	default:
		strlog=_T("P2P�����");
		Log_Info.str(_T(""));
		Log_Info<<strlog<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		break;
	}
	if ( pCmd )
	{
		try
		{
			pCmd->m_dwIP =p->systemAddress.binaryAddress;
			pCmd->m_usPort =p->systemAddress.port;
			int nLength = pCmd->Parse( pIndexBuf,p->length-1);
			m_task->pushIn(pCmd);
		}
		catch ( ... ) 
		{
		}

		//delete pCmd;
	}
	//else
	//{
		//printf("pCmd is null id => %u\n", CBaseCommand::getID( pIndexBuf ) );
	//}
	return 0;
}

int UDPClientOnRakNet::OnHandleP2SProcess(Packet* p)
{
	tostringstream Log_Info;
	tstring strlog;
	char * pIndexBuf=(char *)(p->data+1);
	//CBaseCommand* pCmd = NULL;
	P2S_COMMAND::CP2SBaseCommand * pCmd = NULL;
	strlog=_T("�յ�P2S����");
	Log_Info.str(_T(""));
	Log_Info<<strlog<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	switch( CBaseCommand::getID( pIndexBuf ) ) 
	{
		case P2S_CMD_SHOOTHOLE:
			strlog=_T("P2S�������");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdShootHole();
			break;
		case P2S_CMD_UDP_HEARTBEAT:
			strlog=_T("P2S������");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdUdpHeartbeat();
			break;
		case P2S_CMD_LOGIN:
			strlog=_T("P2S��½��������");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdRegister();
			break;
		case P2S_CMD_GETPEERS:
			strlog=_T("P2Sȡ�ڵ������");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdGetPeers();
			break;			
		case P2S_CMD_REQSHOOTTO:
			strlog=_T("P2S����������");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdReqShootTo();
			break;
		case P2S_CMD_REVERSECONNECT_ASSISTANCE :
			strlog=_T("P2S��������Э�������");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdReverseConnectAssistance();
			break;
		case P2S_CMD_PASSONREVERSECONNECT_ASK :
			strlog=_T("P2S���ݷ������������");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdPassOnReverseConnectAsk();
			break;

	//---------------------------------------------------------------------
		case P2S_CMD_SHOOTHOLE_RET:
			strlog=_T("P2S�򶴻�Ӧ��");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdShootHoleRet();
			break;
		case P2S_CMD_LOGIN_RET:
			strlog=_T("P2S��½��Ӧ��");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdRegisterRet();
			break;

		case P2S_CMD_GETPEERS_RET:
			strlog=_T("P2Sȡ�ڵ��Ӧ��");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdGetPeersRet();
			break;
		case P2S_CMD_REQSHOOTTO_RET:
			strlog=_T("P2S����������Ӧ��");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdReqShootToRet();
			break;
		case P2S_CMD_REVERSECONNECT_ASSISTANCE_RET :
			strlog=_T("P2S��������Э����Ӧ��");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdReverseConnectAssistanceRet();
			break;
		case P2S_CMD_PASSONREVERSECONNECT_ASK_RET :
			strlog=_T("P2S���ݷ������������Ӧ��");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdPassOnReverseConnectAsk();
			break;
		//-------------------------------------------------------------------------------
		default:
			strlog=_T("P2S�����");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;
	}
	if ( pCmd )
	{
		try
		{
			int nLength = pCmd->Parse( pIndexBuf,p->length-1);
			pCmd->m_dwIP=p->systemAddress.binaryAddress;
			pCmd->m_usPort=p->systemAddress.port;
			
			m_task->pushIn(pCmd);

		}
		catch ( ... ) 
		{
		}

		//delete pCmd;
	}
	//else
	//{
		//printf("pCmd is null id => %u\n", CBaseCommand::getID( pIndexBuf ) );
	//}
	return 0;

}

void UDPClientOnRakNet::TransUdpCommand(CBaseCommand* pCmd )
{


	switch( pCmd->ID() ) 
	{
//------------------------------------------------------------------------
	case P2P_CMD_HANDSHAKE:
		OnP2PHandshake( reinterpret_cast<P2P_COMMAND::CCmdHandshake*>(pCmd) );
		break;

	case P2P_CMD_GETBLOCK_AREA:
		OnP2PGetBlockArea( reinterpret_cast<P2P_COMMAND::CCmdGetBlockArea*>(pCmd) );
		break;

	case P2P_CMD_SENDBLOCK:
		OnP2PSendPiece( reinterpret_cast<P2P_COMMAND::CCmdSendPiece*>(pCmd) );
		break;
	case P2P_CMD_ECHO_SHOOTHOLE:
		OnP2PEchoShootHole(reinterpret_cast<P2P_COMMAND::CCmdEchoShootHole*>(pCmd));
		break;
	case P2P_CMD_ACCEPT_REVERSECONNECT:
		OnP2PAcceptReverseConnect(reinterpret_cast<P2P_COMMAND::CCmdAcceptReverseConnect*>(pCmd));
		break;
//-------------------------------------------------------------------------
	case P2P_CMD_HANDSHAKE_RET:
		OnP2PHandshakeRet( reinterpret_cast<P2P_COMMAND::CCmdHandshakeRet *>(pCmd) );
		break;

	case P2P_CMD_GETBLOCK_AREA_RET:
		OnP2PGetBlockAreaRet( reinterpret_cast<P2P_COMMAND::CCmdGetBlockAreaRet*>(pCmd) );
		break;

	case P2P_CMD_SENDBLOCK_RET:
		OnP2PSendPieceRet( reinterpret_cast<P2P_COMMAND::CCmdSendPieceRet*>(pCmd) );
		break;

	case P2P_CMD_ECHO_SHOOTHOLE_RET:
		OnP2PEchoShootHoleRet(reinterpret_cast<P2P_COMMAND::CCmdEchoShootHoleRet*>(pCmd));
		break;
	case P2P_CMD_ACCEPT_REVERSECONNECT_RET :
		OnP2PAcceptReverseConnectRet(reinterpret_cast<P2P_COMMAND::CCmdAcceptReverseConnectRet*>(pCmd));
		break;
//---------------------------------------------------------------------------------------------
	case P2S_CMD_SHOOTHOLE:
		OnP2SShootHole( reinterpret_cast<P2S_COMMAND::CCmdShootHole *>(pCmd) );
		break;
		//------------------------------------------------------------------------
	case P2S_CMD_UDP_HEARTBEAT:
		OnP2SUdpHeartbeat( reinterpret_cast<P2S_COMMAND::CCmdUdpHeartbeat*>(pCmd) );
		break;

	case P2S_CMD_LOGIN:
		OnP2SRegister( reinterpret_cast<P2S_COMMAND::CCmdRegister*>(pCmd) );
		break;

	case P2S_CMD_GETPEERS:
		OnP2SGetPeers( reinterpret_cast<P2S_COMMAND::CCmdGetPeers*>(pCmd) );
		break;
	case P2S_CMD_REQSHOOTTO:
		OnP2SReqShootTo(reinterpret_cast<P2S_COMMAND::CCmdReqShootTo*>(pCmd));
		break;

	case P2S_CMD_REVERSECONNECT_ASSISTANCE :
		OnP2SReverseConnectAssistance(reinterpret_cast<P2S_COMMAND::CCmdReverseConnectAssistance*>(pCmd) );
		break;
	case P2S_CMD_PASSONREVERSECONNECT_ASK :
		OnPassOnReverseConnectAsk(reinterpret_cast<P2S_COMMAND::CCmdPassOnReverseConnectAsk*>(pCmd) );
		break;
	//----------------------------------------------------------------------------------------------
	case P2S_CMD_SHOOTHOLE_RET:
		OnP2SShootHoleRet( reinterpret_cast<P2S_COMMAND::CCmdShootHoleRet*>(pCmd) );
		break;

	case P2S_CMD_LOGIN_RET:
		OnP2SRegisterRet( reinterpret_cast<P2S_COMMAND::CCmdRegisterRet*>(pCmd) );
		break;

	case P2S_CMD_GETPEERS_RET:
		OnP2SGetPeersRet( reinterpret_cast<P2S_COMMAND::CCmdGetPeersRet *>(pCmd) );
		break;
	case P2S_CMD_REQSHOOTTO_RET:
		OnP2SReqShootToRet(reinterpret_cast<P2S_COMMAND::CCmdReqShootToRet*>(pCmd));
		break;
	case P2S_CMD_REVERSECONNECT_ASSISTANCE_RET :
		OnP2SReverseConnectAssistanceRet(reinterpret_cast<P2S_COMMAND::CCmdReverseConnectAssistanceRet*>(pCmd) );
		break;
	case P2S_CMD_PASSONREVERSECONNECT_ASK_RET:
		OnP2SPassOnReverseConnectAskRet(reinterpret_cast<P2S_COMMAND::CCmdPassOnReverseConnectAskRet*>(pCmd) );
		break;
//------------------------------------------------------------------------

	default:
		break;
	}
}


void UDPClientOnRakNet::OnP2PHandshake(P2P_COMMAND::CCmdHandshake * Cmdpack)
{
	tostringstream Log_Info;
	tstring strlog;
	//PST_ST_PEER_SNAPSHOT pstPeer=new ST_PEER_SNAPSHOT();
	//pstPeer->peer_external_addr.binaryAddress=Cmdpack->m_dwIP;
	//pstPeer->peer_external_addr.port=Cmdpack->m_usPort;
	//pstPeer->state=P2P_CMD_HANDSHAKE;
	//m_peersMap.insert(map<int,PST_ST_PEER_SNAPSHOT>::value_type(Cmdpack->m_nSessionID,pstPeer));

	
	Log_Info.str(_T(""));
	Log_Info<<"OnP2PHandshake"<<"  "<<"Cmdpack->m_nSessionID"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	//--------------------------------------------------------------------------------------
	//Ӧ��ͻ�������

	P2P_COMMAND::CCmdHandshakeRet* pCmdPeer= new P2P_COMMAND::CCmdHandshakeRet();
	int nSendLength = 0;
	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=Cmdpack->m_dwIP;
	PeerSysAddr.port=Cmdpack->m_usPort;
	pCmdPeer->m_nSessionID=Cmdpack->m_nSessionID;

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));

	if ( pCmdPeer->Create(buf+1,nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmdPeer;
	Log_Info.str(_T(""));
	Log_Info<<"�ӵ����ְ��������ֻ�Ӧ"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
}
void UDPClientOnRakNet::OnP2PGetBlockArea(P2P_COMMAND::CCmdGetBlockArea * Cmdpack)
{
	/*C_InitInfo *pIniInfo = C_InitInfo::GetInstancePtr();
	map<int,P_ST_FILE_OPERATION>::iterator findIndex,loopIndex;
	findIndex=m_fileIoMap.find(Cmdpack->m_nFileID);
	if (findIndex!=m_fileIoMap.end())
	{

	}
	else
	{

	}*/
	tostringstream Log_Info;
	tstring strlog;

//distance; range
	QWORD offset_=Cmdpack->m_nBlockOffset;
	UQWORD len_	=m_sendFileHandle->m_fd.GetLength();
	QWORD betweenDistance=len_-offset_;
	int sendLen_=betweenDistance>=perPackLen?perPackLen:betweenDistance;
	int actualReadSize=0;

	Log_Info.str(_T(""));
	Log_Info<<"OnP2PGetBlockArea "<<"Cmdpack->m_nBlockOffset :"<<Cmdpack->m_nBlockOffset<<"betweenDistance "<<betweenDistance<<"  "<<"sendLen_ :"<<sendLen_<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	//----------------------------------------------------------------
	//Ӧ�������ļ���
	P2P_COMMAND::CCmdGetBlockAreaRet* pCmdPeer= new P2P_COMMAND::CCmdGetBlockAreaRet();
	int nSendLength = 0;
	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=Cmdpack->m_dwIP;
	PeerSysAddr.port=Cmdpack->m_usPort;

	pCmdPeer->m_nSessionID=Cmdpack->m_nSessionID;
	pCmdPeer->m_nBlockOffset=offset_;
	pCmdPeer->m_nBlocakSize=sendLen_;
	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));

	if ( pCmdPeer->Create(buf+1,nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}
	if (offset_>=len_)
	{
		return;

	}
	if (sendEndFlag==1)
	{
		return;
	}
	//----------------------
	delete pCmdPeer;
	Log_Info.str(_T(""));
	Log_Info<<"Ӧ�������ļ���"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());

	if (betweenDistance==0)
	{
		sendEndFlag=1;
		Log_Info.str(_T(""));
		Log_Info<<"�����ļ����"<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		return;
	}
//------------------------------------------------------
//�����ļ�������
#if 0
	P2P_COMMAND::CCmdSendPiece *pCmdSend= new P2P_COMMAND::CCmdSendPiece();
	nSendLength = 0;
	pCmdSend->m_nSessionID=Cmdpack->m_nSessionID;
	pCmdSend->m_nOrderNumber=0;

	pCmdSend->m_nLeftPos=offset_;
	m_sendFileHandle->m_fd.Seek(offset_,MightyFile::begin);
	
	//pCmdSend->m_nDataLen=m_recvFileHandle->m_fd.Read(pCmdSend->m_szBuffer,sendLen_);

	pCmdSend->m_nDataLen=m_sendFileHandle->m_fd.Read(pCmdSend->m_szBuffer,sendLen_);
	
	Log_Info.str(_T(""));
	Log_Info<<"��ʼ�����ļ������� "<<"pCmdSend->m_nLeftPos :"<<pCmdSend->m_nLeftPos<<"pCmdSend->m_nDataLen "<<pCmdSend->m_nDataLen<<"  "<<"sendLen_ :"<<sendLen_<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	memcpy(m_sendBuf,(void *)(&cond),sizeof(char));

	if ( pCmdSend->Create(m_sendBuf+sizeof(char),nSendLength ) > 0 )
	{
		Send(m_sendBuf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmdSend;
#endif

	nSendLength = 0;
	m_pSendBufMemory->m_nSessionID=Cmdpack->m_nSessionID;
	m_pSendBufMemory->m_nOrderNumber=0;
	m_pSendBufMemory->m_nLeftPos=offset_;
	m_sendFileHandle->m_fd.Seek(offset_,MightyFile::begin);
	m_pSendBufMemory->m_nDataLen=m_sendFileHandle->m_fd.Read(m_pSendBufMemory->m_szBuffer,sendLen_);

	Log_Info.str(_T(""));
	Log_Info<<"��ʼ�����ļ������� "<<"m_pSendBufMemory->m_nLeftPos :"<<m_pSendBufMemory->m_nLeftPos<<"m_pSendBufMemory->m_nDataLen "<<m_pSendBufMemory->m_nDataLen<<"  "<<"sendLen_ :"<<sendLen_<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	memcpy(m_sendBuf,(void *)(&cond),sizeof(char));
	if ( m_pSendBufMemory->Create(m_sendBuf+sizeof(char),nSendLength ) > 0 )
	{
		Send(m_sendBuf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
		Log_Info.str(_T(""));
		Log_Info<<"�����ļ�������"<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
	}




}
void UDPClientOnRakNet::OnP2PSendPiece(P2P_COMMAND::CCmdSendPiece * Cmdpack)
{
	//lOff������ָ����nFrom���ļ��ڲ�ָ��ƫ����
		//nFrom ��MSDN��������ȡֵ��
		//CFile::begin�����ļ���ʼ���൱��0��
		//CFile::current  ��ǰλ�ã��൱��1����ͬ��0��2������ֵ��
		//CFile::end   �� �ļ���β���൱��2����ʱlOff����Ϊ��ֵ��
	//UQWORD totalspan=Cmdpack->m_nLeftPos;
	//if (Cmdpack)
	//{
	//}
	tostringstream Log_Info;
	tstring strlog;
	//��д���ļ�
	map<QWORD,unsigned long>::iterator leftPos_,rightPos_;

	if (!m_PosIndexBitMap.empty())
	{
		rightPos_=m_PosIndexBitMap.end();
		if ((leftPos_=m_PosIndexBitMap.find(Cmdpack->m_nLeftPos))!=rightPos_)
		{
			m_recvFileHandle->m_fd.Seek(Cmdpack->m_nLeftPos,MightyFile::begin);
			m_recvFileHandle->m_fd.Write(Cmdpack->m_szBuffer,Cmdpack->m_nDataLen);

			m_LockMemoryPool.writeEnter();
			m_memoryPool.push_back(Cmdpack->m_szBuffer);
			Cmdpack->m_szBuffer=NULL;
			m_LockMemoryPool.leave();

			Log_Info.str(_T(""));
			Log_Info<<"д���ļ�"<<"  "<<"�ļ�ƫ�� :"<<Cmdpack->m_nLeftPos<<" "<<"���ݳ��� :"<<Cmdpack->m_nDataLen<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			m_PosIndexBitMap.erase(leftPos_);
		}

	}
	


	//����ƫ��λ�ôӶ�����ɾ��,��Ҫ���ǳ�ʱ�������󣬱���ʵ���ȼ򵥵�ÿȡһ���������
	/*list<QWORD>::iterator leftPos,rightPos;
	rightPos=m_requestBitMapList.end();
	for (leftPos=m_requestBitMapList.begin();leftPos!=rightPos;leftPos++)
	{
		if (Cmdpack->m_nLeftPos==leftPos)
		{
			m_requestBitMapList.erase(leftPos);
			break;
		}
	}*/
	//------------------------------------
	//�����ļ����Ӧ
	P2P_COMMAND::CCmdSendPieceRet* pCmdPeer= new P2P_COMMAND::CCmdSendPieceRet();

	int nSendLength = 0;
	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=Cmdpack->m_dwIP;
	PeerSysAddr.port=Cmdpack->m_usPort;

	pCmdPeer->m_nSessionID=Cmdpack->m_nSessionID;

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));

	if ( pCmdPeer->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmdPeer;
	Log_Info.str(_T(""));
	Log_Info<<"�����ļ����Ӧ"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	//------------------------------------------------------
	//������һ�����ݿ�
	
	if(m_PosIndexBitMap.empty())
	{
		Log_Info.str(_T(""));
		Log_Info<<"д���ļ����"<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
		m_recvFileHandle->m_fd.Close();
		completeFlag=1;
		return;
	}
	if (!m_requestBitMapList.empty())
	{
		P2P_COMMAND::CCmdGetBlockArea * pCmdClient=new P2P_COMMAND::CCmdGetBlockArea();
		//QWORD startPos=m_requestBitMapList.front();
		nSendLength=0;
		pCmdClient->m_nSessionID=Cmdpack->m_nSessionID;
		pCmdClient->m_nFileID=0;
		pCmdClient->m_nBlockOffset=m_requestBitMapList.front();
		m_requestBitMapList.pop_front();
		pCmdClient->m_nBlocakSize=perPackLen;
		if ( pCmdClient->Create(buf+sizeof(char),nSendLength ) > 0 )
		{
			Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
		}
		delete pCmdClient;

		Log_Info.str(_T(""));
		Log_Info<<"������һ�����ݿ�"<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());
	}

}

void UDPClientOnRakNet::OnP2PEchoShootHole(P2P_COMMAND::CCmdEchoShootHole* Cmdpack)
{
//�յ�������ݱ�ʾ�򶴳ɹ�


//��ʱ���յ����ֻ�Ӧһ���������������ļ���

	tostringstream Log_Info;
	tstring strlog;

	PST_ST_PEER_SNAPSHOT pstPeer=new ST_PEER_SNAPSHOT();
	pstPeer->peer_external_addr.binaryAddress=Cmdpack->m_dwIP;
	pstPeer->peer_external_addr.port=Cmdpack->m_usPort;
	pstPeer->state=P2P_CMD_HANDSHAKE;
	m_peersMap.insert(map<int,PST_ST_PEER_SNAPSHOT>::value_type(Cmdpack->m_nSessionID,pstPeer));
	//--------------------------------
	//�����ļ���
	if (m_requestBitMapList.empty())
	{
		return;
	}
	P2P_COMMAND::CCmdGetBlockArea * pCmdClient=new P2P_COMMAND::CCmdGetBlockArea();
	//QWORD startPos=m_requestBitMapList.front();
	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=Cmdpack->m_dwIP;
	PeerSysAddr.port=Cmdpack->m_usPort;
	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;
	pCmdClient->m_nSessionID=Cmdpack->m_nSessionID;
	pCmdClient->m_nFileID=0;
	pCmdClient->m_nBlockOffset=m_requestBitMapList.front();
	m_requestBitMapList.pop_front();
	pCmdClient->m_nBlocakSize=perPackLen;
	if ( pCmdClient->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}
	delete pCmdClient;

	Log_Info.str(_T(""));
	Log_Info<<"�򶴳ɹ�,�����ļ���"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
}

void UDPClientOnRakNet::OnP2PAcceptReverseConnect(P2P_COMMAND::CCmdAcceptReverseConnect* pCmd)
{

}


//-------------------------------------------------------------------------


void UDPClientOnRakNet::OnP2PHandshakeRet(P2P_COMMAND::CCmdHandshakeRet *Cmdpack)
{
	tostringstream Log_Info;
	tstring strlog;
	PST_ST_PEER_SNAPSHOT pstPeer=new ST_PEER_SNAPSHOT();
	pstPeer->peer_external_addr.binaryAddress=Cmdpack->m_dwIP;
	pstPeer->peer_external_addr.port=Cmdpack->m_usPort;
	pstPeer->state=P2P_CMD_HANDSHAKE;
	m_peersMap.insert(map<int,PST_ST_PEER_SNAPSHOT>::value_type(Cmdpack->m_nSessionID,pstPeer));
//--------------------------------
//�����ļ���
	if (m_requestBitMapList.empty())
	{
		return;
	}
	P2P_COMMAND::CCmdGetBlockArea * pCmdClient=new P2P_COMMAND::CCmdGetBlockArea();
	//QWORD startPos=m_requestBitMapList.front();
	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=Cmdpack->m_dwIP;
	PeerSysAddr.port=Cmdpack->m_usPort;
	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;
	pCmdClient->m_nSessionID=Cmdpack->m_nSessionID;
	pCmdClient->m_nFileID=0;
	pCmdClient->m_nBlockOffset=m_requestBitMapList.front();
	m_requestBitMapList.pop_front();
	pCmdClient->m_nBlocakSize=perPackLen;
	if ( pCmdClient->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}
	delete pCmdClient;

	Log_Info.str(_T(""));
	Log_Info<<"�յ����ֻ�Ӧ,�����ļ���"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
}
void UDPClientOnRakNet::OnP2PGetBlockAreaRet(P2P_COMMAND::CCmdGetBlockAreaRet *Cmdpack)
{
	//�˴��Ϳ�ʼ׼���յ�CCmdSendPiece��

}
void UDPClientOnRakNet::OnP2PSendPieceRet(P2P_COMMAND::CCmdSendPieceRet *Cmdpack)
{
	//�˴��Ϳ�ʼ׼���յ���һ��CCmdGetBlockArea
}

void UDPClientOnRakNet::OnP2PEchoShootHoleRet(P2P_COMMAND::CCmdEchoShootHoleRet *Cmdpack)
{

}
void UDPClientOnRakNet::OnP2PAcceptReverseConnectRet(P2P_COMMAND::CCmdAcceptReverseConnectRet *pCmd)
{

}
//-----------------------------------------------------------------------
//������P2S�Ĵ���
//-----------------------------------------------------------------------
void UDPClientOnRakNet::OnP2SShootHole(P2S_COMMAND::CCmdShootHole *Cmdpack)
{

}
void UDPClientOnRakNet::OnP2SUdpHeartbeat(P2S_COMMAND::CCmdUdpHeartbeat *Cmdpack)
{

}
void UDPClientOnRakNet::OnP2SRegister(P2S_COMMAND::CCmdRegister * Cmdpack)
{

}
void UDPClientOnRakNet::OnP2SGetPeers(P2S_COMMAND::CCmdGetPeers * Cmdpack)
{

}
void UDPClientOnRakNet::OnP2SReqShootTo(P2S_COMMAND::CCmdReqShootTo *Cmdpack)
{//�յ�����������򶴵�����

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;
	tostringstream Log_Info;

	SystemAddress PeerSysAddr;

	P2P_COMMAND::CCmdEchoShootHole * pCmd=new P2P_COMMAND::CCmdEchoShootHole();

	PeerSysAddr.binaryAddress=Cmdpack->m_nExternalIP;
	PeerSysAddr.port=Cmdpack->m_nExternalPort;
	string strAddress=P2PUtilTools::IpToString(PeerSysAddr.binaryAddress);

	Log_Info.str(_T(""));
	Log_Info<<"�յ�����������򶴵����� "<<"�Է�IP"<<strAddress<<"�˿�"<<Cmdpack->m_nExternalPort<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());

	//ConnectServer(strAddress,PeerSysAddr.port,"test1");
	//Sleep(5);
	ConnectPeer(strAddress,PeerSysAddr.port,"test1");

	Sleep(5);
	SystemAddress usingAddr=GetSystemAddress();
	pCmd->m_nSessionID=0;
	pCmd->m_LocalIp=usingAddr.binaryAddress;;
	pCmd->m_tcpPort=9980;
	pCmd->m_udpPort=usingAddr.port;
	nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmd;
}

void UDPClientOnRakNet::OnP2SReverseConnectAssistance(P2S_COMMAND::CCmdReverseConnectAssistance * Cmdpack)
{//���������ǿͻ��˷�������������Э��֪ͨ��һ�������û���������



}

void UDPClientOnRakNet::OnPassOnReverseConnectAsk(P2S_COMMAND::CCmdPassOnReverseConnectAsk * Cmdpack)
{
	//���������Ƿ����������ͻ��˵�
	sendAcceptReverseConnectPack(Cmdpack->m_nSessionID,Cmdpack->m_nExternalIP,Cmdpack->m_nExternalUdpPort);
}

//-------------------------------------------------------------------
void UDPClientOnRakNet::OnP2SShootHoleRet(P2S_COMMAND::CCmdShootHoleRet * Cmdpack)
{
	//�յ��������İ����򶴻�Ӧ��������Զ˷�������
	char buf[512]={0};
	tostringstream Log_Info;

	//Log_Info.str(_T(""));
	//Log_Info<<"��ͻ��˷�������"<<_T("\r\n");
	//LOG(Log_Debug,Log_Info.str().c_str());


	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	SystemAddress PeerSysAddr;

	P2P_COMMAND::CCmdHandshake * pCmd=new P2P_COMMAND::CCmdHandshake();


	PeerSysAddr.binaryAddress=Cmdpack->m_nExternalIP;
	PeerSysAddr.port=Cmdpack->m_nExternalUdpPort;
	string strAddress=P2PUtilTools::IpToString(PeerSysAddr.binaryAddress);

	Log_Info.str(_T(""));
	Log_Info<<"�յ��������İ����򶴻�Ӧ��������Զ˷������� :"<<strAddress<<"port :"<<PeerSysAddr.port<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	
	//ConnectServer(strAddress,PeerSysAddr.port,"test1");
	
	ConnectPeer(strAddress,PeerSysAddr.port,"test1");
	Sleep(5);
	pCmd->m_nSessionID=m_client_sessionID;
	nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmd;
	

}
void UDPClientOnRakNet::OnP2SRegisterRet(P2S_COMMAND::CCmdRegisterRet * Cmdpack)
{
//CCmdGetPeers
/*	P2S_COMMAND::CCmdGetPeers * pCmd=new P2S_COMMAND::CCmdGetPeers();
	pCmd->m_nSessionID=Cmdpack->m_nSessionID;

	pCmd->m_nPeerCount=MAX_REQUEST_PEERS;

	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=((CDispatchControl *)m_MainCallBack)->m_serverIpAddr;
	PeerSysAddr.port=((CDispatchControl *)m_MainCallBack)->m_server_udpport;

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}
	delete pCmd;
	*/

	isLogin=true;

	m_SelfExternalIpAddr=Cmdpack->m_nExternalIP;
	m_SelfExternalUdpPort=Cmdpack->m_nExternalPort;

	m_IsLocalNetFlag=Cmdpack->m_nIsLocal;
	
}
void UDPClientOnRakNet::OnP2SGetPeersRet(P2S_COMMAND::CCmdGetPeersRet * Cmdpack)
{
//CCmdHandshake
	//�����ÿһ��m_nSessionID��Ҫָ��,��½��������Ҳ��Ҫָ��
	/*tostringstream Log_Info;


	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;
	list<PEERINFO>::iterator loopPos,endPos;
	SystemAddress PeerSysAddr;
	endPos=Cmdpack->m_listPeers.end();
	P2P_COMMAND::CCmdHandshake * pCmd=new P2P_COMMAND::CCmdHandshake();
	for (loopPos=Cmdpack->m_listPeers.begin();loopPos!=endPos;loopPos++)
	{
		PeerSysAddr.binaryAddress=(*loopPos).ulExternalIP;
		string strAddress=P2PUtilTools::IpToString(PeerSysAddr.binaryAddress);
		PeerSysAddr.port=(*loopPos).usExternalUDPPort;
		Log_Info.str(_T(""));
		Log_Info<<"���ӿͻ��� :"<<strAddress<<"port :"<<PeerSysAddr.port<<_T("\r\n");
		LOG(Log_Debug,Log_Info.str().c_str());

		ConnectServer(strAddress,PeerSysAddr.port,"test1");
		Sleep(5);
		pCmd->m_nSessionID=m_client_sessionID;
		nSendLength=0;

		if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
		{
			Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
		}
		
	}
	delete pCmd;*/

	//����򶴹��ܺ��յ����б�����������һ���߳���Ͷ�����ӿͻ��˻������
	
	if (!Cmdpack->m_listPeers.empty())
	{
		m_LockPeerQueue.writeEnter();
		m_PeerQueueList.clear();
		m_PeerQueueList=Cmdpack->m_listPeers;
		m_LockPeerQueue.leave();
	}

}


void UDPClientOnRakNet::OnP2SReqShootToRet(P2S_COMMAND::CCmdReqShootToRet*Cmdpack)
{

}
void UDPClientOnRakNet::OnP2SReverseConnectAssistanceRet(P2S_COMMAND::CCmdReverseConnectAssistanceRet * Cmdpack)
{
//����Ƿ������Է�����������Э���Ļ�Ӧ

}

void UDPClientOnRakNet::OnP2SPassOnReverseConnectAskRet(P2S_COMMAND::CCmdPassOnReverseConnectAskRet * Cmdpack )
{
//����ǿͻ��˷����������Ĵ��ݷ�����������Ӧ��
}
//--------------------------------------------------------------------------------------------------------------------
int UDPClientOnRakNet::GetPieceQueue(const string & szFileName)
{
	MightyFile m_myFile;
	QWORD perSet=0;
	QWORD totalLens=0;
	tostringstream Log_Info;
	int n1,n2,n3,n4;
	n1=0;
	n2=0;
	n3=0;
	n4=0;
	if (m_myFile.Open(szFileName.c_str(),MightyFile::modeRead|MightyFile::osSequentialScan|MightyFile::shareDenyNone))
	{
		/*byte* BaseBuffer = (byte*) malloc (perPackLen);
		int cnt;

		m_requestBitMapList.push_back(0);
		m_PosIndexBitMap.insert(map<QWORD,unsigned long> ::value_type(0,cnt));
		do {
			
			cnt = m_myFile.Read (BaseBuffer, perPackLen);
			if (cnt > 0) {
			


	
				perSet=m_myFile.GetPosition();
				m_requestBitMapList.push_back(perSet);

				m_PosIndexBitMap.insert(map<QWORD,unsigned long> ::value_type(perSet,cnt));
				Log_Info.str(_T(""));
				Log_Info<<"ƫ��: "<<perSet<<"����: "<<cnt<<_T("\r\n");
				LOG(Log_Debug,Log_Info.str().c_str());
			}
		} while (cnt == perPackLen);

		free (BaseBuffer);*/

		totalLens=m_myFile.GetLength();
		n1=totalLens%perPackLen;
		if (n1)
		{
			n2=totalLens/perPackLen;
			n3=totalLens-n2*perPackLen;
		}
		else
		{
			n2=totalLens/perPackLen;
			n3=0;
		}
		while (n4<n2)
		{
			m_requestBitMapList.push_back(n4*perPackLen);
			m_PosIndexBitMap.insert(map<QWORD,unsigned long> ::value_type(n4*perPackLen,perPackLen));
			Log_Info.str(_T(""));
			Log_Info<<"ƫ��: "<<n4*perPackLen<<"����: "<<perPackLen<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			n4++;

		}
		if (n3)
		{
			m_requestBitMapList.push_back(n2*perPackLen);
			m_PosIndexBitMap.insert(map<QWORD,unsigned long> ::value_type(n2*perPackLen,n3));
			Log_Info.str(_T(""));
			Log_Info<<"ƫ��: "<<n2*perPackLen<<"����: "<<n3<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
		}
		m_myFile.Close ();
	}
	return m_requestBitMapList.size();
}

void UDPClientOnRakNet::RepeatRequestPeer()
{
	P2S_COMMAND::CCmdGetPeers * pCmd=new P2S_COMMAND::CCmdGetPeers();
	pCmd->m_nSessionID=m_client_sessionID;

	pCmd->m_nPeerCount=MAX_REQUEST_PEERS;

	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=((CDispatchControl *)m_MainCallBack)->m_serverIpAddr;
	PeerSysAddr.port=((CDispatchControl *)m_MainCallBack)->m_server_udpport;

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}
	delete pCmd;
}

int UDPClientOnRakNet::GetRandSession()
{
	srand(GetTickCount());
	static int addNum=rand();
	addNum+=1;
	//return rand() * GetTickCount();
	return GetTickCount()+addNum;
}

int UDPClientOnRakNet::PeerManageTask()
{
	SystemAddress usingAddr=GetSystemAddress();
	tostringstream Log_Info;


	if (isLogin)
	{
		if (!m_peersMap.empty())
		{
			return 0;
		}
		if (m_PeerQueueList.empty())
		{
			
			int reDoCount=0;
			while(reDoCount<60)
			{
				if (!m_PeerQueueList.empty())
				{
					return 0;
					//break;
				}
				RepeatRequestPeer();
				Sleep(1000);
				reDoCount++;
			}

		}
		else
		{
			int reWorkCount=0;
			list<PEERINFO>::iterator loopPos,endPos;
			loopPos=m_PeerQueueList.begin();
			endPos=m_PeerQueueList.end();
			while(loopPos!=endPos)
			{
				
				if (((*loopPos).usLocalUDPPort==usingAddr.port)&&((*loopPos).ulLocalIP==usingAddr.binaryAddress))
				{
					Log_Info.str(_T(""));
					Log_Info<<"�ÿͻ��������Լ�"<<_T("\r\n");
					LOG(Log_Debug,Log_Info.str().c_str());
					loopPos++;
					continue;
				}
				/*
				if ((*loopPos).activeLifeTimeOut==0)
				{
					SendPeerHandShake((*loopPos));		//��ʼ״̬����ͻ���ֱ�ӷ�������
					(*loopPos).activeLifeTimeOut=GetTickCount();
					loopPos++;

				}
				else
				{
					//DWORD CurTienCount=GetTickCount();
					//if ((CurTienCount-(*loopPos).activeLifeTimeOut)>=PEER_WAIT_TIMEOUT_MAX)
					//{
						SendPeerShootHole((*loopPos));	//��ʱδ�ܵ��ͻ���������Ӧ������������������
						//(*loopPos).activeLifeTimeOut=GetTickCount();

					//}
					m_PeerQueueList.erase(loopPos++);
				}
				*/
				//SendPeerHandShake((*loopPos));
				//
				//m_SelfExternalUdpPort

				if(m_IsLocalNetFlag)
				{//�ұ���������
					if (((*loopPos).ulExternalIP==m_SelfExternalIpAddr)&&((*loopPos).ucIsLocal))
					{//������Ҵ���ͬһ��NAT��
						SendEqualNatPeerHandShake((*loopPos));
					}
					else if ((*loopPos).ucIsLocal==0)
					{//������������һ��������
						SendPeerHandShake((*loopPos));
					}
					else
					{//��������,��һ��Ҳ������,�������Ҳ���ͬһ��NAT��
						SendPeerShootHole((*loopPos));
					}
				}
				else
				{//��������
					
					if ((*loopPos).ucIsLocal==0)
					{//��һ��Ҳ������
						SendPeerHandShake((*loopPos));
					}
					else
					{//��һ��������
						SendReverseConnect((*loopPos));
					}
				}
				m_PeerQueueList.erase(loopPos++);

				
			}

			while(reWorkCount<30)
			{
				if (!m_peersMap.empty())
				{
					return 0;
				}
				Sleep(100);
				reWorkCount++;
			}

			//m_PeerQueueList.clear();
			return 1;
				
		}
	}
}

void UDPClientOnRakNet::SendPeerHandShake(PEERINFO & peer)
{
	//char buf[512]={0};
	tostringstream Log_Info;

	//Log_Info.str(_T(""));
	//Log_Info<<"��ͻ��˷�������"<<_T("\r\n");
	//LOG(Log_Debug,Log_Info.str().c_str());


	//char cond=raknet_wrapper::ID_IGT_CLIENT;
	//memcpy(buf,(void *)(&cond),sizeof(char));
	//int nSendLength=0;
	
	SystemAddress PeerSysAddr;
	
	//P2P_COMMAND::CCmdHandshake * pCmd=new P2P_COMMAND::CCmdHandshake();

	PeerSysAddr.binaryAddress=peer.ulExternalIP;
	PeerSysAddr.port=peer.usExternalUDPPort;
	string strAddress=P2PUtilTools::IpToString(PeerSysAddr.binaryAddress);
	
	Log_Info.str(_T(""));
	Log_Info<<"���ӿͻ��� :"<<strAddress<<"port :"<<PeerSysAddr.port<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	//Sleep(5);
	//ConnectServer(strAddress,PeerSysAddr.port,"test1");
	ConnectPeer(strAddress,PeerSysAddr.port,"test1");
	//Sleep(5);
	/*
	pCmd->m_nSessionID=m_client_sessionID;
	nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmd;*/

}
void UDPClientOnRakNet::SendEqualNatPeerHandShake(PEERINFO & peer)
{
	SystemAddress PeerSysAddr;

	PeerSysAddr.binaryAddress=peer.ulLocalIP;
	PeerSysAddr.port=peer.usLocalUDPPort;

	string strAddress=P2PUtilTools::IpToString(PeerSysAddr.binaryAddress);

	ConnectPeer(strAddress,PeerSysAddr.port,"test1");
}
void UDPClientOnRakNet::SendReverseConnect(PEERINFO & peer)
{

	char ReverseConnect[8192]={0};
	tostringstream Log_Info;

	//Log_Info.str(_T(""));
	//Log_Info<<"֪ͨ�����������Peer����������"<<_T("\r\n");
	//LOG(Log_Debug,Log_Info.str().c_str());

	char cond=raknet_wrapper::ID_IGT_CLIENT;

	memcpy(ReverseConnect,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	SystemAddress ServerSysAddr;

	P2S_COMMAND::CCmdReverseConnectAssistance * pCmd=new P2S_COMMAND::CCmdReverseConnectAssistance();

	ServerSysAddr.binaryAddress=((CDispatchControl *)m_MainCallBack)->m_serverIpAddr;
	ServerSysAddr.port=((CDispatchControl *)m_MainCallBack)->m_server_udpport;

	pCmd->m_nSessionID=0;
	pCmd->m_nReverseConnectPeerCount=1;
	pCmd->m_listPeers.push_back(peer);
	nSendLength=0;

	if ( pCmd->Create(ReverseConnect+sizeof(char),nSendLength ) > 0 )
	{
		Send(ReverseConnect, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,ServerSysAddr,false);
	}

	delete pCmd;
}
//-------------------------------------------------------------------------------------
void UDPClientOnRakNet::ResponsePeerConnect(Packet* p)
{
	char buf[512]={0};
	tostringstream Log_Info;

	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	SystemAddress PeerSysAddr;

	P2P_COMMAND::CCmdHandshake * pCmd=new P2P_COMMAND::CCmdHandshake();
	PeerSysAddr=p->systemAddress;


	Log_Info.str(_T(""));
	Log_Info<<"��ӦPeer���ӷ������ְ� �ͻ����ⲿIP:"<<PeerSysAddr.ToString()<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());

	pCmd->m_nSessionID=m_client_sessionID;
	nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmd;


}

void UDPClientOnRakNet::SendPeerShootHole(const PEERINFO & peer)
{
	char ShootHool[8192]={0};
	tostringstream Log_Info;

	Log_Info.str(_T(""));
	Log_Info<<"����������ʹ�����"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(ShootHool,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	SystemAddress ServerSysAddr;

	P2S_COMMAND::CCmdShootHole * pCmd=new P2S_COMMAND::CCmdShootHole();

	ServerSysAddr.binaryAddress=((CDispatchControl *)m_MainCallBack)->m_serverIpAddr;
	ServerSysAddr.port=((CDispatchControl *)m_MainCallBack)->m_server_udpport;

	pCmd->m_nSessionID=0;
	pCmd->m_nShootHolePeerCount=1;
	pCmd->m_listPeers.push_back(peer);
	nSendLength=0;

	if ( pCmd->Create(ShootHool+sizeof(char),nSendLength ) > 0 )
	{
		Send(ShootHool, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,ServerSysAddr,false);
	}

	delete pCmd;

}

void UDPClientOnRakNet::sendAcceptReverseConnectPack(int isID,int ip,unsigned short port)
{

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_CLIENT;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;
	tostringstream Log_Info;

	SystemAddress PeerSysAddr;

	P2P_COMMAND::CCmdAcceptReverseConnect * pCmd=new P2P_COMMAND::CCmdAcceptReverseConnect();

	PeerSysAddr.binaryAddress=ip;
	PeerSysAddr.port=port;
	string strAddress=P2PUtilTools::IpToString(PeerSysAddr.binaryAddress);

	Log_Info.str(_T(""));
	Log_Info<<"���ͽ��ܷ������������"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());



	ConnectPeer(strAddress,PeerSysAddr.port,"test1");

	Sleep(5);

	SystemAddress usingAddr=GetSystemAddress();
	pCmd->m_nSessionID=isID;
	pCmd->m_LocalIp=usingAddr.binaryAddress;;
	pCmd->m_tcpPort=9980;
	pCmd->m_udpPort=usingAddr.port;
	nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmd;
}