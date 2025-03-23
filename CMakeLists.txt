cmake_minimum_required(VERSION 3.10)

# Set the C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_EXE_LINKER_FLAGS "-static")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
# Set the project name
project(snakec)

# Add the executable
add_executable(snakec main.c)

# Link the standard library and runtime statically
