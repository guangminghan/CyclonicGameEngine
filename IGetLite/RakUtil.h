#ifndef RAKUTIL_INCLUDE_H
#define RAKUTIL_INCLUDE_H
//#pragma once
#include "../CommonInc/common.h"
#include "BitStream.h"

using std::string;
using RakNet::BitStream;

template <>
bool BitStream::Read<string>(string &var)
{
	unsigned short len;
	Read(len);
	if ( len > 0 )
	{
		char * buf = new char[len+1];
		Read(buf,len);
		buf[len] = 0;
		var.assign(buf);
		delete [] buf;
	}
	return true;
}

template <>
void BitStream::Write<string>(string var)
{
	size_t len = var.size();
	Write((unsigned short)len);
	if ( len > 0 )
	{
		Write(var.data(),(const int)var.size());
	}
}
#endif