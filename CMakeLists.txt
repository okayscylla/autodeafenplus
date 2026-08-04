cmake_minimum_required(VERSION 4.0.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CPPZMQ_BUILD_TESTS false)

project(autodeafenplus VERSION 1.0.0)

include_directories(include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES}
    include/keycodes.h)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

add_definitions(-DCMAKE_POLICY_VERSION_MINIMUM=3.5)

CPMFindPackage(
    NAME libzmq

    GITHUB_REPOSITORY zeromq/libzmq
    GIT_TAG v4.3.5

    OPTIONS
        "BUILD_SHARED OFF"
        "CMAKE_POLICY_VERSION_MINIMUM 3.5"
        "BUILD_TESTS OFF"
)

target_link_libraries(autodeafenplus libzmq-static)

setup_geode_mod(${PROJECT_NAME})