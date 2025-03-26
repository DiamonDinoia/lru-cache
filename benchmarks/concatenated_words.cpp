/**
 * @brief Solution and benchmark for the "Concatenated Words" problem.
 * LeetCode 472. Concatenated Words (Hard)
 *
 * Problem Statement:
 * Given an array of strings `words` (without duplicates), return all the
 * concatenated words in the given list. A concatenated word is defined as a
 * string that is comprised entirely of at least two shorter words (not
 * necessarily distinct) from the given array.
 *
 * Example:
 * Input: words =
 * ["cat","cats","catsdogcats","dog","dogcatsdog","rat","ratcatdogcat"] Output:
 * ["catsdogcats","dogcatsdog","ratcatdogcat"]
 *
 * Approach:
 * This problem can be solved using dynamic programming or recursion with
 * memoization. We check each word to see if it can be segmented into two or
 * more smaller words present in the input list.
 *
 * Two namespaces are provided:
 * 1. `naive`: Implements a basic recursive solution without memoization.
 * 2. `caching`: Implements the same recursive logic but uses `lru::Cache`
 * for memoization to avoid redundant computations.
 *
 * `naive` Namespace:
 * - `canForm(s, wordSet)`: Recursively checks if string `s` can be formed by
 * concatenating words from `wordSet`. It tries every possible split point.
 * If a `prefix` is in `wordSet`, it checks if the `suffix` is also in `wordSet`
 * OR if the `suffix` can be recursively formed.
 * - Inefficiency: This approach suffers from overlapping subproblems. The same
 * substring might be passed to `canForm` multiple times, leading to exponential
 * time complexity in the worst case.
 *
 * `caching` Namespace:
 * - Key Improvement: Uses `lru::Cache` to memoize the results of `canForm`.
 * - `lru::Cache cache(canForm, 16384);`:
 * - This line initializes an LRU (Least Recently Used) cache.
 * - It wraps the `caching::canForm` function.
 * - `16384` is the maximum number of results the cache will store. When the
 * cache is full, the least recently used entry is removed to make space.
 * - When `cache(substring)` is called:
 * 1. It checks if the result for `substring` is already in the cache.
 * 2. If yes, it returns the cached result immediately.
 * 3. If no, it calls the original `caching::canForm(substring)`, stores the
 * result in the cache associated with `substring`, and then returns the result.
 * - Emulation: This effectively emulates the behavior of Python's
 * `@functools.cache` or `@lru_cache` decorators, providing automatic
 * memoization for the function.
 * - `canForm(s)`: The logic is the same as the naive version, but the recursive
 * call is made via the cache: `cache(suffix)` instead of `canForm(suffix)`.
 * - `wordSet`: Is a global variable within the namespace, accessible by the
 * `canForm` function when invoked through the cache wrapper.
 * - `findAllConcatenatedWordsInADict`: Similar structure to the naive version
 * but uses `cache(word)` to check each word, benefiting from memoization.
 *
 */

#include <chrono>
#include <iostream>
#include <lru/lru.hpp>
#include <nanobench.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace naive {

bool canForm(std::string_view s,
             const std::unordered_set<std::string_view> &wordSet) {
  // Base case for recursion: an empty string cannot be formed by non-empty
  // words.
  if (s.empty()) {
    return false;
  }

  // Iterate through all possible split points i (1 to length-1)
  // This creates non-empty prefix and suffix.
  for (int i = 1; i < s.length(); ++i) {
    std::string_view prefix = s.substr(0, i);
    std::string_view suffix = s.substr(i);

    // Check if the prefix exists in the dictionary
    if (wordSet.count(prefix)) {
      // If prefix is valid, check the suffix:
      // It must EITHER be a word itself...
      // OR it must be recursively formable.
      // Pass wordSet down the recursive call.
      if (wordSet.count(suffix) || canForm(suffix, wordSet)) {
        // Found a valid segmentation.
        return true;
      }
    }
  }
  return false;
}

