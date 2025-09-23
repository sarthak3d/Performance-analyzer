cmake_minimum_required(VERSION 3.10)
project(cpp_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Add LLVM and Clang definitions and include directories
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Find nlohmann/json
find_package(nlohmann_json 3.2.0 REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/analyzer.cpp
    src/ast_visitor.cpp
    src/complexity_calculator.cpp
    src/pattern_detector.cpp
    src/json_handler.cpp
)

# Create executable
add_executable(cpp_analyzer ${SOURCES})

# Link libraries
llvm_map_components_to_libnames(llvm_libs support core)
target_link_libraries(cpp_analyzer
    ${llvm_libs}
    clangTooling
    clangFrontend
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangAnalysis
    clangAST
    clangASTMatchers
    clangEdit
    clangLex
    clangBasic
    nlohmann_json::nlohmann_json
)

# Set RPATH for runtime
set_target_properties(cpp_analyzer PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)