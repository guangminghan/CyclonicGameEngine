//#include "StdAfx.h"
#include "RakPeerBase.h"
#include <cstdarg>

#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include "CustomPacketLogger.h"
#include "NatPunchthrough.h"
#include "GetTime.h"
#include "MessageIdentifiers.h"

namespace raknet_wrapper {

RakPeerBase::RakPeerBase( unsigned short maxPeerCount )
: m_pPeer(RakNetworkFactory::GetRakPeerInterface())
, m_pLogger(new CustomPacketLogger)
, m_pNatPunchthrough( new NatPunchthrough)
, m_MaxPeerCount(maxPeerCount)
{

}

RakPeerBase::~RakPeerBase()
{
	RakNetworkFactory::DestroyRakPeerInterface(m_pPeer);
}

bool RakPeerBase::Init( unsigned short localPort, const string & localHost /*= "" */ )
{
	SocketDescriptor socketDescriptor(localPort,localHost.empty()?0:localHost.c_str());
	if ( ! m_pPeer->Startup(m_MaxPeerCount,5,&socketDescriptor, 1) )
	{
		return false;
	}

	m_pPeer->SetMaximumIncomingConnections(m_MaxPeerCount);
	m_pPeer->AttachPlugin(m_pLogger);
	m_pPeer->AttachPlugin(m_pNatPunchthrough);

	Log("%s(port=%d,host=%s)",__FUNCTION__,localPort,localHost.c_str());

	return true;
}

void RakPeerBase::SetIncomingPasswd( const string & passwd )
{
	assert ( !passwd.empty() );

	m_pPeer->SetIncomingPassword(passwd.c_str(),(int)passwd.size());
}

string RakPeerBase::GetIncomingPasswd() const
{
	char passwd[128];
	int passwdLen = 128;

	m_pPeer->GetIncomingPassword(passwd,&passwdLen);

	passwd[passwdLen] = 0;

	return string(passwd,passwdLen);
}

bool RakPeerBase::Connect( const string & remoteHost, unsigned short remotePort, const string & passwd /*= "" */ )
{
	return m_pPeer->Connect(remoteHost.c_str(), remotePort, passwd.c_str(), (int)passwd.size());
}

void RakPeerBase::Disconnect( const SystemAddress & addr )
{
	m_pPeer->CloseConnection(addr,true);
}

void RakPeerBase::DisconnectAll()
{
	m_pPeer->Shutdown(1000);
}
void RakPeerBase::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, SystemAddress systemAddress, bool broadcast )
{
	m_pPeer->Send(data,length,priority,reliability,orderingChannel,systemAddress,broadcast);
}

void RakPeerBase::Send( BitStream & out, const SystemAddress & addr )
{
	m_pPeer->Send(&out, DEFAULT_PRIORITY, DEFAULT_RELIBILITY, DEFAULT_CHANNEL, addr, false);
}

void RakPeerBase::SendEx( BitStream & out, const SystemAddress & addr, PacketReliability reliability, char channel, PacketPriority priority )
{
	m_pPeer->Send(&out, priority, reliability, channel, addr, false);
}

void RakPeerBase::Broadcast( BitStream & out, const SystemAddress & except_addr /*= UNASSIGNED_SYSTEM_ADDRESS */ )
{
	m_pPeer->Send(&out, DEFAULT_PRIORITY, DEFAULT_RELIBILITY, DEFAULT_CHANNEL, except_addr, true);
}

void RakPeerBase::BroadcastEx( BitStream & out, const SystemAddress & addr, PacketReliability reliability, char channel, PacketPriority priority )
{
	m_pPeer->Send(&out, priority, reliability, channel, addr, true);
}

void RakPeerBase::LogPacket( Packet* p,const char * func )
{
	string internalID = m_pPeer->GetInternalID().ToString();
	Log("[%.2f] %21s >> %21s : %s()",(float)RakNet::GetTime()/100.f,p->systemAddress.ToString(),internalID.c_str(),func);
}

