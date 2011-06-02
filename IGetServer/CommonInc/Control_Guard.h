#ifndef _CONTROL_GUARD_H_
#define _CONTROL_GUARD_H_

#include "Control_Mutex.h"

class CONTROL_Guard {
public:
  CONTROL_Guard(CONTROL_Mutex &mutex);
  ~CONTROL_Guard();
private:
  CONTROL_Mutex& mutex_;
};
#endif

