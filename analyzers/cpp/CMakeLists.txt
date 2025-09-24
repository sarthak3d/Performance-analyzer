cmake_minimum_required(VERSION 3.10)
project(cpp_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(nlohmann_json 3.2.0 REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/analyzer.cpp
    src/complexity.cpp
    src/patterns.cpp
    src/utils.cpp
)

# Create executable
add_executable(cpp_analyzer ${SOURCES})

# Link libraries
target_link_libraries(cpp_analyzer
    nlohmann_json::nlohmann_json
)