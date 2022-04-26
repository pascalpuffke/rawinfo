cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(rawinfo LANGUAGES CXX)

if (CMAKE_BUILD_TYPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif ()

set(CPM_DOWNLOAD_VERSION 0.35.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()
include(${CPM_DOWNLOAD_LOCATION})

set(SOURCES
        src/TimeUtils.cpp
        src/FormatUtils.cpp
        src/main.cpp)

set(LIBRAW_PATH "${PROJECT_SOURCE_DIR}/LibRaw")

CPMAddPackage(
        NAME fmt
        VERSION 8.1.1
        GITHUB_REPOSITORY fmtlib/fmt
        GIT_TAG 8.1.1
)
CPMAddPackage(
        NAME cxxopts
        VERSION 3.0.0
        GITHUB_REPOSITORY jarro2783/cxxopts
        GIT_TAG v3.0.0
)

add_subdirectory(LibRaw-cmake)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} fmt::fmt libraw::libraw)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
