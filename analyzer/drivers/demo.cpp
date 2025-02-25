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

	arro::Vector3D avg(0, 0, 0);
	for (auto node : graph.nodes()) {
		double lat = node->data().lat * pi / 180, lng = node->data().lng * pi / 180;

		// assume earth is a sphere, and just perform spherical to rectangular conversion (even though we know the earth is obviously flat)
		avg += arro::Vector3D(EARTH_R * cos(lat) * cos(lng), EARTH_R * cos(lat) * sin(lng), EARTH_R * sin(lat));
	}

	// avg is now the normal vector to the "average plane" of the city points
	avg /= graph.nodes().size();

	arro::Vector3D north = arro::Vector3D(0, 0, 1), east = arro::Vector3D(1, 0, 0);
	// project north and east into the plane (will serve as y and x respectively)
	north = north - (north % avg);
	east = east - (east % avg);

	arro::Graph<GraphCity, int> flatGraph = graph.map<GraphCity>([&avg, &north, &east](const MapCity& city) {
		double lat = city.lat * pi / 180, lng = city.lng * pi / 180;

		arro::Vector3D spaceVec = arro::Vector3D(EARTH_R * cos(lat) * cos(lng), EARTH_R * cos(lat) * sin(lng), EARTH_R * sin(lat));
		arro::Vector3D planeVec = spaceVec - (spaceVec % avg);

		double x = planeVec[east], y = planeVec[north];

		return GraphCity{city.id, x, y};
	});
	flatGraph.jsonDumpToFile(path + ".json");

	return 0;
}

istream& operator>>(istream& in, MapCity& city) {
	in >> city.id >> city.lat >> city.lng;

	return in;
}

json MapCity::stringify(const MapCity& city) {
	return {{"id", city.id}, {"lat", city.lat}, {"lng", city.lng}};
}

MapCity MapCity::parse(const string& str) {
	json parsed = json::parse(str);

	if (!parsed["id"].is_string() || !parsed["lat"].is_number_float() || !parsed["lng"].is_number_float()) throw invalid_argument("Invalid MapCity json");

	return MapCity{parsed["id"].get<string>(), parsed["lat"].get<double>(), parsed["lng"].get<double>()};
}

json GraphCity::stringify(const GraphCity& city) {
	return {{"id", city.id}, {"x", city.x}, {"y", city.y}};
}

GraphCity GraphCity::parse(const string& str) {
	json parsed = json::parse(str);

	if (!parsed["id"].is_string() || !parsed["x"].is_number_float() || !parsed["y"].is_number_float()) throw invalid_argument("Invalid GraphCity json");

	return GraphCity{parsed["id"].get<string>(), parsed["x"].get<double>(), parsed["y"].get<double>()};
}