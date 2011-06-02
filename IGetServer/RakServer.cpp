//#include "StdAfx.h"
#include "RakServer.h"
#include "PacketID.h"
#include "RakPeerInterface.h"
#include "GetTime.h"
//#include "RakUtil.h"
#include "p2pdefine.h"
#include "p2sdefine.h"
#include "P2PCommand.h"
#include "P2SCommand.h"
#include ".\CommonInc\P2PUtilTools.h"

#include ".\CommonInc\Log.h"
using namespace raknet_wrapper;
TaskQueueExecute::TaskQueueExecute(void)
{

}
TaskQueueExecute::~TaskQueueExecute(void)
{
	exit();
}

int TaskQueueExecute::threadrun(void * pBuf)
{
	RakServer * ptrUser=(RakServer *)pBuf;

	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;
	CBaseCommand * pCmdPack;
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

				WriteWait();
				pCmdPack=NULL;
				if (!m_taskQueue.empty())
				{
					pCmdPack=m_taskQueue.front();
					m_taskQueue.pop_front();
				}
				CancelWriteWait();
				if (pCmdPack)
				{
					ptrUser->TransUdpCommand(pCmdPack);
					delete pCmdPack;
				}

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
namespace raknet_wrapper {


RakServer::RakServer( unsigned short maxPeerCount )
: RakPeerBase(maxPeerCount)
{
	m_task=new TaskQueueExecute();
	m_task->setTimeOutVal(100);
	m_task->start(this);
	m_task->setExecuteSignal();
}

RakServer::~RakServer()
{
	if (m_task)
	{
		delete m_task;
	}
}

int RakServer::OnNewIncomingConnection( Packet* p )
{
	__LOG_PACKET(p);

	return 0;
}

int RakServer::OnDisconnectionNotification( Packet* p )
{
	__LOG_PACKET(p);

	return 0;
}

int RakServer::OnConnectionLost( Packet* p )
{
	__LOG_PACKET(p);

	return 0;
}

int RakServer::OnUserPacket( Packet* p )
{
	switch(p->data[0])
	{
		case (raknet_wrapper::ID_IGT_CLIENT) :
			return OnHandleIOprocess(p);

		default:
			break;
	}

	string internalID = m_pPeer->GetInternalID().ToString();
	Log("[%.2f] %21s >> %21s : %s(%d)",(float)RakNet::GetTime()/100.f,p->systemAddress.ToString(),internalID.c_str(),__FUNCTION__,p->data[0]);

	return -1;
}

int RakServer::OnRegisterClient( Packet * p )
{
	__LOG_PACKET(p);

	BitStream in(p->data,p->length,false);

	in.IgnoreBits(sizeof(unsigned char)*8);

	string sessionKey;
	in.Read(sessionKey);

	BitStream out;
	//out.Write((unsigned char)ID_BEGIN_HANDSHAKING);
	out.Write(sessionKey);			// password for handshaking
	out.Write(p->systemAddress);	// peer Address

	Log("\t sessionKey=%s", sessionKey.c_str());

	
	Broadcast(out, p->systemAddress);

	return 0;
}
int RakServer::OnHandleIOprocess(Packet* p)
{

	char * pIndexBuf=(char *)(p->data+1);
	P2S_COMMAND::CP2SBaseCommand * pCmd = NULL;
	tostringstream Log_Info;
	tstring strlog;
	//strlog=_T("连接到服务器");
	
	string strAddress=p->systemAddress.ToString();//P2PUtilTools::IpToString(p->systemAddress.binaryAddress);
	
	switch( CBaseCommand::getID( pIndexBuf ) ) 
	{
		case P2S_CMD_SHOOTHOLE:
			pCmd = new P2S_COMMAND::CCmdShootHole();
			Log_Info.str(_T(""));
			Log_Info<<"打洞请求数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;
		case P2S_CMD_UDP_HEARTBEAT:
			pCmd = new P2S_COMMAND::CCmdUdpHeartbeat();
			Log_Info.str(_T(""));
			Log_Info<<"心跳数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;
		case P2S_CMD_LOGIN:
			pCmd = new P2S_COMMAND::CCmdRegister();
			Log_Info.str(_T(""));
			Log_Info<<"登陆注册数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;
		case P2S_CMD_GETPEERS:
			pCmd = new P2S_COMMAND::CCmdGetPeers();
			Log_Info.str(_T(""));
			Log_Info<<"向服务器请求节点数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;			
		case P2S_CMD_REQSHOOTTO:
			pCmd = new P2S_COMMAND::CCmdReqShootTo();
			Log_Info.str(_T(""));
			Log_Info<<"协助打洞通知数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;

		case P2S_CMD_REVERSECONNECT_ASSISTANCE :
			strlog=_T("P2S反向连接协助请求包");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdReverseConnectAssistance();
			break;
		case P2S_CMD_PASSONREVERSECONNECT_ASK :
			strlog=_T("P2S传递反向连接请求包");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdPassOnReverseConnectAsk();
			break;

		//---------------------------------------------------------------------
		case P2S_CMD_SHOOTHOLE_RET:
			pCmd = new P2S_COMMAND::CCmdShootHoleRet();
			Log_Info.str(_T(""));
			Log_Info<<"打洞请求回应数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;
		case P2S_CMD_LOGIN_RET:
			pCmd = new P2S_COMMAND::CCmdRegisterRet();
			Log_Info.str(_T(""));
			Log_Info<<"登陆注册回应数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;

		case P2S_CMD_GETPEERS_RET:		
			pCmd = new P2S_COMMAND::CCmdGetPeersRet();
			Log_Info.str(_T(""));
			Log_Info<<"向服务器请求节点回应数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;
		case P2S_CMD_REQSHOOTTO_RET:
			pCmd = new P2S_COMMAND::CCmdReqShootToRet();
			Log_Info.str(_T(""));
			Log_Info<<"协助打洞通知回应数据包"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<p->systemAddress.port<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			break;
		case P2S_CMD_REVERSECONNECT_ASSISTANCE_RET :
			strlog=_T("P2S反向连接协助回应包");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdReverseConnectAssistanceRet();
			break;
		case P2S_CMD_PASSONREVERSECONNECT_ASK_RET :
			strlog=_T("P2S传递反向连接请求回应包");
			Log_Info.str(_T(""));
			Log_Info<<strlog<<_T("\r\n");
			LOG(Log_Debug,Log_Info.str().c_str());
			pCmd = new P2S_COMMAND::CCmdPassOnReverseConnectAsk();
			break;
		//----------------------------------------------------------------------
		default:
			break;
	}

	if ( pCmd )
	{
		try
		{
			pCmd->m_dwIP =p->systemAddress.binaryAddress;
			pCmd->m_usPort =p->systemAddress.port;
			int nLength = pCmd->Parse(pIndexBuf,p->length-1);
			m_task->pushIn(pCmd);

		}
		catch ( ... ) 
		{
		}

		//delete pCmd;
	}
	else
	{
		printf("pCmd is null id => %u\n", CBaseCommand::getID( pIndexBuf ) );
	}
	return 0;
}

void RakServer::TransUdpCommand(CBaseCommand* pCmd)
{
	switch( pCmd->ID() ) 
	{

	case P2S_CMD_SHOOTHOLE:
		OnP2SShootHole( reinterpret_cast<P2S_COMMAND::CCmdShootHole *>(pCmd) );
		break;
		
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
		OnReqShootTo(reinterpret_cast<P2S_COMMAND::CCmdReqShootTo*>(pCmd) );
		break;
	case P2S_CMD_REVERSECONNECT_ASSISTANCE :
		OnP2SReverseConnectAssistance(reinterpret_cast<P2S_COMMAND::CCmdReverseConnectAssistance*>(pCmd) );
		break;
	case P2S_CMD_PASSONREVERSECONNECT_ASK :
		OnP2SPassOnReverseConnectAsk(reinterpret_cast<P2S_COMMAND::CCmdPassOnReverseConnectAsk*>(pCmd) );
		break;

//-------------------------------------------------------------------------------------
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
		OnReqShootToRet(reinterpret_cast<P2S_COMMAND::CCmdReqShootToRet*>(pCmd) );
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

//-----------------------------------------------------------------------
void RakServer::OnP2SShootHole(P2S_COMMAND::CCmdShootHole *Cmdpack)
{
	
	tostringstream Log_Info;
	string strAddress;
	if ((Cmdpack->m_nShootHolePeerCount>0)&&(!Cmdpack->m_listPeers.empty()))
	{

		P2S_COMMAND::CCmdShootHoleRet * pPackage=new P2S_COMMAND::CCmdShootHoleRet();
		P2S_COMMAND::CCmdReqShootTo * pCmd=new P2S_COMMAND::CCmdReqShootTo();
		pCmd->m_nSessionID=Cmdpack->m_nSessionID;


		pCmd->m_nExternalIP=Cmdpack->m_dwIP;//把发起打洞请求的客户端的外部IP和端口保存在反向请求打洞包中
		pCmd->m_nExternalPort=Cmdpack->m_usPort;
		list<PEERINFO>::iterator loopIndex,endIndex;
		loopIndex=Cmdpack->m_listPeers.begin();
		endIndex=Cmdpack->m_listPeers.end();
		while(loopIndex!=endIndex)
		{
			char data[1024]={0};
			char commandID=raknet_wrapper::ID_IGT_SERVER;
			memcpy(data,(void *)(&commandID),sizeof(char));
			int nSendSize=0;
			SystemAddress ClientSysAddr;
			ClientSysAddr.binaryAddress=Cmdpack->m_dwIP;
			ClientSysAddr.port=Cmdpack->m_usPort;
			//-------------------
			SystemAddress PeerSysAddr;
			PeerSysAddr.binaryAddress=(*loopIndex).ulExternalIP;
			PeerSysAddr.port=(*loopIndex).usExternalUDPPort;
			//-------------------
			MakeHelpShootHoleRetPackage(pPackage,Cmdpack->m_nSessionID,(*loopIndex).ulExternalIP,(*loopIndex).usExternalUDPPort);

			if ( pPackage->Create(data+sizeof(char),nSendSize ) > 0 )
			{
				Send(data, (const int) nSendSize+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,ClientSysAddr,false);

				Log_Info.str(_T(""));
				Log_Info<<"打洞通知立即双向互连"<<" 客户外网IP : "<<ClientSysAddr.ToString()<<"反向打洞客户外网IP :"<<PeerSysAddr.ToString()<<_T("\r\n");
				LOG(Log_Debug,Log_Info.str().c_str());
			}
			//---------------------------------------------------------------------

			strAddress=PeerSysAddr.ToString();
			char buf[512]={0};
			char cond=raknet_wrapper::ID_IGT_SERVER;
			memcpy(buf,(void *)(&cond),sizeof(char));
			int nSendLength=0;

			if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
			{
				Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);

				Log_Info.str(_T(""));
				Log_Info<<"协助打洞通知"<<" 客户外网IP : "<<strAddress<<" 端口 : "<<PeerSysAddr.port<<_T("\r\n");
				LOG(Log_Debug,Log_Info.str().c_str());
			}
			//---------------------------------------------------------------------

			loopIndex++;
		}


		delete pCmd;
		delete pPackage;
	}

}
void RakServer::OnP2SUdpHeartbeat(P2S_COMMAND::CCmdUdpHeartbeat *Cmdpack)
{

}
void RakServer::OnP2SRegister(P2S_COMMAND::CCmdRegister * Cmdpack)
{

	PEERINFO newPeerInfo;
	string strAddress=P2PUtilTools::IpToString(Cmdpack->m_dwIP);
	string strLocalIPAddr;
	newPeerInfo.ulExternalIP=Cmdpack->m_dwIP;
	newPeerInfo.usExternalUDPPort=Cmdpack->m_usPort;
	newPeerInfo.ucIsLocal=Cmdpack->m_dwIP==Cmdpack->m_nLocalIP?0:1;//相等不是内网,不相等是内网
	newPeerInfo.ulLocalIP=Cmdpack->m_nLocalIP;
	newPeerInfo.usLocalTCPPort=Cmdpack->m_sLocalTcpPort;
	newPeerInfo.usLocalUDPPort=Cmdpack->m_sLocalUdpPort;
	
	strLocalIPAddr=P2PUtilTools::IpToString(Cmdpack->m_nLocalIP);	


	
	//m_peer_database.push_back(newPeerInfo);
	m_peer_database.insert(map<int,PEERINFO>::value_type(Cmdpack->m_nSessionID,newPeerInfo));
	
	tostringstream Log_Info;
	Log_Info.str(_T(""));
	Log_Info<<"客户端注册"<<" 客户外网IP : "<<strAddress<<" 外网端口 : "<<Cmdpack->m_usPort<<"内网IP :"<<strLocalIPAddr<<"内网端口 : "<<Cmdpack->m_sLocalUdpPort<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());
	//---------------------------------------------------------------------
	//发送注册回应
	P2S_COMMAND::CCmdRegisterRet * pCmd=new P2S_COMMAND::CCmdRegisterRet();
	pCmd->m_nSessionID=Cmdpack->m_nSessionID;

	pCmd->m_nExternalIP=newPeerInfo.ulExternalIP;
	pCmd->m_nExternalPort=newPeerInfo.usExternalUDPPort;
	pCmd->m_nIsLocal=newPeerInfo.ucIsLocal;

	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=Cmdpack->m_dwIP;
	PeerSysAddr.port=Cmdpack->m_usPort;

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_SERVER;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}
	delete pCmd;

}
void RakServer::OnP2SGetPeers(P2S_COMMAND::CCmdGetPeers * Cmdpack)
{

	int provideCount=Cmdpack->m_nPeerCount;//<m_peer_database.size()?Cmdpack->m_nPeerCount:m_peer_database.size();
	int extractCount=0;
	char buf[4096]={0};
	char cond=raknet_wrapper::ID_IGT_SERVER;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;
	//list<PEERINFO>::iterator loopPos,endPos;
	map<int,PEERINFO>::iterator loopPos,endPos;
	SystemAddress PeerSysAddr;
	endPos=m_peer_database.end();
	P2S_COMMAND::CCmdGetPeersRet * pCmd=new P2S_COMMAND::CCmdGetPeersRet();
	for (loopPos=m_peer_database.begin();loopPos!=endPos;loopPos++)
	{
		if (loopPos->first==Cmdpack->m_nSessionID)
		{
			continue;
		}
		pCmd->m_listPeers.push_back(loopPos->second);
		extractCount++;
		if (extractCount>=provideCount)
		{
			break;
		}

	}
	tostringstream Log_Info;
	Log_Info.str(_T(""));
	Log_Info<<"向客户端发送节点"<<_T("\r\n");
	LOG(Log_Debug,Log_Info.str().c_str());

	Cmdpack->m_nPeerCount=extractCount;
	PeerSysAddr.binaryAddress=Cmdpack->m_dwIP;
	PeerSysAddr.port=Cmdpack->m_usPort;
	pCmd->m_nSessionID=0;
	nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}
	delete pCmd;

}
void RakServer::OnReqShootTo(P2S_COMMAND::CCmdReqShootTo * Cmdpack)
{
//这个命令包是服务器发给客户端的

}

