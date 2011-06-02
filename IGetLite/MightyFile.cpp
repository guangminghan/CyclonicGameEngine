
#include ".\mightyfile.h"



#include <winnetwk.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
inline BOOL IsDirSep(TCHAR ch)
{
	return (ch == '\\' || ch == '/');
}
/*BOOL MightyFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
// lpszPathOut = buffer of _MAX_PATH
// lpszFileIn = file, relative path or absolute path
// (both in ANSI character set)
{
	//ASSERT(AfxIsValidAddress(lpszPathOut, _MAX_PATH));

	// first, fully qualify the path name
	LPTSTR lpszFilePart;
	DWORD dwRet = GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);
	if (dwRet == 0)
	{
#ifdef _DEBUG
		if (lpszFileIn[0] != '\0')
			TRACE(traceAppMsg, 0, _T("Warning: could not parse the path '%s'.\n"), lpszFileIn);
#endif
		lstrcpyn(lpszPathOut, lpszFileIn, _MAX_PATH); // take it literally
		return FALSE;
	}
	else if (dwRet >= _MAX_PATH)
	{
#ifdef _DEBUG
		if (lpszFileIn[0] != '\0')
			TRACE1("Warning: could not parse the path '%s'. Path is too long.\n", lpszFileIn);
#endif
		return FALSE; // long path won't fit in buffer
	}

	string strRoot;
	// determine the root name of the volume
	MightyGetRoot(lpszPathOut, strRoot);

	if (!::PathIsUNC( strRoot ))
	{
		// get file system information for the volume
		DWORD dwFlags, dwDummy;
		if (!GetVolumeInformation(strRoot, NULL, 0, NULL, &dwDummy, &dwFlags,
			NULL, 0))
		{
			TRACE(traceAppMsg, 0, _T("Warning: could not get volume information '%s'.\n"),
				(LPCTSTR)strRoot);
			return FALSE;   // preserving case may not be correct
		}

		// not all characters have complete uppercase/lowercase
		if (!(dwFlags & FS_CASE_IS_PRESERVED))
			CharUpper(lpszPathOut);

		// assume non-UNICODE file systems, use OEM character set
		if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
		{
			WIN32_FIND_DATA data;
			HANDLE h = FindFirstFile(lpszFileIn, &data);
			if (h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);
				if(lpszFilePart != NULL && lpszFilePart > lpszPathOut)
				{
					int nFileNameLen = lstrlen(data.cFileName);
					int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
					if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
						lstrcpy(lpszFilePart, data.cFileName);
					else
						return FALSE; // Path doesn't fit in the buffer.
				}
				else
					return FALSE;
			}
		}
	}

	return TRUE;
}

void MightyGetRoot(LPCTSTR lpszPath, string& strRoot)
{
	//ASSERT(lpszPath != NULL);
	strRoot.reserve(_MAX_PATH+1)
	LPTSTR lpszRoot =(char *) strRoot.c_str();
	memset(lpszRoot, 0, _MAX_PATH);
	lstrcpyn(lpszRoot, lpszPath, _MAX_PATH);
	PathStripToRoot(lpszRoot);
	
}

BOOL MightyComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
{
	// use case insensitive compare as a starter
	if (lstrcmpi(lpszPath1, lpszPath2) != 0)
		return FALSE;

	// on non-DBCS systems, we are done
	if (!GetSystemMetrics(SM_DBCSENABLED))
		return TRUE;

	// on DBCS systems, the file name may not actually be the same
	// in particular, the file system is case sensitive with respect to
	// "full width" roman characters.
	// (ie. fullwidth-R is different from fullwidth-r).
	int nLen = lstrlen(lpszPath1);
	if (nLen != lstrlen(lpszPath2))
		return FALSE;
	//ASSERT(nLen < _MAX_PATH);

	// need to get both CT_CTYPE1 and CT_CTYPE3 for each filename
	LCID lcid = GetThreadLocale();
	WORD aCharType11[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath1, -1, aCharType11));
	WORD aCharType13[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath1, -1, aCharType13));
	WORD aCharType21[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath2, -1, aCharType21));
#ifdef _DEBUG
	WORD aCharType23[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath2, -1, aCharType23));
#endif

	// for every C3_FULLWIDTH character, make sure it has same C1 value
	int i = 0;
	for (LPCTSTR lpsz = lpszPath1; *lpsz != 0; lpsz = _tcsinc(lpsz))
	{
		// check for C3_FULLWIDTH characters only
		if (aCharType13[i] & C3_FULLWIDTH)
		{
#ifdef _DEBUG
			ASSERT(aCharType23[i] & C3_FULLWIDTH); // should always match!
#endif

			// if CT_CTYPE1 is different then file system considers these
			// file names different.
			if (aCharType11[i] != aCharType21[i])
				return FALSE;
		}
		++i; // look at next character type
	}
	return TRUE; // otherwise file name is truly the same
}
UINT MightyGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
	//ASSERT(lpszTitle == NULL ||AfxIsValidAddress(lpszTitle, _MAX_FNAME));
	//ASSERT(AfxIsValidString(lpszPathName));

	// always capture the complete file name including extension (if present)
	LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

	// lpszTitle can be NULL which just returns the number of bytes
	if (lpszTitle == NULL)
		return lstrlen(lpszTemp)+1;

	// otherwise copy it into the buffer provided
	lstrcpyn(lpszTitle, lpszTemp, nMax);
	return 0;
}
UINT MightyGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
	//ASSERT(lpszTitle == NULL ||AfxIsValidAddress(lpszTitle, _MAX_FNAME));
	//ASSERT(AfxIsValidString(lpszPathName));

	// use a temporary to avoid bugs in ::GetFileTitle when lpszTitle is NULL
	TCHAR szTemp[_MAX_PATH];
	LPTSTR lpszTemp = lpszTitle;
	if (lpszTemp == NULL)
	{
		lpszTemp = szTemp;
		nMax = _countof(szTemp);
	}
	if (::GetFileTitle(lpszPathName, lpszTemp, (WORD)nMax) != 0)
	{
		// when ::GetFileTitle fails, use cheap imitation
		return MightyGetFileName(lpszPathName, lpszTitle, nMax);
	}
	return lpszTitle == NULL ? lstrlen(lpszTemp)+1 : 0;
}

void MightyGetModuleShortFileName(HINSTANCE hInst, string& strShortName)
{
	TCHAR szLongPathName[_MAX_PATH];
	strShortName.reserve(_MAX_PATH+1);
	
	::GetModuleFileName(hInst, szLongPathName, _MAX_PATH);
	if (::GetShortPathName(szLongPathName,(char *)strShortName.c_str(), _MAX_PATH) == 0)
	{
		//strShortName.GetBuffer(_MAX_PATH), _MAX_PATH) == 0)
		// rare failure case (especially on not-so-modern file systems)
		strShortName = szLongPathName;
	}
	//strShortName.ReleaseBuffer();
}*/
/*#ifndef _AFX_NO_OLE_SUPPORT

#undef DEFINE_GUID

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C AFX_COMDAT const GUID afx##name \
	= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define DEFINE_SHLGUID(name, l, w1, w2) DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

DEFINE_SHLGUID(CLSID_ShellLink, 0x00021401L, 0, 0);
#ifndef _UNICODE
DEFINE_SHLGUID(IID_IShellLinkA, 0x000214EEL, 0, 0);
#else
DEFINE_SHLGUID(IID_IShellLinkW, 0x000214F9L, 0, 0);
#endif
#define CLSID_ShellLink afxCLSID_ShellLink

#undef IID_IShellLink
#undef IShellLink
#ifndef _UNICODE
#define IID_IShellLink afxIID_IShellLinkA
#define IShellLink IShellLinkA
#else
#define IID_IShellLink afxIID_IShellLinkW
#define IShellLink IShellLinkW
#endif

#endif !_AFX_NO_OLE_SUPPORT*/


////////////////////////////////////////////////////////////////////////////
// MightyFile implementation

const HANDLE MightyFile::hFileNull = INVALID_HANDLE_VALUE;

MightyFile::MightyFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_bCloseOnDelete = FALSE;
}

MightyFile::MightyFile(HANDLE hFile)
{
	m_hFile = hFile;
	m_bCloseOnDelete = FALSE;
}

MightyFile::MightyFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	//ASSERT(AfxIsValidString(lpszFileName));
	m_hFile = INVALID_HANDLE_VALUE;

	//MightyFileException e;
	//if (!Open(lpszFileName, nOpenFlags, &e))
		//AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
}

MightyFile::~MightyFile()
{
	if (m_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete)
		Close();
}

MightyFile* MightyFile::Duplicate() const
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	MightyFile* pFile = new MightyFile(INVALID_HANDLE_VALUE);
	HANDLE hFile;
	if (!::DuplicateHandle(::GetCurrentProcess(), m_hFile,
		::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		delete pFile;
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
	}
	pFile->m_hFile = hFile;
	//ASSERT(pFile->m_hFile != INVALID_HANDLE_VALUE);
	pFile->m_bCloseOnDelete = m_bCloseOnDelete;
	return pFile;
}

