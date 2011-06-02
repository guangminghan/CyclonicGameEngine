//#include "StdAfx.h"
#include "RakClient.h"
#include "RakPeerInterface.h"
#include "GetTime.h"
#include "PacketID.h"
#include "NatPunchthrough.h"
#include "RakUtil.h"
namespace raknet_wrapper {


RakClient::RakClient( unsigned short maxPeerCount, const string & sessionKey ) 
: RakPeerBase(maxPeerCount)
, m_SessionKey(sessionKey)
{

}

RakClient::~RakClient()
{

}

bool RakClient::Init( unsigned short localPort, const string & localHost /*= "" */ )
{
	if ( !RakPeerBase::Init(localPort,localHost) )
	{
		return false;
	}

	SetIncomingPasswd(m_SessionKey);

	return true;
}

bool RakClient::ConnectServer( const string & serverHost, unsigned short serverPort, const string & passwd /*= "" */ )
{
	m_ServerAddress.SetBinaryAddress(serverHost.c_str());
	m_ServerAddress.port = serverPort;

	return m_pPeer->Connect( serverHost.c_str(), serverPort, passwd.c_str(), (int)passwd.size() );
}

// active connection
int RakClient::OnConnectionRequestAccepted( Packet* p )
{
	__LOG_PACKET(p);

	if ( p->systemAddress == m_ServerAddress )
	{
		return OnServerConnected(p);
	}

	return OnPeerConnected(p);
}

// passive connection
int RakClient::OnNewIncomingConnection( Packet* p )
{
	__LOG_PACKET(p);

	return OnPeerConnected(p);
}

int RakClient::OnConnectionAttemptFailed( Packet* p )
{
	__LOG_PACKET(p);
	return 0;
}

int RakClient::OnUserPacket( Packet* p )
{
	//switch(p->data[0])
	//{
	//case ID_BEGIN_HANDSHAKING : return OnBeginHandshaking(p);
	//}

	string internalID = m_pPeer->GetInternalID().ToString();
	Log("[%.2f] %21s >> %21s : %s(%d)",(float)RakNet::GetTime()/100.f,p->systemAddress.ToString(),internalID.c_str(),__FUNCTION__,p->data[0]);
	return -1;
}

int RakClient::OnServerConnected(Packet* p)
{
	Log("\t [%d/%d] Server connected.",m_pPeer->NumberOfConnections(),m_pPeer->GetMaximumNumberOfPeers());

	RakNet::BitStream out;
	//out.Write((unsigned char)ID_REGISTER_CLIENT);
	out.Write(m_SessionKey);

	Send(out, p->systemAddress);

	return 0;
}

int RakClient::OnPeerConnected(Packet* p)
{
	Log("\t [%d/%d] Peer connected from %s.", m_pPeer->NumberOfConnections(),m_pPeer->GetMaximumNumberOfPeers(), p->systemAddress.ToString() );

	return 0;
}

int RakClient::OnBeginHandshaking( Packet* p )
{
	__LOG_PACKET(p);

	BitStream in(p->data,p->length,false);
	in.IgnoreBits(sizeof(unsigned char)*8);

	string sessionKey;
	in.Read(sessionKey);

	SystemAddress peerAddress;
	in.Read(peerAddress);

	Log("\t sessionKey=%s", sessionKey.c_str());
	Log("\t peerAddress=%s", peerAddress.ToString());

	SystemAddress serverID = m_pPeer->GetSystemAddressFromIndex(0);

	if ( !m_pNatPunchthrough->Connect(peerAddress, sessionKey.c_str(), (int)sessionKey.size(), serverID )  )
	{
		Log("\t NatPunchthrough.Connect() failed");
		return -1;
	}

	return 0;
}
} // namespace raknet_wrapper
