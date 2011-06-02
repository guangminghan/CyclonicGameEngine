
#include<iostream>
#include<fstream>
#include <sstream>
#include<iomanip>
#include <list>
#include<vector>
#include<string>
#include <algorithm>
#include <map>
#include <cwctype>
#include <set>
#ifndef _RWSTD_NO_NAMESPACE
using namespace std;
#endif
#define MAXBUF 2048

typedef map<string, string, less<string> > strMap;
typedef list<string> strListType;
typedef strMap::iterator strMapIt;
const char*const MIDDLESTRING = "_____***_______";
template <class T>
basic_string<T>& trim(std::basic_string<T>&);
void PushIps(set<unsigned int> &iplist, const char *ips) {

			unsigned short ip1, ip2, ip3, ip4;

			if(sscanf(ips, "%hu.%hu.%hu.%hu", &ip1, &ip2, &ip3, &ip4)==4) {
				union {
					unsigned int ip;
					unsigned char ipb[4];
				};

				ipb[0]=(unsigned char)ip4;
				ipb[1]=(unsigned char)ip3;
				ipb[2]=(unsigned char)ip2;
				ipb[3]=(unsigned char)ip1;

				if(ip) iplist.insert(ip);
			}
}
template<typename T,typename U>
class CagreementPacket
{
public:
	CagreementPacket();
	~CagreementPacket();
public:
	//int writeField(const T& tx,int isize,U * pBuf=0);
	//int readField(T& tx,int isize,U * pBuf=0);
	int writeField(T * tx,int isize);
	int readField(T& tx,int isize);
	int writeField(const char * chrx,int isize);
	int readField(char * chrx,int isize);
	int writeField(const __int64 * i64x,int isize);
	int readField(__int64 & i64x,int isize);
	int writeField(const string & strx,int isize);
	int readField(string & strx,int isize);
	U * getTotalDataFields();
	bool writeCompleteFields(void * pData,int len);
	CagreementPacket<T,U> * clone();
public:
	unsigned int datasize;
	unsigned int curIndex;
private:
public:
	U packetbuf[10240];
};