void RakServer::OnP2SReverseConnectAssistance(P2S_COMMAND::CCmdReverseConnectAssistance * Cmdpack)
{//这个命令包是客户端发给服务器请求协助通知另一个内网用户反向连接
	if (!Cmdpack->m_listPeers.empty())
	{
		list<PEERINFO>::iterator loopIndex,endIndex;
		loopIndex=Cmdpack->m_listPeers.begin();
		endIndex=Cmdpack->m_listPeers.end();
		while(loopIndex!=endIndex)
		{
			sendReverseConnectAssistancePack((*loopIndex),Cmdpack->m_nSessionID,Cmdpack->m_dwIP,Cmdpack->m_usPort);
			loopIndex++;
		}
	}
	

}

void RakServer::OnP2SPassOnReverseConnectAsk(P2S_COMMAND::CCmdPassOnReverseConnectAsk * Cmdpack)
{
//这个命令包是服务器发给客户端的
}
//-------------------------------------------------------------------
void RakServer::OnP2SShootHoleRet(P2S_COMMAND::CCmdShootHoleRet * Cmdpack)
{

}
void RakServer::OnP2SRegisterRet(P2S_COMMAND::CCmdRegisterRet * Cmdpack)
{
	


}
void RakServer::OnP2SGetPeersRet(P2S_COMMAND::CCmdGetPeersRet * Cmdpack)
{
	//CCmdHandshake
	//这个的每一个m_nSessionID需要指定,登陆服务器的也需要指定


}
void RakServer::OnReqShootToRet(P2S_COMMAND::CCmdReqShootToRet *Cmdpack)
{

}
void RakServer::OnP2SReverseConnectAssistanceRet(P2S_COMMAND::CCmdReverseConnectAssistanceRet * Cmdpack)
{

}

