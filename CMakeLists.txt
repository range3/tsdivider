cmake_minimum_required(VERSION 3.0)
project(tsdivider)
enable_language(C CXX)
set(serial "0.3.1")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/")

# Compile Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Boost
set(Boost_USE_MULTITHREADED OFF)
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
