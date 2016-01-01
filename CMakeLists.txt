cmake_minimum_required (VERSION 3.0)
project (tssplitter_plus)

# C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

# Boost
find_package(Boost 1.60.0 REQUIRED COMPONENTS program_options)

add_executable(tssplitter_plus tssplitter_plus.cpp)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(tssplitter_plus ${Boost_LIBRARIES})