BOOL MightyFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	//ASSERT_VALID(this);
	//ASSERT(AfxIsValidString(lpszFileName));

	//ASSERT(pException == NULL ||AfxIsValidAddress(pException, sizeof(MightyFileException)));
	//ASSERT((nOpenFlags & typeText) == 0);   // text mode not supported

	// shouldn't open an already open file (it will leak)
	//ASSERT(m_hFile == INVALID_HANDLE_VALUE);

	// MightyFile objects are always binary and CreateFile does not need flag
	nOpenFlags &= ~(UINT)typeBinary;

	m_bCloseOnDelete = FALSE;

	m_hFile = INVALID_HANDLE_VALUE;
	//m_strFileName.Empty();
	m_strFileName.clear();
	TCHAR szTemp[_MAX_PATH];
	if (lstrlen(lpszFileName) < _MAX_PATH)
	{
		if(MightyFullPath(szTemp, lpszFileName) == FALSE )
			return FALSE;
	}
	else
		return FALSE; // path is too long

	m_strFileName = szTemp;

	//	ASSERT(sizeof(HANDLE) == sizeof(UINT));
	//ASSERT(shareCompat == 0);

	// map read/write mode
	//ASSERT((modeRead|modeWrite|modeReadWrite) == 3);
	DWORD dwAccess = 0;
	switch (nOpenFlags & 3)
	{
	case modeRead:
		dwAccess = GENERIC_READ;
		break;
	case modeWrite:
		dwAccess = GENERIC_WRITE;
		break;
	case modeReadWrite:
		dwAccess = GENERIC_READ | GENERIC_WRITE;
		break;
	default:
		break;
		//ASSERT(FALSE);  // invalid share mode
	}

	// map share mode
	DWORD dwShareMode = 0;
	switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
	{
	default:
		//ASSERT(FALSE);  // invalid share mode?
		break;
	case shareCompat:
	case shareExclusive:
		dwShareMode = 0;
		break;
	case shareDenyWrite:
		dwShareMode = FILE_SHARE_READ;
		break;
	case shareDenyRead:
		dwShareMode = FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
		break;
	}

	// Note: typeText and typeBinary are used in derived classes only.

	// map modeNoInherit flag
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

	// map creation flags
	DWORD dwCreateFlag;
	if (nOpenFlags & modeCreate)
	{
		if (nOpenFlags & modeNoTruncate)
			dwCreateFlag = OPEN_ALWAYS;
		else
			dwCreateFlag = CREATE_ALWAYS;
	}
	else
		dwCreateFlag = OPEN_EXISTING;

	// special system-level access flags

	// Random access and sequential scan should be mutually exclusive
	//ASSERT((nOpenFlags&(osRandomAccess|osSequentialScan)) != (osRandomAccess|osSequentialScan) );

	DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
	if (nOpenFlags & osNoBuffer)
		dwFlags |= FILE_FLAG_NO_BUFFERING;
	if (nOpenFlags & osWriteThrough)
		dwFlags |= FILE_FLAG_WRITE_THROUGH;
	if (nOpenFlags & osRandomAccess)
		dwFlags |= FILE_FLAG_RANDOM_ACCESS;
	if (nOpenFlags & osSequentialScan)
		dwFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

	// attempt file creation
	HANDLE hFile = ::CreateFile(lpszFileName, dwAccess, dwShareMode, &sa,
		dwCreateFlag, dwFlags, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		/*if (pException != NULL)
		{
			pException->m_lOsError = ::GetLastError();
			pException->m_cause =
				MightyFileException::OsErrorToException(pException->m_lOsError);

			// use passed file name (not expanded vesion) when reporting
			// an error while opening

			pException->m_strFileName = lpszFileName;
		}*/
		return FALSE;
	}
	m_hFile = hFile;
	m_bCloseOnDelete = TRUE;

	return TRUE;
}

UINT MightyFile::Read(void* lpBuf, UINT nCount)
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (nCount == 0)
		return 0;   // avoid Win32 "null-read"

	//ASSERT(lpBuf != NULL);
	//ASSERT(AfxIsValidAddress(lpBuf, nCount));

	DWORD dwRead;
	if (!::ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL))
	{
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
		return -1;
	}

	return (UINT)dwRead;
}

void MightyFile::Write(const void* lpBuf, UINT nCount)
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (nCount == 0)
		return;     // avoid Win32 "null-write" option

	//ASSERT(lpBuf != NULL);
	//ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

	DWORD nWritten;
	if (!::WriteFile(m_hFile, lpBuf, nCount, &nWritten, NULL))
	{
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
		return;
	}

	// Win95 will not return an error all the time (usually DISK_FULL)
	if (nWritten != nCount)
	{
		//AfxThrowFileException(MightyFileException::diskFull, -1, m_strFileName);
		return;
	}
}

ULONGLONG MightyFile::Seek(LONGLONG lOff, UINT nFrom)
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	//ASSERT(nFrom == begin || nFrom == end || nFrom == current);
	//ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

	LARGE_INTEGER liOff;

	liOff.QuadPart = lOff;
	liOff.LowPart = ::SetFilePointer(m_hFile, liOff.LowPart, &liOff.HighPart,(DWORD)nFrom);
	if (liOff.LowPart  == (DWORD)-1)
	{
		//if (::GetLastError() != NO_ERROR)
			//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
		return 0;
	}

	return liOff.QuadPart;
}

ULONGLONG MightyFile::GetPosition() const
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	LARGE_INTEGER liPos;
	liPos.QuadPart = 0;
	liPos.LowPart = ::SetFilePointer(m_hFile, liPos.LowPart, &liPos.HighPart , FILE_CURRENT);
	if (liPos.LowPart == (DWORD)-1)
	{
		//if (::GetLastError() != NO_ERROR)
			//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
		return 0;
	}

	return liPos.QuadPart;
}
BOOL MightyFile::GetStatus(MightyFileStatus& rStatus) const
{
	//ASSERT_VALID(this);

	memset(&rStatus, 0, sizeof(MightyFileStatus));

	// copy file name from cached m_strFileName
	lstrcpyn(rStatus.m_szFullName, m_strFileName.c_str(),_countof(rStatus.m_szFullName));
	if (m_hFile != hFileNull)
	{
		// get time current file size
		FILETIME ftCreate, ftAccess, ftModify;
		if (!::GetFileTime(m_hFile, &ftCreate, &ftAccess, &ftModify))
			return FALSE;

		if ((rStatus.m_size = ::GetFileSize(m_hFile, NULL)) == (DWORD)-1L)
			return FALSE;

		if (m_strFileName.empty())
			rStatus.m_attribute = 0;
		else
		{
			DWORD dwAttribute = ::GetFileAttributes(m_strFileName.c_str());

			// don't return an error for this because previous versions of MFC didn't
			if (dwAttribute == 0xFFFFFFFF)
				rStatus.m_attribute = 0;
			else
			{
				rStatus.m_attribute = (BYTE) dwAttribute;
/*#ifdef _DEBUG
				// MFC BUG: m_attribute is only a BYTE wide
				if (dwAttribute & ~0xFF)
					TRACE(traceAppMsg, 0, "Warning: CFile::GetStatus() returns m_attribute without high-order flags.\n");
#endif*/
			}
		}

		// convert times as appropriate
		rStatus.m_ctime = MightyTime(ftCreate);
		rStatus.m_atime = MightyTime(ftAccess);
		rStatus.m_mtime = MightyTime(ftModify);

		if (rStatus.m_ctime.GetTime() == 0)
			rStatus.m_ctime = rStatus.m_mtime;

		if (rStatus.m_atime.GetTime() == 0)
			rStatus.m_atime = rStatus.m_mtime;
	}
	return TRUE;
}
string MightyFile::GetFileName() const
{
	//ASSERT_VALID(this);

	MightyFileStatus status;
	GetStatus(status);
	string strResult;
	strResult.reserve(_MAX_FNAME+1);
	MightyGetFileName(status.m_szFullName, (char *)strResult.c_str(),_MAX_FNAME);
	//AfxGetFileName(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),_MAX_FNAME);
	//strResult.ReleaseBuffer();
	return strResult;
}
string MightyFile::GetFileTitle() const
{
	//ASSERT_VALID(this);

	MightyFileStatus status;
	GetStatus(status);
	string strResult;
	strResult.reserve(_MAX_FNAME+1);
	MightyGetFileTitle(status.m_szFullName, (char *)strResult.c_str(),_MAX_FNAME);
	return strResult;
}