void RakPeerBase::Log( const char * fmt, ... )
{
	va_list vl;
	va_start(vl, fmt);
	char buf[256];
#if _MSC_VER >= 1400
	int nBuf = vsnprintf_s(buf, 256
#else
	int nBuf = _vsnprintf(buf
#endif
		, 255, fmt, vl);
	if(nBuf < 0)
		buf[255] = 0;
	va_end(vl);
	m_pLogger->WriteLog(buf);
}

int RakPeerBase::Update()
{
	int result = 0;

	//m_pPeer->UpdateNetwork();

	Packet *p = m_pPeer->Receive();
	if ( p )
	{
		switch( p->data[0] )
		{
			//////////////////////////////////////////////////////////////////////////
			/// RakPeer - In a client/server environment, our connection request to the server has been accepted.
		case ID_CONNECTION_REQUEST_ACCEPTED : 
			result = OnConnectionRequestAccepted(p); 
			break;

			/// RakPeer - Sent to the player when a connection request cannot be completed due to inability to connect.
		case ID_CONNECTION_ATTEMPT_FAILED : 
			result = OnConnectionAttemptFailed(p); 
			break;

			/// RakPeer - Sent a connect request to a system we are currently connected to.
		case ID_ALREADY_CONNECTED : 
			result = OnAlreadyConnected(p); 
			break;

			/// RakPeer - A remote system has successfully connected.
		case ID_NEW_INCOMING_CONNECTION : 
			result = OnNewIncomingConnection(p); 
			break;

			/// RakPeer - The system we attempted to connect to is not accepting new connections.
		case ID_NO_FREE_INCOMING_CONNECTIONS : 
			result = OnNoFreeIncomingConnections(p); 
			break;

			/// RakPeer - The system specified in Packet::systemAddress has disconnected from us.  For the client, this would mean the server has shutdown.
		case ID_DISCONNECTION_NOTIFICATION : 
			result = OnDisconnectionNotification(p); 
			break;

			/// RakPeer - Reliable packets cannot be delivered to the system specified in Packet::systemAddress.  The connection to that system has been closed.
		case ID_CONNECTION_LOST : 
			result = OnConnectionLost(p); 
			break;

			///// RakPeer - We preset an RSA public key which does not match what the system we connected to is using.
			//case ID_RSA_PUBLIC_KEY_MISMATCH,

			/// RakPeer - We are banned from the system we attempted to connect to.
		case ID_CONNECTION_BANNED : 
			result = OnConnectionBanned(p); 
			break;

			/// RakPeer - The remote system is using a password and has refused our connection because we did not set the correct password.
		case ID_INVALID_PASSWORD : 
			result = OnInvalidPassword(p); 
			break;

			/// RakPeer - A packet has been tampered with in transit.  The sender is contained in Packet::systemAddress.
		case ID_MODIFIED_PACKET : 
			result = OnModifiedPacket(p); 
			break;

			/// RakPeer - The four bytes following this byte represent an unsigned int which is automatically modified by the difference in system times between the sender and the recipient. Requires that you call SetOccasionalPing.
		case ID_TIMESTAMP : 
			result = OnTimestamp(p); 
			break;
			/// RakPeer - Pong from an unconnected system.  First byte is case ID_PONG, second sizeof(RakNetTime) bytes is the ping, following bytes is system specific enumeration data.
		case ID_PONG : 
			result = OnPong(p); 
			break;

			/// RakPeer - Inform a remote system of our IP/Port, plus some offline data
		case ID_ADVERTISE_SYSTEM : 
			result = OnAdvertiseSystem(p); 
			break;

			/// ConnectionGraph plugin - In a client/server environment, a client other than ourselves has disconnected gracefully.  Packet::systemAddress is modified to reflect the systemAddress of this client.
		case ID_REMOTE_DISCONNECTION_NOTIFICATION : 
			result = OnRemoteDisconnectionNotification(p); 
			break;

			/// ConnectionGraph plugin - In a client/server environment, a client other than ourselves has been forcefully dropped. Packet::systemAddress is modified to reflect the systemAddress of this client.
		case ID_REMOTE_CONNECTION_LOST : 
			result = OnRemoteConnectionLost(p);
			break;

			/// ConnectionGraph plugin - In a client/server environment, a client other than ourselves has connected.  Packet::systemAddress is modified to reflect the systemAddress of this client.
		case ID_REMOTE_NEW_INCOMING_CONNECTION :
			result = OnRemoteNewIncomingConnection(p);
			break;

		case ID_NAT_TARGET_NOT_CONNECTED :		
			result =  OnNatTargetNotConnected(p); 
			break;

		case ID_NAT_TARGET_CONNECTION_LOST :	
			result =  OnNatTargetConnectionLost(p); 
			break;


		default :								
			result =  OnUserPacket(p); 
			break;
		}

		m_pPeer->DeallocatePacket(p);
	}

	return result;

}

int RakPeerBase::OnUserPacket( Packet* p )
{
	string internalID = m_pPeer->GetInternalID().ToString();
	Log("[%.2f] %21s >> %21s : %s(%d)",(float)RakNet::GetTime()/100.f,p->systemAddress.ToString(),internalID.c_str(),__FUNCTION__,p->data[0]);
	return -1;
}

string RakPeerBase::GetLocalIP( size_t index ) const
{
	return m_pPeer->GetLocalIP((unsigned int)index);
}

SystemAddress RakPeerBase::GetSystemAddress() const
{
	return m_pPeer->GetInternalID();
}

} // namespace raknet_wrapper