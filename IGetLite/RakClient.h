#pragma once

#include "RakPeerBase.h"

namespace raknet_wrapper {

	class RakClient 
		: public RakPeerBase
	{
	public :
		RakClient( unsigned short maxPeerCount, const string & sessionKey );
		virtual ~RakClient();

		virtual bool Init( unsigned short localPort, const string & localHost = "" );
		virtual bool ConnectServer( const string & serverHost, unsigned short serverPort, const string & passwd = "" );
		virtual bool ConnectPeer( const string & peerHost, unsigned short peerPort, const string & passwd = "" );
		void BroadcastPeer( BitStream & out ) { Broadcast(out,m_ServerAddress); }

		virtual int OnConnectionRequestAccepted(Packet* p);
		virtual int OnNewIncomingConnection(Packet* p);
		virtual int OnConnectionAttemptFailed(Packet* p);
		virtual int OnUserPacket(Packet* p);
		virtual int OnConnectionLost(Packet* p);

	public :
		virtual int OnServerConnected(Packet* p);
		virtual int OnPeerConnected(Packet* p);
		virtual int OnBeginHandshaking(Packet* p);
		SystemAddress GetServerIPAddr();

	protected :

		SystemAddress	m_ServerAddress;

		// used as a key for matching TCP session in server.
		// used as a passwd for p2p connecting. 
		// should get this value through TCP session.
		string			m_SessionKey;				
	};

} // namespace raknet_wrapper
