#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif		
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <conio.h>
//#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <mswsock.h>
#include <tchar.h>
#include <set>
#include <map>
#include <list>
#include <stack>
#include <deque>
#include <queue>
#include <bitset>
#include <vector>
#include <string>
#include <hash_map>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <strstream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <assert.h>
using namespace std;
#pragma comment(lib,"WS2_32")
#define MAX_PEER 80
//#define _countof(array) (sizeof(array)/sizeof(array[0]))
#define WSA_EVENT_MAX_THREAD_COUNT 512
//#define perPackLen 204800
//#define perPackLen 163840 //160k
#define perPackLen 122880 //120k
//#define perPackLen 40960
//#define perPackLen 81920
#define MAX_REQUEST_PEERS 5
typedef long long QWORD;
typedef unsigned long long UQWORD;

#define UDP_HEARTBEAT_INTERVAL 10000 // UDP 心跳间隔 10 秒
#define TCP_HEARTBEAT_INTERVAL 10000 // TCP 心跳间隔 10 秒

#define PEER_WAIT_TIMEOUT_MAX	10000
//---------------------------------------------
typedef struct  TrackerIP
{
	DWORD ip_;
	WORD udpPort_;
	WORD tcpPort_;
	//string ipstr_;
}TRACKERIP_,*PTRACKERIP_;
//---------------------------------------------

//准备用来产生一个任务task_uuid_为uuid,map中KEY为全文HASH，VECTOR中为块HASH
typedef struct  peer_task
{
	string task_uuid_;
	map<string,vector<string> > * p_fileHash_;
}PEER_TASK_ST;
typedef map<string,vector<string> > p2p_map_task;
typedef map<string,p2p_map_task *>  map_uuid_task;
typedef struct server_list 
{
	DWORD server_ip_;
	WORD tcp_port_;
	unsigned int reTryCount;
	int stat;
}SERVER_LIST_ST;
typedef struct offer_sharefile
{
	unsigned char file_hash_[16];
	__int64 file_size_;
	char file_name[512];
}OFFER_SHAREFILE_ST;
//一个节点信息
typedef struct source_node
{
	DWORD node_ip_;
	WORD port_;
}SOURCE_NODE_ST;
//拥有该源的所有节点的信息
typedef struct node_list 
{
	unsigned char file_hash[16];
	unsigned int nodeCount;
	vector<SOURCE_NODE_ST> peer_in_list;
}NODE_LIST_ST;

#define MAX_P2P_CLINET_COUNT 5
#define HASH_BUF_SZIE 16
typedef struct BlockHashs 
{
	unsigned char Block_hash[16];
	__int64 Block_Start_offset;
	__int64 Block_End_offset;
	unsigned long Block_size;
}BlockHashs_ST;
typedef struct  PartHashs
{
	unsigned char Part_hash[16];
	__int64 Part_Start_offset;
	__int64 Part_End_offset;
	unsigned long Part_size;
	vector<BlockHashs_ST> Blocks_vec;
}PartHashs_ST;
typedef struct FileHash_Record
{
	char fileNme[256];
	char Path[256];
	unsigned char flHash[16];
	__int64 file_len;
	__int64 cur_offset;
	unsigned long  total_piece_count;
	unsigned long  piece_size;
	unsigned long  last_piece_size;
	vector<PartHashs_ST> PartHash_vec;
}FILEHASH_RECORD_ST;
//在CBaseInfo_File中使用了FileHash_Record
typedef struct writeFile_Scale
{
	__int64 from_scale;
	__int64 to_scale;
	__int64 cur_scale;
}WRITEFILE_SCALE_ST;
//共享文件结构
typedef struct share_file_list
{
	char file_name[260];
	unsigned __int64 file_len;
	unsigned char file_hash[16];

}SHARE_FILE_LIST_ST;
#endif /* __COMMON_H__ */
