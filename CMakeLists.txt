cmake_minimum_required(VERSION 3.10..3.28)

project(PingPongOS VERSION 0.0.1 LANGUAGES C)

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

# Compilation
add_executable(${PROJECT_NAME} src/main.c)
target_include_directories(${PROJECT_NAME} PUBLIC include)