string MightyFile::GetFilePath() const
{
	//ASSERT_VALID(this);

	MightyFileStatus status;
	GetStatus(status);
	return status.m_szFullName;
}
ULONGLONG MightyFile::SeekToEnd()
{ 
	return Seek(0, MightyFile::end); 
}
void MightyFile::SeekToBegin()
{ 
	Seek(0, MightyFile::begin); 
}
void MightyFile::SetFilePath(LPCTSTR lpszNewName)
{
	//ASSERT_VALID(this);
	//ASSERT(AfxIsValidString(lpszNewName));
	if(lpszNewName != NULL)
		m_strFileName = lpszNewName;
	else
	{
		//AfxThrowInvalidArgException(); 
		;
	}

}
void MightyFile::Flush()
{
	//ASSERT_VALID(this);

	if (m_hFile == INVALID_HANDLE_VALUE)
		return;

	if (!::FlushFileBuffers(m_hFile))
	{
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
	}
}

void MightyFile::Close()
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	BOOL bError = FALSE;
	if (m_hFile != INVALID_HANDLE_VALUE)
		bError = !::CloseHandle(m_hFile);

	m_hFile = INVALID_HANDLE_VALUE;
	m_bCloseOnDelete = FALSE;
	//m_strFileName.Empty();
	m_strFileName.clear();

	////if (bError)
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

void MightyFile::Abort()
{
	//ASSERT_VALID(this);
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		// close but ignore errors
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
	//m_strFileName.Empty();
	m_strFileName.clear();
}

void MightyFile::LockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	ULARGE_INTEGER liPos;
	ULARGE_INTEGER liCount;

	liPos.QuadPart = dwPos;
	liCount.QuadPart = dwCount;
	if (!::LockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,liCount.HighPart))
	{
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
	}
}

void MightyFile::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	ULARGE_INTEGER liPos;
	ULARGE_INTEGER liCount;

	liPos.QuadPart = dwPos;
	liCount.QuadPart = dwCount;
	if (!::UnlockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,
		liCount.HighPart))
	{
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
	}
}

void MightyFile::SetLength(ULONGLONG dwNewLen)
{
	//ASSERT_VALID(this);
	//ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	Seek(dwNewLen, (UINT)begin);

	if (!::SetEndOfFile(m_hFile))
	{
		;
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
	}
}

ULONGLONG MightyFile::GetLength() const
{
	//ASSERT_VALID(this);

	ULARGE_INTEGER liSize;
	liSize.LowPart = ::GetFileSize(m_hFile, &liSize.HighPart);
	if (liSize.LowPart == (DWORD)-1)
	{
		//if (::GetLastError() != NO_ERROR)
			//MightyFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
	}

	return liSize.QuadPart;
}

// MightyFile does not support direct buffering (CMemFile does)
UINT MightyFile::GetBufferPtr(UINT nCommand, UINT /*nCount*/,
							  void** /*ppBufStart*/, void** /*ppBufMax*/)
{
	//ASSERT(nCommand == bufferCheck);
	//UNUSED(nCommand);    // not used in retail build

	return 0;   // no support
}

void PASCAL MightyFile::Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
{
	if (!::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName))
	{
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), lpszOldName);
	}
}

void PASCAL MightyFile::Remove(LPCTSTR lpszFileName)
{
	if (!::DeleteFile((LPTSTR)lpszFileName))
	{
		//MightyFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
	}
}
BOOL PASCAL MightyFile::GetStatus(LPCTSTR lpszFileName, MightyFileStatus& rStatus)
{
	//ASSERT( lpszFileName != NULL );

	if ( lpszFileName == NULL ) 
	{
		return FALSE;
	}

	if ( lstrlen(lpszFileName) >= _MAX_PATH )
	{
		//ASSERT(FALSE); // MFC requires paths with length < _MAX_PATH
		return FALSE;
	}

	// attempt to fully qualify path first
	if (!MightyFullPath(rStatus.m_szFullName, lpszFileName))
	{
		rStatus.m_szFullName[0] = '\0';
		return FALSE;
	}

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((LPTSTR)lpszFileName, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	//VERIFY(FindClose(hFind));
	FindClose(hFind);
	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	rStatus.m_attribute = (BYTE)
		(findFileData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

	// get just the low DWORD of the file size
	//ASSERT(findFileData.nFileSizeHigh == 0);
	rStatus.m_size = (LONG)findFileData.nFileSizeLow;

	// convert times as appropriate
	rStatus.m_ctime = MightyTime(findFileData.ftCreationTime);
	rStatus.m_atime = MightyTime(findFileData.ftLastAccessTime);
	rStatus.m_mtime = MightyTime(findFileData.ftLastWriteTime);

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

void PASCAL MightyFile::SetStatus(LPCTSTR lpszFileName, const MightyFileStatus& status)
{
	DWORD wAttr;
	FILETIME creationTime;
	FILETIME lastAccessTime;
	FILETIME lastWriteTime;
	LPFILETIME lpCreationTime = NULL;
	LPFILETIME lpLastAccessTime = NULL;
	LPFILETIME lpLastWriteTime = NULL;

	if ((wAttr = GetFileAttributes((LPTSTR)lpszFileName)) == (DWORD)-1L)
	{
		//CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
		;
	}

	if ((DWORD)status.m_attribute != wAttr && (wAttr & readOnly))
	{
		// Set file attribute, only if currently readonly.
		// This way we will be able to modify the time assuming the
		// caller changed the file from readonly.

		if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
		{
			//CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
			;
		}
	}

	// last modification time
	if (status.m_mtime.GetTime() != 0)
	{
		//AfxTimeToFileTime(status.m_mtime, &lastWriteTime);
		MightyTimeToFileTime(status.m_mtime, &lastWriteTime);
		lpLastWriteTime = &lastWriteTime;

		// last access time
		if (status.m_atime.GetTime() != 0)
		{
			MightyTimeToFileTime(status.m_atime, &lastAccessTime);
			lpLastAccessTime = &lastAccessTime;
		}

		// create time
		if (status.m_ctime.GetTime() != 0)
		{
			MightyTimeToFileTime(status.m_ctime, &creationTime);
			lpCreationTime = &creationTime;
		}

		HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			//CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
			return;
		}

		if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
		{
			//CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
		}

		if (!::CloseHandle(hFile))
		{
			//CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
			;
		}
	}

	if ((DWORD)status.m_attribute != wAttr && !(wAttr & readOnly))
	{
		if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
		{
			//CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
			;
		}
	}
}
UINT __stdcall MightyGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
	//ASSERT(lpszTitle == NULL ||AfxIsValidAddress(lpszTitle, _MAX_FNAME));
	//ASSERT(AfxIsValidString(lpszPathName));

	// always capture the complete file name including extension (if present)
	LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

	// lpszTitle can be NULL which just returns the number of bytes
	if (lpszTitle == NULL)
		return lstrlen(lpszTemp)+1;

	// otherwise copy it into the buffer provided
	lstrcpyn(lpszTitle, lpszTemp, nMax);
	return 0;
}
void __stdcall MightyTimeToFileTime(const MightyTime& time, LPFILETIME pFileTime)
{
	//ASSERT(pFileTime != NULL);

	if (pFileTime == NULL) 
	{
		//AfxThrowInvalidArgException();
		return;
	}

	SYSTEMTIME sysTime;
	sysTime.wYear = (WORD)time.GetYear();
	sysTime.wMonth = (WORD)time.GetMonth();
	sysTime.wDay = (WORD)time.GetDay();
	sysTime.wHour = (WORD)time.GetHour();
	sysTime.wMinute = (WORD)time.GetMinute();
	sysTime.wSecond = (WORD)time.GetSecond();
	sysTime.wMilliseconds = 0;

	// convert system time to local file time
	FILETIME localTime;
	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))
	{
		//CFileException::ThrowOsError((LONG)::GetLastError());
		;
	}

	// convert local file time to UTC file time
	if (!LocalFileTimeToFileTime(&localTime, pFileTime))
	{
		//CFileException::ThrowOsError((LONG)::GetLastError());
		;
	}
}
// turn a file, relative path or other into an absolute path
BOOL __stdcall MightyFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
// lpszPathOut = buffer of _MAX_PATH
// lpszFileIn = file, relative path or absolute path
// (both in ANSI character set)
{
	//ASSERT(AfxIsValidAddress(lpszPathOut, _MAX_PATH));

	// first, fully qualify the path name
	LPTSTR lpszFilePart;
	DWORD dwRet = GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);
	if (dwRet == 0)
	{
/*#ifdef _DEBUG
		if (lpszFileIn[0] != '\0')
			TRACE(traceAppMsg, 0, _T("Warning: could not parse the path '%s'.\n"), lpszFileIn);
#endif*/
		lstrcpyn(lpszPathOut, lpszFileIn, _MAX_PATH); // take it literally
		return FALSE;
	}
	else if (dwRet >= _MAX_PATH)
	{
/*#ifdef _DEBUG
		if (lpszFileIn[0] != '\0')
			TRACE1("Warning: could not parse the path '%s'. Path is too long.\n", lpszFileIn);
#endif*/
		return FALSE; // long path won't fit in buffer
	}

	string strRoot;
	// determine the root name of the volume
	MightyGetRoot(lpszPathOut, strRoot);

	if (!::PathIsUNC(strRoot.c_str()))
	{
		// get file system information for the volume
		DWORD dwFlags, dwDummy;
		if (!GetVolumeInformation(strRoot.c_str(), NULL, 0, NULL, &dwDummy, &dwFlags,
			NULL, 0))
		{
			//TRACE(traceAppMsg, 0, _T("Warning: could not get volume information '%s'.\n"),(LPCTSTR)strRoot.c_str());
			return FALSE;   // preserving case may not be correct
		}

		// not all characters have complete uppercase/lowercase
		if (!(dwFlags & FS_CASE_IS_PRESERVED))
			CharUpper(lpszPathOut);

		// assume non-UNICODE file systems, use OEM character set
		if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
		{
			WIN32_FIND_DATA data;
			HANDLE h = FindFirstFile(lpszFileIn, &data);
			if (h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);
				if(lpszFilePart != NULL && lpszFilePart > lpszPathOut)
				{
					int nFileNameLen = lstrlen(data.cFileName);
					int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
					if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
						lstrcpy(lpszFilePart, data.cFileName);
					else
						return FALSE; // Path doesn't fit in the buffer.
				}
				else
					return FALSE;
			}
		}
	}

	return TRUE;
}

