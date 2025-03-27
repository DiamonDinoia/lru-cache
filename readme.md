# Simple PMR LRU Cache

A header-only C++17 library providing a fixed-size, Least Recently Used (LRU) cache designed to minimize dynamic memory
allocations during operation using `std::pmr`.

## Description

This library implements an LRU cache (`lru::Cache`) that stores the results of function calls. When the cache reaches
its capacity, the least recently used item is evicted to make space for new entries.

It uses `emhash7::HashMap` for efficient lookups and a `std::pmr::list` backed by a
`std::pmr::monotonic_buffer_resource` to manage the LRU order and cache storage. By pre-allocating a buffer, it aims to
avoid dynamic memory allocations (`new`/`delete`) during cache hits and misses once the cache is initialized.

**Note:** This cache is designed for **plain function pointers** only. It cannot directly cache lambdas with captures,
`std::function` objects, or member functions due to the use of `R (*)(Args...)`.

## Features

* **Fixed Capacity:** The maximum number of items is set at construction.
* **LRU Eviction:** Automatically removes the least recently used item when capacity is reached.
* **Fast Lookups:** O(1) average time complexity for cache lookups, insertions, and deletions leveraging
  `emhash7::HashMap`.
* **Minimal Dynamic Allocation:** Uses `std::pmr::monotonic_buffer_resource` with a pre-allocated buffer for the
  internal list nodes, aiming to avoid heap allocations during typical operation.
* **Tuple Keys:** Function arguments are combined into a `std::tuple` to serve as the cache key.
* **Custom Tuple Hashing:** Includes an internal, optimized hash function implementation for `std::tuple`.
* **Header-Only:** Easy to integrate by just including the header file.
* **C++17:** Requires a C++17 compliant compiler.

## Dependencies

* **C++17 Standard Library:** (`<tuple>`, `<list>`, `<vector>`, `<memory_resource>`, `<functional>`, `<limits>`,
  `<type_traits>`, etc.)
* **emhash Hash Map Library:** Requires the `emhash` library, specifically version 7 (`emhash/hash_table7.hpp`). You can
  find it here: [https://github.com/martinus/emhash](https://github.com/martinus/emhash)

## Usage

0. **Installation**: using cmake fetch_content:
   ```cmake
    # 1. Include the FetchContent module

    include(FetchContent)

    # 2. Declare the dependency information

    FetchContent_Declare(
    lru-cache # Name for this dependency within the project
    GIT_REPOSITORY https://github.com/DiamonDinoia/lru-cache.git
    GIT_TAG main # Specify the branch, tag, or commit hash
    )
    
    # 3. Make the dependency available
    
    # This will download the source (if not already present) and process its CMakeLists.txt
    
    FetchContent_MakeAvailable(lru-cache)
   
    # or use cpm-cmake:
   
    CPMAddPackage("gh:DiamonDinoia/lru-cache@main")

   
    # then link against the target:
    
    target_link_libraries(your_target PRIVATE LRUCache)
   

    ```

1. **Include:** Add the cache header file (e.g., `lru/lru.hpp`) to your project.
2. **Define Function:** Have a plain function whose results you want to cache.
   ```c++
   // Must be a plain function or static member function
   ReturnType my_function(Arg1Type arg1, Arg2Type arg2, ...) {
       // ... computation ...
       return result;
   }
   ```
3. **Instantiate Cache:** Create an `lru::Cache` object, specifying the return type, argument types, the function
   pointer, and optionally the capacity.
   ```c++
   #include <lru/lru.hpp>
   #include <string>

   // Example function
   std::string process_data(int id, double value) {
       // ... potentially expensive processing ...
       return "Processed(" + std::to_string(id) + ", " + std::to_string(value) + ")";
   }

   // Create a cache with capacity 100 (default is 1024)
   auto cache = lru::make_cache(process_data, 100);
   ```
4. **Call via Cache:** Use the cache object's `operator()` like you would call the original function. The cache handles
   lookup, computation (on miss), caching, and eviction automatically.
   ```c++
   // First call: computes, caches, returns result
   std::string result1 = cache(1, 3.14);

   // Second call with same args: cache hit, returns cached result instantly
   std::string result2 = cache(1, 3.14);

   // Third call with different args: computes, caches, returns result
   std::string result3 = cache(2, 2.71);
   ```

## Example

```c++
#include <lru/lru.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

// Example function to cache (simulates some work)
long long expensive_calculation(int x, const std::string& label) {
    std::cout << ">> Performing calculation for (" << x << ", \"" << label << "\")..." << std::endl;
    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate delay
    return static_cast<long long>(x) * label.length();
}

int main() {
    // Create a cache for the function with a small capacity of 3 for demonstration
    auto cache = lru::make_cache(expensive_calculation, 3);

    std::cout << "Cache Capacity: " << cache.capacity << std::endl << std::endl;

    // --- First calls (will compute and cache) ---
    std::cout << "1. Calling cache(10, \"apple\")" << std::endl;
    std::cout << "   Result: " << cache(10, "apple") << std::endl << std::endl; // Computes

    std::cout << "2. Calling cache(20, \"banana\")" << std::endl;
    std::cout << "   Result: " << cache(20, "banana") << std::endl << std::endl; // Computes

    std::cout << "3. Calling cache(10, \"apple\")" << std::endl;
    std::cout << "   Result: " << cache(10, "apple") << std::endl << std::endl; // Cache Hit! (Should not print "Performing calculation...")

    // --- Fill the cache ---
    std::cout << "4. Calling cache(30, \"cherry\")" << std::endl;
    std::cout << "   Result: " << cache(30, "cherry") << std::endl << std::endl; // Computes (Cache size is now 3)

    // --- Access existing item (moves it to front of LRU) ---
    std::cout << "5. Calling cache(20, \"banana\")" << std::endl;
    std::cout << "   Result: " << cache(20, "banana") << std::endl << std::endl; // Cache Hit! (LRU order now: banana, cherry, apple)

    // --- Add a new item, causing eviction ---
    // Cache is full. "apple" (key {10, "apple"}) is the least recently used.
    std::cout << "6. Calling cache(40, \"date\")" << std::endl;
    std::cout << "   Result: " << cache(40, "date") << std::endl << std::endl;   // Computes, Evicts {10, "apple"} (LRU order: date, banana, cherry)

    // --- Verify eviction ---
    std::cout << "7. Calling cache(10, \"apple\") again" << std::endl;
    std::cout << "   Result: " << cache(10, "apple") << std::endl << std::endl; // Computes again (was evicted)

    // --- Final state check ---
    std::cout << "8. Calling cache(20, \"banana\")" << std::endl;
    std::cout << "   Result: " << cache(20, "banana") << std::endl << std::endl; // Cache Hit!
    std::cout << "9. Calling cache(40, \"date\")" << std::endl;
    std::cout << "   Result: " << cache(40, "date") << std::endl << std::endl;   // Cache Hit!

    return 0;
}
