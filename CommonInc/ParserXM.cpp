// ParserXM.cpp: implementation of the ParserXM class.
//
//////////////////////////////////////////////////////////////////////

#include "ParserXM.h"

#ifdef SUPPORT_FILESYSTEM
	#include <stdio.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParserXM::ParserXM()
{
	m_pRoot = m_pNext = new AllocXM();
	m_pData = NULL;
	m_pHead = NULL;
	m_pItem = NULL;
	m_bAutoRelease = FALSE;
	m_bNoError     = FALSE;
}

ParserXM::~ParserXM()
{
	Release();
	AllocXM *pData, *pNext;
	pData = m_pRoot;
	while(pData)
	{
		pNext = pData->m_pNext;
		delete pData;
		pData = pNext;
	}
}

void ParserXM::ReadXML(TCHAR *XML)
{
	Release();
	if(XML == NULL || *XML == 0)
		return;

	m_bNoError = TRUE;
	ParseItem(XML, NULL);
	if(m_bNoError && m_pRoot->m_nFree < ITEM_GROW_SIZE)
		m_pHead = m_pItem = (ItemXM *)(m_pRoot + 1) + ITEM_GROW_SIZE - 1;
}

void ParserXM::ToRoot()
{
	if(m_bNoError)
		m_pItem = m_pHead;
}

BOOL ParserXM::HeadItem()
{
	if(m_pItem && m_pItem->Parent)
		m_pItem = m_pItem->Parent->Child;
	else
		m_pItem = m_pHead;
	return TRUE;
}

BOOL ParserXM::FindItem(TCHAR *pName)
{
	ItemXM  *pItem = m_pItem;
	HeadItem();

	while(m_pItem)
	{
		TCHAR Name[256];
		FindToken(m_pItem->Data, Name);
		if(_tcsicmp(Name, pName) == 0)
		{
			return TRUE;
		}

		m_pItem = m_pItem->Next;
	}

	m_pItem = pItem;
	return FALSE;
}

BOOL ParserXM::NextItem()
{
	if(m_pItem && m_pItem->Next)
	{
		m_pItem = m_pItem->Next;
		return TRUE;
	}
	return FALSE;
}

BOOL ParserXM::IntoItem()
{
	if(m_pItem && m_pItem->Child)
	{
		m_pItem = m_pItem->Child;
		return TRUE;
	}
	return FALSE;
}

void ParserXM::ExitItem()
{
	if(m_pItem)
		m_pItem = m_pItem->Parent;
}

BOOL ParserXM::GetData(TCHAR *pData)
{
	if(m_pItem == NULL || m_pItem->Child)
		return FALSE;

	TCHAR *pHead = m_pItem->Data;
	while(*pHead++);

	FindToken(pHead, pData);
	return TRUE;
}

BOOL ParserXM::GetAttr(TCHAR *pName, TCHAR *pAttr)
{
	if(m_pItem == NULL)
		return FALSE;

	TCHAR Name[256];
	TCHAR *pData = m_pItem->Data;

	while(pData && *pData)
	{
		pData = FindToken(pData, Name);
		if(_tcsicmp(Name, pName) == 0)
		{
			FindToken(pData, pAttr);
			return TRUE;
		}
	}
	return FALSE;
}

#ifdef SUPPORT_FILESYSTEM

extern tstring g_strWorkPath;

BOOL ParserXM::LoadXML(TCHAR *szFileName)
{
	TCHAR *szFullFileName = szFileName;
	tstring strFilePathName;

	if (!_tcschr(szFileName, _T(':')))
	{
		strFilePathName = g_strWorkPath + szFileName;
		szFullFileName = (TCHAR *)strFilePathName.c_str();
	}

	FILE *fp;
	fp = _tfopen(szFullFileName, _T("rb"));
	if(fp == NULL)
	{
		return FALSE;
	}

	fseek(fp, 0, SEEK_END);
	int nLen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *pData = new char[nLen + 1];
	if(pData)
	{
		fread(pData, 1, nLen, fp);
		pData[nLen] = '\0';
		int nNeedCharCount = (nLen + 1) * 2;
		m_pData = new TCHAR[nNeedCharCount];
#ifdef _UNICODE
		Utf8ToUnicode(pData, m_pData, nNeedCharCount * sizeof(TCHAR));
#else
		Utf8ToAnsi(pData, m_pData, nNeedCharCount * sizeof(TCHAR));
#endif
		delete [] pData;
		ReadXML(m_pData);
		m_bAutoRelease = TRUE;
	}

	fclose(fp);

	return TRUE;
}
#endif

