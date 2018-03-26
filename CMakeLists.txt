cmake_minimum_required(VERSION 3.0)
project(tsdivider)
set(serial "0.2.3")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/")

# C++11
set(CMAKE_CXX_FLAGS "-std=c++11")

# Boost
find_package(Boost 1.60.0 REQUIRED COMPONENTS program_options filesystem system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_SOURCE_DIR}/config.h
)

include_directories(
  ./
)

add_executable(
  tsdivider
  tsdivider.cpp
  aribstr.c
)

target_link_libraries(
  tsdivider
  ${Boost_LIBRARIES}
)

install(TARGETS tsdivider RUNTIME DESTINATION "bin")
