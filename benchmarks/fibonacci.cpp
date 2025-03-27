// Description: Example of a good use of the library: the function is worth caching.

#include "lru/lru.hpp"
#include <nanobench.h>

// naive recursive fibonacci implementation
int fibonacci(const int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    ankerl::nanobench::Bench bench;
    bench.title("LRU Cache Fibonacci Benchmark")
        .unit("call")
        .epochIterations(1);

    constexpr auto evals = 35;

    // Fibonacci only needs the last two values
    auto cache = lru::make_cache(fibonacci ,2);


    bench.run("Direct evaluation", [&]() {
        for (auto i = 0; i < evals; ++i) {
            fibonacci(i);
        }
    });

    bench.run("Cache evaluation", [&]() {
        for (auto i = 0; i < evals; ++i) {
            cache(30);
        }
    });


    return 0;
}