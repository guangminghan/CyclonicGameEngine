#pragma once
//#include "./MightyCommon.h"
#include "../CommonInc/common.h"
#include "MightyTime.h"
typedef struct MightyFileStatus
{
	MightyTime m_ctime;       
	MightyTime m_mtime;
	MightyTime m_atime;
	ULONGLONG m_size;
	BYTE m_attribute;
	BYTE _m_padding;
	TCHAR m_szFullName[_MAX_PATH];
}st_file_status;
void __stdcall MightyTimeToFileTime(const MightyTime& time, LPFILETIME pFileTime);
BOOL __stdcall MightyFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
void __stdcall MightyGetRoot(LPCTSTR lpszPath, string& strRoot);
UINT __stdcall MightyGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
UINT __stdcall MightyGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
string MightyStringFromCLSID(REFCLSID rclsid);
BOOL  MightyGetInProcServer(LPCTSTR lpszCLSID, string& str);
class Mighty_COM
{
public:
	HRESULT CreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
		REFIID riid, LPVOID* ppv);
	HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
};
class MightyFile
{
public:
	// Flag values
	enum OpenFlags {
		modeRead =         (int) 0x00000,
		modeWrite =        (int) 0x00001,
		modeReadWrite =    (int) 0x00002,
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000,
		modeNoTruncate =   (int) 0x02000,
		typeText =         (int) 0x04000, // typeText and typeBinary are
		typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000,
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
	};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
	};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	static const HANDLE hFileNull;

	// Constructors
	MightyFile();
	MightyFile(HANDLE hFile);
	MightyFile(LPCTSTR lpszFileName, UINT nOpenFlags);

	// Attributes
	HANDLE m_hFile;
	operator HANDLE() const;

	virtual ULONGLONG GetPosition() const;
	BOOL GetStatus(MightyFileStatus& rStatus) const;
	virtual string GetFileName() const;
	virtual string GetFileTitle() const;
	virtual string GetFilePath() const;
	virtual void SetFilePath(LPCTSTR lpszNewName);

	// Operations
	//virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,MightyFileException* pError = NULL);
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
	static void PASCAL Rename(LPCTSTR lpszOldName,LPCTSTR lpszNewName);
	static void PASCAL Remove(LPCTSTR lpszFileName);
	static BOOL PASCAL GetStatus(LPCTSTR lpszFileName,MightyFileStatus& rStatus);
	static void PASCAL SetStatus(LPCTSTR lpszFileName,const MightyFileStatus& status);

	ULONGLONG SeekToEnd();
	void SeekToBegin();

	// Overridables
	virtual MightyFile* Duplicate() const;

	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual void SetLength(ULONGLONG dwNewLen);
	virtual ULONGLONG GetLength() const;

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	// Implementation
public:
	virtual ~MightyFile();
/*#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif*/
	enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };
	enum BufferFlags 
	{ 
		bufferDirect = 0x01,
		bufferBlocking = 0x02
	};
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,void** ppBufStart = NULL, void** ppBufMax = NULL);

protected:
	BOOL m_bCloseOnDelete;
	string m_strFileName;
};
class MightyStdioFile : public MightyFile
{
public:
	// Constructors
	MightyStdioFile();
	MightyStdioFile(FILE* pOpenStream);
	MightyStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags);

	// Attributes
	FILE* m_pStream;    // stdio FILE
	// m_hFile from base class is _fileno(m_pStream)

	// Operations
	// reading and writing strings
	virtual void WriteString(LPCTSTR lpsz);
	virtual LPTSTR ReadString(LPTSTR lpsz, UINT nMax);
	virtual BOOL ReadString(string& rString);

	// Implementation
public:
	virtual ~MightyStdioFile();
/*#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif*/
	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG GetLength() const;
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	// Unsupported APIs
	virtual MightyFile* Duplicate() const;
	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);
};

class InternetFileFind
{
public:
	InternetFileFind();
	virtual ~InternetFileFind();

	// Attributes
public:
	ULONGLONG GetLength() const;
	virtual string GetFileName() const;
	virtual string GetFilePath() const;
	virtual string GetFileTitle() const;
	virtual string GetFileURL() const;
	virtual string GetRoot() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(MightyTime& refTime) const;
	virtual BOOL GetLastAccessTime(MightyTime& refTime) const;
	virtual BOOL GetCreationTime(MightyTime& refTime) const;

	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	// these aren't virtual because they all use MatchesMask(), which is
	BOOL IsReadOnly() const;
	BOOL IsDirectory() const;
	BOOL IsCompressed() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsTemporary() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;

	// Operations
	void Close();
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
	virtual BOOL FindNextFile();

protected:
	virtual void CloseContext();

	// Implementation
protected:
	void* m_pFoundInfo;
	void* m_pNextInfo;
	HANDLE m_hContext;
	string m_strRoot;
	TCHAR m_chDirSeparator;     // not '\\' for Internet classes

/*#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
	void AssertValid() const;
#endif

	DECLARE_DYNAMIC(InternetFileFind)*/
};

class MightyMemFile : public MightyFile
{
public:
	// Constructors
	/* explicit */ 
	MightyMemFile(UINT nGrowBytes = 1024);
	MightyMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);

	// Operations
	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);
	BYTE* Detach();

	// Advanced Overridables
protected:
	virtual BYTE* Alloc(SIZE_T nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, SIZE_T nBytes);
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, SIZE_T nBytes);
	virtual void Free(BYTE* lpMem);
	virtual void GrowFile(SIZE_T dwNewLen);

	// Implementation
protected:
	SIZE_T m_nGrowBytes;
	SIZE_T m_nPosition;
	SIZE_T m_nBufferSize;
	SIZE_T m_nFileSize;
	BYTE* m_lpBuffer;
	BOOL m_bAutoDelete;

public:
	virtual ~MightyMemFile();
/*#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif*/
	virtual ULONGLONG GetPosition() const;
	BOOL GetStatus(MightyFileStatus& rStatus) const;
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual void SetLength(ULONGLONG dwNewLen);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,void** ppBufStart = NULL, void** ppBufMax = NULL);
	virtual ULONGLONG GetLength() const;

	// Unsupported APIs
	virtual MightyFile* Duplicate() const;
	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);
};

class MightyFileFind
{
public:
	MightyFileFind();
	virtual ~MightyFileFind();

	// Attributes
public:
	ULONGLONG GetLength() const;
	virtual string GetFileName() const;
	virtual string GetFilePath() const;
	virtual string GetFileTitle() const;
	virtual string GetFileURL() const;
	virtual string GetRoot() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(MightyTime& refTime) const;
	virtual BOOL GetLastAccessTime(MightyTime& refTime) const;
	virtual BOOL GetCreationTime(MightyTime& refTime) const;


	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	// these aren't virtual because they all use MatchesMask(), which is
	BOOL IsReadOnly() const;
	BOOL IsDirectory() const;
	BOOL IsCompressed() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsTemporary() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;

	DWORD GetFileAttributes() const;
	// Operations
	void Close();
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
	virtual BOOL FindNextFile();

protected:
	virtual void CloseContext();

	// Implementation
protected:
	void* m_pFoundInfo;
	void* m_pNextInfo;
	HANDLE m_hContext;
	string m_strRoot;
	TCHAR m_chDirSeparator;     // not '\\' for Internet classes

/*#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
	void AssertValid() const;
#endif

	DECLARE_DYNAMIC(CFileFind)*/
};