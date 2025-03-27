#pragma once

#include <boost/functional/hash.hpp>
#include <emhash/hash_table7.hpp>
#include <functional>
#include <limits>
#include <list>
#include <memory_resource>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace lru {

namespace detail {

template <typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};

template <typename F, std::size_t... I>
auto make_cache_helper(F &&f, std::size_t capacity, std::index_sequence<I...>);

template <typename R, typename... Args> struct function_traits<R (*)(Args...)> {
  using return_type = R;
  using argument_types = std::tuple<Args...>;
};

template <typename ClassType, typename R, typename... Args>
struct function_traits<R (ClassType::*)(Args...) const> {
  using return_type = R;
  using argument_types = std::tuple<Args...>;
};

template <typename... Args> struct tuple_hash;
template <typename... Args> struct tuple_hash<std::tuple<Args...>>;

} // namespace detail

template <typename R, typename... Args> class Cache {
public:
  using Function = std::function<R(Args...)>;
  using Key = std::tuple<std::decay_t<Args>...>;

  // For simpler reference to the custom tuple-hash
  using MapHash = detail::tuple_hash<Key>;
  const std::size_t capacity;

  explicit Cache(Function func, std::size_t capacity = 1024)
      : capacity{capacity}, func{std::move(func)}, buffer(capacity),
        cache_map(capacity) {}

  constexpr R operator()(Args... args) {
    Key key{std::forward<Args>(args)...};

    if (const auto it = cache_map.find(key); it != cache_map.end()) {
      // Move to front => most recently used
      cache_list.splice(cache_list.begin(), cache_list, it->second);
      return cache_list.begin()->second;
    }
    put(key, func(std::forward<Args>(args)...));
    return cache_list.begin()->second;
  }

private:
  constexpr void put(Key const &key, R const &val) noexcept {
    if (cache_list.size() >= capacity) {
      // Evict LRU => the list back
      cache_map.erase(cache_list.back().first);
      cache_list.pop_back();
    }
    // Insert new item at the front
    cache_list.emplace_front(key, val);
    cache_map[key] = cache_list.begin();
  }

  const Function func;

  std::vector<std::pair<Key, R>> buffer;
  std::pmr::monotonic_buffer_resource mbr{buffer.data(),
                                          buffer.size() * sizeof(buffer[0])};
  std::pmr::polymorphic_allocator<std::pair<Key, R>> pa{&mbr};

  std::pmr::list<std::pair<Key, R>> cache_list{pa};
  using ListIt = typename decltype(cache_list)::iterator;

  emhash7::HashMap<Key, ListIt, MapHash> cache_map;
};

template <typename F> auto make_cache(F &&f, std::size_t capacity = 1024) {
  using traits = detail::function_traits<std::decay_t<F>>;
  using ArgsT = typename traits::argument_types;
  constexpr std::size_t N = std::tuple_size_v<ArgsT>;

  return detail::make_cache_helper(std::forward<F>(f), capacity,
                                   std::make_index_sequence<N>{});
}

namespace detail {

template <typename F, std::size_t... I>
auto make_cache_helper(F &&f, std::size_t capacity, std::index_sequence<I...>) {
  using traits = detail::function_traits<std::decay_t<F>>;
  using R = typename traits::return_type;
  using ArgsT = typename traits::argument_types;

  // Expand the argument types directly in the Cache template parameters:
  return Cache<R, std::tuple_element_t<I, ArgsT>...>(std::forward<F>(f),
                                                     capacity);
}

template <typename... Args> struct tuple_hash;
template <typename... Args> struct tuple_hash<std::tuple<Args...>> {
  std::size_t operator()(std::tuple<Args...> const &tpl) const noexcept {
    if constexpr (sizeof...(Args) == 1) {
      // Single element tuple => direct
      using T0 = std::tuple_element_t<0, std::tuple<Args...>>;
      return std::hash<T0>{}(std::get<0>(tpl));
    } else {
      // 2+ elements => combine
      std::size_t seed = 0;
      std::apply([&](auto const &...vals) { (..., hash_combine(seed, vals)); },
                 tpl);
      return seed;
    }
  }

private:
  template <typename T, typename S>
  static constexpr auto rotl(const T n, const S i) noexcept ->
      typename std::enable_if<std::is_unsigned<T>::value, T>::type {
    const T m = (std::numeric_limits<T>::digits - 1);
    const T c = i & m;
    return (n << c) | (n >> ((T(0) - c) & m));
  }

  template <typename T> static constexpr T xorshift(const T n, const int i) noexcept {
    return n ^ (n >> i);
  }

  static constexpr uint32_t distribute(const uint32_t n) noexcept {
    constexpr uint32_t p = 0x55555555u;
    constexpr uint32_t c = 3423571495u;
    return c * xorshift(p * xorshift(n, 16), 16);
  }

  static constexpr uint64_t distribute(const uint64_t n) noexcept {
    constexpr uint64_t p = 0x5555555555555555ULL;
    constexpr uint64_t c = 17316035218449499591ULL;
    return c * xorshift(p * xorshift(n, 32), 32);
  }

  template <typename V>
  static constexpr std::size_t hash_combine(const std::size_t seed, V const &val) {
    const auto distributed = [h = std::hash<V>{}(val)] {
      static_assert(sizeof(std::size_t) == 4 || sizeof(std::size_t) == 8,
                    "non-standard size_t is not supported");
      if constexpr (sizeof(std::size_t) == 4) {
        return distribute(static_cast<uint32_t>(h));
      }
      return distribute(static_cast<uint64_t>(h));
    }();

    return rotl(seed, std::numeric_limits<std::size_t>::digits / 3) ^
           distributed;
  }
};

} // namespace detail

} // namespace lru
