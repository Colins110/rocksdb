#include <cstdlib>
#include <ctime>
namespace ROCKSDB_NAMESPACE {
class multipath {
 public:
  static int RandomPathId(int length) {
    srandom(time(0));
    return rand() % length;
  }
};
}  // namespace ROCKSDB_NAMESPACE