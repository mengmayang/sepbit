//#include "src/logstore/scheduler/scheduler1.h"
#include "scheduler1.h"

Scheduler1::Scheduler1(Manager *manager)
{
  mSelection = std::unique_ptr<Selection>(SelectionFactory::GetInstance(Config::GetInstance().selection));
  mWorker = std::thread(&Scheduler1::scheduling, this, manager);
  mWorker.detach();
}

void Scheduler1::scheduling(Manager *manager) {
  using namespace std::chrono_literals;
  struct timeval current_time;
  while (true) {
    std::this_thread::sleep_for(0.05s);
    if (mShutdown) {
      break;
    }

    double gpt = Config::GetInstance().gpt;
    // while (manager->GetGp() >= 0.15) {
    while (manager->GetGp() >= gpt) {
      printf("GP: %.2f\n", manager->GetGp());
      // select a segment
      int segmentId = select(manager);
      {
        gettimeofday(&current_time, NULL);
        printf("GC start: %ld.%ld\n",
            current_time.tv_sec, current_time.tv_usec);
      }
      Segment segment = manager->ReadSegment(segmentId);
      {
        gettimeofday(&current_time, NULL);
        printf("GC finish read: %ld.%ld, GP: %.2f, SegmentsNum: %ld\n",
            current_time.tv_sec, current_time.tv_usec, segment.GetGp(), manager->GetSegmentsNum());
      }
      // collect the segment
      collect(manager, segment);
      {
        gettimeofday(&current_time, NULL);
        printf("GC finish rewrite: %ld.%ld\n",
            current_time.tv_sec, current_time.tv_usec);
      }
    }
  }
}

int Scheduler1::select(Manager *manager) {
  // prepare the segments_
  std::vector<Segment> segments;
  manager->GetSegments(segments);

  auto res = mSelection->Select(segments);

  return res[0].second;
}

void Scheduler1::collect(Manager *manager, Segment &segment) {
  uint64_t nRewriteBlocks = 0;
  manager->CollectSegment(segment.GetSegmentId());
  for (int i = 0; i < 131072; ++i) {
    off64_t blockAddr = segment.GetBlockAddr(i);
    if (blockAddr == UINT32_MAX) continue;
    off64_t oldPhyAddr = segment.GetPhyAddr(i);
    char* data = segment.GetBlockData(i);
    if (!manager->GcAppend(data, blockAddr, oldPhyAddr)) {
      nRewriteBlocks += 1;
    }
  }
  manager->RemoveSegment(segment.GetSegmentId(), segment.GetTotalInvalidBlocks() + nRewriteBlocks);
}

