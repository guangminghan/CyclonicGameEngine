#ifndef P2SDEFINE_INCLUDE_H
#define P2SDEFINE_INCLUDE_H
#pragma once
#include <string.h>


#define PROTOCOL_VERSION '1001'



#pragma pack(push)
#pragma pack(1)
struct PEERINFO {
	unsigned long   ulID;
	unsigned long   ulExternalIP;
	unsigned short  usExternalUDPPort;
	unsigned short  usExternalTCPPort;
	unsigned long   ulLocalIP;
	unsigned short  usLocalUDPPort;
	unsigned short  usLocalTCPPort;
	unsigned char   ucSource;
	unsigned char	ucIsLocal;
	unsigned long activeLifeTimeOut;
	PEERINFO()
	{
		memset( this, 0, sizeof(PEERINFO) );
	}
};
#pragma pack(pop)

typedef enum P2S_CMD_TYPE
{
	// UDP DEFINE 
	P2S_CMD_SHOOTHOLE = 0x00001000,
	P2S_CMD_SHOOTHOLE_RET,
	P2S_CMD_UDP_HEARTBEAT,

	P2S_CMD_LOGIN,
	P2S_CMD_LOGIN_RET,

	P2S_CMD_LOGOUT,
	P2S_CMD_LOGOUT_RET,

	P2S_CMD_GETPEERS,
	P2S_CMD_GETPEERS_RET,

	P2S_CMD_PEERLOGIN,
	P2S_CMD_PEERLOGOUT,

	P2S_CMD_REQSHOOTTO,
	P2S_CMD_REQSHOOTTO_RET,

	

	P2S_CMD_REVERSECONNECT_ASSISTANCE,
	P2S_CMD_REVERSECONNECT_ASSISTANCE_RET,

	
	P2S_CMD_PASSONREVERSECONNECT_ASK,
	P2S_CMD_PASSONREVERSECONNECT_ASK_RET

};

#endif