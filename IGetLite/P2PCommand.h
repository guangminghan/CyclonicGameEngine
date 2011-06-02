#ifndef P2PCOMMAND_INCLUDE_H
#define P2PCOMMAND_INCLUDE_H

#pragma once

#include "basecommand.h"
#include <WTypes.h>

namespace P2P_COMMAND
{
	// Peer to peer 的 m_nSessionID 用来区分是否非法回复消息,只有应答方式消息才有意义
	//------------------------------------------------------------------------
	class CP2PCommandQuery
	{
	public:
		static string CommandString( unsigned int unCommand );
	};
	//------------------------------------------------------------------------
	class CP2PBaseCommand : public CBaseCommand
	{
	public:
		CP2PBaseCommand( unsigned int nCommandID ) : CBaseCommand( nCommandID ){};
		virtual ~CP2PBaseCommand(void){};

		DWORD m_dwIP;
		unsigned short m_usPort;
	};
	//------------------------------------------------------------------------

	//------------------------------------------------------------------------
	class CCmdHandshake : public CP2PBaseCommand
	{
	public:
		CCmdHandshake();
		virtual ~CCmdHandshake(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
	};

	class CCmdHandshakeRet : public CP2PBaseCommand
	{
	public:
		CCmdHandshakeRet();
		virtual ~CCmdHandshakeRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
	};
	//------------------------------------------------------------------------
	class CCmdGetBlockArea : public CP2PBaseCommand
	{
	public:
		CCmdGetBlockArea();
		virtual ~CCmdGetBlockArea(void);

		virtual int Create( char* pBuffer, int& nLen );
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		QWORD m_nFileID;
		QWORD m_nBlockOffset;
		int m_nBlocakSize;
	};

	class CCmdGetBlockAreaRet : public CP2PBaseCommand
	{
	public:
		CCmdGetBlockAreaRet();
		virtual ~CCmdGetBlockAreaRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		
		int m_nSessionID;
		QWORD m_nBlockOffset;
		int m_nBlocakSize;
		
	};
	//------------------------------------------------------------------------

	class CCmdSendPiece : public CP2PBaseCommand
	{
	public:
		CCmdSendPiece();
		virtual ~CCmdSendPiece(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		int m_nSessionID;
		int m_nOrderNumber;
		QWORD m_nLeftPos;
		int m_nDataLen;
		//char m_szBuffer[UDP_PACK_LEN];
		char * m_szBuffer;
	};

	class CCmdSendPieceRet : public CP2PBaseCommand
	{
	public:
		CCmdSendPieceRet();
		virtual ~CCmdSendPieceRet(void);

		virtual int Create( char* pBuffer, int& nLen ); 
		virtual int Parse( char* pBuffer, int nLen );

	public:
		
		int m_nSessionID;
		int m_nBlockNum;		// 块号
	};

	//------------------------------------------------------------------------
	
	class CCmdEchoShootHole:public CP2PBaseCommand
	{
		public:
			CCmdEchoShootHole();
			virtual ~CCmdEchoShootHole(void);

			virtual int Create( char* pBuffer, int& nLen ); 
			virtual int Parse( char* pBuffer, int nLen );

		public:

			int m_nSessionID;
			int m_LocalIp;
			short m_tcpPort;
			short m_udpPort;

	};

	class CCmdEchoShootHoleRet:public CP2PBaseCommand
	{
		
		public:
			CCmdEchoShootHoleRet();
			virtual ~CCmdEchoShootHoleRet(void);

			virtual int Create( char* pBuffer, int& nLen ); 
			virtual int Parse( char* pBuffer, int nLen );

		public:

			int m_nSessionID;
	};
	//--------------------------------------------------------------------------
	class CCmdAcceptReverseConnect:public CP2PBaseCommand
	{
		public:
			CCmdAcceptReverseConnect();
			virtual ~CCmdAcceptReverseConnect(void);

			virtual int Create( char* pBuffer, int& nLen ); 
			virtual int Parse( char* pBuffer, int nLen );

		public:

			int m_nSessionID;

			int m_LocalIp;
			short m_tcpPort;
			short m_udpPort;
	};


	class CCmdAcceptReverseConnectRet:public CP2PBaseCommand
	{
		public:
			CCmdAcceptReverseConnectRet();
			virtual ~CCmdAcceptReverseConnectRet(void);

			virtual int Create( char* pBuffer, int& nLen ); 
			virtual int Parse( char* pBuffer, int nLen );

		public:

			int m_nSessionID;
	};

};

#endif