#include <lru/lru.hpp>
#include <nanobench.h>
#include <string>

// Function with no side effects
int exampleFunction(int a, double b, char c, const std::string &d, bool e,
                    float f, long g, short h, unsigned int i, unsigned long j) {
  // Simple operation: return the sum of the integer arguments
  // use chrono literals for sleep_for
  return a + static_cast<int>(b) + c + static_cast<int>(d.length()) + e +
         static_cast<int>(f) + g + h + i + j;
}

int main() {
  ankerl::nanobench::Bench bench;
  bench.title("Function with No Side Effects Benchmark")
      .unit("call")
      .warmup(100)
      .minEpochIterations(10000);

  using ankerl::nanobench::doNotOptimizeAway;
  bench.run("exampleFunction", [&]() {
      doNotOptimizeAway(
          exampleFunction(1, 2.0, 'c', "example", true, 3.0f, 4L, 5, 6U, 7UL));
  });
  auto cache = lru::make_cache(exampleFunction);

  bench.run("Cache Hit", [&]() {
      doNotOptimizeAway(
          cache(1, 2.0, 'c', "example", true, 3.0f, 4L, 5, 6U, 7UL));
  });

  return 0;
}
