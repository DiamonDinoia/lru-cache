# Enable testing
enable_testing()

# Add Google Test
CPMAddPackage(
        NAME GTest
        GITHUB_REPOSITORY google/googletest
        VERSION 1.16.0
)

# Add the test executable
add_executable(LRUCacheTest lru_cache_test.cpp)

# Link the test executable with the necessary libraries
target_link_libraries(LRUCacheTest PRIVATE LRUCache GTest::gtest_main)

# Discover and register the tests
include(GoogleTest)
gtest_discover_tests(LRUCacheTest)
