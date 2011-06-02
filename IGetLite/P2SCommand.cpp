#include ".\p2scommand.h"
#include <assert.h>

namespace P2S_COMMAND
{	
	//------------------------------------------------------------------------------------
	string CP2SCommandQuery::CommandString( unsigned int unCommand )
	{
		string sRet = "-- no name command --";

		switch( unCommand ) 
		{
		case P2S_CMD_SHOOTHOLE:
			sRet = "UDP_P2S_CMD_SHOOTHOLE";
			break;
		case P2S_CMD_SHOOTHOLE_RET:
			sRet = "UDP_P2S_CMD_SHOOTHOLE_RET";
			break;
		case P2S_CMD_UDP_HEARTBEAT:
			sRet = "UDP_P2S_CMD_UDP_HEARTBEAT";
			break;
		case P2S_CMD_GETPEERS:
			sRet = "P2S_CMD_GETPEERS";
			break;
		case P2S_CMD_GETPEERS_RET:
			sRet = "P2S_CMD_GETPEERS_RET";
			break;
		case P2S_CMD_REQSHOOTTO:
			sRet = "P2S_CMD_REQSHOOTTO";
			break;

		case P2S_CMD_REQSHOOTTO_RET:
			sRet = "P2S_CMD_REQSHOOTTO_RET";
			break;
		default:
			break;
		}

		return sRet;
	}
	//------------------------------------------------------------------------------------
	/*CBaseCommand* CCommandFactory::CreateRegisterCmd(int nSessionID,int nLocalIP, short sLocalUdpPort, short sLocalTcpPort)
	{
		CCmdRegister* pCmd = new CCmdRegister();
		if ( pCmd )
		{
			pCmd->m_nSessionID=nSessionID;
			pCmd->m_nLocalIP = nLocalIP;
			pCmd->m_sLocalUdpPort = sLocalUdpPort;
			pCmd->m_sLocalTcpPort = sLocalTcpPort;
		}
		
		return pCmd;
	}
	//------------------------------------------------------------------------------------
	CBaseCommand* CCommandFactory::CreateGetPeersCmd( int nCount, int nSessionID)
	{
		CCmdGetPeers* pCmd = new CCmdGetPeers();
		if ( pCmd )
		{
			pCmd->m_nPeerCount= nCount;
			pCmd->m_nSessionID = nSessionID;
		}
		return pCmd;
	}
	//------------------------------------------------------------------------------------
	CBaseCommand* CCommandFactory::CreateUdpHeartbeatCmd( int nUserID, int nSessionID, string sAuthStr )
	{
		CCmdUdpHeartbeat* pCmd = new CCmdUdpHeartbeat();
		if ( pCmd )
		{
			pCmd->m_nSessionID = nSessionID;
		}

		return pCmd;
	}*/
	//-----------------------------------------------------------------------------------
	CCmdRegister::CCmdRegister() : 
	CP2SBaseCommand(P2S_CMD_LOGIN)
	{
	}

	CCmdRegister::~CCmdRegister(void)
	{
	}

