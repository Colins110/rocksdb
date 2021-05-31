#include <cstdlib>
#include <ctime>
namespace ROCKSDB_NAMESPACE {
class multipath {
 public:
  static int RandomPathId(int length) {
    // if (!inited) {
    //   srandom(time(0));
    //   inited = true;
    // }
    // srandom(time(0));
    return rand() % length;
  }

  //  private:
  //   static bool inited;
};
// bool multipath::inited = false;
}  // namespace ROCKSDB_NAMESPACE