std::vector<std::string_view>
findAllConcatenatedWordsInADict(const std::vector<std::string> &words) {
  if (words.empty()) {
    return {}; // Return empty vector if input is empty
  }

  // Create an unordered_set for O(1) average time word lookups.
  std::unordered_set<std::string_view> wordSet(words.begin(), words.end());

  std::vector<std::string_view> result;

  // Iterate through the sorted words
  for (std::string_view word : words) {
    // Empty strings cannot be concatenated words.
    if (word.empty()) {
      continue;
    }

    // Check if the current word can be formed by calling the helper function.
    // Pass the wordSet to the top-level call.
    if (canForm(word, wordSet)) {
      result.push_back(word);
    }
  }

  return result;
}
} // namespace naive

namespace caching {

// forward declaration to allow  canForm to use the cache instead of calling
// itself recursively
bool canForm(std::string_view s);
lru::Cache cache(canForm);

std::unordered_set<std::string_view> wordSet;
bool canForm(std::string_view s) {
  // Base case for recursion: an empty string cannot be formed by non-empty
  // words.
  if (s.empty()) {
    return false;
  }
  // Iterate through all possible split points i (1 to length-1)
  // This creates non-empty prefix and suffix.
  for (int i = 1; i < s.length(); ++i) {
    std::string_view prefix = s.substr(0, i);
    std::string_view suffix = s.substr(i);

    // Check if the prefix exists in the dictionary
    if (wordSet.count(prefix)) {
      // If prefix is valid, check the suffix:
      // It must EITHER be a word itself...
      // OR it must be recursively formable.
      // Pass wordSet down the recursive call.
      if (wordSet.count(suffix) || cache(suffix)) {
        // Found a valid segmentation.
        return true;
      }
    }
  }

  return false;
}

/**
 * @brief Finds all concatenated words in a dictionary.
 *
 * @param words The list of words.
 * @return A list of words from the input that are concatenations of >= 2 other
 * words from the input.
 */
std::vector<std::string_view>
findAllConcatenatedWordsInADict(const std::vector<std::string> &words) {
  if (words.empty()) {
    return {}; // Return empty vector if input is empty
  }

  // Create an unordered_set for O(1) average time word lookups.
  wordSet = std::unordered_set<std::string_view>(words.begin(), words.end());

  std::vector<std::string_view> result;

  // Iterate through the sorted words
  for (std::string_view word : words) {
    // Empty strings cannot be concatenated words.
    if (word.empty()) {
      continue;
    }

    // Check if the current word can be formed by calling the helper function.
    // Pass the wordSet to the top-level call.
    if (cache(word)) {
      result.push_back(word);
    }
  }

  return result;
}

} // namespace caching

extern const std::vector<std::string> test_words;

int main() {
  // this benchmark takes too long so we only use nanobench to prevent the
  // compiler from throwing the code away
  using ankerl::nanobench::doNotOptimizeAway;

  {
    const auto start = std::chrono::high_resolution_clock::now();
    doNotOptimizeAway(caching::findAllConcatenatedWordsInADict(test_words));
    const auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "caching elapsed time: " << elapsed_seconds.count() << "s\n";
  }

  {
    const auto start = std::chrono::high_resolution_clock::now();
    doNotOptimizeAway(naive::findAllConcatenatedWordsInADict(test_words));
    const auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "naive elapsed time: " << elapsed_seconds.count() << "s\n";
  }
}

const std::vector<std::string> test_words = {
    "a",
    "aa",
    "aaa",
    "aaaa",
    "aaaaa",
    "aaaaaa",
    "aaaaaaa",
    "aaaaaaaa",
    "aaaaaaaaa",
    "aaaaaaaaaa",
    "aaaaaaaaaaa",
    "aaaaaaaaaaaa",
    "aaaaaaaaaaaaa",
    "aaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaz",
};