void __stdcall MightyGetRoot(LPCTSTR lpszPath, string& strRoot)
{
	//ASSERT(lpszPath != NULL);
	strRoot.reserve(_MAX_PATH);
	LPTSTR lpszRoot = (char *)strRoot.c_str();
	memset(lpszRoot, 0, _MAX_PATH);
	lstrcpyn(lpszRoot, lpszPath, _MAX_PATH);
	PathStripToRoot(lpszRoot);
	//strRoot.ReleaseBuffer();
}

BOOL __stdcall MightyComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
{
	// use case insensitive compare as a starter
	if (lstrcmpi(lpszPath1, lpszPath2) != 0)
		return FALSE;

	// on non-DBCS systems, we are done
	if (!GetSystemMetrics(SM_DBCSENABLED))
		return TRUE;

	// on DBCS systems, the file name may not actually be the same
	// in particular, the file system is case sensitive with respect to
	// "full width" roman characters.
	// (ie. fullwidth-R is different from fullwidth-r).
	int nLen = lstrlen(lpszPath1);
	if (nLen != lstrlen(lpszPath2))
		return FALSE;
	//ASSERT(nLen < _MAX_PATH);

	// need to get both CT_CTYPE1 and CT_CTYPE3 for each filename
	LCID lcid = GetThreadLocale();
	WORD aCharType11[_MAX_PATH];
	//VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath1, -1, aCharType11));
	GetStringTypeEx(lcid, CT_CTYPE1, lpszPath1, -1, aCharType11);
	WORD aCharType13[_MAX_PATH];
	//VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath1, -1, aCharType13));
	GetStringTypeEx(lcid, CT_CTYPE3, lpszPath1, -1, aCharType13);
	WORD aCharType21[_MAX_PATH];
	//VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath2, -1, aCharType21));
	GetStringTypeEx(lcid, CT_CTYPE1, lpszPath2, -1, aCharType21);
/*#ifdef _DEBUG
	WORD aCharType23[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath2, -1, aCharType23));
#endif*/

	// for every C3_FULLWIDTH character, make sure it has same C1 value
	int i = 0;
	for (LPCTSTR lpsz = lpszPath1; *lpsz != 0; lpsz = _tcsinc(lpsz))
	{
		// check for C3_FULLWIDTH characters only
		if (aCharType13[i] & C3_FULLWIDTH)
		{
/*#ifdef _DEBUG
			ASSERT(aCharType23[i] & C3_FULLWIDTH); // should always match!
#endif*/

			// if CT_CTYPE1 is different then file system considers these
			// file names different.
			if (aCharType11[i] != aCharType21[i])
				return FALSE;
		}
		++i; // look at next character type
	}
	return TRUE; // otherwise file name is truly the same
}

UINT __stdcall MightyGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
	///ASSERT(lpszTitle == NULL ||AfxIsValidAddress(lpszTitle, _MAX_FNAME));
	//ASSERT(AfxIsValidString(lpszPathName));

	// use a temporary to avoid bugs in ::GetFileTitle when lpszTitle is NULL
	TCHAR szTemp[_MAX_PATH];
	LPTSTR lpszTemp = lpszTitle;
	if (lpszTemp == NULL)
	{
		lpszTemp = szTemp;
		nMax = _countof(szTemp);
	}
	if (::GetFileTitle(lpszPathName, lpszTemp, (WORD)nMax) != 0)
	{
		// when ::GetFileTitle fails, use cheap imitation
		//return MightyGetFileName(lpszPathName, lpszTitle, nMax);
		LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

		// lpszTitle can be NULL which just returns the number of bytes
		if (lpszTitle == NULL)
			return lstrlen(lpszTemp)+1;

		// otherwise copy it into the buffer provided
		lstrcpyn(lpszTitle, lpszTemp, nMax);
		return 0;
	}
	return lpszTitle == NULL ? lstrlen(lpszTemp)+1 : 0;
}

void __stdcall MightyGetModuleShortFileName(HINSTANCE hInst, string& strShortName)
{
	TCHAR szLongPathName[_MAX_PATH];
	strShortName.reserve(_MAX_PATH);
	::GetModuleFileName(hInst, szLongPathName, _MAX_PATH);
	if (::GetShortPathName(szLongPathName,(char *)strShortName.c_str(), _MAX_PATH) == 0)
	{
		// rare failure case (especially on not-so-modern file systems)
		strShortName = szLongPathName;
	}
	//strShortName.ReleaseBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// MightyFile implementation helpers



HRESULT Mighty_COM::CreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
								REFIID riid, LPVOID* ppv)
{
	// find the object's class factory
	LPCLASSFACTORY pf = NULL;

	if (ppv == NULL)
		return E_INVALIDARG;
	*ppv = NULL;

	HRESULT hRes = GetClassObject(rclsid, IID_IClassFactory, (LPVOID*)&pf);
	if (FAILED(hRes))
		return hRes;

	if (pf == NULL)
		return E_POINTER;

	// call it to create the instance	
	hRes = pf->CreateInstance(pUnkOuter, riid, ppv);

	// let go of the factory
	pf->Release();
	return hRes;
}

HRESULT Mighty_COM::GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	*ppv = NULL;
	HINSTANCE hInst = NULL;

	// find server name for this class ID

	string strCLSID = MightyStringFromCLSID(rclsid);
	string strServer;
	if (!MightyGetInProcServer(strCLSID.c_str(), strServer))
		return REGDB_E_CLASSNOTREG;

	// try to load it
	hInst = LoadLibrary(strServer.c_str());
	if (hInst == NULL)
		return REGDB_E_CLASSNOTREG;

#pragma warning(disable:4191)
	// get its entry point
	HRESULT (STDAPICALLTYPE* pfn)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
	pfn = (HRESULT (STDAPICALLTYPE*)(REFCLSID rclsid, REFIID riid, LPVOID* ppv))
		GetProcAddress(hInst, "DllGetClassObject");
#pragma warning(default:4191)

	// call it, if it worked
	if (pfn != NULL)
		return pfn(rclsid, riid, ppv);
	return CO_E_ERRORINDLL;
}

string MightyStringFromCLSID(REFCLSID rclsid)
{
	TCHAR szCLSID[256];
	wsprintf(szCLSID, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
		rclsid.Data1, rclsid.Data2, rclsid.Data3,
		rclsid.Data4[0], rclsid.Data4[1], rclsid.Data4[2], rclsid.Data4[3],
		rclsid.Data4[4], rclsid.Data4[5], rclsid.Data4[6], rclsid.Data4[7]);
	return szCLSID;
}

BOOL  MightyGetInProcServer(LPCTSTR lpszCLSID, string& str)
{
	HKEY hKey = NULL;
	BOOL b = FALSE;
	str.reserve(_MAX_PATH);
	LPTSTR lpsz =(char *) str.c_str();
	DWORD dwSize = _MAX_PATH * sizeof(TCHAR);
	DWORD dwType = REG_NONE;
	LONG lRes = ~ERROR_SUCCESS;

	if (RegOpenKey(HKEY_CLASSES_ROOT, _T("CLSID"), &hKey) == ERROR_SUCCESS)
	{
		HKEY hKeyCLSID = NULL;
		if (RegOpenKey(hKey, lpszCLSID, &hKeyCLSID) == ERROR_SUCCESS)
		{
			HKEY hKeyInProc = NULL;
			if (RegOpenKey(hKeyCLSID, _T("InProcServer32"), &hKeyInProc) ==
				ERROR_SUCCESS)
			{
				lRes = ::RegQueryValueEx(hKeyInProc, _T(""),
					NULL, &dwType, (BYTE*)lpsz, &dwSize);
				b = (lRes == ERROR_SUCCESS);
				RegCloseKey(hKeyInProc);
			}
			RegCloseKey(hKeyCLSID);
		}
		RegCloseKey(hKey);
	}
	//str.ReleaseBuffer();
	return b;
}





////////////////////

