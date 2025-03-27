/*
* Problem Description:
 *
 * There is a robot on an m x n grid. The robot is initially located at the top-left corner (i.e., grid[0][0]).
 * The robot tries to move to the bottom-right corner (i.e., grid[m - 1][n - 1]). The robot can only move either
 * down or right at any point in time.
 *
 * Given the two integers m and n, return the number of possible unique paths that the robot can take to reach
 * the bottom-right corner.
 *
 * This is a dynamic programming problem. But it can be solved using combinatorics.
 *
 */

#include <nanobench.h>
#include <lru/lru.hpp>

namespace iterative {
double factorial(double n) {
  double result = 1;
  for (int i = 2; i <= n; ++i) {
    result *= i;
  }
  return result;
}

double binomial(double n, double k) {
  return factorial(n) / (factorial(k) * factorial(n - k));
}

double uniquePaths(double m, double n) {
  double steps = (m - 1) + (n - 1);
  return binomial(steps, n - 1);
}
} // namespace iterative

namespace recursive {
// std::tgamma is slower than the naive recursive implementation
double factorial(double n) {
  if (n <= 1) {
    return 1;
  }
  return n * factorial(n - 1);
}

// Function to calculate binomial coefficient
double binomial(double n, double k) {
  return factorial(n) / (factorial(k) * factorial(n - k));
}

// Function to calculate unique paths
double uniquePaths(double m, double n) {
  double steps = (m - 1) + (n - 1);
  return binomial(steps, n - 1);
}
} // namespace recursive

namespace caching {

double factorial(double n) {
  if (n <= 1) {
    return 1;
  }
  return n * factorial(n - 1);
}

auto cache = lru::make_cache(factorial);

// Function to calculate binomial coefficient
double binomial(double n, double k) {
  return cache(n) / (cache(k) * cache(n - k));
}

// Function to calculate unique paths
double uniquePaths(double m, double n) {
  double steps = (m - 1) + (n - 1);
  return binomial(steps, n - 1);
}
} // namespace caching

namespace reference {

double uniquePaths(int m, int n) {
  if (m <= 0 || n <= 0) {
    return 0.0;
  }
  if (m == 1 || n == 1) {
    return 1.0;
  }

  std::vector dp_row(n, 1.0);

  for (int i = 1; i < m; ++i) {
    for (int j = 1; j < n; ++j) {
      dp_row[j] = dp_row[j - 1] + dp_row[j];
    }
  }

  return dp_row[n - 1];
}
} // namespace reference

int main() {
  ankerl::nanobench::Bench bench;
  bench.title("Unique Paths Benchmark")
      .unit("call")
      .warmup(100)
      .minEpochIterations(100000);

  using ankerl::nanobench::doNotOptimizeAway;
  int m = 20, n = 25;

  assert(recursive::uniquePaths(m, n) == reference::uniquePaths(m, n));
  assert(iterative::uniquePaths(m, n) == reference::uniquePaths(m, n));
  assert(caching::uniquePaths(m, n) == reference::uniquePaths(m, n));

  bench.run("reference",
            [&]() { doNotOptimizeAway(reference::uniquePaths(m, n)); });

  bench.run("Iterative",
            [&]() { doNotOptimizeAway(iterative::uniquePaths(m, n)); });

  bench.run("Recursive",
            [&]() { doNotOptimizeAway(recursive::uniquePaths(m, n)); });

  bench.run("Cache", [&]() { doNotOptimizeAway(caching::uniquePaths(m, n)); });

  return 0;
}
