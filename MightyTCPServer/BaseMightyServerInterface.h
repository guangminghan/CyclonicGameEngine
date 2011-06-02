#ifndef BASE_MIGHTY_SERVER_INTERFACE_INCLUDE_H
#define BASE_MIGHTY_SERVER_INTERFACE_INCLUDE_H
#define MIGHTY_TCPSERVER_EXPORTS

#ifdef MIGHTY_TCPSERVER_EXPORTS
#define MIGHTY_TCPSERVER_API __declspec(dllexport)
#else
#define MIGHTY_TCPSERVER_API __declspec(dllimport)
#endif
#include <string>
using std::string;
//------------------------------

//------------------------------
enum EServerTCPType{S_SVR=0,AS_SVR,ES_SVR,OVENT_SVR,CP_SVR,CR_SVR};
class ITcpServerNotify
{
public:
	virtual void OnAccept(const string & strLinkNo) = 0;
	virtual void OnClosed(const string & strLinkNo) = 0;
	virtual void OnRecvData(const char* pData, const int & nLen, const string & strLinkNo) = 0;
};
//--------------------------------
class ITcpServer
{
public:

	virtual bool Create(const unsigned short & usPort) = 0;
	virtual void Destroy(void) = 0;
	virtual bool SendData(const char *pData, int nLen, const string & strLinkNo) = 0;
	virtual bool CloseLink(const string & strLinkNo) = 0;

};
MIGHTY_TCPSERVER_API ITcpServer* CreateTcpServer(ITcpServerNotify * pNotify, EServerTCPType ioStyle);
#endif