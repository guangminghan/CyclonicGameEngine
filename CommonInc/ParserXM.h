// ParserXM.h: interface for the ParserXM class.
//
// version 1.0.0, 2004/3/16
//
// Copyright (C) 2004 Maptrix
//
// mailto:maptrix@sohu.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely.
//
//////////////////////////////////////////////////////////////////////

#ifndef __MINIXML_PARSER_XM__
#define __MINIXML_PARSER_XM__

//#define SUPPORT_BREW
//#define SUPPORT_WINCE

#if defined SUPPORT_BREW
	#define ITEM_GROW_SIZE 128
#elif defined SUPPORT_WINCE
	#define SUPPORT_FILESYSTEM
	#define ITEM_GROW_SIZE 256
#else
	#define SUPPORT_FILESYSTEM
	#define ITEM_GROW_SIZE 1024
#endif

#if defined SUPPORT_BREW
	#include "mcBrew.h"
	#define _strchr STRCHR
	#define _strcmp STRCMP
	#define _malloc MALLOC
#else
	#include <string.h>
	#include <malloc.h>
	#define _strchr strchr
	#define _strcmp strcmp
	#define _malloc malloc
#endif

#include "mcAlloc.h"

class ParserXM  
{
public:
	ParserXM();
	~ParserXM();

	void ReadXML(TCHAR *XML);
	void ToRoot();
	BOOL HeadItem();
	BOOL FindItem(TCHAR *pName);
	BOOL NextItem();
	BOOL FindNextItem(TCHAR *pName);
	BOOL IntoItem();
	void ExitItem();

	BOOL GetData(TCHAR *pData);
	BOOL GetAttr(TCHAR *pName, TCHAR *pAttr);

#ifdef SUPPORT_FILESYSTEM
	BOOL LoadXML(TCHAR *szFileName);
#endif

protected:
	class ItemXM
	{
	public:
		ItemXM *Next;
		ItemXM *Child;
		ItemXM *Parent;
		TCHAR   *Data;
	};

	class AllocXM : public mcAlloc<ItemXM, ITEM_GROW_SIZE>
	{
	public:
		AllocXM()
		{
			m_pHead = (ItemXM *)(this + 1);
			m_pNext = NULL;
		}

		AllocXM *m_pNext;
	};

	//for ARM Compiler
	friend class AllocXM;

protected:
	void GetFreeItem(ItemXM *&pItem);
	void FreeAllItem();
	void Release();

	TCHAR *FindToken(TCHAR *pData, TCHAR*pBuff);
	TCHAR *ParseItem(TCHAR *pData, ItemXM *pParent);

protected:
	AllocXM *m_pRoot;
	AllocXM *m_pNext;

	BOOL     m_bNoError;
	BOOL     m_bAutoRelease;
	ItemXM  *m_pHead;
	ItemXM  *m_pItem;
	TCHAR    *m_pData;
};

#endif
