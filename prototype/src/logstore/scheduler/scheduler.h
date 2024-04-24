#ifndef LOGSTORE_SCHEDULER_H
#define LOGSTORE_SCHEDULER_H

#include "src/logstore/scheduler/scheduler.h"
class Scheduler {
public:
    virtual void Shutdown() = 0;
};

#endif //LOGSTORE_SCHEDULER_H
