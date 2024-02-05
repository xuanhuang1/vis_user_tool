cmake_minimum_required(VERSION 3.5)
project(mini_vistool)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

add_executable(basic_usage
    basic_usage.cpp
    loader.cpp
    util.cpp)

set_target_properties(basic_usage PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)


