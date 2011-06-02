#if !defined(CSQLSCRIPTPARSE__INCLUDED_)
#define CSQLSCRIPTPARSE__INCLUDED_

#pragma once
#include <list>
#include<vector>
#include<string>
#include <algorithm>
#include<iostream>
#include<fstream>
#include <sstream>
using namespace std;
class CSqlScriptParse
{
public:
	vector<string> strvect;
	ostringstream  m_Ostr_query;
	map<string,vector<string> > m_sqlFile_Map_;
	struct analyzeSqlScript
	{    

		ostringstream * ostr_query;
		vector<string> * pstr_vect;
		vector<string> * pResultvect;
		analyzeSqlScript(vector<string> & resultvect,ostringstream & postr_query,vector<string> & pstrVec):pResultvect(&resultvect),ostr_query(&postr_query),pstr_vect(&pstrVec){};    
		void operator()( const string & strScript)    
		{        
			int index=0;
			int left=0;   
			
			vector<string>::iterator fIte,lIte;
			fIte=pstr_vect->begin();
			lIte=pstr_vect->end();
			(*ostr_query).str("");
			while((index=strScript.find('$',index)) != string::npos)        
			{   
				(*ostr_query) << strScript.substr(left,index-left);
				if (fIte!=lIte)
				{
					(*ostr_query) <<(*fIte);
					fIte++;
				}
				left=++index;
			}  
			if (left<strScript.size())
			{
				(*ostr_query) << strScript.substr(left,strScript.size()-left);
			}
			
			pResultvect->push_back((*ostr_query).str());
			return;    

		}
	};
public:
	CSqlScriptParse( ){};
	~CSqlScriptParse( ){};
	bool createQuery(vector<string> & plist,vector<string> & pVect,const string & filename)
	{
		map<string,vector<string> >::iterator file_sqlQuery_pair;
		m_Ostr_query.str("");
		strvect.clear();
		if ((file_sqlQuery_pair=m_sqlFile_Map_.find(filename))!=m_sqlFile_Map_.end())
		{
			for_each(file_sqlQuery_pair->second.begin(),file_sqlQuery_pair->second.end(),analyzeSqlScript(plist,m_Ostr_query,pVect));  
			
			return true;
		}
		else
		{
			if (do_open(filename))
			{
				if ((file_sqlQuery_pair=m_sqlFile_Map_.find(filename))!=m_sqlFile_Map_.end())
				{
					for_each(file_sqlQuery_pair->second.begin(),file_sqlQuery_pair->second.end(),analyzeSqlScript(plist,m_Ostr_query,pVect));  
					
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
protected:    
	bool do_open(const string & pinipath)    
	{       
		ifstream fin(pinipath.c_str());        
		if(!fin.is_open())            
			return false;        
		while(!fin.eof())        
		{            
			string inbuf;            
			getline(fin,inbuf,'\n');
			if (inbuf.empty())
			{
				continue;
			}
			strvect.push_back(inbuf);        
		} 
		m_sqlFile_Map_.insert(make_pair(pinipath,strvect));      
		return !strvect.empty();    
	}   
};
#endif //CSQLSCRIPTPARSE__INCLUDED_