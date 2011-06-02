#ifndef XMLCONTROLINTERFACE_INCLUDE_H
#define XMLCONTROLINTERFACE_INCLUDE_H
#pragma once
#include "../CommonInc/common.h"
#include "../tinyxml/tinyxml.h"
typedef struct server_info 
{
	string _ip;
	string _port;
	string _root;
	string _redirect;
}SERVER_INFO_,*PSERVER_INFO_;

class XMLControlInterface
{
public:
	XMLControlInterface(void);
	~XMLControlInterface(void);
public:

	bool Load(const char * filepath);
	void Save(const char * filepath);
private:
	TiXmlDocument *m_configure;
	vector<SERVER_INFO_> m_serList;

};
#endif