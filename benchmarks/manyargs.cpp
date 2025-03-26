#include <nanobench.h>
#include <string>

// Function with no side effects
int exampleFunction(int a, double b, char c, const std::string &d, bool e,
                    float f, long g, short h, unsigned int i, unsigned long j) {
  // Simple operation: return the sum of the integer arguments
  return a + static_cast<int>(b) + c + static_cast<int>(d.length()) + e +
         static_cast<int>(f) + g + h + i + j;
}

int main() {
  ankerl::nanobench::Bench bench;
  bench.title("Function with No Side Effects Benchmark")
      .unit("call")
      .warmup(100)
      .minEpochIterations(1000);

  using ankerl::nanobench::doNotOptimizeAway;
  bench.run("exampleFunction", [&]() {
    for (int i = 0; i < 1000; ++i) {
      doNotOptimizeAway(
          exampleFunction(1, 2.0, 'c', "example", true, 3.0f, 4L, 5, 6U, 7UL));
    }
  });

  return 0;
} //
// Created by marco on 3/26/25.
//