void RakServer::OnP2SPassOnReverseConnectAskRet(P2S_COMMAND::CCmdPassOnReverseConnectAskRet * Cmdpack )
{

}
//-----------------------------------------------------------------------
P2S_COMMAND::CCmdShootHoleRet * RakServer::MakeHelpShootHoleRetPackage(P2S_COMMAND::CCmdShootHoleRet * pCmpPackage,const int & iNum,const unsigned int & iIP,const short & port)
{
	//P2S_COMMAND::CCmdShootHoleRet * pCmpPackage=new P2S_COMMAND::CCmdShootHoleRet();
	if (pCmpPackage!=NULL)
	{
		pCmpPackage->m_nSessionID=iNum;
		pCmpPackage->m_nState=0;
		pCmpPackage->m_nExternalIP=iIP;
		pCmpPackage->m_nExternalUdpPort=port;
	}
	return pCmpPackage;

}

void RakServer::sendReverseConnectAssistancePack(PEERINFO & peer,int isID,DWORD requestPeerIP,unsigned short requestPeerPort)
{

	P2S_COMMAND::CCmdPassOnReverseConnectAsk * pCmd=new P2S_COMMAND::CCmdPassOnReverseConnectAsk();
	SystemAddress PeerSysAddr;
	PeerSysAddr.binaryAddress=peer.ulExternalIP;
	PeerSysAddr.port=peer.usExternalUDPPort;

	pCmd->m_nSessionID=isID;
	pCmd->m_nExternalIP=requestPeerIP;
	pCmd->m_nExternalUdpPort=requestPeerPort;

	//strAddress=PeerSysAddr.ToString();

	char buf[512]={0};
	char cond=raknet_wrapper::ID_IGT_SERVER;
	memcpy(buf,(void *)(&cond),sizeof(char));
	int nSendLength=0;

	if ( pCmd->Create(buf+sizeof(char),nSendLength ) > 0 )
	{
		Send(buf, (const int) nSendLength+sizeof(char),HIGH_PRIORITY, RELIABLE_ORDERED,0,PeerSysAddr,false);
	}

	delete pCmd;

}
} //namespace raknet_wrapper