MightyStdioFile::MightyStdioFile()
{
	m_pStream = NULL;
}

MightyStdioFile::MightyStdioFile(FILE* pOpenStream) : MightyFile(hFileNull)
{
	//ASSERT(pOpenStream != NULL);

	if (pOpenStream == NULL)
	{
		//AfxThrowInvalidArgException();
		;
	}

	m_pStream = pOpenStream;
	m_hFile = (HANDLE) _get_osfhandle(_fileno(pOpenStream));
	//ASSERT(!m_bCloseOnDelete);
}

MightyStdioFile::MightyStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	//ASSERT(lpszFileName != NULL);
	//ASSERT(AfxIsValidString(lpszFileName));

	if (lpszFileName == NULL)
	{
		//AfxThrowInvalidArgException();
		return;
	}

	//CFileException e;
	if (!Open(lpszFileName, nOpenFlags))
	{
		//AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
		;
	}
}

MightyStdioFile::~MightyStdioFile()
{
	//ASSERT_VALID(this);

	if (m_pStream != NULL && m_bCloseOnDelete)
		Close();
}

BOOL MightyStdioFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	//ASSERT(pException == NULL || AfxIsValidAddress(pException, sizeof(CFileException)));
	//ASSERT(lpszFileName != NULL);
	//ASSERT(AfxIsValidString(lpszFileName));

	if (lpszFileName == NULL)
	{
		return FALSE;
	}

	m_pStream = NULL;
	if (!MightyFile::Open(lpszFileName, (nOpenFlags & ~typeText)))
		return FALSE;

	//ASSERT(m_hFile != hFileNull);
	//ASSERT(m_bCloseOnDelete);

	char szMode[4]; // C-runtime open string
	int nMode = 0;

	// determine read/write mode depending on CFile mode
	if (nOpenFlags & modeCreate)
	{
		if (nOpenFlags & modeNoTruncate)
			szMode[nMode++] = 'a';
		else
			szMode[nMode++] = 'w';
	}
	else if (nOpenFlags & modeWrite)
		szMode[nMode++] = 'a';
	else
		szMode[nMode++] = 'r';

	// add '+' if necessary (when read/write modes mismatched)
	if (szMode[0] == 'r' && (nOpenFlags & modeReadWrite) ||
		szMode[0] != 'r' && !(nOpenFlags & modeWrite))
	{
		// current szMode mismatched, need to add '+' to fix
		szMode[nMode++] = '+';
	}

	// will be inverted if not necessary
	int nFlags = _O_RDONLY|_O_TEXT;
	if (nOpenFlags & (modeWrite|modeReadWrite))
		nFlags ^= _O_RDONLY;

	if (nOpenFlags & typeBinary)
		szMode[nMode++] = 'b', nFlags ^= _O_TEXT;
	else
		szMode[nMode++] = 't';
	szMode[nMode++] = '\0';

	// open a C-runtime low-level file handle
	int nHandle = _open_osfhandle((UINT_PTR) m_hFile, nFlags);

	// open a C-runtime stream from that handle
	if (nHandle != -1)
		m_pStream = _fdopen(nHandle, szMode);

	if (m_pStream == NULL)
	{
		// an error somewhere along the way...
		/*if (pException != NULL)
		{
			pException->m_lOsError = _doserrno;
			pException->m_cause = CFileException::OsErrorToException(_doserrno);
		}*/

		MightyFile::Abort(); // close m_hFile
		return FALSE;
	}

	return TRUE;
}

UINT MightyStdioFile::Read(void* lpBuf, UINT nCount)
{
	//ASSERT_VALID(this);
	//ASSERT(m_pStream != NULL);

	if (nCount == 0)
		return 0;   // avoid Win32 "null-read"

	//ASSERT(AfxIsValidAddress(lpBuf, nCount));

	if (lpBuf == NULL)
	{
		//AfxThrowInvalidArgException();
		;
	}

	UINT nRead = 0;

	if ((nRead = (UINT)fread(lpBuf, sizeof(BYTE), nCount, m_pStream)) == 0 && !feof(m_pStream))
	{
		//AfxThrowFileException(CFileException::generic, _doserrno, m_strFileName);
		;
	}
	if (ferror(m_pStream))
	{
		clearerr(m_pStream);
		//AfxThrowFileException(CFileException::generic, _doserrno, m_strFileName);
	}
	return nRead;
}

void MightyStdioFile::Write(const void* lpBuf, UINT nCount)
{
	//ASSERT_VALID(this);
	//ASSERT(m_pStream != NULL);
	//ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

	if (lpBuf == NULL)
	{
		//AfxThrowInvalidArgException();
		return;
	}

	if (fwrite(lpBuf, sizeof(BYTE), nCount, m_pStream) != nCount)
	{
		//AfxThrowFileException(CFileException::generic, _doserrno, m_strFileName);
		;
	}
}

void MightyStdioFile::WriteString(LPCTSTR lpsz)
{
	//ASSERT(lpsz != NULL);
	//ASSERT(m_pStream != NULL);

	if (lpsz == NULL)
	{
		//AfxThrowInvalidArgException();
		return;
	}

	if (_fputts(lpsz, m_pStream) == _TEOF)
	{
		//AfxThrowFileException(CFileException::diskFull, _doserrno, m_strFileName);
		;
	}
}

LPTSTR MightyStdioFile::ReadString(LPTSTR lpsz, UINT nMax)
{
	//ASSERT(lpsz != NULL);
	//ASSERT(AfxIsValidAddress(lpsz, nMax));
	//ASSERT(m_pStream != NULL);

	if (lpsz == NULL)
	{
		//AfxThrowInvalidArgException();
		return "";
	}

	LPTSTR lpszResult = _fgetts(lpsz, nMax, m_pStream);
	if (lpszResult == NULL && !feof(m_pStream))
	{
		clearerr(m_pStream);
		//AfxThrowFileException(CFileException::generic, _doserrno, m_strFileName);
	}
	return lpszResult;
}

BOOL MightyStdioFile::ReadString(string& rString)
{//本函数需要慎重考虑
	//ASSERT_VALID(this);

	rString = _T("");    // empty string without deallocating
	const int nMaxSize =512; //128;
	char bufRead_[nMaxSize+1]={0};
	rString.reserve(nMaxSize+1);
	LPTSTR lpsz =bufRead_;//(char *)rString.c_str();
	LPTSTR lpszResult;
	string tempStrVal="";
	int nLen = 0;
	for (;;)
	{
		lpszResult = _fgetts(lpsz, nMaxSize+1, m_pStream);
		//rString.ReleaseBuffer();

		// handle error/eof case
		if (lpszResult == NULL && !feof(m_pStream))
		{
			clearerr(m_pStream);
			//AfxThrowFileException(CFileException::generic, _doserrno,m_strFileName);
			return FALSE;
		}

		// if string is read completely or EOF
		if (lpszResult == NULL ||(nLen = (int)lstrlen(lpsz)) < nMaxSize ||lpsz[nLen-1] == '\n')
		{
			tempStrVal=lpsz;
			rString.append(tempStrVal.c_str(),tempStrVal.length());
			break;
		}

		//nLen = rString.length();
		//rString.reserve(nMaxSize + nLen);
		//lpsz =(char *)rString.c_str()+nLen;
		tempStrVal=lpsz;
		rString.append(tempStrVal.c_str(),tempStrVal.length());


		//lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	//lpsz = rString.GetBuffer(0);
	//nLen = rString.GetLength();
	lpsz =(char *)rString.c_str();
	nLen = rString.length();
	if (nLen != 0 && lpsz[nLen-1] == '\n');
		//rString.GetBufferSetLength(nLen-1);

	return nLen != 0;
}

ULONGLONG MightyStdioFile::Seek(LONGLONG lOff, UINT nFrom)
{
	//ASSERT_VALID(this);
	//ASSERT(nFrom == begin || nFrom == end || nFrom == current);
	//ASSERT(m_pStream != NULL);

	LONG lOff32;

	if ((lOff < LONG_MIN) || (lOff > LONG_MAX))
	{
		//AfxThrowFileException(CFileException::badSeek, -1, m_strFileName);
		;
	}

	lOff32 = (LONG)lOff;
	if (fseek(m_pStream, lOff32, nFrom) != 0)
	{
		//AfxThrowFileException(CFileException::badSeek, _doserrno,m_strFileName);
		;
	}

	long pos = ftell(m_pStream);
	return pos;
}

ULONGLONG MightyStdioFile::GetLength() const
{
	//ASSERT_VALID(this);

	LONG nCurrent;
	LONG nLength;
	LONG nResult;

	nCurrent = ftell(m_pStream);
	if (nCurrent == -1)
	{
		//AfxThrowFileException(CFileException::invalidFile, _doserrno,m_strFileName);
		return 0;
	}

	nResult = fseek(m_pStream, 0, SEEK_END);
	if (nResult != 0)
	{
		//AfxThrowFileException(CFileException::badSeek, _doserrno,m_strFileName);
		;
	}

	nLength = ftell(m_pStream);
	if (nLength == -1)
	{
		//AfxThrowFileException(CFileException::invalidFile, _doserrno,m_strFileName);
		;
	}
	nResult = fseek(m_pStream, nCurrent, SEEK_SET);
	if (nResult != 0)
	{
		//AfxThrowFileException(CFileException::badSeek, _doserrno,m_strFileName);
		;
	}

	return nLength;
}

ULONGLONG MightyStdioFile::GetPosition() const
{
	//ASSERT_VALID(this);
	//ASSERT(m_pStream != NULL);

	long pos = ftell(m_pStream);
	if (pos == -1)
	{
		//AfxThrowFileException(CFileException::invalidFile, _doserrno,m_strFileName);
		return 0;
	}
	return pos;
}

void MightyStdioFile::Flush()
{
	//ASSERT_VALID(this);

	if (m_pStream != NULL && fflush(m_pStream) != 0)
	{
		//AfxThrowFileException(CFileException::diskFull, _doserrno,m_strFileName);
		;
	}
}

void MightyStdioFile::Close()
{
	//ASSERT_VALID(this);
	//ASSERT(m_pStream != NULL);

	int nErr = 0;

	if (m_pStream != NULL)
		nErr = fclose(m_pStream);

	m_hFile = hFileNull;
	m_bCloseOnDelete = FALSE;
	m_pStream = NULL;

	//if (nErr != 0)
		//AfxThrowFileException(CFileException::diskFull, _doserrno,m_strFileName);
}

void MightyStdioFile::Abort()
{
	//ASSERT_VALID(this);

	if (m_pStream != NULL && m_bCloseOnDelete)
		fclose(m_pStream);  // close but ignore errors
	m_hFile = hFileNull;
	m_pStream = NULL;
	m_bCloseOnDelete = FALSE;
}

MightyFile* MightyStdioFile::Duplicate() const
{
	//ASSERT_VALID(this);
	//ASSERT(m_pStream != NULL);

	//AfxThrowNotSupportedException();
	return NULL;
}

void MightyStdioFile::LockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	//ASSERT_VALID(this);
	//ASSERT(m_pStream != NULL);

	//AfxThrowNotSupportedException();
}

