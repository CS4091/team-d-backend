#define CATCH_CONFIG_MAIN
#include <utils/Graph.h>

#include <catch2/catch_test_macros.hpp>
#include <string>

using namespace std;

TEST_CASE("Node") {
	SECTION("Node Data") {
		arro::Node<string> node("Hello World!");

		REQUIRE(node.data() == "Hello World!");
	}
}