#if !defined(P2PUTILTOOLS__INCLUDED_)
#define P2PUTILTOOLS__INCLUDED_
#include "./common.h"
//#include <string>
//#include <sstream>
//#include <vector>
//#include <set>
//using std::string;
using namespace std;
#define COMPILER_MSVC
typedef long int32;
typedef short int16;
typedef char int8;

#ifdef COMPILER_MSVC
typedef unsigned __int64 uint64;
typedef __int64 int64;
#else
typedef unsigned long long uint64;
typedef long long int64;
#endif

typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
#define MAXBUF 512
#define _TWINAPI(fname)	fname "W"
typedef std::basic_string<TCHAR> tstring;
typedef std::basic_stringstream<TCHAR> tstringstream;
typedef std::basic_ostringstream<TCHAR> tostringstream;
typedef std::ostream_iterator<tstring,TCHAR> tostream_iterator;

template <class MapT>
class eraseFct {
public:
	eraseFct(MapT* m) : theMap(m) {}
	typename MapT::value_type operator() (__int64 id)
	{ 
		typename MapT::iterator iter = theMap->find(id);

		if (iter==theMap->end()) 
			throw invalid_argument(id);
		typename MapT::value_type res = *iter;
		theMap->erase(iter);
		return res;
	}
private:
	MapT* theMap;
};

template <class MapT>
eraseFct<MapT> eraser(MapT* m) 
{
	return eraseFct<MapT>(m);
}
typedef std::map<std::string,std::string> StringMap;
namespace P2PUtilTools  
{
	uint32 Time();

	uint32 StartTime();
	bool TimeIsBetween(uint32 later, uint32 middle, uint32 earlier);
	int32 TimeDiff(uint32 later, uint32 earlier);
	uint32 ElapsedTime();

	bool is_private_ipa(int a);
	string time2a();
	string& trim(std::string &s) ;
	void to_lower_tool(string &srcstr);
	void to_ouper_tool(string & srcStr);
	char normalise_delim(char c);
	string determineBinaryLocation(const string& launchcmd);
	template<typename T> inline std::string ConvNumeric(const T &in)
	{
		if (in == 0) return "0";
		char res[MAXBUF];
		memset(res,0,MAXBUF);
		char* out = res;
		T quotient = in;

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
	};
	template <class T> inline std::string ConvToStr(const T &in)
	{
		std::stringstream tmp;
		if (!(tmp << in)) return std::string();
		return tmp.str();
	};
	template<typename T> inline long ConvToInt(const T &in)
	{
		std::stringstream tmp;
		if (!(tmp << in)) return 0;
		return atoi(tmp.str().c_str());
	};
	/*bool IsUtf8FirstByte(int c) {
		return (((c)&0x80)==0) || // is single byte
			((unsigned char)((c)-0xc0)<0x3e); // or is lead byte
	}*/
	void ParseMap(const std::string& string, StringMap& map);

	void split_str(const std::string &str, std::vector<std::string> &ret, char c);
	const char *IpToString(DWORD ip);
	char * getIniPath(char *pszInifFileFullPath);
	char * getRunPath(char * pszcurrentPath);
	tstring format_ip(unsigned int ip);
	unsigned int ParseIp(LPCTSTR str);
	void PushIps(set<unsigned int> &iplist, const char *ips);
	string n(long long v);
	string GetStringUuid();
	std::string filesize_to_string(unsigned int size);
	bool Parse_HashRequest(string & hash_str,map<string,vector<string> > * filehash_map);
	bool Parse_HashLinkRequest(string & hash_str,SHARE_FILE_LIST_ST * filehash_link);
	inline std::string second2Str(unsigned int second);
	inline std::string byteCount2Str(__int64 count);
	inline __int64 htonll(__int64 number);
	inline __int64 ntohll(__int64 number);
	unsigned __int64 GetDiskFileSize(const string & path);
	void findAllShareFile(const vector<string> & sharePath,map<string,vector<string> > & path_files);

	string  getFileNameFromPath(const string & comptPath);
	__int64 get64BitFileLen(const char * file_name);
	inline unsigned int FromHexDigit(TCHAR digit);
	void ComposeLink(string& lnk,unsigned char *m_hash);
	bool GetIpList(list<string> &r_iplist);
	string UTIL_HashToString( const string &strHash );
	string UTIL_StringToHash( const string &strString );
	void ReportHash(char *szReport, unsigned char *m_digest);
	__int64 htonllTo( __int64 src);
	__int64 ntohllTo( __int64 src);
	void * p2pcalloc( size_t nmemb, size_t size);
	void * p2pmalloc( size_t size);
	void * p2prealloc( void *ptr, size_t size);
	void p2pfree( void *ptr);


};

#endif // !defined(P2PUTILTOOLS__INCLUDED_)
