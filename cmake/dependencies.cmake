# USING CPM TO HANDLE DEPENDENCIES
if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(
            DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})


CPMFindPackage(
        NAME emhash
        GITHUB_REPOSITORY ktprime/emhash
        GIT_TAG ${EMHASH_VERSION}
        DOWNLOAD_ONLY TRUE
)

# create directory emhash in build directory then copy hash_table*.hpp to emhash
file(GLOB EMHASH_FILES ${emhash_SOURCE_DIR}/hash_table*.h*)
file(COPY ${EMHASH_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/emhash)
# now create an interface library that includes the directory
add_library(emhash INTERFACE)
target_include_directories(emhash INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
