//#include "StdAfx.h"
#include "RakServer.h"
#include "PacketID.h"
#include "RakPeerInterface.h"
#include "GetTime.h"
//#include "RakUtil.h"
namespace raknet_wrapper {


RakServer::RakServer( unsigned short maxPeerCount )
: RakPeerBase(maxPeerCount)
{

}

RakServer::~RakServer()
{

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
	//switch(p->data[0])
	//{
		//case ID_REGISTER_CLIENT : return OnRegisterClient(p);
	//}

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

} //namespace raknet_wrapper