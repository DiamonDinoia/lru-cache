cmake_minimum_required(VERSION 3.15)

project(LRUCache VERSION 1.0 LANGUAGES CXX)


set(CPM_VERSION 0.40.8)
set(EMHASH_VERSION 139319e)


option(LRU_BUILD_TESTING "Build tests" OFF)
option(LRU_BUILD_BENCHMARKS "Build benchmarks" OFF)

include(cmake/dependencies.cmake)

add_library(LRUCache INTERFACE)

target_include_directories(LRUCache INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(LRUCache INTERFACE cxx_std_17)

target_link_libraries(LRUCache INTERFACE emhash)

install(TARGETS LRUCache EXPORT LRUCacheConfig)
install(DIRECTORY include/ DESTINATION include)
install(TARGETS emhash EXPORT LRUCacheConfig)

export(EXPORT LRUCacheConfig
        FILE "${CMAKE_CURRENT_BINARY_DIR}/LRUCacheConfig.cmake"
        NAMESPACE LRUCache::
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/LRUCacheConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/LRUCacheConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/LRUCacheConfigVersion.cmake"
        DESTINATION lib/cmake/LRUCache
)

if (LRU_BUILD_TESTING)
    add_subdirectory(tests)
endif ()

if (LRU_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()