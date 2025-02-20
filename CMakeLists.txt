cmake_minimum_required(VERSION 3.11)

Include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FetchContent_Declare(
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG        v3.4.0 # or a later release
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
	json
	GIT_REPOSITORY	https://github.com/nlohmann/json.git
	GIT_TAG			v3.11.3
)
FetchContent_MakeAvailable(json)

project(analyzer)

add_executable(daemon analyzer/drivers/daemon.cpp)
set_target_properties(daemon PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(daemon PRIVATE nlohmann_json::nlohmann_json)
target_sources(daemon PRIVATE analyzer/lib/Vector3D.cpp)

add_executable(demo analyzer/drivers/demo.cpp)
set_target_properties(demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(demo PRIVATE nlohmann_json::nlohmann_json)
target_sources(demo PRIVATE analyzer/lib/Vector3D.cpp)

add_executable(graph analyzer/drivers/tests/graph.cpp)
set_target_properties(graph PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin/tests)
target_link_libraries(graph PRIVATE Catch2::Catch2WithMain)
target_link_libraries(graph PRIVATE nlohmann_json::nlohmann_json)
target_sources(graph PRIVATE analyzer/lib/Vector3D.cpp)