cmake_minimum_required(VERSION 3.11)

Include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

project(analyzer)

add_executable(daemon analyzer/drivers/daemon.cpp)
set_target_properties(daemon PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)

add_executable(graph analyzer/drivers/tests/graph.cpp)
set_target_properties(graph PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/tests)
target_link_libraries(graph PRIVATE Catch2::Catch2WithMain)