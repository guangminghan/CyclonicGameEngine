//#include "stdafx.h"
//#include <windows.h>

#include "./P2PUtilTools.h"
//#include <algorithm>
#include <Rpcdce.h>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
namespace P2PUtilTools
{
uint32 Time() 
{
	return GetTickCount();
}
uint32 StartTime() 
{

	static const uint32 g_start = Time();
	return g_start;
}

bool TimeIsBetween(uint32 later, uint32 middle, uint32 earlier) {
	if (earlier <= later) {
		return ((earlier <= middle) && (middle <= later));
	} else {
		return !((later < middle) && (middle < earlier));
	}
}
int32 TimeDiff(uint32 later, uint32 earlier) {
	uint32 LAST = 0xFFFFFFFF;
	uint32 HALF = 0x80000000;
	if (TimeIsBetween(earlier + HALF, later, earlier)) {
		if (earlier <= later) {
			return static_cast<long>(later - earlier);
		} else {
			return static_cast<long>(later + (LAST - earlier) + 1);
		}
	} else {
		if (later <= earlier) {
			return -static_cast<long>(earlier - later);
		} else {
			return -static_cast<long>(earlier + (LAST - later) + 1);
		}
	}
}
uint32 ElapsedTime() {
	return TimeDiff(Time(), StartTime());
}
bool is_private_ipa(int a)
{
	return (ntohl(a) & 0xff000000) == 0x0a000000
		|| (ntohl(a) & 0xff000000) == 0x7f000000
		|| (ntohl(a) & 0xfff00000) == 0xac100000
		|| (ntohl(a) & 0xffff0000) == 0xc0a80000;
}
std::string time2a()
{
	time_t v;
	time(&v);
	const tm* date = localtime(&v);
	if (!date)
		return "";
	char b[20];
	sprintf(b, "%04d-%02d-%02d %02d:%02d:%02d", date->tm_year + 1900, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
	return b;
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
/*void Trim(std::string& str) {
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		str.clear();
		return;
	}

	size_t last = str.find_last_not_of(" \t\r\n");
	ASSERT(last != std::string::npos);
}*/
void
split_str(const std::string &str, std::vector<std::string> &ret, char c)
{
	std::istringstream      is(str);
	std::string     value("");
	char            cc;

	while (is.get(cc)) {
		if (cc == c) {
			if (!value.empty())
			{
				if (value[value.length()-1]!='\\')
				{
					value.append("\\");
				}
				ret.push_back(value);
				value = "";
			}
			continue;
		}
		value.push_back(cc);
	}

	if (value != "") {
		if (value[value.length()-1]!='\\')
		{
			value.append("\\");
		}
		ret.push_back(value);
	}
}
const char *IpToString(DWORD ip)
{

	in_addr ipAddr;	
	ipAddr.S_un.S_addr=ip; //= htonl(ip);
	return inet_ntoa(ipAddr);

}
char * getIniPath(char *pszInifFileFullPath)
{
	char	path[_MAX_PATH];
	char	drv[_MAX_DRIVE];
	char	dir[_MAX_DIR];
	char	fName[_MAX_PATH];

	GetModuleFileName(NULL, path, _MAX_PATH);
	_splitpath(path, drv, dir, fName, NULL);
	_makepath(path, drv, dir, fName,"ini");

	strcpy(pszInifFileFullPath, path);
	return pszInifFileFullPath;
}
char * getRunPath(char * pszcurrentPath)
{
	char path_config[MAX_PATH];

	*path_config= 0;
	GetModuleFileName(NULL,path_config, MAX_PATH);
	if (*path_config)
		strrchr(path_config,'\\')[1] = 0;

	strcpy(pszcurrentPath,path_config);
	SetCurrentDirectory(path_config);
	return pszcurrentPath;
}
tstring format_ip(unsigned int ip) {
	const unsigned char *bytes=reinterpret_cast<const unsigned char*>(&ip);

	tstringstream ss;
	ss << (int)bytes[3] << _T('.') << (int)bytes[2] << _T('.') << (int)bytes[1] << _T('.') << (int)bytes[0];

	return ss.str();
}
unsigned int ParseIp(LPCTSTR str) {
	unsigned int ipa, ipb, ipc, ipd;

	if(_stscanf(str, _T("%u.%u.%u.%u"), &ipa, &ipb, &ipc, &ipd)==4) {
		union {
			unsigned int ip;
			unsigned char bytes[4];
		};

		bytes[0]=ipd;
		bytes[1]=ipc;
		bytes[2]=ipb;
		bytes[3]=ipa;

		return ip;
	}

	return 0;
}
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
std::string n(long long v)
{
	char b[21];
#ifdef WIN32
	sprintf(b, "%I64d", v);
#else
	sprintf(b, "%lld", v);
#endif
	return b;
}
void to_lower_tool(string &srcstr)
{
	transform(srcstr.begin(),srcstr.end(),srcstr.begin(),tolower);
}
void to_ouper_tool(string & srcStr)
{
	transform(srcStr.begin(),srcStr.end(),srcStr.begin(),::toupper);
}
char normalise_delim(char c) {
	if ('\\' == c) {
		return '/';
	}
	return c;
}
string determineBinaryLocation(const string& launchcmd) {
	string path(launchcmd);
	string binloc;
	transform(path.begin(), path.end(), path.begin(), normalise_delim);
	string::size_type delim = path.find_last_of('/');

	if (string::npos == delim) {
		return binloc = ".";
	}
	return binloc = path.substr(0, delim+1);
}
string GetStringUuid()
{
	//GUID syncIdGuid;
	static int index_tail=0;
	string sync_guid_str;
	if (index_tail>100000)
	{
		index_tail=0;
	}
	UUID uuid;
	char* pBuffer;
	bool uuid_create_fail=false;
	time_t uuid_time;
	time(&uuid_time); 
	if(UuidCreate(&uuid) == RPC_S_OK) 
	{
		if(UuidToString(&uuid,(BYTE**)&pBuffer) == RPC_S_OK) 
		{
			sync_guid_str.append(pBuffer);
			if(RpcStringFree((BYTE**)&pBuffer) != RPC_S_OK)
			{
				uuid_create_fail=true;
			}
				
		}
		else 
		{
			uuid_create_fail=true;
		}
	}
	else 
	{
		uuid_create_fail=true;
	}
	if (uuid_create_fail)
	{
		sync_guid_str=time2a()+'$'+ConvToStr(index_tail);
	}
	return sync_guid_str;
	/*if( CoCreateGuid(&syncIdGuid) != S_OK )
	{
		return string();
	}
	char* pBuffer;
	::UuidToString(&syncIdGuid,(BYTE**)&pBuffer);*/
	
	//RpcStringFree((BYTE**)&pBuffer);

}
std::string filesize_to_string(unsigned int size) {
	double size_display;
	std::string format;
	std::stringstream ret;

	// the comparisons to 1000 * (2^(n10)) are intentional
	// it's so you don't see something like "1023 bytes",
	// instead you'll see ".9 KB"

	if (size < 1000) {
		format = "Bytes";
		size_display = size;
	} else if (size < 1000 * 1024) {
		format = "KiB";
		size_display = (double)size / 1024.0;
	} else if (size < 1000 * 1024 * 1024) {
		format = "MiB";
		size_display = (double)size / (1024.0 * 1024.0);
	} else {
		format = "GiB";
		size_display = (double)size / (1024.0 * 1024.0 * 1024.0);
	}

	ret << std::setprecision(1) << std::setiosflags(std::ios::fixed) << size_display << " " << format;    
	return ret.str();
}
void ParseMap(const std::string& string, StringMap& map) {
	size_t start_of_line = 0;
	size_t end_of_line = 0;

	for (;;) { // for each line
		start_of_line = string.find_first_not_of("\r\n", end_of_line);
		if (start_of_line == std::string::npos)
			break;

		end_of_line = string.find_first_of("\r\n", start_of_line);
		if (end_of_line == std::string::npos) {
			end_of_line = string.length();
		}

		size_t equals = string.find('=', start_of_line);
		if ((equals >= end_of_line) || (equals == std::string::npos))
			continue;

		std::string key(string, start_of_line, equals - start_of_line);
		std::string value(string, equals + 1, end_of_line - equals - 1);

		//Trim(key);
		//Trim(value);

		if ((key.size() > 0) && (value.size() > 0))
			map[key] = value;
	}
}
bool Parse_HashRequest(string & hash_str,map<string,vector<string> > * filehash_map)
{
	//ed2k://|file|[maplesnow&52wy][kanokon][03][jap_chn][x264_aac_psp][720x480][F3922128].mp4|120912448|A2A6E3709F2C843BD2CB618DE961C9DB|/
	size_t _indexBegin_;
	size_t _indexCurent_;
	size_t _IndexEnd_;
	vector<string> temp_val;
	string temp_Key("");
	string loop_team_val;
	string hash_head_flag("value://|file|");
	bool isHead=true;

	_indexBegin_=hash_str.find("value://|file|");
	if (_indexBegin_==std::string::npos)
	{
		return false;
	}
	else
	{
		_indexBegin_+=hash_head_flag.length();
		//_indexBegin_+=1;
		for (;;)
		{
			
			_indexCurent_ = hash_str.find_first_not_of("|", _indexBegin_+1);
			if (_indexCurent_ ==std::string::npos)
			{
				break;
			}
			_IndexEnd_= hash_str.find_first_of("|", _indexCurent_);
			if (_indexCurent_ ==std::string::npos)
			{
				break;
			}
			loop_team_val=hash_str.substr(_indexBegin_,_IndexEnd_-_indexCurent_+1);
			_indexBegin_=_IndexEnd_+1;
			if (isHead)
			{
				temp_Key=loop_team_val;
				isHead=false;
			}
			else
			{
				temp_val.push_back(loop_team_val);
			}
		}
		filehash_map->insert(make_pair(temp_Key,temp_val));
		

	}
	return true;
}
bool Parse_HashLinkRequest(string & hash_str,SHARE_FILE_LIST_ST * filehash_link)
{
	size_t _indexBegin_;
	size_t _indexCurent_;
	size_t _IndexEnd_;
	vector<string> temp_val;
	string temp_Key("");
	string loop_team_val;
	string hash_head_flag("value://|file|");
	int value_index=0;
	char hash_buf[32]={0};
	char * p_hash=hash_buf;
	_indexBegin_=hash_str.find("value://|file|");
	if (_indexBegin_==std::string::npos)
	{
		return false;
	}
	else
	{
		_indexBegin_+=hash_head_flag.length();
		//_indexBegin_+=1;
		for (;;)
		{

			_indexCurent_ = hash_str.find_first_not_of("|", _indexBegin_+1);
			if (_indexCurent_ ==std::string::npos)
			{
				break;
			}
			_IndexEnd_= hash_str.find_first_of("|", _indexCurent_);
			if (_indexCurent_ ==std::string::npos)
			{
				break;
			}
			loop_team_val=hash_str.substr(_indexBegin_,_IndexEnd_-_indexCurent_+1);
			_indexBegin_=_IndexEnd_+1;
			if (value_index==0)
			{
				//提取文件名可能需要url解码
				strcpy(filehash_link->file_name,loop_team_val.c_str());
				value_index++;
				continue;
			}
			if(value_index==1)
			{
				//提取文件长度
				filehash_link->file_len=_tstoi64(loop_team_val.c_str());
				value_index++;
				
				continue;
			}
			if (value_index==2)
			{//提取全文Hash值
				strcpy(hash_buf,loop_team_val.c_str());

				for (int idx = 0; idx < 16; ++idx) {
					filehash_link->file_hash[idx] = (BYTE)(FromHexDigit(*p_hash++)*16);
					filehash_link->file_hash[idx] = (BYTE)(filehash_link->file_hash[idx] + FromHexDigit(*p_hash++));
				}
				break;
			}
		}

	}
	return true;
}
inline std::string second2Str(unsigned int second)
{
	char buf[128];

	if(second<60)
	{
		sprintf(buf, "%u sec", second);
		return buf;
	}

	if(second>=60 && second<60*60)
	{
		unsigned int min = second / 60;
		unsigned int sec = second % 60;
		sprintf(buf, "%u min %u sec", min, sec);
		return buf;
	}

	unsigned int hou = second/(60*60);
	unsigned int min = (second%(60*60))/60;
	unsigned int sec = second % 60;
	sprintf(buf, "%u hou %u min %u sec", hou, min, sec);
	return buf;	
}

inline std::string byteCount2Str(__int64 count)
{
	char buf[128];

	if(count < 1024)
	{
		sprintf(buf, "%d B", (int)count);
		return buf;
	}

	if(count>=1024 && count<1024*1024)
	{
		float fcount = ((float)count)/1024.0f;
		sprintf(buf, "%01.2f KB", fcount);
		return buf;
	}

	if(count>=1024*1024 && count<1024*1024*1024)
	{
		float fcount = ((float)count)/(1024.0f*1024.0f);
		sprintf(buf, "%01.2f MB", fcount);
		return buf;		
	}

	float fcount = ((float)count)/(1024.0f*1024.0f*1024.0f);
	sprintf(buf, "%01.2f GB", fcount);
	return buf;		
}
__int64 htonllTo( __int64 src) {
	__int64 dst;
	unsigned char *x = (unsigned char *)&dst;
	x[0] = (src >> 56) & 0xff;
	x[1] = (src >> 48) & 0xff;
	x[2] = (src >> 40) & 0xff;
	x[3] = (src >> 32) & 0xff;
	x[4] = (src >> 24) & 0xff;
	x[5] = (src >> 16) & 0xff;
	x[6] = (src >> 8) & 0xff;
	x[7] = (src) & 0xff;
	return dst;
}

__int64 ntohllTo( __int64 src) {
	__int64 dst = 0;
	unsigned char *x = (unsigned char *)&src;
	dst |= ((__int64)x[0])<<56;
	dst |= ((__int64)x[1])<<48;
	dst |= ((__int64)x[2])<<40;
	dst |= ((__int64)x[3])<<32;
	dst |= ((__int64)x[4])<<24;
	dst |= ((__int64)x[5])<<16;
	dst |= ((__int64)x[6])<<8;
	dst |= ((__int64)x[7]);
	return dst;
}
inline __int64 htonll(__int64 number)
{
	if(htons(1) == 1)
	{
		return number;
	}

	return ( htonl( (number >> 32) & 0xFFFFFFFF) |
		((__int64) (htonl(number & 0xFFFFFFFF))  << 32));
}

inline __int64 ntohll(__int64 number)
{
	if(htons(1) == 1)
	{
		return number;
	}

	return ( htonl( (number >> 32) & 0xFFFFFFFF) |
		((__int64) (htonl(number & 0xFFFFFFFF))  << 32));
}
unsigned __int64 GetDiskFileSize(const string & path)
{
	static BOOL _bInitialized = FALSE;
	static DWORD (WINAPI *_pfnGetCompressedFileSize)(LPCTSTR, LPDWORD) = NULL;

	if (!_bInitialized){
		_bInitialized = TRUE;
		(FARPROC&)_pfnGetCompressedFileSize = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), _TWINAPI("GetCompressedFileSize"));
	}

	// If the file is not compressed nor sparse, 'GetCompressedFileSize' returns the 'normal' file size.
	if (_pfnGetCompressedFileSize)
	{
		unsigned __int64 ullCompFileSize;
		LPDWORD pdwCompFileSize = (LPDWORD)&ullCompFileSize;
		pdwCompFileSize[0] = (*_pfnGetCompressedFileSize)(path.c_str(), &pdwCompFileSize[1]);
		if (pdwCompFileSize[0] != INVALID_FILE_SIZE || GetLastError() == NO_ERROR)
			return ullCompFileSize;
	}

	// If 'GetCompressedFileSize' failed or is not available, use the default function
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(path.c_str(), &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;
	FindClose(hFind);

	return (unsigned __int64)fd.nFileSizeHigh << 32 | (unsigned __int64)fd.nFileSizeLow;
}
void findAllShareFile(const vector<string> & sharePath,map<string,vector<string> > & path_files)
{
	FILE *fp;
	long done;
	struct _finddata_t f;
	vector<string> file_room;
	char pathOld[512]={0};
	_getcwd(pathOld,_MAX_PATH);//取当前路径
	unsigned long loopIndex=0;
	unsigned long endIndex=sharePath.size();
	for(;loopIndex<endIndex;loopIndex++)
	{
		file_room.clear();
		chdir(sharePath[loopIndex].c_str());
		if((done=_findfirst("*.*",&f))==-1L)
			continue;
		else
		{
			if((f.attrib&0x00000010)&&(f.name[0]!='.'))
			{
				;
			}
			else if(f.name[0]=='.')
			{
				;
			}
			else
			{
				//fprintf(fp,"%s\\%s",path,f.name);
				//fprintf(fp,"\t %.24s",ctime( &( f.time_write ) ));
				//fprintf(fp,"\t %91d\n",f.size);

				//string complete_path=sharePath[loopIndex]+f.name;
				file_room.push_back(f.name);
			}
			while(_findnext(done,&f)==0)
			{
				if((f.attrib&0x00000010)&&(f.name[0]!='.'))
				{
					;
				}
				else if(f.name[0]=='.')
				{
					;
				}
				else
				{
					//string complete_path=sharePath[loopIndex]+f.name;
					file_room.push_back(f.name);

				}
			}
			_findclose(done);
			path_files.insert(make_pair(sharePath[loopIndex],file_room));
			//file_room.clear();

		}
		
	}
	chdir(pathOld);
}
string  getFileNameFromPath(const string & comptPath)
{
	size_t indexX=comptPath.rfind('\\',comptPath.length());
	string valc=comptPath.substr(indexX+1,comptPath.length()-indexX);
	return valc;
}
__int64 get64BitFileLen(const char * file_name)
{
	struct _stati64 hash_stat;
	__int64 file_lenX=0;
	//if(_fstat64("c:\\hanguangming.dat",&hash_stat)==0)

	if(_stati64(file_name,&hash_stat)==0)
	{
		file_lenX=hash_stat.st_size;
	}
	return file_lenX;
}

inline unsigned int FromHexDigit(TCHAR digit) {
	switch (digit) {
		case _T('0'): return 0;
		case _T('1'): return 1;
		case _T('2'): return 2;
		case _T('3'): return 3;
		case _T('4'): return 4;
		case _T('5'): return 5;
		case _T('6'): return 6;
		case _T('7'): return 7;
		case _T('8'): return 8;
		case _T('9'): return 9;
		case _T('A'): return 10;
		case _T('B'): return 11;
		case _T('C'): return 12;
		case _T('D'): return 13;
		case _T('E'): return 14;
		case _T('F'): return 15;
		case _T('a'): return 10;
		case _T('b'): return 11;
		case _T('c'): return 12;
		case _T('d'): return 13;
		case _T('e'): return 14;
		case _T('f'): return 15;
		default: return 0;
	}
}
void ComposeLink(string& lnk,unsigned char *m_hash)
{
	lnk = _T("ed2k://|friend|");
	//lnk += m_sUserName + _T("|");
	for (int idx = 0; idx < 16; ++idx)
	{
		unsigned int ui1 = m_hash[idx] / 16;
		unsigned int ui2 = m_hash[idx] % 16;
		lnk += static_cast<TCHAR>( ui1 < 10 ? (_T('0')+ui1) : (_T('A')+(ui1-10)) );
		lnk += static_cast<TCHAR>( ui2 < 10 ? (_T('0')+ui2) : (_T('A')+(ui2-10)) );
	}
	lnk += _T("|/");
}
bool GetIpList(list<string> &r_iplist)
{
	char szhn[256]={0}; 
	int nStatus = gethostname(szhn, sizeof(szhn));
	if (nStatus == SOCKET_ERROR )
	{
			return false;
	}
	HOSTENT *host = gethostbyname(szhn);
	if (host != NULL)
	{ 
		for ( int i=0;;i++)
		{ 
			r_iplist.push_back(inet_ntoa(*(IN_ADDR*)host->h_addr_list[i])) ;

			if ( host->h_addr_list[i] + host->h_length >= host->h_name )
				break;
		} 
	}


	return true;
}
string UTIL_HashToString( const string &strHash )
{
	// convert a hash to a readable string

	string strString;

	for( unsigned long i = 0; i < strHash.size( ); i++ )
	{
		char pBuf[4];
		memset( pBuf, 0, sizeof( char ) * 4 );

		unsigned char c = strHash[i];

		sprintf( pBuf, "%02x", c );

		strString += pBuf;
	}

	return strString;
}
string UTIL_StringToHash( const string &strString )
{
	// convert a readable string hash to a 20 character hash
	//这是针对sha
	string strHash;

	if( strString.size( ) != 40 )
		return string( );

	for( unsigned long i = 0; i < strString.size( ); i += 2 )
	{
		char pBuf[4];

		memset( pBuf, 0, sizeof( char ) * 4 );

		pBuf[0] = strString[i];
		pBuf[1] = strString[i + 1];

		unsigned int c;

		sscanf( pBuf, "%02x", &c );

		strHash += c;
	}

	return strHash;
}

void ReportHash(char *szReport, unsigned char *m_digest)
{
	unsigned char i = 0;
	char szTemp[4];
	sprintf(szTemp, "%02x", m_digest[0]);
	strcat(szReport, szTemp);
	for(i = 1; i < 20; i++)
	{
		sprintf(szTemp, "%02x", m_digest[i]);
		strcat(szReport, szTemp);
	}
	
	/*else if(uReportType == REPORT_DIGIT)
	{
		sprintf(szTemp, "%u", m_digest[0]);
		strcat(szReport, szTemp);

		for(i = 1; i < 20; i++)
		{
			sprintf(szTemp, " %u", m_digest[i]);
			strcat(szReport, szTemp);
		}
	}*/

}
void * p2pcalloc( size_t nmemb, size_t size) 
{
	void *region = calloc( nmemb, size);

	return region;
}

void * p2pmalloc( size_t size) 
{
	void *region = malloc( size);

	return region;
}

void * p2prealloc( void *ptr, size_t size) 
{

	void *region = realloc( ptr, size);

	return region;
}

void p2pfree( void *ptr) 
{
	free( ptr);
}



}
