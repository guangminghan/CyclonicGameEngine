#ifndef _WIN32_WINNT		
#define _WIN32_WINNT 0x0501	
#endif	
#include "./CommonInc/common.h"
#include "PacketID.h"
#include "RakServer.h"
#include "CustomPacketLogger.h"
#include ".\CommonInc\Log.h"
//#include "pauseme.h"
using raknet_wrapper::RakServer;
using raknet_wrapper::CustomPacketLogger;
/*PAUSEME pauseApp;
struct   GLBOBJ{  
	~GLBOBJ(){  
		system("pause");  
	}  
}glbobj;   */
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL+C signal.
	case CTRL_C_EVENT:
		{
			Beep(1000, 1000);
			return TRUE;
		}
		// CTRL+CLOSE: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		{

			//自定义的处理操作

			

			//自定义的处理操作-结束
			return TRUE;
		}
		// Pass other signals to the next handler.
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	default:
		return FALSE;
	}
} 
int main( int argc, char *argv[] )
{
	SetConsoleCtrlHandler(
		(PHANDLER_ROUTINE)CtrlHandler, // 处理函数句柄
		TRUE);
	int max_peer =1024;
	string passwd = "server";
	unsigned short port = 60000;

	SetLogFile(_T("IGeTracker"));
	SetLogLevel(Log_Debug);
	StartLog(NULL);
	RakServer* pServer = new RakServer(max_peer);

	CustomPacketLogger* pLogger = dynamic_cast<CustomPacketLogger*>(pServer->GetPacketLogger());
	//CustomPacketLogger* pLogger =(CustomPacketLogger*)pServer->GetPacketLogger();
	//pLogger->SetRawLogging(true);
	pLogger->SetConsoleTrace(true);

	pServer->Init(port);
	pServer->SetIncomingPasswd(passwd);
	//assert( pServer->GetIncomingPasswd() == passwd );


	while (1)
	{
		pServer->Update();
		Sleep(1);
	}
}