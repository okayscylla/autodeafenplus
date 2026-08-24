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

CPMAddPackage(
    URI  "gh:zeromq/libzmq#master"

    OPTIONS
        "ZMQ_BUILD_TESTS off"
)

target_link_libraries(autodeafenplus libzmq)

find_package(Python3 REQUIRED)

add_custom_target(bridge ALL)

add_custom_command(
    TARGET bridge POST_BUILD

    COMMAND "python -m ensurepip --upgrade"
    COMMAND "python -m pip install -r requirements.txt"
    COMMAND "python -m pyinstaller --distpath build/dist --workpath build --specpath build/spec --onefile --optimize 2 --collect-all --hide-console --disable-windowed-traceback src/bridge.py"
)

setup_geode_mod(${PROJECT_NAME})