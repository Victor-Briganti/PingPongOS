cmake_minimum_required(VERSION 3.10..3.28)

project(PingPongOS VERSION 0.0.1 LANGUAGES C)

# Compiler flags
set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -Wconversion")

# Enable testing
include(CTest)
enable_testing()

# Defines the valgrind for testing memory
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

# Define "compile_commands.json" to be used
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   

# Enables the ccache to be used with the program
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Ccache found: ${CCACHE_PROGRAM}")
else ()
    message(STATUS "Ccache not found")
endif ()

# Define the queue library
add_library(QueueLib STATIC src/lib/queue.c)
target_include_directories(QueueLib PUBLIC include)

# Define the test executable
add_executable(QueueTest test/lib/queue_test.c)
target_include_directories(QueueTest PUBLIC include)

# Link the queue library to the test executable
target_link_libraries(QueueTest PRIVATE QueueLib)

# Add the test
add_test(NAME QueueTests COMMAND QueueTest)