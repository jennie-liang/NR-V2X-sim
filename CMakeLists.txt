cmake_minimum_required(VERSION 3.15)
project(nr_v2x_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(v2xsim
  src/main.cpp
  src/ResourcePool.cpp
  src/UE.cpp
  src/Channel.cpp
  src/Simulator.cpp
)

target_include_directories(v2xsim PRIVATE include)
