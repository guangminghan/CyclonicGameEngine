#include "Log.h"

tstring logString[] =
{
  _T("LOG_ERROR"), _T("LOG_WARNING"), _T("LOG_INFO"), _T("LOG_DEBUG")
};

bool
StartLog(void * instance)
{
  return CLog::getInstance()->StartLog(instance);
}

void
StopLog()
{
  CLog::getInstance()->StopLog();
}

void		   
SetLogLevel(LogLevel level)
{
  CLog::getInstance()->SetLogLevel(level);
}

void
SetLogFile(const TCHAR* pwszFileName)
{
  const tstring wsFile = pwszFileName;
  CLog::getInstance()->SetLogFile(wsFile);
}

CLog::CLog()
:m_logLevel(Log_Warn)
,m_bRunning(false)
,pTarget_log(NULL)
{
}

CLog::~CLog()
{
	StopLog();
}

bool
CLog::SetLogFile(const tstring& fileName)
{

	if (fileName.empty())
	{
		m_logFile_Name_=FileName();
	}
	else
	{
		m_logFile_Name_=fileName+_T(".log");
	}

	return true;
}

bool
CLog::WriteLog(LogLevel level, const TCHAR* log)
{
  if (level <= m_logLevel)
  {
      tstring sLog;
	  TCHAR  pszLog[8192]={_T("")};
      time_t now = time(0);
      tm* pTmNow = localtime(&now);

      _stprintf(pszLog, _T("[%d-%02d-%02d %02d:%02d:%02d] %s %s"),
              pTmNow->tm_year + 1900, pTmNow->tm_mon + 1, pTmNow->tm_mday,
              pTmNow->tm_hour, pTmNow->tm_min, pTmNow ->tm_sec,
              logString[level].c_str(),log);
     sLog = pszLog;

	 WriteWait();
	 m_logFifo.push_back(sLog);
	 CancelWriteWait();
  }
  return true;
}

bool
CLog::StartLog(void * instance)
{
  if (IsRuning() == false)
  {
      if (OpenFile() == false)
      {
          return false;
      }
      m_bRunning = true;
	  setTimeOutVal(1000);
	  start(instance);
  }
  return true;
}

void
CLog::StopLog()
{
  if (IsRuning())
  {
      m_bRunning = false;
	  exit();
	  CloseFile();
  }
}

bool
CLog::IsRuning()
{
  return m_bRunning;
}

bool
CLog::OpenFile()
{
	pTarget_log= _tfopen(m_logFile_Name_.c_str(), _T("wb"));
	if (pTarget_log)
	{
		#ifdef _UNICODE
		fputwc(0xFEFF, pTarget_log);
		fflush(pTarget_log);
		#endif
		return true;
	}
	else
	{
		return false;
	}

}

void
CLog::CloseFile()
{
	if (pTarget_log)
	{
		fclose(pTarget_log);
		pTarget_log = NULL;
	}
}

/*bool CLog::WriteGBLog(LogLevel level,const char * log)
{
	TCHAR  wCode_Dst[4096];   

	AnsiToUnicode(log, wCode_Dst, 4096);

	if (level <= m_logLevel)
	{
		tstring sLog;
		TCHAR  pszLog[8192]={_T("")};
		time_t now = time(0);
		tm* pTmNow = localtime(&now);

		_stprintf(pszLog, _T("[%d-%d-%d %d:%d:%d] %s %s"),
			pTmNow->tm_year + 1900, pTmNow->tm_mon + 1, pTmNow->tm_mday,
			pTmNow->tm_hour, pTmNow->tm_min, pTmNow ->tm_sec,
			logString[level].c_str(),wCode_Dst);
		sLog = pszLog;

		WriteWait();
		m_logFifo.push_back(sLog.c_str());
		CancelWriteWait();
	}

	return true;

}*/

bool
CLog::WriteStream_impl(const tstring& info)
{
	fwrite(info.c_str(), 1,_tcslen(info.c_str())*sizeof(TCHAR),pTarget_log);
	fflush(pTarget_log);
	return true;
}

void
CLog::SetLogLevel(LogLevel level)
{
  m_logLevel = level;
}
tstring CLog::FileName()
{
	
	time_t timet;
	struct tm *tm;
	tstring fileFullPathName;
	time(&timet);
	tm=localtime(&timet);
	tostringstream tname;
	static unsigned long long numberIdex=0;
	tname<<tm->tm_year+1900<<std::setfill(_T('0'))<<std::setw(2)<<tm->tm_mon+1<<std::setw(2)<<tm->tm_mday<<std::setw(2)<<tm->tm_hour<<std::setw(2)<<tm->tm_min<<std::setw(2)<<tm->tm_sec<<_T("_")<<numberIdex<<_T(".log");
	fileFullPathName=tname.str();
	numberIdex++;
	return fileFullPathName;
}
int CLog::threadrun(void * pBuf)
{
	HANDLE MightyEvent[2];
	MightyEvent[0] = m_ExitSignal;
	MightyEvent[1] = m_ExecuteSignal;
	int retCode=0;

	while(1)
	{
		DWORD Index = WaitForMultipleObjects(2,MightyEvent,FALSE,m_dwTimeOut);
		switch(Index) 
		{
		case WAIT_OBJECT_0 + 0:
		case WAIT_ABANDONED_0:
		case WAIT_ABANDONED_0+1:
			{
				_endthreadex(0);
				return 0;
			}
		case WAIT_TIMEOUT:
		case WAIT_OBJECT_0 + 1:
			{
				WriteWait();
				DoThreadTask();
				CancelWriteWait();
				break;
			}
		default:
			break;
		}
	}

	return 0;
}
int CLog::ExecuteTask(void * pBuf)
{
	return 0;
}

int CLog::branchEvent(void * pBuf)
{
	return 0;
}
void CLog::DoThreadTask(void)
{
	tstring logInfo;
	while(!(m_logFifo.empty()))
	{			
		logInfo=m_logFifo.front();
		m_logFifo.pop_front();
		WriteStream_impl(logInfo);
	}
}