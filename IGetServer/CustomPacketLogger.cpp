#include "CommonInc/common.h"
#include "CustomPacketLogger.h"

#include "RakPeerInterface.h"
#include "InternalPacket.h"
#include "GetTime.h"

#include "PacketID.h"

namespace raknet_wrapper {

CustomPacketLogger::CustomPacketLogger()
{
	SetPrintID(true);
	SetPrintAcks(false);
	//SetSuffix("\n");
	SetLogging(true);
	m_bRawLogging = false;	// ignore OnDirectSocketSend, OnDirectSocketReceive, OnInternalPacket logs
	m_bConsoleTrace = false;
}

void CustomPacketLogger::OnAttach( RakPeerInterface *peer )
{
	char str[256];
#if _MSC_VER >= 1400
	sprintf_s(str,256
#else
	sprintf(str
#endif
		,"%s_%d",peer->GetInternalID().ToString(false),peer->GetInternalID().port);

	StartLog(str);

	PacketFileLogger::OnAttach(peer);
}

const char* CustomPacketLogger::UserIDTOString( unsigned char id )
{
	switch (id)
	{
	//case ID_REGISTER_CLIENT : return "ID_REGISTER_CLIENT"; break;
	//case ID_BEGIN_HANDSHAKING : return "ID_BEGIN_HANDSHAKING"; break;
	default: return "Unknown user message ID";
	}
}

void CustomPacketLogger::OnDirectSocketSend( const char *data, const unsigned bitsUsed, SystemAddress remoteSystemAddress )
{
	if ( !m_bRawLogging ) return;

	char str[256];
	SystemAddress localSystemAddress;
	localSystemAddress = rakPeer->GetInternalID();

	string remoteAddr = remoteSystemAddress.ToString();
	string localAddr = localSystemAddress.ToString();

#if _MSC_VER >= 1400
	sprintf_s(str,256
#else
	sprintf(str
#endif
		, "%s[%.2f] %21s << %21s : %s(%dB)%s"
		, prefix
		, (float)RakNet::GetTime()/100.f
		, remoteAddr.c_str()
		, localAddr.c_str()
		, IDTOString(data[0])
		, bitsUsed/8+1
		, suffix);

	AddToLog(str);
}

void CustomPacketLogger::OnDirectSocketReceive( const char *data, const unsigned bitsUsed, SystemAddress remoteSystemAddress )
{
	if ( !m_bRawLogging ) return;

	char str[256];
	SystemAddress localSystemAddress;
	localSystemAddress = rakPeer->GetInternalID();

	string remoteAddr = remoteSystemAddress.ToString();
	string localAddr = localSystemAddress.ToString();

#if _MSC_VER >= 1400
	sprintf_s(str,256
#else
	sprintf(str
#endif
		, "%s[%.2f] %21s >> %21s : %s(%dB)%s"
		, prefix
		, (float)RakNet::GetTime()/100.f
		, remoteAddr.c_str()
		, localAddr.c_str()
		, IDTOString(data[0])
		, bitsUsed/8+1
		, suffix);

	AddToLog(str);
}

void CustomPacketLogger::OnInternalPacket( InternalPacket *internalPacket, unsigned /*frameNumber*/, SystemAddress remoteSystemAddress, RakNetTime time, bool isSend )
{
	if ( !m_bRawLogging ) return;

	char str[256];
	char sendType[4];
	SystemAddress localSystemAddress = rakPeer->GetInternalID();

	if (isSend)
#if _MSC_VER >= 1400
		strcpy_s(sendType,4
#else
		strcpy(sendType
#endif
			, "<<");
	else
#if _MSC_VER >= 1400
		strcpy_s(sendType,4
#else
		strcpy(sendType
#endif
			, ">>");

	string remoteAddr = remoteSystemAddress.ToString();
	string localAddr = localSystemAddress.ToString();

	{
		if (internalPacket->data[0]==ID_TIMESTAMP && internalPacket->data[sizeof(unsigned char)+sizeof(RakNetTime)]!=ID_RPC)
		{
#if _MSC_VER >= 1400
			sprintf_s(str,256
#else
			sprintf(str
#endif
				, "%s[%.2f] %21s %s %21s : %s(%dB)%s"
				, prefix
				, (float)time/100.0f
				, remoteAddr.c_str()
				, sendType
				, localAddr.c_str()
				, "ID_TIMESTAMP"
				, internalPacket->dataBitLength/8+1
				, suffix);

		}
		else if (internalPacket->data[0]==ID_RPC || (internalPacket->dataBitLength>(sizeof(unsigned char)+sizeof(RakNetTime))*8 && internalPacket->data[0]==ID_TIMESTAMP && internalPacket->data[sizeof(unsigned char)+sizeof(RakNetTime)]==ID_RPC))
		{
			const char *uniqueIdentifier = rakPeer->GetRPCString((const char*) internalPacket->data, internalPacket->dataBitLength, isSend==true ? remoteSystemAddress : UNASSIGNED_SYSTEM_ADDRESS);

			if (internalPacket->data[0]==ID_TIMESTAMP)
			{
#if _MSC_VER >= 1400
				sprintf_s(str,256
#else
				sprintf(str
#endif
					, "%s[%.2f] %21s %s %21s : %s(%dB) : %s%s"
					, prefix
					, (float)time/100.0f
					, remoteAddr.c_str()
					, sendType
					, localAddr.c_str()
					, "RPC-TIMESTAMP"
					, internalPacket->dataBitLength/8+1
					, uniqueIdentifier
					, suffix);
			}
			else
			{
#if _MSC_VER >= 1400
				sprintf_s(str,256
#else
				sprintf(str
#endif
					, "%s[%.2f] %21s %s %21s : %s(%dB) : %s%s"
					, prefix
					, (float)time/100.0f
					, remoteAddr.c_str()
					, sendType
					, localAddr.c_str()
					, "RPC"
					, internalPacket->dataBitLength/8+1
					, uniqueIdentifier
					, suffix);				
			}
		}
		else
		{
#if _MSC_VER >= 1400
			sprintf_s(str,256
#else
			sprintf(str
#endif
				, "%s[%.2f] %21s %s %21s : %s(%dB)%s"
				, prefix
				, (float)time/100.0f
				, remoteAddr.c_str()
				, sendType
				, localAddr.c_str()
				, IDTOString(internalPacket->data[0])
				, internalPacket->dataBitLength/8+1
				, suffix);	
		}
	}

	AddToLog(str);
}

void CustomPacketLogger::WriteLog( const char *str )
{
	if ( m_bLogging )
	{
		PacketFileLogger::WriteLog(str);
	}

	if ( m_bConsoleTrace )
	{
		printf(str);
		printf("\n");
	}

	if ( m_bOutputDebugString )
	{
		OutputDebugString(str);
		OutputDebugString("\n");
	}
}

} //namespace raknet_wrapper