#include ".\mightytcpasyncselectclient.h"
#include ".\mightytcpcompletionportclient.h"
#include ".\mightytcpcompletionroutineclient.h"
#include ".\mightytcpoverlappedeventclient.h"
#include ".\mightytcpselectclient.h"
#include "./MightyTCPEventSelectClient.h"
TCPCLIENT_API ITcpClient* CreateTcpClient( ITcpClientNotify* pNotify,EClientTCPType ioStyle)
{
	ITcpClient* pExeClient=NULL;

	switch(ioStyle)
	{
		case S_CLT:
		{
			pExeClient=new MightyTCPSelectClient(pNotify);
			break;
		}
		case AS_CLT:
		{
			pExeClient=new MightyTCPAsyncSelectClient(pNotify);
			break;
		}
		case ES_CLT:
		{
			pExeClient=new  MightyTCPEventSelectClient(pNotify);
			break;
		}
		case OVENT_CLT:
		{
			pExeClient=new MightyTCPOverlappedEventClient(pNotify);
			break;
		}
		case CP_CLT:
		{
			pExeClient=new MightyTCPCompletionPortClient(pNotify);
			break;
		}
		case CR_CLT:
		{
			pExeClient=new MightyTCPCompletionRoutineClient(pNotify);
			break;
		}
		default:
		{
			break;
		}

	}
	return pExeClient;
}