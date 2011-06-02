#include "stdafx.h"
#include "Control_Mutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CONTROL_Mutex::CONTROL_Mutex()
{
	CONTROL_MUTEX_INIT(&mutex_);
}

CONTROL_Mutex::~CONTROL_Mutex()
{
	CONTROL_MUTEX_RELEASE(&mutex_);
}
void CONTROL_Mutex::lock()
{
	CONTROL_MUTEX_LOCK(&mutex_);
}
void CONTROL_Mutex::unlock()
{
	CONTROL_MUTEX_UNLOCK(&mutex_);
}

