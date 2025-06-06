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

link_libraries(nlohmann_json::nlohmann_json)
include_directories(analyzer/lib)

OPTION(bench "Add benchmarking" OFF)
IF(bench)
	ADD_DEFINITIONS(-DBENCHMARK)
ENDIF(bench)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin/lib)

set(LIB "analyzer/lib")

add_library(
	math SHARED
	"${LIB}/math/Vector3D.cpp"
	"${LIB}/math/aviation.cpp"
	"${LIB}/math/geospatial.cpp"
)

add_library(
	algo SHARED
	"${LIB}/algo/fw.cpp"
	"${LIB}/algo/routing.cpp"
	"${LIB}/algo/exceptions.cpp"
)
target_link_libraries(algo math)

add_library(
	utils SHARED
	"${LIB}/utils/bench.cpp"
)

add_executable(daemon analyzer/drivers/daemon.cpp)
target_link_libraries(daemon math algo utils)

add_executable(demo analyzer/drivers/demo.cpp)
target_link_libraries(demo math algo utils)

add_executable(mapgen analyzer/drivers/mapgen.cpp)
target_link_libraries(mapgen math algo utils)

add_executable(read-map analyzer/drivers/read-map.cpp)
target_link_libraries(read-map math algo utils)

add_executable(graph analyzer/drivers/tests/graph.cpp)
target_link_libraries(graph PRIVATE Catch2::Catch2WithMain math algo utils)
set_target_properties(graph PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/tests)