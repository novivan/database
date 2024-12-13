cmake_minimum_required(VERSION 3.10)
project(MemoryDB)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

include_directories(include)

file(GLOB SOURCES src/*.cpp)

add_executable(memorydb ${SOURCES}
        include/conditional_execute.h)

set(TEST_SOURCES tests/test.cpp
        include/conditional_execute.h)

find_package(GTest REQUIRED)

add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests GTest::gtest GTest::gtest_main pthread)

enable_testing()

add_test(NAME DatabaseTests COMMAND tests)