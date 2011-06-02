#ifndef P2SCOMMAND_INCLUDE_H
#define P2SCOMMAND_INCLUDE_H

#pragma once

#include "basecommand.h"

namespace P2S_COMMAND
{
	//------------------------------------------------------------------------
	class CP2SCommandQuery
	{
	public:
		static string CommandString( unsigned int unCommand );
	};
	//-----------------------------------------------------------------------
	class CP2SBaseCommand : public CBaseCommand
	{
	public:
		CP2SBaseCommand( unsigned int nCommandID ) : CBaseCommand( nCommandID ){};
		virtual ~CP2SBaseCommand(void){};

		DWORD m_dwIP;
		unsigned short m_usPort;
	};
	//------------------------------------------------------------------------
	/*class CCommandFactory
	{
	public:
		static CBaseCommand* CreateRegisterCmd(int nSessionID,int nLocalIP, short sLocalUdpPort, short sLocalTcpPort);
		
		static CBaseCommand* CreateGetPeersCmd(int nCount, int nSessionID);
		
		static CBaseCommand* CreateUdpHeartbeatCmd( int nUserID, int nSessionID, string sAuthStr );

	};*/
	//------------------------------------------------------------------------
	class CCmdRegister : public CP2SBaseCommand
	{
	public:
		CCmdRegister();
		virtual ~CCmdRegister(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		//short m_clientVersion;
		//unsigned char m_MaxAddr[6];
		int	   m_nLocalIP;
		short  m_sLocalUdpPort;
		short  m_sLocalTcpPort;
	};

	class CCmdRegisterRet : public CP2SBaseCommand
	{
	public:
		CCmdRegisterRet();
		virtual ~CCmdRegisterRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		int m_nExternalIP;
		short m_nExternalPort;
		char m_nIsLocal;
	};
	//------------------------------------------------------------------------ 向服务器发送的UDP打洞包
	class CCmdShootHole : public CP2SBaseCommand
	{
	public:
		CCmdShootHole();
		virtual ~CCmdShootHole(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		int m_nShootHolePeerCount;
		list<PEERINFO> m_listPeers;
	};

	class CCmdShootHoleRet : public CP2SBaseCommand
	{
	public:
		CCmdShootHoleRet();
		virtual ~CCmdShootHoleRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		int m_nState;

		int m_nExternalIP;
		short m_nExternalUdpPort;

	};


	//------------------------------------------------------------------------
	class CCmdGetPeers : public CP2SBaseCommand
	{
	public:
		CCmdGetPeers();
		virtual ~CCmdGetPeers(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		int m_nPeerCount;
		
		//int m_nResoureID;
	};

	class CCmdGetPeersRet : public CP2SBaseCommand
	{
	public:
		CCmdGetPeersRet();
		virtual ~CCmdGetPeersRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		int m_nPeerCount;
		list<PEERINFO> m_listPeers;
	};

	class CCmdPeerLogin : public CP2SBaseCommand
	{
	public:
		CCmdPeerLogin();
		virtual ~CCmdPeerLogin(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nUserID;
		int m_nSessionID;
		string m_sChannelID;
		int m_nPeerCount;
		list<PEERINFO> m_listPeers;
	};

	class CCmdPeerLogout : public CP2SBaseCommand
	{
	public:
		CCmdPeerLogout();
		virtual ~CCmdPeerLogout(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nUserID;
		int m_nSessionID;
		string m_sChannelID;
		int m_nPeerCount;
		list<PEERINFO> m_listPeers;
	};

	class CCmdUdpHeartbeat : public CP2SBaseCommand
	{
	public:
		CCmdUdpHeartbeat();
		virtual ~CCmdUdpHeartbeat(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
	};

	class CCmdReqShootTo : public CP2SBaseCommand
	{
	public:

		CCmdReqShootTo();
		virtual ~CCmdReqShootTo(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );
	public:
		int m_nSessionID;
		int m_nExternalIP;
		short m_nExternalPort;

	};
	class CCmdReqShootToRet: public CP2SBaseCommand
	{
	public:

		CCmdReqShootToRet();
		virtual ~CCmdReqShootToRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );
	public:
		int m_nSessionID;
	};

	//----------------------------------------------------------

	class CCmdReverseConnectAssistance: public CP2SBaseCommand
	{

	public:
		CCmdReverseConnectAssistance();
		virtual ~CCmdReverseConnectAssistance();


		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		int m_nReverseConnectPeerCount;
		list<PEERINFO> m_listPeers;

	};

	class CCmdReverseConnectAssistanceRet: public CP2SBaseCommand
	{
	public:

		CCmdReverseConnectAssistanceRet();
		virtual ~CCmdReverseConnectAssistanceRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );
	public:
		int m_nSessionID;

		int m_nState;

		int m_nExternalIP;
		short m_nExternalUdpPort;
	};


	//----------------------------------------------------------------------------------
	class CCmdPassOnReverseConnectAsk: public CP2SBaseCommand
	{
	
	public:

		CCmdPassOnReverseConnectAsk();
		virtual ~CCmdPassOnReverseConnectAsk(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );
	public:
		int m_nSessionID;
		int m_nExternalIP;
		short m_nExternalUdpPort;
	};


	class CCmdPassOnReverseConnectAskRet: public CP2SBaseCommand
	{

	public:

		CCmdPassOnReverseConnectAskRet();
		virtual ~CCmdPassOnReverseConnectAskRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );
	public:
		int m_nSessionID;
	};
};

#endif