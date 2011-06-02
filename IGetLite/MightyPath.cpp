//#include "StdAfx.h"
#include ".\mightypath.h"

MightyPath::MightyPath(void)
{
}

MightyPath::~MightyPath(void)
{
}
long MightyPath::CopyFile(char* szSourceFile,char* szDesFile)
{
	FILE* fps = fopen(szSourceFile,"rb");
	if(fps == NULL)
		return COPY_FAILED;
	FILE* fpd = fopen(szDesFile,"wb");
	if(fpd == NULL)
	{
		fclose(fps);
		return COPY_FAILED;
	}

	char* szBuffer = new char[MAXBUFFERSIZE];
	if(szBuffer == NULL)
	{
		fclose(fps);
		fclose(fpd);
		return COPY_FAILED;
	}
	long lReadNum;
	while(!feof(fps))
	{
		lReadNum = fread(szBuffer,1,MAXBUFFERSIZE,fps);
		fwrite(szBuffer,1,lReadNum,fpd);
	}
	fclose(fps);
	fclose(fpd);
	delete []szBuffer;
	return COPY_SUCCESS;
}
///////////////////////////////////////////////////////
/********************* 文件夹拷贝*********************/
long MightyPath::Copy_FromFolderToFolder(char* dbSourcePath,char* dbDesPath)
{
	char*  pcTemp1 = new char[2*MAXPATHLENGTH];
	if(pcTemp1 == NULL)
		return COPY_FAILED;
	char*  pcTemp2 = pcTemp1 +MAXPATHLENGTH;
	GetFullPath(dbSourcePath,"*.*",pcTemp1);

#ifdef WIN32
	struct _finddata_t find_file;
	long  hFile;
	if(access(dbSourcePath,00) == -1)
	{
		delete []pcTemp1;
		return COPY_FAILED;
	}
	if((hFile = _findfirst(pcTemp1,&find_file)) == -1)
		return COPY_SUCCESS;
	else
	{
		if(strcmp(find_file.name,".") && strcmp(find_file.name,".."))
		{
			GetFullPath(dbSourcePath,find_file.name,pcTemp1);
			GetFullPath(dbDesPath,find_file.name,pcTemp2);
			if(find_file.attrib & _A_SUBDIR)
			{
				mkdir(pcTemp2);
				if(Copy_FromFolderToFolder(pcTemp1,pcTemp2) == COPY_FAILED)
				{
					delete []pcTemp1;
					return COPY_FAILED;
				}
			}
			else
			{
				if(CopyFile(pcTemp1,pcTemp2) == COPY_FAILED)
				{
					delete []pcTemp1;
					return COPY_FAILED;
				}
			}
		}
		while(_findnext(hFile,&find_file) != -1)
		{
			if(strcmp(find_file.name,".") && strcmp(find_file.name,".."))
			{
				GetFullPath(dbSourcePath,find_file.name,pcTemp1);
				GetFullPath(dbDesPath,find_file.name,pcTemp2);
				if(find_file.attrib & _A_SUBDIR)
				{
					mkdir(pcTemp2);
					if(Copy_FromFolderToFolder(pcTemp1,pcTemp2) == COPY_FAILED)
					{
						delete []pcTemp1;
						return COPY_FAILED;
					}
				}
				else
				{
					if(CopyFile(pcTemp1,pcTemp2) == COPY_FAILED)
					{
						delete []pcTemp1;
						return COPY_FAILED;
					}
				}
			}
		}
		delete []pcTemp1;
		_findclose(hFile);
		return COPY_SUCCESS;
	}
#elif defined(linux)
	struct dirent* ep;
	DIR* dp;
	if(access(dbSourcePath,00) == -1)
	{
		delete []pcTemp1;
		return COPY_FAILED;
	}
	if((dp = opendir(dbSourcePath)) == NULL)
		return COPY_SUCCESS;
	else
	{

		while((ep = readdir(dp)) != NULL)
		{
			if(strcmp(ep->d_name,".") && strcmp(ep->d_name,".."))
			{
				GetFullPath(dbSourcePath,ep->d_name,pcTemp1);
				GetFullPath(dbDesPath,ep->d_name,pcTemp2);
				if(ep->d_type & DT_DIR)
				{
					mkdir(pcTemp2,0777);
					if(Copy_FromFolderToFolder(pcTemp1,pcTemp2) == COPY_FAILED)
					{
						delete []pcTemp1;
						return COPY_FAILED;
					}
				}
				else if(ep->d_type & DT_REG)
				{
					if(CopyFile(pcTemp1,pcTemp2) == COPY_FAILED)
					{
						delete []pcTemp1;
						return COPY_FAILED;
					}
				}
			}
		}
		delete []pcTemp1;
		closedir(dp);
		return COPY_SUCCESS;
	}
#elif defined(SLOARIS)
	//#else
	struct dirent* ep;
	DIR* dp;
	char filename[300];
	struct stat st;
	if(access(dbSourcePath,00) == -1)
	{
		delete []pcTemp1;
		return COPY_FAILED;
	}
	if((dp = opendir(dbSourcePath)) == NULL)
		return COPY_SUCCESS;
	else
	{

		while((ep = readdir(dp)) != NULL)
		{
			if(strcmp(ep->d_name,".") && strcmp(ep->d_name,".."))
			{
				GetFullPath(dbSourcePath,ep->d_name,pcTemp1);
				GetFullPath(dbDesPath,ep->d_name,pcTemp2);
				stat(pcTemp1, &st);		//ligx
				//if(ep->d_type & DT_DIR)
				if((st.st_mode&S_IFDIR) == S_IFDIR)
				{
					mkdir(pcTemp2,0777);
					if(Copy_FromFolderToFolder(pcTemp1,pcTemp2) == COPY_FAILED)
					{
						delete []pcTemp1;
						return COPY_FAILED;
					}
				}
				//else if(ep->d_type & DT_REG)
				else if((st.st_mode&S_IFREG) == S_IFREG)
				{
					if(CopyFile(pcTemp1,pcTemp2) == COPY_FAILED)
					{
						delete []pcTemp1;
						return COPY_FAILED;
					}
				}
			}
		}
		delete []pcTemp1;
		closedir(dp);
		return COPY_SUCCESS;
	}
#endif
}