template<typename T,typename U>
CagreementPacket<T,U>::CagreementPacket()
:datasize(0)
,curIndex(0)
{

}
template<typename T,typename U>
CagreementPacket<T,U> ::~CagreementPacket()
{

}
template<typename T,typename U>
int
CagreementPacket<T,U>::writeField(T* tx,int isize)
{
	T * typeLeft=reinterpret_cast<T *>(tx);
	memcpy(packetbuf+datasize,tx,isize);
	return datasize+=isize;
}
template<typename T,typename U>
int
CagreementPacket<T,U>::readField(T& tx,int isize)
{
	T * typeLeft=reinterpret_cast<T *>(packetbuf+curIndex);
	tx=(*typeLeft);
	return curIndex+=isize;

}
template<typename T,typename U>
int
CagreementPacket<T,U>:: writeField(const char * chrx,int isize)
{
	memcpy(packetbuf+datasize,chrx,isize);
	return datasize+=isize;
}
template<typename T,typename U>
int
CagreementPacket<T,U>::readField(char * chrx,int isize)
{
	memcpy(chrx,reinterpret_cast<char *>(packetbuf+curIndex),isize);
	*(chrx+isize)='\0';
	return curIndex+=isize;
}
template<typename T,typename U>
int
CagreementPacket<T,U>::writeField(const __int64 * i64x,int isize)
{
	memcpy(packetbuf+datasize,i64x,isize);
	return datasize+=isize;
}
template<typename T,typename U>
int
CagreementPacket<T,U>::readField(__int64 & i64x,int isize)
{
	__int64 * tempvalue=reinterpret_cast<__int64 *>(packetbuf+curIndex);
	i64x=(*tempvalue);
	return curIndex+=isize;
}
template<typename T,typename U>
int
CagreementPacket<T,U>::writeField(const string & strx,int isize)
{
	memcpy(packetbuf+datasize,strx.c_str(),isize);
	return datasize+=isize;
}
template<typename T,typename U>
int
CagreementPacket<T,U>::readField(string & strx,int isize)
{
	char tempvalue[256];
	memcpy(tempvalue,reinterpret_cast<char *>(packetbuf+curIndex),isize);
	tempvalue[isize]='\0';
	strx.append(tempvalue);
	//strx.append(isize,reinterpret_cast<const char *>(packetbuf+curIndex));
	return curIndex+=isize;
}
template<typename T,typename U>
U *
CagreementPacket<T,U>::getTotalDataFields()
{
	return packetbuf;
}
template<typename T,typename U>
bool 
CagreementPacket<T,U>::writeCompleteFields(void * pData,int len)
{
	memcpy(packetbuf+datasize,pData,len);
	return datasize+=len;
}
template<typename T,typename U>
CagreementPacket<T,U> *
CagreementPacket<T,U>:: clone()
{
	CagreementPacket<T,U> * pAgreementPacket=new CagreementPacket<T,U>();
	pAgreementPacket->datasize=this->datasize;
	pAgreementPacket->curIndex=this->curIndex;
	memcpy(pAgreementPacket->packetbuf,this->packetbuf,this->datasize);
	return pAgreementPacket;
}
template <class T>
std::basic_string<T>& trim(std::basic_string<T>& s) {
	if (s.empty()) {
		return s;
	}
	std::basic_string<T>::iterator c;
	for (c = s.begin(); c != s.end() && iswspace(*c++););
	s.erase(s.begin(), --c);

	for (c = s.end(); c != s.begin() && iswspace(*--c););
	s.erase(++c, s.end());
	return s;
}
std::string& trim(std::string &s) 
{
	if (s.empty()) {
	 return s;
	}
	s.erase(0,s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}
struct analyzeSqlScript
{    
	//string strsect;    
	ostringstream * ostr_query;
	//vector<string> *pvec;  
	strListType * plist;
	analyzeSqlScript(strListType & strlist,ostringstream & postr_query):plist(&strlist),ostr_query(&postr_query){};    
	void operator()( const string & strScript)    
	{        
		int index=0;
		int left=0;       
		while((index=strScript.find('@',index)) != string::npos)        
		{   
			if ((strScript[index-1]=='\'')&&(strScript[index+1]=='\''))
			{
				//(*ostr_query) << strScript.substr(left,index-left)<<"\'"<<"test"<<"\'";
				(*ostr_query) << strScript.substr(left,index-left)<<"test";
			}
			else
			{
				(*ostr_query) << strScript.substr(left,index-left)<<"1234";
			}
			
			left=++index;
			//next =strScript.find('@',);

		}        
		plist->push_back((*ostr_query).str());
		return;    
	}
};
struct analyzeini
{    
	string strsect;    
	strMap *pmap;    
	analyzeini(strMap & strmap):pmap(&strmap){}    
	void operator()( const string & strini)    
	{        
		int first =strini.find('[');        
		int last = strini.rfind(']');        
		if( first != string::npos && last != string::npos && first != last+1)        
		{            
			strsect = strini.substr(first+1,last-first-1);            
			return ;        
		}        
		if(strsect.empty())            
			return ;        
		if((first=strini.find('='))== string::npos)            
			return ;        
		string strtmp1= strini.substr(0,first);        
		string strtmp2=strini.substr(first+1, string::npos);        
		first= strtmp1.find_first_not_of(" \t");        
		last = strtmp1.find_last_not_of(" \t");        
		if(first == string::npos || last == string::npos)            
			return ;        
		string strkey = strtmp1.substr(first, last-first+1);        
		first = strtmp2.find_first_not_of(" \t");        
		if(((last = strtmp2.find("\t#", first )) != string::npos)||((last = strtmp2.find(" #", first )) != string::npos)||((last = strtmp2.find("\t//", first )) != string::npos)||((last = strtmp2.find(" //", first )) != string::npos))        
		{            
			strtmp2 = strtmp2.substr(0, last-first);        
		}        
		last = strtmp2.find_last_not_of(" \t");        
		if(first == string::npos || last == string::npos)            
			return ;        
		string value = strtmp2.substr(first, last-first+1);        
		string mapkey = strsect + MIDDLESTRING;        
		mapkey += strkey;        
		(*pmap)[mapkey]=value;        
		return ;    
	}
};
class IniFile
{
public:    
	IniFile( ){};    
	~IniFile( ){};    
	bool open(const char* pinipath)    
	{       
		return do_open(pinipath);    
	}    
	string read(const char*psect, const char*pkey)    
	{        
		string mapkey = psect;        
		mapkey += MIDDLESTRING;        
		mapkey += pkey;        
		strMapIt it = c_inimap.find(mapkey);        
		if(it == c_inimap.end())            
			return "";        
		else            
			return it->second;    
	}
protected:    
	bool do_open(const char* pinipath)    
	{       
		ifstream fin(pinipath);        
		if(!fin.is_open())            
			return false;        
		vector<string> strvect;        
		while(!fin.eof())        
		{            
			string inbuf;            
			getline(fin, inbuf,'\n');            
			strvect.push_back(inbuf);        
		}        
		if(strvect.empty())            
			return false;        
		for_each(strvect.begin(), strvect.end(), analyzeini(c_inimap));        
		return !c_inimap.empty();    
	}    
	strMap c_inimap;
	
};

class CSqlScriptParse
{
public:
	CSqlScriptParse( ){};
	~CSqlScriptParse( ){};
	bool open(const char* pinipath)    
	{       
		return do_open(pinipath);    
	}    
	string read()    
	{        
		if (m_SqlQuery_list.empty())
		{
			return "";
		}
		else
		{
			string sqlValue=m_SqlQuery_list.front();
			m_SqlQuery_list.pop_front;
			return sqlValue;
		}
  
	}
protected:    
	bool do_open(const char* pinipath)    
	{       
		ifstream fin(pinipath);        
		if(!fin.is_open())            
			return false;        
		vector<string> strvect;        
		while(!fin.eof())        
		{            
			string inbuf;            
			getline(fin, inbuf,'\n');
			if (inbuf.empty())
			{
				continue;
			}
			strvect.push_back(inbuf);        
		}        
		if(strvect.empty())            
			return false;
		for_each(strvect.begin(),strvect.end(),analyzeSqlScript(m_SqlQuery_list,m_Ostr_query));        
		return !m_SqlQuery_list.empty();    
	}    
	//strMap c_inimap;
	strListType m_SqlQuery_list;
	ostringstream  m_Ostr_query;
	//vector<string> m_SqlQuery_vec;

};
//#include "E:\vc60\gogo\Deitel4en\examples\hanguangming\hanguangming\HtmlTag.h"
std::string translate(const std::string& aString,
					  const std::string& inputChars,
					  const std::string& outputChars,
					  const char pad)
{
	std::string rv = aString;
	std::string::size_type aspos = 0;
	std::string::size_type inpos = std::string::npos;
	char toc = pad;

	// By starting at the last position, we avoid infinite
	// loops in case someone did something dumb, like, for
	// example, setting inputChars=outputChars.
	while ((aspos = rv.find_first_of(inputChars, aspos))
		!= std::string::npos)
	{
		// figure out which char we found;
		inpos = inputChars.find(rv[aspos]);
		if (outputChars.length() < inpos)
			toc = pad;
		else
			toc = outputChars[inpos];
		rv[aspos] = toc;

		aspos++; // try to guarantee no infinite loops
	}

	return rv;
}
template<typename T> inline std::string ConvNumeric(const T &in)
{
	if (in == 0) return "0";
	char res[MAXBUF];
	memset(res,0,MAXBUF);
	char* out = res;
	T quotient = in;
	/*while (quotient) {
		*out = "0123456789"[ std::abs( (long)quotient % 10 ) ];
		++out;
		quotient /= 10;
	}*/
	while (quotient) {
		*out = "0123456789"[ std::abs( (T)quotient % 10 ) ];
		++out;
		quotient /= 10;
	}
	if ( in < 0)
		*out++ = '-';
	*out = 0;
	std::reverse(res,out);
	return res;
}

/** Template function to convert any input type to std::string
*/
inline std::string ConvToStr(const int in)
{
	return ConvNumeric(in);
}

/** Template function to convert any input type to std::string
*/
inline std::string ConvToStr(const long in)
{
	return ConvNumeric(in);
}

/** Template function to convert any input type to std::string
*/
inline std::string ConvToStr(const unsigned long in)
{
	return ConvNumeric(in);
}

/** Template function to convert any input type to std::string
*/
inline std::string ConvToStr(const char* in)
{
	return in;
}

/** Template function to convert any input type to std::string
*/
inline std::string ConvToStr(const bool in)
{
	return (in ? "1" : "0");
}

/** Template function to convert any input type to std::string
*/
inline std::string ConvToStr(char in)
{
	return std::string(in,1);
}

/** Template function to convert any input type to std::string
*/
template <class T> inline std::string ConvToStr(const T &in)
{
	std::stringstream tmp;
	if (!(tmp << in)) return std::string();
	return tmp.str();
}

/** Template function to convert any input type to any other type
* (usually an integer or numeric type)
*/
template<typename T> inline long ConvToInt(const T &in)
{
	std::stringstream tmp;
	if (!(tmp << in)) return 0;
	return atoi(tmp.str().c_str());
}

/** Template function to convert integer to char, storing result in *res and
* also returning the pointer to res. Based on Stuart Lowe's C/C++ Pages.
* @param T input value
* @param V result value
* @param R base to convert to
*/
template<typename T, typename V, typename R> inline char* itoa(const T &in, V *res, R base)
{
	if (base < 2 || base > 16) { *res = 0; return res; }
	char* out = res;
	int quotient = in;
	while (quotient) {
		*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
		++out;
		quotient /= base;
	}
	if ( in < 0 && base == 10) *out++ = '-';
	std::reverse( res, out );
	*out = 0;
	return res;
}

//参数定义为int但传入的是的unsigned int最大值，所以到这个函数内，是-1

template<typename T> string UINT64_to_string(T val)
{
	string result;
	while(1)
	{
		string::size_type pos = 0; //don't delete this line
		result.insert(pos, 1, '0' + val % 10);
		val = val / 10;
		if(val == 0)
			break;
	}

	return result;
}
void translateML(string mldata,map<string,string> & map_key_val) {

	string ntmp, vtmp;
	int    open, close;

	open  = mldata.find("<",0);
	close = mldata.find(">",open);
	ntmp  = mldata.substr(open+1,(close - open) -1);
	// ntmp  = mldata.substr(open+1,close-1);

	open  = mldata.find("</",close);
	vtmp  = mldata.substr(close+1,(open - close -1));

	//name  = ntmp;
	//value = vtmp;
	map_key_val.insert(make_pair(ntmp,vtmp));
}
const char * strstr1(const char * stringsrc,const char * strCharSet)
{
	for (int i=0;stringsrc[i]!='\0';i++)
	{
		int j=0;
		int temp=i;
		if (stringsrc[i]==strCharSet[j])
		{
			while (stringsrc[i++]==strCharSet[j++])
			{
				if (strCharSet[j]=='\0')
				{
					return &stringsrc[i-j];
				}
			}
			i=temp;
		}
	}
	return NULL;
}
int main ( )
{
	CSqlScriptParse SqlScript;    
	if(!SqlScript.open("c:\\tset.sql"))    
		return -1;    
	string ls1=SqlScript.read();
	CagreementPacket<int,unsigned char> syncDataAgreement;
	__int64 syncId=8978675;
	int userId=1231;

	char cuncuApp[]="insertinoUserNameAndPass";
	int cuncuAppSzie=strlen(cuncuApp);
	int paramCount=3;
	string para1="hanguangming";
	int para1Size=para1.length();
	string param2="12345678";
	int para2Size=param2.length();
	string param3="2008-03-03 10:57:23";
	int para3Size=param3.length();

	__int64 value1;
	int value2;
	char value3[128];
	int value4;
	string value5;
	string value6;
	string value7;

	int value3Len,value5Len,value6Len,value7Len;

	syncDataAgreement.writeField(&syncId,sizeof(__int64));
	syncDataAgreement.writeField(&userId,sizeof(int));
	syncDataAgreement.writeField(&cuncuAppSzie,sizeof(int));
	syncDataAgreement.writeField(cuncuApp,strlen(cuncuApp));
	syncDataAgreement.writeField(&paramCount,sizeof(int));
	syncDataAgreement.writeField(&para1Size,sizeof(int));
	syncDataAgreement.writeField(para1,para1.length());
	syncDataAgreement.writeField(&para2Size,sizeof(int));
	syncDataAgreement.writeField(param2,param2.length());
	syncDataAgreement.writeField(&para3Size,sizeof(int));
	syncDataAgreement.writeField(param3,param3.length());


	syncDataAgreement.readField(value1,sizeof(__int64));
	syncDataAgreement.readField(value2,sizeof(int));
	syncDataAgreement.readField(value3Len,sizeof(int));
	syncDataAgreement.readField(value3,value3Len);
	syncDataAgreement.readField(value4,sizeof(int));
	syncDataAgreement.readField(value5Len,sizeof(int));
	syncDataAgreement.readField(value5,value5Len);
	syncDataAgreement.readField(value6Len,sizeof(int));
	syncDataAgreement.readField(value6,value6Len);
	syncDataAgreement.readField(value7Len,sizeof(int));
	syncDataAgreement.readField(value7,value7Len);

	unsigned char * p_AgreementBuf=syncDataAgreement.getTotalDataFields();

	/*char * strHg="12345dfghj";
	char * strgm="5df";
	const char * retPtr=strstr1(strHg,strgm);
	char uu[20];
	char *p;
	char *sub="ask";
	char *str="askdaskaskdaskg";
	char *str2;
	int n=0,i=0,v;
	p=sub;
	str2=str;
	while (*str2)
	{
		while (*p)
		{
			if (*p==*str2)
			{
				break;
			}
			p++;
		}
		if (*p=='\0')
		{
			uu[i]=*str2;
			i++;
		}
		str2++;
		p=sub;
	}
	uu[i]=0;
	str=uu;
	printf("\n %s",str);
	return 0;*/

	/*ifstream xmlfile("c:\userInfo.xml");
	char datatbox[4096];
	memset(datatbox,0,sizeof(datatbox));
	map<string,string> xmlmap_;
	string inbuf;  
	
	while(!xmlfile.eof())        
	{            
		          
		getline(xmlfile,inbuf,'\n');           
		translateML(inbuf,xmlmap_);     
	}*/
	IniFile ini;    
	if(!ini.open("c:\\SINFO.ini"))    
		return -1;    
	string strvalue = ini.read("entry","url12");    
	if(strvalue.empty())        
		return -1;    
	else        
		cout<<"value="<<strvalue<<endl;

  //long   l= 20;
  //const char   *ntbs="Le minot passait la piece a frotter";
  //char   c;
  //char   buf[50];

//#ifndef _RWSTD_NO_EXCEPTIONS
  //try {
//#endif

    // create a read/write file-stream object on char
    // and attach it to an ifstream object
  //tellg();tellp();seekg()seekp();
  //string str1="#$%^&*fgt5678";
 // string str2=":";
  //string strkey="&gft6";
  //string retstr=translate(str1,strkey,str2,'A');
  //ifstream in("stdistream 和 'r' - Mythma‘s C++ blog - C++博客.htm");
  //int myint=200898989898;//
  unsigned int mountY=4294967295;
  string Astr=UINT64_to_string(mountY);
  unsigned int longsize=sizeof(long);
  unsigned int unsignedlongsize=sizeof(unsigned long);
  unsigned int intsize=sizeof(int);
  unsigned int unsignedintsize=sizeof(unsigned int);
	unsigned int shortsize=sizeof(short);
	unsigned int int64size=sizeof(__int64);
	unsigned int longlongsize=sizeof(long long);
	__int64 myint64=200898989898;
 // long mylong=200898989898;
string str1=ConvNumeric(myint64);
unsigned int inval=0x200808EF;
char putres[128];
//string str2=itoa(inval,putres,16);


		string::size_type	b, e,p1;
		string	var_name;
		string  oldinput="13245678989#13678789897#13909099986#15890012020#15908030456";
		string	input=oldinput;

	/* Look for any 'snmp:<x>' substrings. */
		p1=0;
		//while ((b = input.find("#",p1)) != input.npos) {
	/* Find the end of the SNMP variable name. */
	while ((e = input.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.",p1))!= input.npos)
	{
		var_name = input.substr(p1,e-p1);
		p1=e+1;
	}
	/* Don't consume a trailing '.' */
	/*if (e > 1 && input[e - 1] == '.') {
		e--;
	}*/

	/* Strip out the SNMP variable name. */	
	//var_name = input.substr(b + 5, e - b - 5);

	/* Look up the SNMP variable in the MIB and convert it to an OID. */
	
	//input.replace(b, e - b, var_name);

	/*_Count 
		从以_Off开始查找第_Count次出现的不属于_Ptr中的字符索引数。 

		例如： 
		str = "444-555-GGG" 
		str.find_first_not_of ( "45G", 0 );返回第一个"-"的索引值3。 
		str.find_first_not_of ( "45G", 0, 2 );返回第二个"-"的索引值7。因为从第0个字符开始，第2次不是‘45G’中的字符，为第二个"-"的索引值7。
		*/
  /*  if ( !in.is_open() ) 
#ifndef _RWSTD_NO_EXCEPTIONS
      throw(ios_base::failure("Open error"));
#else
    abort();
#endif*/
	 
  //HtmlTag * phtml=new HtmlTag(in);
    // tie the ostream object to the ifstream object
 /*   ostream out(in.rdbuf());   

    // output ntbs in out
    out << ntbs << endl;

    // seek to the beginning of the file
    in.seekg(0);

    // output each word on a separate line
    while ( in.get(c) )
    {
      if ( ifstream::traits_type::eq(c,' ') ) 
        cout << endl;
      else
        cout << c;
    }
    cout << endl << endl;

	// clear the state flags, fail bit set by last call to in.get(c)
	in.clear();

    // move back to the beginning of the file
    in.seekg(0); 

    // does the same thing as the previous code
    // output each word on a separate line
    while ( in >> buf )
      cout << buf << endl; 
    
    cout << endl << endl;

	// clear the flags, last in >> buf set fail bit because of nl at end of string
	in.clear(); 

    // output the base info before each integer
    out << showbase;

    ostream::pos_type pos= out.tellp();

    // output l in hex with a field with of 20 
    out << hex << setw(20) << l << endl;

    // output l in oct with a field with of 20
    out << oct << setw(20) << l << endl;

    // output l in dec with a field with of 20
    out << dec << setw(20) << l << endl;

    // move back to the beginning of the file
    in.seekg(0);

    // output the all file
    cout << in.rdbuf();

	// clear the flags 
	in.clear(); 

    // seek the input sequence to pos  
    in.seekg(pos);
  
    int a,b,d;

    in.unsetf(ios_base::basefield);

    // read the previous outputted integer
    in >> a >> b >> d;

    // output 3 times 20
    cout << a << endl << b << endl << d << endl;

#ifndef _RWSTD_NO_EXCEPTIONS
  }
  catch( ios_base::failure var )
  {
    cout << var.what();
  }
#endif  */ 
 
  return 0;
}
