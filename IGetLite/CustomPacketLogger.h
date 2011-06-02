#pragma once

#include "PacketFileLogger.h"

class RakPeerInterface;

namespace raknet_wrapper {

	class CustomPacketLogger : public PacketFileLogger
	{
	public:
		CustomPacketLogger();
		virtual void OnAttach(RakPeerInterface *peer);
		virtual void OnDirectSocketSend(const char *data, const unsigned bitsUsed, SystemAddress remoteSystemAddress);
		virtual void OnDirectSocketReceive(const char *data, const unsigned bitsUsed, SystemAddress remoteSystemAddress);
		virtual void OnInternalPacket(InternalPacket *internalPacket, unsigned /*frameNumber*/, SystemAddress remoteSystemAddress, RakNetTime time, bool isSend);
		virtual void WriteLog(const char *str);
		virtual void LogHeader() {}
		virtual const char* UserIDTOString(unsigned char id);

	public :
		bool IsLogging() const { return m_bLogging; }
		void SetLogging(bool val) { m_bLogging = val; }

		bool IsRawLogging() const { return m_bRawLogging; }
		void SetRawLogging(bool val) { m_bRawLogging = val; }

		bool IsConsoleTrace() const { return m_bConsoleTrace; }
		void SetConsoleTrace(bool val) { m_bConsoleTrace = val; }

		bool IsOutputDebugString() const { return m_bOutputDebugString; }
		void SetOutputDebugString(bool val) { m_bOutputDebugString = val; }

	protected :
		bool m_bLogging;
		bool m_bRawLogging;
		bool m_bConsoleTrace;				// console trace
		bool m_bOutputDebugString;			// to debug console

	};

} // namespace raknet_wrapper
