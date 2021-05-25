#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <map>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

#include "rocksdb/rocksdb_namespace.h"

#define pathPropertys 4
#define readProperty 0
#define flushProperty 1
#define lcompaction 2
#define ocompaction 3
namespace ROCKSDB_NAMESPACE {
class multipath {
 public:
  static int RandomPathId(int length) {
    srandom(time(0));
    return rand() % length;
  }
  multipath(int pathNum) : num(pathNum) {
    pathCounter = new std::vector<std::vector<uint64_t>>(
        num, std::vector<uint64_t>(pathPropertys, 0));
    socre2path = new std::map<long long, int>;
    path2score = new std::vector<int>(num, 0);
    for (int i = 0; i < pathNum; i++) {
      socre2path->insert({0, i});
    }
  }
  ~multipath() {
    delete pathCounter;
    delete socre2path;
    delete path2score;
  }
  int getNext() const { return socre2path->begin()->second; }
  void add(int pathId, int propertyIndex, int change) {
    (*pathCounter)[pathId][propertyIndex] =
        (*pathCounter)[pathId][propertyIndex] + change;
    long long score = 0;
    for (auto i : (*pathCounter)[pathId]) {
      score += i;
    }

    m.lock();
    socre2path->erase((*path2score)[pathId]);
    socre2path->insert(std::pair<long long, int>(score, pathId));
    (*path2score)[pathId] = score;
    m.unlock();
  }
  void sub(int pathId, int propertyIndex, int change) {
    (*pathCounter)[pathId][propertyIndex] =
        (*pathCounter)[pathId][propertyIndex] - change;
    uint64_t tmp = (*pathCounter)[pathId][propertyIndex];
    // assert(tmp >= 0);
    long long score = 0;
    for (auto i : (*pathCounter)[pathId]) {
      score += i;
    }

    m.lock();
    socre2path->erase((*path2score)[pathId]);
    socre2path->insert(std::pair<long long, int>(score, pathId));
    (*path2score)[pathId] = score;
    m.unlock();
  }

 private:
  int num;
  std::mutex m;
  std::vector<std::vector<uint64_t>> *pathCounter;
  std::map<long long, int> *socre2path;
  std::vector<int> *path2score;
};
}  // namespace ROCKSDB_NAMESPACE