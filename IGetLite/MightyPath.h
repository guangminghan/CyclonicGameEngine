#pragma once
#include <stdio.h>
#include <string.h>


//Windows平台
#ifdef WIN32

#include <direct.h>
#include <io.h>

//linux平台
#elif defined(linux)
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#elif defined(SLOARIS)
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#define MAXPATHLENGTH				256
#define MAXBUFFERSIZE				102400
#define DELFILEMODE					"movefile.sys"

#define COPY_SUCCESS                0
#define COPY_FAILED                -8
#define RIGHT 						0
#define WRONG 					   -1

#define LowCase(x)	((x)>='A'&&(x)<='Z'?(x)+32:(x))

//////////////////大小写不敏感的字符串比较函数/////////////////
/*************************stricmp***************************/
/*long stricmp(unsigned char* str1,unsigned char* str2)
{
	for(; *str1 && LowCase(*str1) == LowCase(*str2); str1++, str2++);
	return (long)LowCase(*str1) - (long)LowCase(*str2);
}*/
class MightyPath
{
public:
	MightyPath(void);
	~MightyPath(void);
public:
	long ClearDir(char* pszPath, long lFlag);
	long CheckPath(char* szpath);
	void RemoveBlank(char* szBuffer);
	void GetFullPath(char* pcPath,char* szFileName,char* pcResult);
	long Copy_FromFolderToFolder(char* dbSourcePath,char* dbDesPath);
	long CopyFile(char* szSourceFile,char* szDesFile);
};
