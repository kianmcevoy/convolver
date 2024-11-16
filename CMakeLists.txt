cmake_minimum_required(VERSION 3.10)

# Set the project name
project(convolver)

# Add the include directory
include_directories(include)
include_directories(benchmarker)

# Add the executable
add_executable(convolver convolver.cpp)