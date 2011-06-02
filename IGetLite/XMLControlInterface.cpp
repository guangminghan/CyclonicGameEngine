#include ".\xmlcontrolinterface.h"
static bool GetChild(const TiXmlElement *root, const string &node, string &value) {
	const TiXmlElement *e=root->FirstChildElement(node.c_str());
	if(!e) return false;

	const TiXmlNode *n=e->FirstChild();
	if(!n) return false;

	const TiXmlText *t=n->ToText();
	if(!t) return false;

	const char *v=t->Value();
	if(!v) return false;
	value=v;
	
	return true;
}
XMLControlInterface::XMLControlInterface(void)
{
}

XMLControlInterface::~XMLControlInterface(void)
{
}
bool XMLControlInterface::Load(const char * filepath)
{
	try
	{

			m_configure = new TiXmlDocument(filepath);
			m_configure->LoadFile();
			TiXmlElement *l_root = m_configure->RootElement();

			//------------------------------------------------
			TiXmlElement* l_LeftElement = NULL;
			TiXmlElement* l_IndexitemElement = NULL;

			

			SERVER_INFO_ svrinfo_;

			string strTxt,strVal;
			strVal=l_root->Value();
			//strVal=l_root->GetText();
			if(!l_root || strcmp(l_root->Value(), "I_hosts")) 
				return false;

			/*if(const TiXmlElement *updates=l_root->FirstChildElement("address")) 
			{
				//GetChild(updates, "ip",strTxt);

				TiXmlAttribute *IDAttribute = (TiXmlAttribute *)updates->FirstAttribute();
				strTxt=IDAttribute->Value();
				for (TiXmlAttribute * nextID=IDAttribute->Next();nextID;)
				{
					strTxt=nextID->Value();
					nextID=nextID->Next();
				}

			}*/
			for( l_LeftElement = l_root->FirstChildElement();
				l_LeftElement!=NULL;
				l_LeftElement = l_LeftElement->NextSiblingElement() )	
			{	
				strTxt=l_LeftElement->Value();

				//for( l_IndexitemElement = l_LeftElement->FirstChildElement();
					//l_IndexitemElement!=NULL;
					//l_IndexitemElement = l_IndexitemElement->NextSiblingElement() )	
				//{
				TiXmlAttribute *IDAttribute = (TiXmlAttribute *)l_LeftElement->FirstAttribute();
				
				for (;IDAttribute;IDAttribute=IDAttribute->Next())
				{
					strTxt=IDAttribute->Name();
					if( 0 == strncmp( "ip" ,IDAttribute->Name(), 2 ) )
					{
						svrinfo_._ip=IDAttribute->Value();
					}

					else if(0 == strncmp( "port" ,IDAttribute->Name(), 4 ))
					{
						svrinfo_._port=IDAttribute->Value();
					}
					else if(0 == strncmp( "root" ,IDAttribute->Name() , 4 ))
					{
						svrinfo_._root =IDAttribute->Value();


					}
					else if(0 == strncmp( "redirect" ,IDAttribute->Name() , 8 ))
					{
						svrinfo_._redirect=IDAttribute->Value();

					}
				}
				
					/*strTxt=nextID->Value();
					nextID=nextID->Next();
				
					strTxt=l_IndexitemElement->Value();
					strVal=l_IndexitemElement->GetText();
					if( 0 == strncmp( "ip" , l_IndexitemElement->Value() , 2 ) )
					{
						svrinfo_._ip= l_IndexitemElement->GetText();
					}
					else if(0 == strncmp( "port" , l_IndexitemElement->Value() , 2 ))
					{
						svrinfo_._port= l_IndexitemElement->GetText();
					}
					else if(0 == strncmp( "root" , l_IndexitemElement->Value() , 3 ))
					{
						svrinfo_._root =l_IndexitemElement->GetText();


					}
					else if(0 == strncmp( "redirect" , l_IndexitemElement->Value() , 8 ))
					{
						svrinfo_._redirect= l_IndexitemElement->GetText();

					}*/

				//}

				m_serList.push_back( svrinfo_);

				
			}
			if( NULL != m_configure )
				delete m_configure;
	}

	catch (...)
	{
		if( NULL != m_configure )
			delete m_configure;
	}
	return m_serList.empty();

}
void XMLControlInterface::Save(const char * filepath)
{

}