#pragma once
#include "./CommonInc/common.h"
#include "PacketPriority.h"
#include "BitStream.h"


using std::string;
using RakNet::BitStream;
class RakPeerInterface;
class PacketLogger;
class NatPunchthrough;

namespace raknet_wrapper {


	class RakPeerBase 
	{
	public :

		enum {
			DEFAULT_CHANNEL = 0
		};

		static const PacketReliability DEFAULT_RELIBILITY = RELIABLE;
		//static const PacketReliability DEFAULT_RELIBILITY=RELIABLE_ORDERED
		static const PacketPriority DEFAULT_PRIORITY = HIGH_PRIORITY;

	public:

		RakPeerBase( unsigned short maxPeerCount );

		virtual ~RakPeerBase();

		virtual bool Init( unsigned short localPort, const string & localHost = "" );
		bool Connect( const string & remoteHost, unsigned short remotePort, const string & passwd = "" );
		void Disconnect( const SystemAddress & addr = UNASSIGNED_SYSTEM_ADDRESS );
		void DisconnectAll();
		void Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, SystemAddress systemAddress, bool broadcast );
		void Send( BitStream & out, const SystemAddress & addr );
		void SendEx( BitStream & out, const SystemAddress & addr, PacketReliability reliability, char channel, PacketPriority priority );
		void Broadcast( BitStream & out, const SystemAddress & except_addr = UNASSIGNED_SYSTEM_ADDRESS );
		void BroadcastEx( BitStream & out, const SystemAddress & addr, PacketReliability reliability, char channel, PacketPriority priority );

#define __LOG_PACKET(p) LogPacket(p,__FUNCTION__)

		virtual int Update();
		virtual int OnDisconnectionNotification(Packet* p)			{ __LOG_PACKET(p); return 0; }
		virtual int OnConnectionLost(Packet* p)						{ __LOG_PACKET(p); return 0; }
		virtual int OnNoFreeIncomingConnections(Packet* p)			{ __LOG_PACKET(p); return 0; }
		virtual int OnNewIncomingConnection(Packet* p)				{ __LOG_PACKET(p); return 0; }
		virtual int OnConnectionRequestAccepted(Packet* p)			{ __LOG_PACKET(p); return 0; }
		virtual int OnConnectionAttemptFailed(Packet* p)			{ __LOG_PACKET(p); return 0; }
		virtual int OnAlreadyConnected(Packet* p)					{ __LOG_PACKET(p); return 0; }
		virtual int OnConnectionBanned(Packet* p)					{ __LOG_PACKET(p); return 0; }
		virtual int OnInvalidPassword(Packet* p)					{ __LOG_PACKET(p); return 0; }
		virtual int OnModifiedPacket(Packet* p)						{ __LOG_PACKET(p); return 0; }
		virtual int OnTimestamp(Packet* p)							{ __LOG_PACKET(p); return 0; }
		virtual int OnPong(Packet* p)								{ __LOG_PACKET(p); return 0; }
		virtual int OnAdvertiseSystem(Packet* p)					{ __LOG_PACKET(p); return 0; }
		virtual int OnRemoteDisconnectionNotification(Packet* p)	{ __LOG_PACKET(p); return 0; }
		virtual int OnRemoteConnectionLost(Packet* p)				{ __LOG_PACKET(p); return 0; }
		virtual int OnRemoteNewIncomingConnection(Packet* p)		{ __LOG_PACKET(p); return 0; }
		virtual int OnNatTargetNotConnected(Packet* p)				{ __LOG_PACKET(p); return 0; }
		virtual int OnNatTargetConnectionLost(Packet* p)			{ __LOG_PACKET(p); return 0; }
		
		virtual int OnUserPacket(Packet* p);

		RakPeerInterface* GetPeer() { return m_pPeer; }

		void SetIncomingPasswd( const string & passwd );
		string GetIncomingPasswd() const;

		string GetLocalIP( size_t index ) const;

		SystemAddress GetSystemAddress() const;

		void LogPacket(Packet* p,const char * func );

		virtual void Log( const char * fmt, ... );

		PacketLogger* GetPacketLogger() { return m_pLogger; }

	protected :

		RakPeerInterface *	m_pPeer;
		PacketLogger*		m_pLogger;
		NatPunchthrough*	m_pNatPunchthrough;
		unsigned short		m_MaxPeerCount;
	};
} // namespace raknet_wrapper