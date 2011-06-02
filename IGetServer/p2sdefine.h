#ifndef P2SDEFINE_INCLUDE_H
#define P2SDEFINE_INCLUDE_H
#pragma once
#include <string.h>

#define TCP_MAX_BUFFER_LENGTH 8192
#define UDP_MAX_BUFFER_LENGTH 1024
#define ROOM_PACKET_MAX_LEN   733
#define UDP_HEARTBEAT_INTERVAL 10000 // UDP ������� 10 ��
#define TCP_HEARTBEAT_INTERVAL 10000 // TCP ������� 10 ��
#define SERVER_VERSION "08.01.10.001"
#define CLIENT_VERSION "08.01.10.001"
#define PROTOCOL_VERSION '1001'

//source peer type define for PEERINFO::ucSource
#define PEER_TYPE_K8_GENERAL	'0'			//��ͨK8Э��ͻ���
#define PEER_TYPE_K8_SOURCE		'1'			//K8Э��Դ��ӵ�����п�
#define PEER_TYPE_HTTP			'2'			//HTTPЭ�飬Ӧ��ӵ�����п�
#define PEER_TYPE_FTP			'3'			//FTPЭ�飬Ӧ��ӵ�����п�
#define PEER_TYPE_BITTORRENT	'4'			//BittorrentЭ��Դ��ӵ�в��ֿ�
#define PEER_TYPE_EDONKEY		'5'			//eDonkeyЭ��(emule)��ӵ�в��ֿ�

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