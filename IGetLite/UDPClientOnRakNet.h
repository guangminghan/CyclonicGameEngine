#ifndef UDPCLIENTONRAKNET_INCLUDE_H
#define UDPCLIENTONRAKNET_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include "MightyFile.h"
#include "RakClient.h"

#include "PacketID.h"
#include "p2pdefine.h"
#include "p2sdefine.h"
#include "P2PCommand.h"
#include "P2SCommand.h"
using raknet_wrapper::RakClient;
typedef struct peer_snapshot
{
	SystemAddress peer_addr;
	SystemAddress peer_external_addr;
	int state;
	DWORD m_dwLastUpdateTime;

}ST_PEER_SNAPSHOT,*PST_ST_PEER_SNAPSHOT;
typedef struct file_operation
{
	//DWORD dwFileID;
	MightyFile m_fd;
	MightyFile m_fd_bitmap;
	bool m_isopen;
	map<UQWORD,int> m_bitmap_state;
}ST_FILE_OPERATION,*P_ST_FILE_OPERATION;
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
class FileIOQueueExecute : public MightyThread
{
public:
	FileIOQueueExecute(void);
	virtual ~FileIOQueueExecute(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
};

class PeerQueueMaintain: public MightyThread
{
public:
	PeerQueueMaintain(void);
	virtual ~PeerQueueMaintain(void);
public:
	virtual int threadrun(void * pBuf);

	virtual int ExecuteTask(void * pBuf);

	virtual int branchEvent(void * pBuf);
};
class UDPClientOnRakNet: public RakClient
{
public:
	UDPClientOnRakNet(const string & passWord,void * pPtrCall);
	virtual ~UDPClientOnRakNet(void);
public:
	virtual int Update();
	virtual int OnPeerConnected(Packet* p);
	virtual int OnUserPacket(Packet* p);
	virtual int OnServerConnected(Packet* p);
	virtual int OnConnectionAttemptFailed( Packet* p );
//	virtual int OnPeerConnected(Packet* p);
	virtual int OnConnectionLost(Packet* p);
	void TransUdpCommand(CBaseCommand* pCmd);
public:
	int OnHandleP2PProcess(Packet* p);
	int OnHandleP2SProcess(Packet* p);

	void OnP2PHandshake(P2P_COMMAND::CCmdHandshake * Cmdpack);


	void OnP2PGetBlockArea(P2P_COMMAND::CCmdGetBlockArea * Cmdpack);

	void OnP2PSendPiece(P2P_COMMAND::CCmdSendPiece * Cmdpack);

	void OnP2PEchoShootHole(P2P_COMMAND::CCmdEchoShootHole* Cmdpack);
	void OnP2PAcceptReverseConnect(P2P_COMMAND::CCmdAcceptReverseConnect* pCmd);

	//------------------------------------------
	void OnP2PHandshakeRet(P2P_COMMAND::CCmdHandshakeRet *Cmdpack);

	void OnP2PGetBlockAreaRet(P2P_COMMAND::CCmdGetBlockAreaRet *Cmdpack);

	void OnP2PSendPieceRet(P2P_COMMAND::CCmdSendPieceRet *Cmdpack);
	void OnP2PEchoShootHoleRet(P2P_COMMAND::CCmdEchoShootHoleRet *Cmdpack);

	void OnP2PAcceptReverseConnectRet(P2P_COMMAND::CCmdAcceptReverseConnectRet *pCmd);

	//-----------------------------------------------------------------------
	void OnP2SShootHole(P2S_COMMAND::CCmdShootHole *Cmdpack);

	void OnP2SUdpHeartbeat(P2S_COMMAND::CCmdUdpHeartbeat *Cmdpack);

	void OnP2SRegister(P2S_COMMAND::CCmdRegister * Cmdpack);

	void OnP2SGetPeers(P2S_COMMAND::CCmdGetPeers * Cmdpack);

	void OnP2SReqShootTo(P2S_COMMAND::CCmdReqShootTo *Cmdpack);
	void OnP2SReverseConnectAssistance(P2S_COMMAND::CCmdReverseConnectAssistance * Cmdpack);
	void OnPassOnReverseConnectAsk(P2S_COMMAND::CCmdPassOnReverseConnectAsk * Cmdpack);

	//-------------------------------------------------------------------
	void OnP2SShootHoleRet(P2S_COMMAND::CCmdShootHoleRet * Cmdpack);

	void OnP2SRegisterRet(P2S_COMMAND::CCmdRegisterRet * Cmdpack);

	void OnP2SGetPeersRet(P2S_COMMAND::CCmdGetPeersRet * Cmdpack);
	void OnP2SReqShootToRet(P2S_COMMAND::CCmdReqShootToRet*Cmdpack);

	void OnP2SReverseConnectAssistanceRet(P2S_COMMAND::CCmdReverseConnectAssistanceRet * Cmdpack);
	void OnP2SPassOnReverseConnectAskRet(P2S_COMMAND::CCmdPassOnReverseConnectAskRet * Cmdpack );
	//----------------------------------------------------------------------
	void RepeatRequestPeer();
	int GetRandSession();


	//----------------------------------------------------------------------
	int PeerManageTask();

	void SendPeerHandShake(PEERINFO & peer);
	void SendEqualNatPeerHandShake(PEERINFO & peer);
	void SendReverseConnect(PEERINFO & peer);
	void ResponsePeerConnect(Packet* p);
	void SendPeerShootHole(const PEERINFO & peer);

	void sendAcceptReverseConnectPack(int isID,int ip,unsigned short port);

private:
	int GetPieceQueue(const string & szFileName);

public:
	void * m_MainCallBack;
	TaskQueueExecute * m_task;
	PeerQueueMaintain * m_PeerQueueControl;
	DWORD m_dwExternalIP;
	WORD m_externalPort;
	bool isLogin;
	map<int,PST_ST_PEER_SNAPSHOT> m_peersMap;

	map<QWORD,P_ST_FILE_OPERATION> m_fileIoMap;

	map<QWORD,unsigned long> m_PosIndexBitMap;

	list<PEERINFO> m_PeerQueueList;

	//list<P2P_COMMAND::CCmdSendPiece *> m_memoryPool;
	list<char *> m_memoryPool;
	list<P2P_COMMAND::CCmdSendPiece *> m_sendDataBufferPool;
	P2P_COMMAND::CCmdSendPiece * m_pSendBufMemory;
	
	P_ST_FILE_OPERATION m_recvFileHandle;

	P_ST_FILE_OPERATION m_sendFileHandle;
	//char * m_recvBuf;
	char * m_sendBuf;
	list<QWORD> m_requestBitMapList;
	int m_client_sessionID;

	CMultipleLock m_LockPeerQueue;

	CMultipleLock m_LockMemoryPool;


	unsigned int m_SelfExternalIpAddr;
	unsigned short m_SelfExternalUdpPort;

	int m_IsLocalNetFlag;


};
#endif
