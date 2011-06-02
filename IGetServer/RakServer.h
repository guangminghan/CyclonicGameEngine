#pragma once
#include "./CommonInc/common.h"
#include "./CommonInc/MightyThread.h"
#include "RakPeerBase.h"

#include "PacketID.h"
#include "p2pdefine.h"
#include "p2sdefine.h"
#include "P2PCommand.h"
#include "P2SCommand.h"
class TaskQueueExecute : public MightyThread
{
public:
	TaskQueueExecute(void);
	virtual ~TaskQueueExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
public:
	void pushIn(CBaseCommand* pCmd );
private:
	list<CBaseCommand * > m_taskQueue;

};
namespace raknet_wrapper {

	class RakServer 
		: public RakPeerBase
	{
	public :
		RakServer( unsigned short maxPeerCount );
		virtual ~RakServer();

		virtual int OnNewIncomingConnection(Packet* p);
		virtual int OnDisconnectionNotification(Packet* p);
		virtual int OnConnectionLost(Packet* p);
		virtual int OnUserPacket(Packet* p);

	public :

		virtual int OnRegisterClient(Packet * p);
		virtual int OnHandleIOprocess(Packet* p);

		void TransUdpCommand(CBaseCommand* pCmd);

	private:
		//-----------------------------------------------------------------------
		void OnP2SShootHole(P2S_COMMAND::CCmdShootHole *Cmdpack);

		void OnP2SUdpHeartbeat(P2S_COMMAND::CCmdUdpHeartbeat *Cmdpack);

		void OnP2SRegister(P2S_COMMAND::CCmdRegister * Cmdpack);

		void OnP2SGetPeers(P2S_COMMAND::CCmdGetPeers * Cmdpack);
		void OnReqShootTo(P2S_COMMAND::CCmdReqShootTo * Cmdpack);

		void OnP2SReverseConnectAssistance(P2S_COMMAND::CCmdReverseConnectAssistance * Cmdpack);
		void OnP2SPassOnReverseConnectAsk(P2S_COMMAND::CCmdPassOnReverseConnectAsk * Cmdpack);
		//-------------------------------------------------------------------
		void OnP2SShootHoleRet(P2S_COMMAND::CCmdShootHoleRet * Cmdpack);

		void OnP2SRegisterRet(P2S_COMMAND::CCmdRegisterRet * Cmdpack);

		void OnP2SGetPeersRet(P2S_COMMAND::CCmdGetPeersRet * Cmdpack);
		void OnReqShootToRet(P2S_COMMAND::CCmdReqShootToRet *Cmdpack);

		void OnP2SReverseConnectAssistanceRet(P2S_COMMAND::CCmdReverseConnectAssistanceRet * Cmdpack);


		void OnP2SPassOnReverseConnectAskRet(P2S_COMMAND::CCmdPassOnReverseConnectAskRet * Cmdpack );
		//-----------------------------------------------------------------------------
		P2S_COMMAND::CCmdShootHoleRet * MakeHelpShootHoleRetPackage(P2S_COMMAND::CCmdShootHoleRet * pCmpPackage,const int & iNum,const unsigned int & iIP,const short & port);
		void sendReverseConnectAssistancePack(PEERINFO & peer,int isID,DWORD requestPeerIP,unsigned short requestPeerPort);
	public:
		//list<PEERINFO> m_peer_database;
		TaskQueueExecute * m_task;
		map<int,PEERINFO> m_peer_database;

	};
}