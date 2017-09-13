# General CMakeLists.txt file for building MortyChess
# See https://github.com/nscorley/morty-chess for more information

# force version cmake >= 3.9
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# set project name
set (PROJECT_NAME "morty-chess" CXX)
project (${PROJECT_NAME})

# build compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMAND 1)

# use C++11
set (CMAKE_CXX_STANDARD 11)

# boost configuration
set(Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)

# find boost packages
find_package(Boost 1.61.0 COMPONENTS system filesystem REQUIRED)

# prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR lib/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})
find_package(Threads)

# bring the headers into the project
include_directories(core/include)

# glob core sources and add to SOURCES variable
file(GLOB SOURCES "core/src/*.cpp")

# glob tests and add to TESTS variable
file(GLOB TESTS "core/test/*.cpp")

# glob main app information and add to APP variable
file(GLOB APP "app/src/*.cpp")

# create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# add executable
add_executable(tests ${TESTS})
add_executable(app ${APP})

# link libraries
target_link_libraries (tests ${Boost_LIBRARIES} ${PROJECT_NAME} Catch)
target_link_libraries (app ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
