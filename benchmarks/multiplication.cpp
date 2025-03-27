// Example of a bad use of the library: the function is so simple that caching
// only adds overhead.

#include "lru/lru.hpp"
#include <nanobench.h>

int test_function(const int x) { return x * x; }

int main() {
  ankerl::nanobench::Bench bench;
  bench.title("LRU Cache Benchmark")
      .unit("call")
      .warmup(100)
      .minEpochIterations(1000);

  auto cache = lru::make_cache(test_function);

  bench.run("Cache Insertion", [&]() {
    for (auto i = 0; i < cache.capacity; ++i) {
      cache(i);
    }
  });

  bench.run("Cache Hit", [&]() {
    for (auto i = 0; i < cache.capacity; ++i) {
      cache(i);
    }
  });

  bench.run("Cache Miss", [&]() {
    for (auto i = int(cache.capacity); i < cache.capacity * 2; ++i) {
      cache(i);
    }
  });

  return 0;
}