void MightyPath::GetFullPath(char* pcPath,char* szFileName,char* pcResult)
{
	while(*pcResult++ = *pcPath++);
#ifdef WIN32
	*(pcResult-1) = '\\';
#else
	*(pcResult-1) = '/';
#endif
	while(*pcResult++ = *szFileName++);

}

void MightyPath::RemoveBlank(char* szBuffer) 
{
	char* pcTemp1 = szBuffer;
	while(*pcTemp1)
	{
		if(*pcTemp1 != ' ' && *pcTemp1 != 0x0a && *pcTemp1 != 0x0d && *pcTemp1 != '\t'&& *pcTemp1)
			*szBuffer++ = *pcTemp1;
		pcTemp1++;
	}
	*szBuffer = 0;
}
//检查主辅库路径是否在（至少二级目录下）
long MightyPath::CheckPath(char* szpath)
{
#ifndef WIN32
	char* szSPath = szpath;
	if(*szpath++ != '/')
		return WRONG;
	char* szTmp = szpath;
	while(*szpath && *szpath != '/') szpath++;
	if(!*szpath || szpath - szTmp == 0)
		return WRONG;
	*szpath = 0;
	if(access(szSPath,0) == -1)
	{
		*szpath = '/';
		return WRONG;
	}
	*szpath++ = '/';
	szTmp = szpath;
	while(*szpath && *szpath != '/') szpath++;
	if(!*szpath || szpath - szTmp == 0)
		return WRONG;
#endif
	return RIGHT;
}

//lFlag >0:   删文件夹
//		 <=0: 保留文件夹
long MightyPath::ClearDir(char* pszPath, long lFlag)
{
	char szBuf[300];
	char szFileName[300];
	long lLen;
	FILE* fpList;

	sprintf(szFileName, "%s/%s", pszPath, DELFILEMODE);
	fpList = fopen(szFileName, "rt");
	if(fpList == NULL)
		return -1;
#ifdef WIN32
	struct _finddata_t find_file;
	long hFile;
	while(fgets(szBuf, 256, fpList))
	{
		lLen = strlen(szBuf)-1;
		while(lLen > 0 && (szBuf[lLen] == '\n' || szBuf[lLen] == ' ' || szBuf[lLen] == '\t')) lLen--;
		szBuf[lLen+1] = 0;
		sprintf(szFileName, "%s/%s", pszPath, szBuf);
		if((hFile = _findfirst(szFileName, &find_file)) == -1)
			continue;
		do
		{
			if(strcmp(find_file.name, ".") && strcmp(find_file.name, ".."))
			{
				sprintf(szFileName, "%s/%s", pszPath, find_file.name);
				if(find_file.attrib & _A_SUBDIR)
				{
					ClearDir(szBuf, lFlag);
				}
				else
				{
					remove(szFileName);
				}			
			}
		} while(_findnext(hFile, &find_file) == 0);
		_findclose(hFile);
	}
	fclose(fpList);
	sprintf(szFileName, "%s/%s", pszPath, DELFILEMODE);
	remove(szFileName);
#elif defined(linux)
	struct dirent* ep;
	DIR* dp;

	if((dp = opendir(pszPath)) == NULL)
		return 0;
	while((ep = readdir(dp)) != NULL)
	{
		if(strcmp(ep->d_name,".") && strcmp(ep->d_name,".."))
		{
			sprintf(szBuf, "%s/%s", szFileName, ep->d_name);
			if(ep->d_type & DT_DIR)
			{
				ClearDir(szBuf, lFlag);
			}
			else
			{
				remove(szFileName);
			}
		}
	}
	closedir(dp);
#elif defined(SLOARIS)
	struct dirent* ep;
	DIR* dp;
	struct stat st;
	if((dp = opendir(pszPath)) == NULL)
		return 0;
	while((ep = readdir(dp)) != NULL)
	{
		if(strcmp(ep->d_name, ".") && strcmp(ep->d_name,".."))
		{
			sprintf(szBuf, "%s/%s", szFileName, ep->d_name);
			stat(szBuf, &st);
			if((st.st_mode&S_IFDIR) == S_IFDIR)
			{
				ClearDir(szBuf, lFlag);
			}
			else
			{
				remove(szBuf);	
			}
		}
	}
	closedir(dp);
#endif
	if(lFlag) rmdir(pszPath);
	return 0;
}
