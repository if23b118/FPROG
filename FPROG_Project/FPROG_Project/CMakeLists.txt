cmake_minimum_required(VERSION 3.30)
project(FPROG_Project)

set(CMAKE_CXX_STANDARD 17)

# Find Boost (make sure you specify the version and components you need)
find_package(Boost 1.86 REQUIRED COMPONENTS regex)

# Include Boost headers
include_directories(${Boost_INCLUDE_DIRS})

# Create the executable
add_executable(FPROG_Project main.cpp)

# Link Boost libraries
target_link_libraries(FPROG_Project ${Boost_LIBRARIES})