void ParserXM::GetFreeItem(ItemXM *&pItem)
{
	pItem = m_pNext->Alloc();
	if(pItem == NULL)
	{
		AllocXM *pData = m_pNext->m_pNext;
		if(pData == NULL)
		{
			pData = new AllocXM();
			if(pData == NULL)
				return;
			m_pNext->m_pNext = pData;
		}

		m_pNext = pData;
		pItem = pData->Alloc();
	}
}

void ParserXM::FreeAllItem()
{
	AllocXM *pData;
	pData = m_pRoot;
	while(pData)
	{
		pData->FreeAll();
		pData = pData->m_pNext;
	}
	m_pNext = m_pRoot;
}

void ParserXM::Release()
{
	if(m_bAutoRelease)
	{
		m_bAutoRelease = FALSE;
		delete m_pData;
		m_pData = NULL;
	}
	m_pHead = NULL;
	m_pItem = NULL;
	FreeAllItem();
}

TCHAR *ParserXM::FindToken(TCHAR *pData, TCHAR*pBuff)
{
	while(*pData == ' ' ||
		  *pData == '\t' ||
		  *pData == '\r' ||
		  *pData == '\n' ||
		  *pData == '=')
	{
		pData++;
	}

	TCHAR *pTemp;
	TCHAR ch = *pData;
	if(ch == '\'' || ch == '"')
	{
		pTemp = ++pData;
		while(!(*pTemp == 0 ||
			*pTemp == ch))
			pTemp++;
	}
	else
	{
		pTemp = pData;
		while(!(*pTemp == 0 ||
			*pTemp == ' ' ||
			*pTemp == '\t' ||
			*pTemp == '\r' ||
			*pTemp == '\n' ||
			*pTemp == '='))
			pTemp++;
	}

	int nLen = pTemp - pData;
	pBuff[nLen] = 0;
	while(nLen-- > 0)
		pBuff[nLen] = pData[nLen];
	
	return ++pTemp;
}

TCHAR *ParserXM::ParseItem(TCHAR *pData, ItemXM *pParent)
{
	ItemXM *pPrev = NULL;
	while(pData)
	{
		TCHAR *pName;
		pName = _tcschr(pData, '<');
		if(pName == NULL)
			return NULL;
		*pName++ = 0;

		pData = _tcschr(pName, '>');
		if(pData == NULL)
		{
			m_bNoError = FALSE;
			return NULL;
		}
		*pData++ = 0;

		if(*pName == '!' || *pName == '?')
			continue;
		else if(*pName == '/')
			return pData;
		else
		{
			ItemXM *pNext;
			GetFreeItem(pNext);
			if(pNext == NULL)
			{
				m_bNoError = FALSE;
				return NULL;
			}
			pNext->Next   = NULL;
			pNext->Child  = NULL;
			pNext->Parent = pParent;
			pNext->Data   = pName;

			if(pPrev)
				pPrev->Next = pNext;
			else if(pParent)
				pParent->Child = pNext;
			pPrev = pNext;
			if(pData[-2] == '/')
				pData[-2] = 0;
			else
				pData = ParseItem(pData, pNext);
		}
	}

	return pData;
}

BOOL ParserXM::FindNextItem(TCHAR *pName)
{
	if (!NextItem())
	{
		return FALSE;
	}

	ItemXM  *pItem = m_pItem;

	while(m_pItem)
	{
		TCHAR Name[256];
		FindToken(m_pItem->Data, Name);
		if(_tcsicmp(Name, pName) == 0)
		{
			return TRUE;
		}

		m_pItem = m_pItem->Next;
	}

	m_pItem = pItem;
	return FALSE;
}
