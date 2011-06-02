#include "../CommonInc/common.h"


#include "../CommonInc/Log.h"
#include "DispatchControl.h"
#include "HttpDefine.h"
//using raknet_wrapper::RakClient;
//using raknet_wrapper::CustomPacketLogger;
template<class out_type,class in_value> out_type convert(const in_value &t)
{
    stringstream stream;
    out_type result;

    stream<<t;
    stream>>result;

    return result;
}
int main( int argc, char *argv[] )
{
	std::string nstr="198110230022";
	__int64 rst=convert<__int64,std::string>(nstr);
	CDispatchControl colManage;

	int pid = GetCurrentProcessId();
	srand((unsigned int)time(0)+pid*10000);

	SetLogFile(_T("IGeTLite"));
	SetLogLevel(Log_Debug);
	StartLog(NULL);

	colManage.mf_main(NULL);

	return 0;
}