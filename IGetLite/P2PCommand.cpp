#include ".\p2pcommand.h"

namespace P2P_COMMAND
{
	//------------------------------------------------------------------------------------
		
	string CP2PCommandQuery::CommandString( unsigned int unCommand )
	{
		string sRet = "-- no name command --";

		switch( unCommand ) 
		{
		case P2P_CMD_HANDSHAKE:
			sRet = "P2P_CMD_HANDSHAKE";
			break;

		case P2P_CMD_HANDSHAKE_RET:
			sRet = "P2P_CMD_HANDSHAKE_RET";
			break;

		case P2P_CMD_GETBLOCK_AREA:
			sRet = "P2P_CMD_GETBLOCK_AREA";
			break;

		case P2P_CMD_GETBLOCK_AREA_RET:
			sRet = "P2P_CMD_GETBLOCK_AREA_RET";
			break;

		case P2P_CMD_GETSEGMENT:
			sRet = "P2P_CMD_GETSEGMENT";
			break;

		case P2P_CMD_GETSEGMENT_RET:
			sRet = "P2P_CMD_GETSEGMENT_RET";
			break;

		case P2P_CMD_STAT_UPLOAD:
			sRet = "P2P_CMD_STAT_UPLOAD";
			break;

		case P2P_CMD_STAT_UPLOAD_RET:
			sRet = "P2P_CMD_STAT_UPLOAD_RET";
			break;

		case P2P_CMD_HEARTBEAT:
			sRet = "P2P_CMD_HEARTBEAT";
			break;

		case P2P_CMD_EXIT:
			sRet = "P2P_CMD_EXIT";
			break;

		default:
			break;
		}

		return sRet;
	}





	//____________________________________________________________________________________

	CCmdHandshake::CCmdHandshake() : 
	CP2PBaseCommand( P2P_CMD_HANDSHAKE )
	{
	}

	CCmdHandshake::~CCmdHandshake(void)
	{
	}

	int CCmdHandshake::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = push_int32_2( p, m_nSessionID, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdHandshake::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = pop_int32( p, m_nSessionID );
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdHandshakeRet::CCmdHandshakeRet() : 
	CP2PBaseCommand( P2P_CMD_HANDSHAKE_RET )
	{
	}

	CCmdHandshakeRet::~CCmdHandshakeRet(void)
	{
	}

	int CCmdHandshakeRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = push_int32_2( p, m_nSessionID, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdHandshakeRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = pop_int32( p, m_nSessionID );
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdGetBlockArea::CCmdGetBlockArea() : 
	CP2PBaseCommand( P2P_CMD_GETBLOCK_AREA )
	{
	}

	CCmdGetBlockArea::~CCmdGetBlockArea(void)
	{
	}

	int CCmdGetBlockArea::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_int64_2( p, m_nFileID, nLen);
		p = push_int64_2( p, m_nBlockOffset, nLen);
		p = push_int32_2( p, m_nBlocakSize, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdGetBlockArea::Parse( char* pBuffer, int nLen )
	{
		short len = 0;
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );
		p = pop_int64( p, m_nFileID);
		p = pop_int64( p, m_nBlockOffset);
		p = pop_int32( p, m_nBlocakSize);
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdGetBlockAreaRet::CCmdGetBlockAreaRet() : 
	CP2PBaseCommand( P2P_CMD_GETBLOCK_AREA_RET )
	{
	}

	CCmdGetBlockAreaRet::~CCmdGetBlockAreaRet(void)
	{
	}

	int CCmdGetBlockAreaRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = push_int32_2( p, m_nSessionID, nLen);
		
		p = push_int64_2( p, m_nBlockOffset, nLen);

		p = push_int32_2( p, m_nBlocakSize, nLen);

		return (int)(p - pBuffer);
	}

	int CCmdGetBlockAreaRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = pop_int32( p, m_nSessionID );

		p = pop_int64( p, m_nBlockOffset);
		p = pop_int32( p, m_nBlocakSize );

		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdSendPiece::CCmdSendPiece() : 
	CP2PBaseCommand( P2P_CMD_SENDBLOCK )
	,m_szBuffer(NULL)
	{
	}

	CCmdSendPiece::~CCmdSendPiece(void)
	{
		if (m_szBuffer!=NULL)
		{
			delete [] m_szBuffer;
		}
	}

	int CCmdSendPiece::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;


		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_int32_2( p, m_nOrderNumber, nLen);
		p = push_int64_2( p,m_nLeftPos,nLen);

		p = push_int32_2( p,m_nDataLen,nLen);

		p = push_buf( p, m_szBuffer,m_nDataLen, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdSendPiece::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );
		p = pop_int32( p, m_nOrderNumber );
		p = pop_int64( p, m_nLeftPos );
		p = pop_int32( p, m_nDataLen );
		
		p =pop_buf_2( p, m_szBuffer,m_nDataLen);//pop_buf_2与push_buf对应我后加的没有往缓冲区中写入 sizeof short大小的数据长度


		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdSendPieceRet::CCmdSendPieceRet() : 
	CP2PBaseCommand( P2P_CMD_GETSEGMENT_RET )
	{
	}

	CCmdSendPieceRet::~CCmdSendPieceRet(void)
	{
	}

	int CCmdSendPieceRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_int32_2( p, m_nBlockNum, nLen);
		return (int)(p - pBuffer);
	}

	int CCmdSendPieceRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;
		
		p = pop_int32( p, m_nSessionID );
		p = pop_int32( p, m_nBlockNum );
		return (int)(p - pBuffer);
	}

