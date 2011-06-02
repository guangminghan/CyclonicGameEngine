#include "stdafx.h"
#include "Control_Guard.h"
CONTROL_Guard::CONTROL_Guard(CONTROL_Mutex &mutex)
:mutex_(mutex)
{
  mutex_.lock();
}
CONTROL_Guard::~CONTROL_Guard()
{
  mutex_.unlock();
}

