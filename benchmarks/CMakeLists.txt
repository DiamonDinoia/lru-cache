# Add nanobench using CPM
CPMAddPackage(
        NAME nanobench
        GITHUB_REPOSITORY martinus/nanobench
        GIT_TAG v4.3.11
)

add_executable(multiplication multiplication.cpp)
target_link_libraries(multiplication PRIVATE LRUCache nanobench)


add_executable(fibonacci fibonacci.cpp)
target_link_libraries(fibonacci PRIVATE LRUCache nanobench)

add_executable(unique_paths unique_paths.cpp)
target_link_libraries(unique_paths PRIVATE LRUCache nanobench)

add_executable(concatenated_words concatenated_words.cpp)
target_link_libraries(concatenated_words PRIVATE LRUCache nanobench)

add_executable(manyargs manyargs.cpp)
target_link_libraries(manyargs PRIVATE LRUCache nanobench)