	//------------------------------------------------------------------------------------

	CCmdEchoShootHole::CCmdEchoShootHole() :
	CP2PBaseCommand(P2P_CMD_ECHO_SHOOTHOLE)
	{

	}
	CCmdEchoShootHole:: ~CCmdEchoShootHole(void)
	{

	}

	int CCmdEchoShootHole::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_int32_2( p, m_LocalIp, nLen);

		p =push_int16_2(p,m_tcpPort,nLen);

		p =push_int16_2(p,m_udpPort,nLen);
		return (int)(p - pBuffer);

	}
	int CCmdEchoShootHole::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		p = pop_int32( p, m_LocalIp);

		p =pop_int16(p,m_tcpPort);

		p =pop_int16(p,m_udpPort);

		return (int)(p - pBuffer);

	}
	//---------------------------------------------------------------------------------------
	CCmdEchoShootHoleRet::CCmdEchoShootHoleRet() :
	CP2PBaseCommand(P2P_CMD_ECHO_SHOOTHOLE_RET)
	{

	}
	CCmdEchoShootHoleRet:: ~CCmdEchoShootHoleRet(void)
	{

	}

	int CCmdEchoShootHoleRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nSessionID, nLen);
		return (int)(p - pBuffer);
	}
	int CCmdEchoShootHoleRet::Parse( char* pBuffer, int nLen )
	{
		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		return (int)(p - pBuffer);
	}

	//----------------------------------------------------------------------------------------------

	CCmdAcceptReverseConnect::CCmdAcceptReverseConnect() :
	CP2PBaseCommand(P2P_CMD_ACCEPT_REVERSECONNECT)
	{

	}
	CCmdAcceptReverseConnect::~CCmdAcceptReverseConnect(void)
	{

	}

	int CCmdAcceptReverseConnect::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nSessionID, nLen);
		p = push_int32_2( p, m_LocalIp, nLen);

		p =push_int16_2(p,m_tcpPort,nLen);

		p =push_int16_2(p,m_udpPort,nLen);
		return (int)(p - pBuffer);

	}
	int CCmdAcceptReverseConnect::Parse( char* pBuffer, int nLen )
	{

		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		p = pop_int32( p, m_LocalIp);

		p =pop_int16(p,m_tcpPort);

		p =pop_int16(p,m_udpPort);

		return (int)(p - pBuffer);
	}

	//---------------------------------------------------------------------------------------------

	CCmdAcceptReverseConnectRet::CCmdAcceptReverseConnectRet() :
	CP2PBaseCommand(P2P_CMD_ACCEPT_REVERSECONNECT_RET)
	{

	}
	CCmdAcceptReverseConnectRet::~CCmdAcceptReverseConnectRet(void)
	{

	}

	int CCmdAcceptReverseConnectRet::Create( char* pBuffer, int& nLen )
	{
		int nRet = CBaseCommand::Create( pBuffer, nLen );
		char* p = pBuffer + nRet;
		p = push_int32_2( p, m_nSessionID, nLen);
		return (int)(p - pBuffer);

	}
	int CCmdAcceptReverseConnectRet::Parse( char* pBuffer, int nLen )
	{

		int nRet = CBaseCommand::Parse( pBuffer, nLen );
		char* p = pBuffer + nRet;

		p = pop_int32( p, m_nSessionID );

		return (int)(p - pBuffer);
	}
};

