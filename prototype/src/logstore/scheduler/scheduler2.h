#ifndef LOGSTORE_SCHEDULER2_H
#define LOGSTORE_SCHEDULER2_H

#include <thread>
#include <zconf.h>
#include <iostream>
#include <sys/time.h>
#include <map>
#include <algorithm>

#include "src/logstore/manager.h"
#include "src/selection/factory.h"
#include "src/logstore/config.h"
#include "src/logstore/scheduler/scheduler.h"

class Scheduler2: public Scheduler {

public:
    Scheduler2(Manager *manager);
    void Shutdown() { mShutdown = true; }

private:
    void scheduling(Manager *manager);
    std::vector<int> select(Manager *manager);
    void collect(Manager *manager, std::map<uint64_t, std::pair<uint64_t, int>> &block_segment_map, std::map<uint64_t, std::shared_ptr<Segment>> &segments);
    // Segment GetSegment(Manager *manager, uint64_t segmentId, std::map<uint64_t, Segment> &segments);

    std::unique_ptr<Selection> mSelection;
    std::thread mWorker;
    bool mShutdown = false;
};
#endif //LOGSTORE_SCHEDULER2_H

