#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

#include "../lib/Graph.h"
#include "../lib/Vector3D.h"

using namespace std;
using namespace numbers;
using json = nlohmann::json;

const double EARTH_R = 6.378e6;

struct City {
	string id;
};

struct MapCity : public City {
	double lat;
	double lng;

	static json stringify(const MapCity& city);
	static MapCity parse(const string& str);
};

struct GraphCity : public City {
	double x;
	double y;

	static json stringify(const GraphCity& city);
	static GraphCity parse(const string& str);
};

istream& operator>>(istream& in, MapCity& city);

int main(int argc, char* argv[]) {
	string path = argc > 1 ? argv[1] : "test.graph";

	arro::Graph<MapCity, int> graph = arro::Graph<MapCity, int>::readFromFile(path);

	return 0;
}