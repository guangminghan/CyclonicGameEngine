
//Copyright (c) 2005, ����
//
//�ļ����ƣ�BufferManage.cpp
//����:����ǿ
//last update 2005-8-31 0:18:20 by ����ǿ

#include "StdAfx.h"
#include ".\buffermanage.h"

CBufferManage::CBufferManage(void)
{
	this->m_count = 0;
}

CBufferManage::~CBufferManage(void)
{
}
ICE_BUF *CBufferManage::AllocBuffer(int len)
{
	ICE_BUF *pRetValue = new ICE_BUF(len);

	if ( pRetValue->buffer.len != len ) {

		delete pRetValue;

		pRetValue = NULL;
	}
	return pRetValue;
}
void CBufferManage::FreeBuffer(ICE_BUF *pBuffer)
{
	if ( pBuffer != NULL ) delete pBuffer;
}

ICE_OVERLAPPED_BUF *CBufferManage::AllocOverLappedBuf(int len)
{
	ICE_OVERLAPPED_BUF *pRetValue = new ICE_OVERLAPPED_BUF(len);

	InterlockedIncrement(&this->m_count);

	if ( pRetValue->pBuffer == NULL ) {

		if ( len == 0 ) return pRetValue;

		delete pRetValue; return NULL;

	}

	if ( pRetValue->pBuffer->buffer.len != len ) {

		delete pRetValue; return NULL;

	}
	return pRetValue;
}
void CBufferManage::FreeOverLappedBuf(ICE_OVERLAPPED_BUF *pOverLappedBuf)
{

	InterlockedDecrement(&this->m_count);

	K_PRINT("\n����������Ϊ%d", this->m_count);

	if ( pOverLappedBuf != NULL ) delete pOverLappedBuf;

}