	int CCmdRegister::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p, m_nLocalIP, nLen);

		p =push_int16_2(p,m_sLocalUdpPort,nLen);

		p =push_int16_2(p,m_sLocalTcpPort,nLen);
		return (int)(p - pBuffer);
	}

	int CCmdRegister::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		p = pop_int32( p, m_nLocalIP);

		p =pop_int16(p,m_sLocalUdpPort);

		p =pop_int16(p,m_sLocalTcpPort);

		return (int)(p - pBuffer);
	}
	//------------------------------------------------------------------------------------
	CCmdRegisterRet::CCmdRegisterRet() : 
	CP2SBaseCommand(P2S_CMD_LOGIN_RET)
	{
	}

	CCmdRegisterRet::~CCmdRegisterRet(void)
	{
	}

	int CCmdRegisterRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p, m_nExternalIP, nLen);

		p =push_int16_2(p,m_nExternalPort,nLen);

		p =push_int8_2(p,m_nIsLocal,nLen);
		return (int)(p - pBuffer);
	}

	int CCmdRegisterRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		p = pop_int32( p, m_nExternalIP);

		p =pop_int16(p,m_nExternalPort);

		p =pop_int8(p,m_nIsLocal);

		return (int)(p - pBuffer);
	}
	//------------------------------------------------------------------------------------
	CCmdShootHole::CCmdShootHole() : 
	CP2SBaseCommand(P2S_CMD_SHOOTHOLE)
	{
	}

	CCmdShootHole::~CCmdShootHole(void)
	{
	}

	int CCmdShootHole::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p, m_nShootHolePeerCount, nLen);

		list<PEERINFO>::iterator it = m_listPeers.begin();
		while ( it != m_listPeers.end() )
		{
			PEERINFO& peerinfo = *it;
			int len = sizeof(peerinfo);
			p = push_buf_2( p, (const char *)&peerinfo, len, nLen );
			it++;
		}
		return (int)(p - pBuffer);
	}

	int CCmdShootHole::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = pop_int32( p, m_nSessionID );
		p = pop_int32( p,m_nShootHolePeerCount);
		PEERINFO peerinfo;
		short slen = sizeof(peerinfo);
		for ( int i = 0; i < m_nShootHolePeerCount; i++ )
		{
			p = pop_buf( p, (char *)&peerinfo, slen );
			m_listPeers.push_back(peerinfo);
		}
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdShootHoleRet::CCmdShootHoleRet() : 
	CP2SBaseCommand(P2S_CMD_SHOOTHOLE_RET)
	{
	}

	CCmdShootHoleRet::~CCmdShootHoleRet(void)
	{
	}

	int CCmdShootHoleRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p, m_nState, nLen);

		p = push_int32_2( p,m_nExternalIP, nLen);

		p =push_int16_2(p,m_nExternalUdpPort, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdShootHoleRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = pop_int32( p, m_nSessionID );

		p = pop_int32( p, m_nState);

		p = pop_int32( p, m_nExternalIP );
		p = pop_int16( p, m_nExternalUdpPort );
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdGetPeers::CCmdGetPeers() : 
	CP2SBaseCommand(P2S_CMD_GETPEERS)
	{
	}

	CCmdGetPeers::~CCmdGetPeers(void)
	{
	}

	int CCmdGetPeers::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_int32_2( p, m_nPeerCount, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdGetPeers::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = pop_int32( p, m_nSessionID );
		p = pop_int32( p, m_nPeerCount );
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdGetPeersRet::CCmdGetPeersRet() : 
	CP2SBaseCommand(P2S_CMD_GETPEERS_RET)
	{
	}

	CCmdGetPeersRet::~CCmdGetPeersRet(void)
	{
	}

	int CCmdGetPeersRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = push_int32_2( p, m_nSessionID, nLen);
		

		m_nPeerCount = (int)m_listPeers.size();
		p = push_int32_2( p, m_nPeerCount, nLen );
		list<PEERINFO>::iterator it = m_listPeers.begin();
		while ( it != m_listPeers.end() )
		{
			PEERINFO& peerinfo = *it;
			int len = sizeof(peerinfo);
			p = push_buf_2( p, (const char *)&peerinfo, len, nLen );
			it++;
		}
		return (int)(p - pBuffer);
	}

	int CCmdGetPeersRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		p = pop_int32( p, m_nPeerCount );
		PEERINFO peerinfo;
		short slen = sizeof(peerinfo);
		for ( int i = 0; i < m_nPeerCount; i++ )
		{
			p = pop_buf( p, (char *)&peerinfo, slen );
			m_listPeers.push_back(peerinfo);
		}
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdPeerLogin::CCmdPeerLogin() : 
	CP2SBaseCommand(P2S_CMD_PEERLOGIN)
	{
	}

	CCmdPeerLogin::~CCmdPeerLogin(void)
	{
	}

	int CCmdPeerLogin::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nUserID, nLen);
		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_str_2( p, m_sChannelID, nLen);
		m_nPeerCount = (int)m_listPeers.size();
		p = push_int32_2( p, m_nPeerCount, nLen );
		list<PEERINFO>::iterator it = m_listPeers.begin();
		while ( it != m_listPeers.end() )
		{
			PEERINFO& peerinfo = *it;
			int len = sizeof(peerinfo);
			p = push_buf_2( p, (const char *)&peerinfo, len, nLen );
			it++;
		}
		return (int)(p - pBuffer);
	}

	int CCmdPeerLogin::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = pop_int32( p, m_nUserID );
		p = pop_int32( p, m_nSessionID );
		p = pop_str( p, m_sChannelID );
		p = pop_int32( p, m_nPeerCount );
		PEERINFO peerinfo;
		short slen = sizeof(peerinfo);
		for ( int i = 0; i < m_nPeerCount; i++ )
		{
			p = pop_buf( p, (char *)&peerinfo, slen );
			m_listPeers.push_back(peerinfo);
		}
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdPeerLogout::CCmdPeerLogout() : 
	CP2SBaseCommand(P2S_CMD_PEERLOGOUT)
	{
	}

	CCmdPeerLogout::~CCmdPeerLogout(void)
	{
	}

	int CCmdPeerLogout::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nUserID, nLen);
		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_str_2( p, m_sChannelID, nLen);
		m_nPeerCount = (int)m_listPeers.size();
		p = push_int32_2( p, m_nPeerCount, nLen );
		list<PEERINFO>::iterator it = m_listPeers.begin();
		while ( it != m_listPeers.end() )
		{
			PEERINFO& peerinfo = *it;
			int len = sizeof(peerinfo);
			p = push_buf_2( p, (const char *)&peerinfo, len, nLen );
			it++;
		}
		return (int)(p - pBuffer);
	}

	int CCmdPeerLogout::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = pop_int32( p, m_nUserID );
		p = pop_int32( p, m_nSessionID );
		p = pop_str( p, m_sChannelID );
		p = pop_int32( p, m_nPeerCount );
		PEERINFO peerinfo;
		short slen = sizeof(peerinfo);
		for ( int i = 0; i < m_nPeerCount; i++ )
		{
			p = pop_buf( p, (char *)&peerinfo, slen );
			m_listPeers.push_back(peerinfo);
		}
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdUdpHeartbeat::CCmdUdpHeartbeat() : 
	CP2SBaseCommand(P2S_CMD_UDP_HEARTBEAT)
	{
	}

	CCmdUdpHeartbeat::~CCmdUdpHeartbeat(void)
	{
	}

	int CCmdUdpHeartbeat::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdUdpHeartbeat::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );
		return (int)(p - pBuffer);
	}

	//----------------------------------------------------------------------------------------
	CCmdReqShootTo::CCmdReqShootTo() : 
	CP2SBaseCommand(P2S_CMD_REQSHOOTTO)
	{
	}

	CCmdReqShootTo::~CCmdReqShootTo(void)
	{
	}

	int CCmdReqShootTo::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p,m_nExternalIP, nLen);

		p = push_int16_2( p,m_nExternalPort,nLen);
		return (int)(p - pBuffer);
	}

	int CCmdReqShootTo::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );
		p = pop_int32( p, m_nExternalIP );
		p = pop_int16( p, m_nExternalPort );
		return (int)(p - pBuffer);
	}
	//------------------------------------------------------------------
	CCmdReqShootToRet::CCmdReqShootToRet() : 
	CP2SBaseCommand(P2S_CMD_REQSHOOTTO_RET)
	{
	}

	CCmdReqShootToRet::~CCmdReqShootToRet(void)
	{
	}

	int CCmdReqShootToRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		return (int)(p - pBuffer);
	}

	int CCmdReqShootToRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		return (int)(p - pBuffer);
	}

	//----------------------------------------------------------------------
	CCmdReverseConnectAssistance::CCmdReverseConnectAssistance() :
	CP2SBaseCommand(P2S_CMD_REVERSECONNECT_ASSISTANCE)
	{

	}
	CCmdReverseConnectAssistance::~CCmdReverseConnectAssistance(void)
	{

	}


	int CCmdReverseConnectAssistance::Create( char* pBuffer, int& nLen )
	{

		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p, m_nReverseConnectPeerCount, nLen);

		list<PEERINFO>::iterator it = m_listPeers.begin();
		while ( it != m_listPeers.end() )
		{
			PEERINFO& peerinfo = *it;
			int len = sizeof(peerinfo);
			p = push_buf_2( p, (const char *)&peerinfo, len, nLen );
			it++;
		}
		return (int)(p - pBuffer);
	}
	int CCmdReverseConnectAssistance::Parse( char* pBuffer, int nLen )
	{

		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );
		p = pop_int32( p,m_nReverseConnectPeerCount);
		PEERINFO peerinfo;
		short slen = sizeof(peerinfo);
		for ( int i = 0; i < m_nReverseConnectPeerCount; i++ )
		{
			p = pop_buf( p, (char *)&peerinfo, slen );
			m_listPeers.push_back(peerinfo);
		}
		return (int)(p - pBuffer);
	}

	//--------------------------------------------------------------------------

	CCmdReverseConnectAssistanceRet::CCmdReverseConnectAssistanceRet() :
	CP2SBaseCommand(P2S_CMD_REVERSECONNECT_ASSISTANCE)
	{

	}
	CCmdReverseConnectAssistanceRet::~CCmdReverseConnectAssistanceRet(void)
	{

	}


	int CCmdReverseConnectAssistanceRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p, m_nState, nLen);

		p = push_int32_2( p,m_nExternalIP, nLen);

		p =push_int16_2(p,m_nExternalUdpPort, nLen);
		return (int)(p - pBuffer);

	}
	int CCmdReverseConnectAssistanceRet::Parse( char* pBuffer, int nLen )
	{

		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		p = pop_int32( p, m_nState);

		p = pop_int32( p, m_nExternalIP );
		p = pop_int16( p, m_nExternalUdpPort );
		return (int)(p - pBuffer);
	}

	//---------------------------------------------------------------------------------------

	CCmdPassOnReverseConnectAsk::CCmdPassOnReverseConnectAsk() :
	CP2SBaseCommand(P2S_CMD_PASSONREVERSECONNECT_ASK)
	{

	}
	CCmdPassOnReverseConnectAsk::~CCmdPassOnReverseConnectAsk(void)
	{

	}

	int CCmdPassOnReverseConnectAsk::Create( char* pBuffer, int& nLen )
	{

		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		p = push_int32_2( p,m_nExternalIP, nLen);

		p = push_int16_2( p,m_nExternalUdpPort,nLen);
		return (int)(p - pBuffer);

	}
	int CCmdPassOnReverseConnectAsk::Parse( char* pBuffer, int nLen )
	{

		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );
		p = pop_int32( p, m_nExternalIP );
		p = pop_int16( p, m_nExternalUdpPort );
		return (int)(p - pBuffer);
	}

	//--------------------------------------------------------------------------------------


	CCmdPassOnReverseConnectAskRet::CCmdPassOnReverseConnectAskRet() :
	CP2SBaseCommand(P2S_CMD_PASSONREVERSECONNECT_ASK_RET)
	{

	}
	CCmdPassOnReverseConnectAskRet::~CCmdPassOnReverseConnectAskRet(void)
	{

	}

	int CCmdPassOnReverseConnectAskRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = push_int32_2( p, m_nSessionID, nLen);

		return (int)(p - pBuffer);




	}
	int CCmdPassOnReverseConnectAskRet::Parse( char* pBuffer, int nLen )
	{

		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		return (int)(p - pBuffer);
	}

}
