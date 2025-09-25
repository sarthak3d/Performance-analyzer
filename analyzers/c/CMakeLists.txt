cmake_minimum_required(VERSION 3.10)
project(c_analyzer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find pkg-config
find_package(PkgConfig REQUIRED)

# Find cJSON
pkg_check_modules(CJSON REQUIRED libcjson)

# Try to find LLVM and Clang
find_package(LLVM 14 CONFIG)
if(NOT LLVM_FOUND)
    # Fallback: try using llvm-config
    execute_process(
        COMMAND llvm-config-14 --cmakedir
        OUTPUT_VARIABLE LLVM_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    find_package(LLVM REQUIRED CONFIG PATHS ${LLVM_DIR})
endif()

find_package(Clang CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM libraries: ${LLVM_LIBRARIES}")
message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIRS}
)

# Add definitions
add_definitions(${LLVM_DEFINITIONS})

# Source files
set(SOURCES
    src/main.c
    src/analyzer.c
    src/ast_visitor.c
    src/complexity_calculator.c
    src/pattern_detector.c
    src/json_handler.c
)

# Create executable
add_executable(c_analyzer ${SOURCES})

# Link libraries - using the C API of libclang
target_link_libraries(c_analyzer
    clang-14
    ${CJSON_LIBRARIES}
    m
)

# Compile options
target_compile_options(c_analyzer PRIVATE
    ${CJSON_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wno-unused-parameter
)