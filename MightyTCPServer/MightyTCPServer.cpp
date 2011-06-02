//#include "BaseMightyServerInterface.h"
#include ".\mightytcpasyncselectserver.h"
#include ".\mightytcpcompletionportserver.h"
#include ".\MightyTCPCompletionRoutineServer.h"
#include ".\mightytcpoverlappedeventserver.h"
#include "MightyTCPEventSelectServer.h"
#include ".\mightytcpselectserver.h"
MIGHTY_TCPSERVER_API ITcpServer* CreateTcpServer(ITcpServerNotify * pNotify, EServerTCPType ioStyle)
{

	ITcpServer* pExeServer=NULL;
	switch(ioStyle)
	{
		case S_SVR:
		{
			pExeServer=new MightyTCPSelectServer(pNotify);
			break;
		}
		case AS_SVR:
		{
			pExeServer=new MightyTCPAsyncSelectServer(pNotify);
			break;
		}
		case ES_SVR:
		{
			pExeServer=new MightyTCPEventSelectServer(pNotify);
			break;
		}
		case OVENT_SVR:
		{
			pExeServer=new MightyTCPOverlappedEventServer(pNotify);
			break;
		}
		case CP_SVR:
		{
			pExeServer=new MightyTCPCompletionPortServer(pNotify);
			break;
		}
		case CR_SVR:
		{
			pExeServer=new MightyTCPCompletionRoutineServer(pNotify);
			break;
		}
		default:
		{
			break;
		}

	}
	return pExeServer;
}