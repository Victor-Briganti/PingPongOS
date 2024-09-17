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

# Define the ADT Task library
add_library(TaskADT STATIC src/adt/pptask_manager.c)
target_include_directories(TaskADT PUBLIC include)
# Link the queue library with the ADT Task
target_link_libraries(TaskADT PRIVATE QueueLib)

# Define the test executable for the queue
add_executable(QueueTest test/lib/queue_test.c)
target_include_directories(QueueTest PUBLIC include)
# Link the queue library with the test executable
target_link_libraries(QueueTest PRIVATE QueueLib)

# Define the ppos library
add_library(PingPongLib STATIC src/ppos_core.c)
target_include_directories(PingPongLib PUBLIC include)
# Link the queue library with the PingPongLib
target_link_libraries(PingPongLib PUBLIC QueueLib LogLib TaskADT)

# Define the test executable for the task(simple)
add_executable(TaskTest test/tasks/pptask_test.c)
target_include_directories(TaskTest PUBLIC include)
# Link the PingPongOs with the task test executable
target_link_libraries(TaskTest PRIVATE PingPongLib)

# Define the test executable for the task(max)
add_executable(TaskMaxTest test/tasks/pptask_max_test.c)
target_include_directories(TaskMaxTest PUBLIC include)
# Link the PingPongOs with the task test executable
target_link_libraries(TaskMaxTest PRIVATE PingPongLib)

# Define the test executable for the task(max sequential)
add_executable(TaskMaxSeqTest test/tasks/pptask_max_seq_test.c)
target_include_directories(TaskMaxSeqTest PUBLIC include)
# Link the PingPongOs with the task test executable
target_link_libraries(TaskMaxSeqTest PRIVATE PingPongLib)

# Define the test executable for the dispatcher
add_executable(DispatcherTest test/dispatcher/ppdisp.c)
target_include_directories(DispatcherTest PUBLIC include)
# Link the PingPongOs with the dispatcher test executable
target_link_libraries(DispatcherTest PRIVATE PingPongLib)

# Define the test executable for the scheduler
add_executable(SchedulerTest test/scheduler/ppschedule.c)
target_include_directories(DispatcherTest PUBLIC include)
# Link the PingPongOs with the scheduler test executable
target_link_libraries(SchedulerTest PRIVATE PingPongLib)

# Define the test executable for the timer interrupt
add_executable(TimerIntTest test/timer/pptimer_int.c)
target_include_directories(TimerIntTest PUBLIC include)
# Link the PingPongOs with the timer interrupt test executable
target_link_libraries(TimerIntTest PRIVATE PingPongLib)

# Define the test executable for system timer
add_executable(TimerTest test/timer/pptimer.c)
target_include_directories(TimerTest PUBLIC include)
# Link the PingPongOs with the system timer test executable
target_link_libraries(TimerTest PRIVATE PingPongLib)

# Define the test executable for system timer with interrupt
add_executable(TimerPrioTest test/timer/pptimer_prio.c)
target_include_directories(TimerPrioTest PUBLIC include)
# Link the PingPongOs with the system timer with priority test executable
target_link_libraries(TimerPrioTest PRIVATE PingPongLib)

# Define the test executable for waiting a task
add_executable(WaitTest test/wait/ppwait.c)
target_include_directories(WaitTest PUBLIC include)
# Link the PingPongOs with the system timer with priority test executable
target_link_libraries(WaitTest PRIVATE PingPongLib)

# Define the test executable for sleeping a task
add_executable(SleepTest test/sleep/ppsleep.c)
target_include_directories(SleepTest PUBLIC include)
# Link the PingPongOs with the sleep test
target_link_libraries(SleepTest PRIVATE PingPongLib)

# Add the test
add_test(NAME QueueTests COMMAND QueueTest)
add_test(NAME TaskTests COMMAND TaskTest TaskMaxTest TaskMaxSeqTest)
add_test(NAME DispatcherTests COMMAND DispatcherTest)
add_test(NAME SchedulerTests COMMAND SchedulerTest)
add_test(NAME TimerTests COMMAND TimerIntTest TimerTest TimerPrioTest)  
add_test(NAME WaitTests COMMAND WaitTest)  
add_test(NAME SleepTests COMMAND SleepTest)  