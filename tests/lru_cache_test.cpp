#include "lru/lru.hpp"
#include <gtest/gtest.h>

auto call_count = 0u;

int test_function(const int x) {
  call_count++;
  return x * x;
}

int mul(const int x) { return x * x; }

TEST(LRUCacheTest, BasicFunctionality) {
  auto cache= lru::make_cache(test_function);
  EXPECT_EQ(call_count, 0);
  // Test insertion and retrieval
  EXPECT_EQ(cache(cache.capacity + 2), mul(cache.capacity + 2));
  EXPECT_EQ(cache(cache.capacity + 3), mul(cache.capacity + 3));

  EXPECT_EQ(call_count, 2);

  EXPECT_EQ(cache(cache.capacity + 2),
            mul(cache.capacity + 2)); // Should hit the cache

  EXPECT_EQ(call_count, 2);

  call_count = 0;

  for (auto i = 0; i < cache.capacity; ++i) {
    cache(i);
  }

  EXPECT_EQ(call_count, cache.capacity);

  for (int i = 0; i < cache.capacity; ++i) {
    cache(i);
  }

  EXPECT_EQ(call_count, cache.capacity);

  EXPECT_EQ(cache(cache.capacity + 2),
            mul(cache.capacity + 2)); // Should be evicted and recomputed
  EXPECT_EQ(call_count, cache.capacity + 1);

  EXPECT_EQ(cache(0), 0);

  EXPECT_EQ(call_count, cache.capacity + 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}