void MightyStdioFile::UnlockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	//ASSERT_VALID(this);
	//ASSERT(m_pStream != NULL);

	//AfxThrowNotSupportedException();
}


// InternetFileFind implementation

InternetFileFind::InternetFileFind()
{
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
	m_hContext = NULL;
	m_chDirSeparator = '\\';
}

InternetFileFind::~InternetFileFind()
{
	Close();
}

void InternetFileFind::Close()
{
	if (m_pFoundInfo != NULL)
	{
		delete m_pFoundInfo;
		m_pFoundInfo = NULL;
	}

	if (m_pNextInfo != NULL)
	{
		delete m_pNextInfo;
		m_pNextInfo = NULL;
	}

	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		CloseContext();
		m_hContext = NULL;
	}
}

void InternetFileFind::CloseContext()
{
	::FindClose(m_hContext);
	return;
}

BOOL InternetFileFind::FindFile(LPCTSTR pstrName /* = NULL */,
								DWORD dwUnused /* = 0 */)
{
	//UNUSED_ALWAYS(dwUnused);
	Close();

	if (pstrName == NULL)
		pstrName = _T("*.*");
	else if (lstrlen(pstrName) >= (_countof(((WIN32_FIND_DATA*) m_pNextInfo)->cFileName)))
	{
		::SetLastError(ERROR_BAD_ARGUMENTS);
		return FALSE;		
	}

	m_pNextInfo = new WIN32_FIND_DATA;

	WIN32_FIND_DATA *pFindData = (WIN32_FIND_DATA *)m_pNextInfo;

	lstrcpy(pFindData->cFileName, pstrName);

	m_hContext = ::FindFirstFile(pstrName, (WIN32_FIND_DATA*) m_pNextInfo);

	if (m_hContext == INVALID_HANDLE_VALUE)
	{
		DWORD dwTemp = ::GetLastError();
		Close();
		::SetLastError(dwTemp);
		return FALSE;
	}
	char rootBuf_[_MAX_PATH]={0};
	m_strRoot.reserve(_MAX_PATH);
	LPTSTR pstrRoot =rootBuf_;// m_strRoot.GetBufferSetLength(_MAX_PATH);
	LPCTSTR pstr = _tfullpath(pstrRoot, pstrName, _MAX_PATH);

	// passed name isn't a valid path but was found by the API
	//ASSERT(pstr != NULL);
	if (pstr == NULL)
	{
		//m_strRoot.ReleaseBuffer(0);
		Close();
		::SetLastError(ERROR_INVALID_NAME);
		return FALSE;
	}
	else
	{
		// find the last forward or backward whack
		LPTSTR pstrBack  = _tcsrchr(pstrRoot, '\\');
		LPTSTR pstrFront = _tcsrchr(pstrRoot, '/');

		if (pstrFront != NULL || pstrBack != NULL)
		{
			if (pstrFront == NULL)
				pstrFront = pstrRoot;
			if (pstrBack == NULL)
				pstrBack = pstrRoot;

			// from the start to the last whack is the root

			if (pstrFront >= pstrBack)
				*pstrFront = '\0';
			else
				*pstrBack = '\0';
		}
		m_strRoot=rootBuf_;
		//m_strRoot.ReleaseBuffer(-1);
	}

	return TRUE;
}

BOOL InternetFileFind::MatchesMask(DWORD dwMask) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
		return (!!(((LPWIN32_FIND_DATA) m_pFoundInfo)->dwFileAttributes & dwMask));
	else
		return FALSE;
}

BOOL InternetFileFind::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT(pTimeStamp != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL InternetFileFind::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT(pTimeStamp != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL InternetFileFind::GetCreationTime(FILETIME* pTimeStamp) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL InternetFileFind::GetLastAccessTime(MightyTime& refTime) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = MightyTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL InternetFileFind::GetLastWriteTime(MightyTime& refTime) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = MightyTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL InternetFileFind::GetCreationTime(MightyTime& refTime) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = MightyTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL InternetFileFind::IsDots() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	// return TRUE if the file name is "." or ".." and
	// the file is a directory

	BOOL bResult = FALSE;
	if (m_pFoundInfo != NULL && IsDirectory())
	{
		LPWIN32_FIND_DATA pFindData = (LPWIN32_FIND_DATA) m_pFoundInfo;
		if (pFindData->cFileName[0] == '.')
		{
			if (pFindData->cFileName[1] == '\0' ||
				(pFindData->cFileName[1] == '.' &&
				pFindData->cFileName[2] == '\0'))
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

BOOL InternetFileFind::IsReadOnly() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_READONLY); 
}
BOOL InternetFileFind::IsDirectory() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_DIRECTORY); 
}
BOOL InternetFileFind::IsCompressed() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_COMPRESSED); 
}
BOOL InternetFileFind::IsSystem() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_SYSTEM); 
}
BOOL InternetFileFind::IsHidden() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_HIDDEN); 
}
BOOL InternetFileFind::IsTemporary() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_TEMPORARY); 
}
BOOL InternetFileFind::IsNormal() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_NORMAL); 
}
BOOL InternetFileFind::IsArchived() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_ARCHIVE); 
}
///
BOOL InternetFileFind::FindNextFile()
{
	//ASSERT(m_hContext != NULL);

	if (m_hContext == NULL)
		return FALSE;
	if (m_pFoundInfo == NULL)
		m_pFoundInfo = new WIN32_FIND_DATA;

	//ASSERT_VALID(this);

	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return ::FindNextFile(m_hContext, (LPWIN32_FIND_DATA) m_pNextInfo);
}

string InternetFileFind::GetFileURL() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string strResult("file://");
	strResult += GetFilePath();
	return strResult;
}

string InternetFileFind::GetRoot() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	return m_strRoot;
}

string InternetFileFind::GetFilePath() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string strResult = m_strRoot;
	LPCTSTR pszResult;
	LPCTSTR pchLast;
	pszResult = strResult.c_str();
	pchLast = _tcsdec( pszResult, pszResult+strResult.length());
	if ((*pchLast != _T('\\')) && (*pchLast != _T('/')))
		strResult += m_chDirSeparator;
	strResult += GetFileName();
	return strResult;
}

string InternetFileFind::GetFileTitle() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string strFullName = GetFileName();
	string strResult;
	strResult.reserve(_MAX_PATH+1);
	char RetBuf_[_MAX_PATH+1]={0};
	_tsplitpath(strFullName.c_str(), NULL, NULL,RetBuf_, NULL);
	strResult=RetBuf_;
	return strResult;
}

string InternetFileFind::GetFileName() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string ret;

	if (m_pFoundInfo != NULL)
		ret = ((LPWIN32_FIND_DATA) m_pFoundInfo)->cFileName;
	return ret;
}

