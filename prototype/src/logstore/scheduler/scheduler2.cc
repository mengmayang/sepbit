#include "scheduler2.h"
#include <cassert>

Scheduler2::Scheduler2(Manager *manager)
{
  mSelection = std::unique_ptr<Selection>(SelectionFactory::GetInstance(Config::GetInstance().selection));
  mWorker = std::thread(&Scheduler2::scheduling, this, manager);
  mWorker.detach();
}

void Scheduler2::scheduling(Manager *manager) {
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
      // select some segment from one class
      std::vector<int> segmentIds = select(manager);
      {
        gettimeofday(&current_time, NULL);
        printf("GC start: %ld.%ld\n",
            current_time.tv_sec, current_time.tv_usec);
      }
      // TODO
      // 将Cost-BenefitGreedy选出的segment放到segments map中
      std::map<uint64_t, std::shared_ptr<Segment>> segments;
      assert(segments.size() > 0);
      auto first_segment = manager->GetSegmentById(segmentIds[0]);
      auto first_segment_Id = first_segment->GetSegmentId();
      segments[first_segment_Id] = first_segment;
      if (segmentIds.size() > 1) {
        for (auto i = 1; i < segmentIds.size(); i++) {
          auto segment = manager->GetSegmentById(segmentIds[i]);
          if (segment->GetClassNum() == first_segment->GetClassNum()) {
            auto segmentId = segment->GetSegmentId();
            segments[segmentId] = segment;
          }
        }
      }
      // get rewritten range
      // map<block_addr, <segmentId, block_index>>
      std::map<uint64_t, std::pair<uint64_t, int>> block_segment_map;
      
      for(auto it = segments.begin() ; it != segments.end(); it++) {
        auto segment = it->second;
        for (int i = 0; i < 131072; ++i) {
          off64_t blockAddr = segment->GetBlockAddr(i);
          if (blockAddr == ~0ull) continue;
          auto it = block_segment_map.find(blockAddr);
          if (it == block_segment_map.end() || it->second.first <= segment->GetSegmentId()) {
            block_segment_map[blockAddr] = std::make_pair(segment->GetSegmentId(), i);
          }
        }
      }

      // read segment
      for (const auto& range : block_segment_map) {
        auto segmentId = range.second.first;
        auto blockAddr = range.first; 
        auto blockIndex = range.second.second;
        manager->ReadSegment(segmentId, blockAddr, blockIndex);
      }

      {
        gettimeofday(&current_time, NULL);
        printf("GC finish read: %ld.%ld\n",
            current_time.tv_sec, current_time.tv_usec);
      }
      // TODO
      // collect the segment
      collect(manager, block_segment_map, segments);
      {
        gettimeofday(&current_time, NULL);
        printf("GC finish rewrite: %ld.%ld\n",
            current_time.tv_sec, current_time.tv_usec);
      }
    }
  }
}

std::vector<int> Scheduler2::select(Manager *manager) {
  // prepare the segments_
  std::vector<Segment> segments;
  manager->GetSegments(segments);

  auto res = mSelection->Select(segments);

  // select segments from one class
  std::vector<int> segmentIds;
  for (auto i = 0; i < res.size(); i++) {
    segmentIds.push_back(res[i].second);
  }
  return segmentIds;
}

// Segment Scheduler2::GetSegment(Manager *manager, uint64_t segmentId, std::map<uint64_t, Segment> &segments) {
//   auto it = segments.find(segmentId);
//   if (it == segments.end()) {
//     segments[segmentId] = manager->GetSegmentById(segmentId);
//     return segments[segmentId];
//   }
//   return it->second;
// }

void Scheduler2::collect(Manager *manager, std::map<uint64_t, std::pair<uint64_t, int>> &block_segment_map, std::map<uint64_t, std::shared_ptr<Segment>> &segments) {
  uint64_t nRewriteBlocks = 0;
  std::vector<uint64_t> gc_segment_ids;
  for (const auto &range : block_segment_map) {
    auto segmentId = range.second.first;
    auto blockAddr = range.first;
    auto blockIndex = range.second.second;
    auto it = std::find(gc_segment_ids.begin(), gc_segment_ids.end(), segmentId);
    if (it == gc_segment_ids.end()) {
      gc_segment_ids.push_back(segmentId);
    }
    auto segment = segments[segmentId];
    off64_t oldPhyAddr = segment->GetPhyAddr(blockIndex);
    char* data = segment->GetBlockData(blockIndex);
    if (!manager->GcAppend(data, blockAddr, oldPhyAddr)) {
      nRewriteBlocks += 1;
    }
  }

  for (auto gc_segment_id : gc_segment_ids) {
    auto segment = segments[gc_segment_id];
    manager->RemoveSegment(segment->GetSegmentId());
  }
}

