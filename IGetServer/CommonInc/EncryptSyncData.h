#if !defined(ENCRYPT_SYNCDATA__INCLUDED_)
#define ENCRYPT_SYNCDATA__INCLUDED_
#pragma once
#include <windows.h>
#include <wincrypt.h>
#define KEYLENGTH  0x00800000
#define ENCRYPT_ALGORITHM CALG_DES 
#define ENCRYPT_BLOCK_SIZE 8

class CEncryptSyncData
{
public:
	CEncryptSyncData(void);
	~CEncryptSyncData(void);
	bool InitEncryptHandle(const char * pchrPassWord);
	bool EncryptData(unsigned char * pchrSrcBuf,DWORD nOldLen,DWORD& nLen); 
	bool DecryptData(unsigned char * pchrSrcBuf,DWORD nLen);
public:
	char m_chKeyPassWord[128];
	HCRYPTPROV hCryptProv; 
	HCRYPTKEY hKey; 
	HCRYPTHASH hHash; 
};
#endif