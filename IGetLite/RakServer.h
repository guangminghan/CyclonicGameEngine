#pragma once

#include "RakPeerBase.h"

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

	};
}