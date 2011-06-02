#if !defined(_LOG_INCLUDE_H_)
#define _LOG_INCLUDE_H_


#include <tchar.h>
#include <list>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <strstream>
#include <time.h>
#include <utility>
#include "./MightyThread.h"

using namespace std;

#ifdef _UNICODE
typedef wstring tstring;
typedef wostringstream  tostringstream;
typedef wistringstream tistringstream;
typedef wfstream tfstream;
typedef wofstream tofstream;
typedef wifstream tifstream;
#else
typedef string  tstring;
typedef ostringstream tostringstream;
typedef istringstream tistringstream;
typedef fstream tfstream;
typedef ofstream tofstream;
typedef ifstream tifstream;
#endif

enum LogLevel
{
  Log_Err= 0, //错误信息级别
  Log_Warn, //警告信息
  Log_Info, //提示信息
  Log_Debug //调试信息
};

extern bool StartLog(void * instance);
extern void StopLog();
extern void SetLogLevel(LogLevel level);
extern void SetLogFile(const TCHAR* pwszFileName);

#define LOG(LogLevel,log) \
 CLog::getInstance()->WriteLog(LogLevel,log)

#define LOG_GB(LogLevel,log) \
CLog::getInstance()->WriteGBLog(LogLevel,log)

class CLog : public MightyThread
{

public:
  void SetLogLevel(LogLevel level);
  bool SetLogFile(const tstring& fileName);

  static CLog* getInstance(void)
  {
	  static CLog	logObject ;
	  return &logObject ;
  }

  bool WriteLog(LogLevel level,const TCHAR * log);
  
  bool WriteGBLog(LogLevel level,const char * log);
  void StopLog();
  bool StartLog(void * instance);

  virtual int threadrun(void * pBuf);

  virtual int ExecuteTask(void * pBuf);

  virtual int branchEvent(void * pBuf);

private:
  bool IsRuning();
  void CloseFile();
  bool OpenFile();  
  tstring FileName();
  CLog();
  virtual ~CLog();

  void DoThreadTask(void);

private:
  bool WriteStream_impl(const tstring& info);
  bool m_bRunning;

  list<tstring> m_logFifo;

  //tstring m_logFile;
  tstring m_logFile_Name_;

  FILE *pTarget_log;

  LogLevel m_logLevel;
};
#endif 
