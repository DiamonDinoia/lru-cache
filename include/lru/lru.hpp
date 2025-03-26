#pragma once

#include <boost/functional/hash.hpp>
#include <emhash/hash_table7.hpp>
#include <list>
#include <memory_resource>
#include <tuple>
#include <functional>
namespace lru {

namespace internal {
template <typename... Args> struct hash;
}

// A fixed-signature LRU cache using emhash7::HashMap
// Template parameters:
//    R         - return type of the cached function
//    Args...   - function argument types
//    CACHE_SIZE - maximum number of cached items
template <typename R, typename... Args> class Cache {
public:
  // Function pointer type, is faster than std::function.
  // But it can only store plain functions, not lambdas or functors.
  using Function = R (*)(Args...);
  using Key = std::tuple<std::decay_t<Args>...>;
  const size_t capacity;
  // Each list node holds a key and its cached result.

  constexpr explicit Cache(Function func, size_t capacity = 1024)
      : func(std::move(func)), capacity(capacity), buffer(capacity),
        cacheMap_(capacity) {}

  constexpr R operator()(const Args &...args) noexcept {
    Key key = std::make_tuple(args...);
    if (auto &&it = cacheMap_.find(key); it != cacheMap_.end()) [[likely]] {
      // Move accessed item to the front (most recently used)
      cacheList_.splice(cacheList_.begin(), cacheList_, it->second);
      return cacheList_.begin()->second;
    }
    R result = func(args...);
    put(key, result);
    return result;
  }

private:
  constexpr void put(const Key &key, const R &result) noexcept {
    if (cacheList_.size() >= capacity) {
      // Evict the least recently used item (back of list)
      cacheMap_.erase((--cacheList_.end())->first);
      cacheList_.pop_back();
    }
    // Insert new item at the front.
    cacheList_.emplace_front(key, result);
    cacheMap_[key] = cacheList_.begin();
  }

  const Function func;

  // monotonic allocator for the list to avoid dynamic memory allocation
  std::vector<std::pair<Key, R>> buffer; // enough to fit in all nodes
  std::pmr::monotonic_buffer_resource mbr{buffer.data(), buffer.size()};
  std::pmr::polymorphic_allocator<std::pair<Key, R>> pa{&mbr};
  // Doubly-linked list to maintain LRU order.
  std::pmr::list<std::pair<Key, R>> cacheList_{pa};
  using ListIt = typename decltype(cacheList_)::iterator;
  // Unordered map for O(1) key lookup; using Boost hash for std::tuple.
  emhash7::HashMap<Key, ListIt, internal::hash<Key>> cacheMap_;
};

namespace internal {

// --- Specialization of std::hash for std::tuple (C++17 with optimization) ---
template <typename... Args> struct hash<std::tuple<Args...>> {
  constexpr std::size_t
  operator()(const std::tuple<Args...> &tuple) const noexcept {
    // Compile-time check for tuple size
    if constexpr (sizeof...(Args) == 1) {
      // --- Case 1: Tuple has exactly one element ---
      // Get the type of the single element
      using FirstArgType = std::tuple_element_t<0, std::tuple<Args...>>;
      // Directly hash the single element using its std::hash specialization
      return std::hash<FirstArgType>{}(std::get<0>(tuple));
    } else {
      // --- Case 2: Tuple has 0 or 2+ elements ---
      std::size_t seed = 0; // Initial hash seed

      // Use std::apply and fold expression for combining hashes
      std::apply(
          [&seed](const auto &...args) {
            // Note: This fold expression relies on the hash_combine
            // function that modifies the seed by reference.
            (..., hash_combine(seed, args));
          },
          tuple);

      // Return the combined hash value (will be 0 for an empty tuple)
      return seed;
    }
  }

private:
  template <typename T, typename S>
  static constexpr auto rotl(const T n, const S i) noexcept ->
      typename std::enable_if<std::is_unsigned<T>::value, T>::type {
    const T m = (std::numeric_limits<T>::digits - 1);
    const T c = i & m; // Use T for consistency if S could be signed
    return (n << c) | (n >> ((T(0) - c) & m));
  }

  // Optimized xorshift: constexpr, pass fundamental by value
  template <typename T>
  static constexpr T xorshift(const T n,
                              const int i) noexcept { // Pass n by value
    return n ^ (n >> i);
  }

  // Optimized distribute32: constexpr, pass fundamental by value
  static constexpr uint32_t
  distribute(const uint32_t n) noexcept { // Pass n by value
    constexpr uint32_t p = 0x55555555ul;  // pattern of alternating 0 and 1
    constexpr uint32_t c = 3423571495ul;  // random uneven integer constant
    return c * xorshift(p * xorshift(n, 16), 16);
  }

  // Optimized distribute64: constexpr, pass fundamental by value
  static constexpr uint64_t
  distribute(const uint64_t n) noexcept { // Pass n by value
    constexpr uint64_t p =
        0x5555555555555555ull; // pattern of alternating 0 and 1
    constexpr uint64_t c =
        17316035218449499591ull; // random uneven integer constant
    return c * xorshift(p * xorshift(n, 32), 32);
  }

  // Optimized hash_combine: pass seed by value, return new seed
  // Kept 'v' as const T& for potentially large types. Marked inline.
  template <class T>
  static size_t hash_combine(const size_t seed,
                             const T &v) { // Pass seed by value
    constexpr size_t h = std::hash<T>{}(v);

    // Distribute the hash value based on size_t
    constexpr size_t distributed_hash = [] constexpr noexcept {
      if constexpr (sizeof(size_t) == 4) {
        return distribute(static_cast<uint32_t>(h));
      } else if constexpr (sizeof(size_t) == 8) {
        return distribute(static_cast<uint64_t>(h));
      } else {
        static_assert(sizeof(size_t) != 8 && sizeof(size_t) != 4,
                      "Unsupported size_t size");
      }
    }();

    // Combine using rotation and XOR
    return rotl(seed, std::numeric_limits<size_t>::digits / 3) ^
           distributed_hash;
  }
};
} // namespace internal

} // namespace lru