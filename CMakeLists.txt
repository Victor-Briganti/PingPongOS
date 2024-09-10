cmake_minimum_required(VERSION 3.10..3.28)

project(PingPongOS VERSION 0.0.1 LANGUAGES C)

# Compiler flags
# set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -Wconversion")
set(CMAKE_C_FLAGS "-DDEBUG -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -Wconversion")

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

# Define the log library
add_library(LogLib STATIC src/debug/log.c)
target_include_directories(LogLib PUBLIC include)

# Define the test executable for the queue
add_executable(QueueTest test/lib/queue_test.c)
target_include_directories(QueueTest PUBLIC include)
# Link the queue library with the test executable
target_link_libraries(QueueTest PRIVATE QueueLib)

# Define the ppos library
add_library(PingPongLib STATIC src/ppos.c)
target_include_directories(PingPongLib PUBLIC include)
# Link the queue library with the PingPongLib
target_link_libraries(PingPongLib PUBLIC QueueLib LogLib)

# Define the test executable for the task
add_executable(TaskTest test/pptask_test.c)
target_include_directories(TaskTest PUBLIC include)
# Link the PingPongOs with the task test executable
target_link_libraries(TaskTest PRIVATE PingPongLib)

# Define the test executable for the task
add_executable(TaskMaxTest test/pptask_max_test.c)
target_include_directories(TaskMaxTest PUBLIC include)
# Link the PingPongOs with the task test executable
target_link_libraries(TaskMaxTest PRIVATE PingPongLib)

# Add the test
add_test(NAME QueueTests COMMAND QueueTest)
add_test(NAME TaskTests COMMAND TaskTest TaskMaxTest)