
//Copyright (c) 2005, ����
//
//�ļ����ƣ�BufferManage.h
//����:����ǿ
//last update 2005-8-30 23:33:27 by ����ǿ

#pragma once
#include "localStruct.h"
class CBufferManage
{
public:
	CBufferManage(void);
	~CBufferManage(void);
	ICE_BUF *AllocBuffer(int len);
	void FreeBuffer(ICE_BUF *pBuffer);
	ICE_OVERLAPPED_BUF *AllocOverLappedBuf(int len);
	void FreeOverLappedBuf(ICE_OVERLAPPED_BUF *pOverLappedBuf);
private:
	volatile long m_count;
};