ULONGLONG InternetFileFind::GetLength() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	ULARGE_INTEGER nFileSize;

	if (m_pFoundInfo != NULL)
	{
		nFileSize.LowPart = ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeLow;
		nFileSize.HighPart = ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeHigh;
	}
	else
	{
		nFileSize.QuadPart = 0;
	}

	return nFileSize.QuadPart;
}

MightyMemFile::MightyMemFile(UINT nGrowBytes)
{
	//ASSERT(nGrowBytes <= UINT_MAX);

	m_nGrowBytes = nGrowBytes;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	m_lpBuffer = NULL;
	m_bAutoDelete = TRUE;
}

MightyMemFile::MightyMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes)
{
	if (lpBuffer == NULL && nBufferSize != 0) 
	{
		//AfxThrowInvalidArgException();
		return;
	}

	//ASSERT(nGrowBytes <= UINT_MAX);

	m_nGrowBytes = nGrowBytes;
	m_nPosition = 0;
	m_nBufferSize = nBufferSize;
	m_nFileSize = nGrowBytes == 0 ? nBufferSize : 0;
	m_lpBuffer = lpBuffer;
	m_bAutoDelete = FALSE;
}

void MightyMemFile::Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes) 
{
	if (lpBuffer == NULL && nBufferSize != 0) 
	{
		//AfxThrowInvalidArgException();
		return;
	}

	//ASSERT(m_lpBuffer == NULL);

	m_nGrowBytes = nGrowBytes;
	m_nPosition = 0;
	m_nBufferSize = nBufferSize;
	m_nFileSize = nGrowBytes == 0 ? nBufferSize : 0;
	m_lpBuffer = lpBuffer;
	m_bAutoDelete = FALSE;
}

BYTE* MightyMemFile::Detach()
{
	BYTE* lpBuffer = m_lpBuffer;
	m_lpBuffer = NULL;
	m_nFileSize = 0;
	m_nBufferSize = 0;
	m_nPosition = 0;

	return lpBuffer;
}

MightyMemFile::~MightyMemFile()
{
	// Close should have already been called, but we check anyway
	if (m_lpBuffer)
		Close();
	//ASSERT(m_lpBuffer == NULL);

	m_nGrowBytes = 0;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
}

BYTE* MightyMemFile::Alloc(SIZE_T nBytes)
{
	return (BYTE*)malloc(nBytes);
}

BYTE* MightyMemFile::Realloc(BYTE* lpMem, SIZE_T nBytes)
{
	//ASSERT(nBytes > 0);	// nBytes == 0 means free		
	return (BYTE*)realloc(lpMem, nBytes);
}

#pragma intrinsic(memcpy)
BYTE* MightyMemFile::Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource,
							SIZE_T nBytes)
{
	//ASSERT(lpMemTarget != NULL);
	//ASSERT(lpMemSource != NULL);

	//ASSERT(AfxIsValidAddress(lpMemTarget, nBytes));
	//ASSERT(AfxIsValidAddress(lpMemSource, nBytes, FALSE));

	return (BYTE*)memcpy(lpMemTarget, lpMemSource, nBytes);
}
#pragma function(memcpy)

void MightyMemFile::Free(BYTE* lpMem)
{
	//ASSERT(lpMem != NULL);

	free(lpMem);
}

ULONGLONG MightyMemFile::GetPosition() const
{
	//ASSERT_VALID(this);
	return m_nPosition;
}

void MightyMemFile::GrowFile(SIZE_T dwNewLen)
{
	//ASSERT_VALID(this);

	if (dwNewLen > m_nBufferSize)
	{
		// grow the buffer
		SIZE_T dwNewBufferSize = m_nBufferSize;

		// watch out for buffers which cannot be grown!
		//ASSERT(m_nGrowBytes != 0);
		if (m_nGrowBytes == 0)
		{
			return;
		}

		//AfxThrowMemoryException();

		// determine new buffer size
		while (dwNewBufferSize < dwNewLen)
			dwNewBufferSize += m_nGrowBytes;

		// allocate new buffer
		BYTE* lpNew;
		if (m_lpBuffer == NULL)
			lpNew = Alloc(dwNewBufferSize);
		else
			lpNew = Realloc(m_lpBuffer, dwNewBufferSize);

		if (lpNew == NULL)
		{
			return;
		}
			//AfxThrowMemoryException();

		m_lpBuffer = lpNew;
		m_nBufferSize = dwNewBufferSize;
	}
	//ASSERT_VALID(this);
}

ULONGLONG MightyMemFile::GetLength() const
{
	//ASSERT_VALID(this);

	return m_nFileSize;
}

void MightyMemFile::SetLength(ULONGLONG dwNewLen)
{
	//ASSERT_VALID(this);

#ifdef WIN32
	if (dwNewLen > ULONG_MAX)
	{
		return;
	}
		//AfxThrowMemoryException();
#endif  // WIN32
	if (dwNewLen > m_nBufferSize)
		GrowFile((SIZE_T)dwNewLen);

	if (dwNewLen < m_nPosition)
		m_nPosition = (SIZE_T)dwNewLen;

	m_nFileSize = (SIZE_T)dwNewLen;
	//ASSERT_VALID(this);
}

UINT MightyMemFile::Read(void* lpBuf, UINT nCount)
{
	//ASSERT_VALID(this);

	if (nCount == 0)
		return 0;

	//ASSERT(lpBuf != NULL);

	if (lpBuf == NULL) 
	{
		//AfxThrowInvalidArgException();
		return 0;
	}

	//ASSERT(AfxIsValidAddress(lpBuf, nCount));

	if (m_nPosition > m_nFileSize)
		return 0;

	UINT nRead;
	if (m_nPosition + nCount > m_nFileSize)
		nRead = (UINT)(m_nFileSize - m_nPosition);
	else
		nRead = nCount;

	Memcpy((BYTE*)lpBuf, (BYTE*)m_lpBuffer + m_nPosition, nRead);
	m_nPosition += nRead;

	//ASSERT_VALID(this);

	return nRead;
}

void MightyMemFile::Write(const void* lpBuf, UINT nCount)
{
	//ASSERT_VALID(this);

	if (nCount == 0)
		return;

	//ASSERT(lpBuf != NULL);
	//ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

	if (lpBuf == NULL) 
	{
		//AfxThrowInvalidArgException();
		return;
	}

	if (m_nPosition + nCount > m_nBufferSize)
		GrowFile(m_nPosition + nCount);

	//ASSERT(m_nPosition + nCount <= m_nBufferSize);

	Memcpy((BYTE*)m_lpBuffer + m_nPosition, (BYTE*)lpBuf, nCount);

	m_nPosition += nCount;

	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;

	//ASSERT_VALID(this);
}

ULONGLONG MightyMemFile::Seek(LONGLONG lOff, UINT nFrom)
{
	//ASSERT_VALID(this);
	//ASSERT(nFrom == begin || nFrom == end || nFrom == current);

	LONGLONG lNewPos = m_nPosition;

	if (nFrom == begin)
		lNewPos = lOff;
	else if (nFrom == current)
		lNewPos += lOff;
	else if (nFrom == end) {
		if (lOff > 0) 
		{
			return 0;
			//AfxThrowFileException(CFileException::badSeek);// offsets must be negative when 

			//seeking from the end
		}
		lNewPos = m_nFileSize + lOff;	
	} else
		return m_nPosition;

	if (lNewPos < 0)
	{
		return 0;
	}
		//AfxThrowFileException(CFileException::badSeek);
	if (static_cast<DWORD>(lNewPos) > m_nFileSize)
		GrowFile((SIZE_T)lNewPos);

	m_nPosition = (SIZE_T)lNewPos;

	//ASSERT_VALID(this);
	return m_nPosition;
}

void MightyMemFile::Flush()
{
	//ASSERT_VALID(this);
}

void MightyMemFile::Close()
{
	//ASSERT((m_lpBuffer == NULL && m_nBufferSize == 0) ||
		//!m_bAutoDelete || AfxIsValidAddress(m_lpBuffer, (UINT)m_nBufferSize, FALSE));
	//ASSERT(m_nFileSize <= m_nBufferSize);

	m_nGrowBytes = 0;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	if (m_lpBuffer && m_bAutoDelete)
		Free(m_lpBuffer);
	m_lpBuffer = NULL;
}

void MightyMemFile::Abort()
{
	//ASSERT_VALID(this);

	Close();
}

void MightyMemFile::LockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	//ASSERT_VALID(this);
	//AfxThrowNotSupportedException();
}


void MightyMemFile::UnlockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	//ASSERT_VALID(this);
	//AfxThrowNotSupportedException();
}

MightyFile* MightyMemFile::Duplicate() const
{
	//ASSERT_VALID(this);
	//AfxThrowNotSupportedException();
	return NULL;
}

// only MightyMemFile supports "direct buffering" interaction with CArchive
UINT MightyMemFile::GetBufferPtr(UINT nCommand, UINT nCount,
								 void** ppBufStart, void**ppBufMax)
{
	//ASSERT(nCommand == bufferCheck || nCommand == bufferCommit ||nCommand == bufferRead || nCommand == bufferWrite);



	if (nCommand == bufferCheck)
	{
		// only allow direct buffering if we're 
		// growable
		if (m_nGrowBytes > 0)
			return bufferDirect;
		else
			return 0;
	}

	if (nCommand == bufferCommit)
	{
		// commit buffer
		//ASSERT(ppBufStart == NULL);
		//ASSERT(ppBufMax == NULL);
		m_nPosition += nCount;
		if (m_nPosition > m_nFileSize)
			m_nFileSize = m_nPosition;
		return 0;
	}


	//ASSERT(nCommand == bufferWrite || nCommand == bufferRead);
	//ASSERT(ppBufStart != NULL);
	//ASSERT(ppBufMax != NULL);

	if (ppBufStart == NULL || ppBufMax == NULL) 
	{
		return 0;
	}

	// when storing, grow file as necessary to satisfy buffer request
	if (nCommand == bufferWrite && m_nPosition + nCount > m_nBufferSize)
		GrowFile(m_nPosition + nCount);

	// store buffer max and min
	*ppBufStart = m_lpBuffer + m_nPosition;

	// end of buffer depends on whether you are reading or writing
	if (nCommand == bufferWrite)
		*ppBufMax = m_lpBuffer + min(m_nBufferSize, m_nPosition + nCount);
	else
	{
		if (nCount == (UINT)-1)
			nCount = UINT(m_nBufferSize - m_nPosition);
		*ppBufMax = m_lpBuffer + min(m_nFileSize, m_nPosition + nCount);
		m_nPosition += LPBYTE(*ppBufMax) - LPBYTE(*ppBufStart);
	}

	// return number of bytes in returned buffer space (may be <= nCount)
	return ULONG(LPBYTE(*ppBufMax) - LPBYTE(*ppBufStart));
}


//////////
MightyFileFind::MightyFileFind()
{
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
	m_hContext = NULL;
	m_chDirSeparator = '\\';
}

MightyFileFind::~MightyFileFind()
{
	Close();
}

void MightyFileFind::Close()
{
	if (m_pFoundInfo != NULL)
	{
		delete m_pFoundInfo;
		m_pFoundInfo = NULL;
	}

	if (m_pNextInfo != NULL)
	{
		delete m_pNextInfo;
		m_pNextInfo = NULL;
	}

	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		CloseContext();
		m_hContext = NULL;
	}
}

void MightyFileFind::CloseContext()
{
	::FindClose(m_hContext);
	return;
}

BOOL MightyFileFind::FindFile(LPCTSTR pstrName /* = NULL */,
							  DWORD dwUnused /* = 0 */)
{
	//UNUSED_ALWAYS(dwUnused);
	Close();

	if (pstrName == NULL)
		pstrName = _T("*.*");
	else if (lstrlen(pstrName) >= (_countof(((WIN32_FIND_DATA*) m_pNextInfo)->cFileName)))
	{
		::SetLastError(ERROR_BAD_ARGUMENTS);
		return FALSE;		
	}

	m_pNextInfo = new WIN32_FIND_DATA;

	WIN32_FIND_DATA *pFindData = (WIN32_FIND_DATA *)m_pNextInfo;

	lstrcpy(pFindData->cFileName, pstrName);

	m_hContext = ::FindFirstFile(pstrName, (WIN32_FIND_DATA*) m_pNextInfo);

	if (m_hContext == INVALID_HANDLE_VALUE)
	{
		DWORD dwTemp = ::GetLastError();
		Close();
		::SetLastError(dwTemp);
		return FALSE;
	}
	char rootbuf_[_MAX_PATH]={0};
	LPTSTR pstrRoot =rootbuf_;// m_strRoot.GetBufferSetLength(_MAX_PATH);
	LPCTSTR pstr = _tfullpath(pstrRoot, pstrName, _MAX_PATH);

	// passed name isn't a valid path but was found by the API
	//ASSERT(pstr != NULL);
	if (pstr == NULL)
	{
		//m_strRoot.ReleaseBuffer(0);
		Close();
		::SetLastError(ERROR_INVALID_NAME);
		return FALSE;
	}
	else
	{
		// find the last forward or backward whack
		LPTSTR pstrBack  = _tcsrchr(pstrRoot, '\\');
		LPTSTR pstrFront = _tcsrchr(pstrRoot, '/');

		if (pstrFront != NULL || pstrBack != NULL)
		{
			if (pstrFront == NULL)
				pstrFront = pstrRoot;
			if (pstrBack == NULL)
				pstrBack = pstrRoot;

			// from the start to the last whack is the root

			if (pstrFront >= pstrBack)
				*pstrFront = '\0';
			else
				*pstrBack = '\0';
		}
		//m_strRoot.ReleaseBuffer(-1);
		m_strRoot=rootbuf_;
	}

	return TRUE;
}

BOOL MightyFileFind::MatchesMask(DWORD dwMask) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
		return (!!(((LPWIN32_FIND_DATA) m_pFoundInfo)->dwFileAttributes & dwMask));
	else
		return FALSE;
}

BOOL MightyFileFind::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT(pTimeStamp != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL MightyFileFind::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT(pTimeStamp != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL MightyFileFind::GetCreationTime(FILETIME* pTimeStamp) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL MightyFileFind::GetLastAccessTime(MightyTime& refTime) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = MightyTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL MightyFileFind::GetLastWriteTime(MightyTime& refTime) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = MightyTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL MightyFileFind::GetCreationTime(MightyTime& refTime) const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = MightyTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL MightyFileFind::IsDots() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	// return TRUE if the file name is "." or ".." and
	// the file is a directory

	BOOL bResult = FALSE;
	if (m_pFoundInfo != NULL && IsDirectory())
	{
		LPWIN32_FIND_DATA pFindData = (LPWIN32_FIND_DATA) m_pFoundInfo;
		if (pFindData->cFileName[0] == '.')
		{
			if (pFindData->cFileName[1] == '\0' ||
				(pFindData->cFileName[1] == '.' &&
				pFindData->cFileName[2] == '\0'))
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

BOOL MightyFileFind::FindNextFile()
{
	//ASSERT(m_hContext != NULL);

	if (m_hContext == NULL)
		return FALSE;
	if (m_pFoundInfo == NULL)
		m_pFoundInfo = new WIN32_FIND_DATA;

	//ASSERT_VALID(this);

	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return ::FindNextFile(m_hContext, (LPWIN32_FIND_DATA) m_pNextInfo);
}

string MightyFileFind::GetFileURL() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string strResult("file://");
	strResult += GetFilePath();
	return strResult;
}

string MightyFileFind::GetRoot() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	return m_strRoot;
}

string MightyFileFind::GetFilePath() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string strResult = m_strRoot;
	LPCTSTR pszResult;
	LPCTSTR pchLast;
	pszResult = strResult.c_str();
	pchLast = _tcsdec( pszResult, pszResult+strResult.length());
	if ((*pchLast != _T('\\')) && (*pchLast != _T('/')))
		strResult += m_chDirSeparator;
	strResult += GetFileName();
	return strResult;
}

string MightyFileFind::GetFileTitle() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string strFullName = GetFileName();
	char namebuf_[512]={0};
	string strResult;

	//_tsplitpath(strFullName, NULL, NULL, strResult.GetBuffer(_MAX_PATH), NULL);
	_tsplitpath(strFullName.c_str(), NULL, NULL,namebuf_, NULL);
	//strResult.ReleaseBuffer();
	strResult=namebuf_;
	return strResult;
}

string MightyFileFind::GetFileName() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	string ret;

	if (m_pFoundInfo != NULL)
		ret = ((LPWIN32_FIND_DATA) m_pFoundInfo)->cFileName;
	return ret;
}

ULONGLONG MightyFileFind::GetLength() const
{
	//ASSERT(m_hContext != NULL);
	//ASSERT_VALID(this);

	ULARGE_INTEGER nFileSize;

	if (m_pFoundInfo != NULL)
	{
		nFileSize.LowPart = ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeLow;
		nFileSize.HighPart = ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeHigh;
	}
	else
	{
		nFileSize.QuadPart = 0;
	}

	return nFileSize.QuadPart;
}


BOOL MightyFileFind::IsReadOnly() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_READONLY); 
}
BOOL MightyFileFind::IsDirectory() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_DIRECTORY); 
}
BOOL MightyFileFind::IsCompressed() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_COMPRESSED); 
}
BOOL MightyFileFind::IsSystem() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_SYSTEM); 
}
BOOL MightyFileFind::IsHidden() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_HIDDEN); 
}
BOOL MightyFileFind::IsTemporary() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_TEMPORARY); 
}
BOOL MightyFileFind::IsNormal() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_NORMAL); 
}
BOOL MightyFileFind::IsArchived() const
{ 
	return MatchesMask(FILE_ATTRIBUTE_ARCHIVE); 
}
DWORD MightyFileFind::GetFileAttributes() const
{
	if (m_pFoundInfo != NULL)
		return ((LPWIN32_FIND_DATA) m_pFoundInfo)->dwFileAttributes;
	else
		return 0;

}