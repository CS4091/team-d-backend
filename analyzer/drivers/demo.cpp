#include <algo/dijkstra.h>
#include <algo/fw.h>
#include <algo/routing.h>
#include <math/Vector3D.h>
#include <math/aviation.h>
#include <math/geospatial.h>
#include <utils/Graph.h>
#include <utils/rand.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <numbers>
#include <queue>
#include <string>
#include <vector>

using namespace std;
using namespace numbers;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
	vector<arro::algo::data::AirportLatLng> airports = arro::JSONStruct::parseArray<arro::algo::data::AirportLatLng>("airports.json");
	vector<arro::algo::data::CityLatLng> cities = arro::JSONStruct::parseArray<arro::algo::data::CityLatLng>("cities.json");
	vector<arro::algo::data::RouteReq> routes = arro::JSONStruct::parseArray<arro::algo::data::RouteReq>("routes.json");
	vector<arro::aviation::Plane> planes = arro::JSONStruct::parseArray<arro::aviation::Plane>("planes.json");

	cout << "Done reading inputs" << endl;
	cout << airports.size() << " airports" << endl;

	arro::algo::Routing routing = arro::algo::findRoute(airports, cities, routes, planes);

	cout << "Baseline:\n";
	for (auto [id, route] : routing.baseline) {
		cout << "Plane[" << id << "]: " << route.front();
		for (auto it = next(route.begin()); it != route.end(); it++) cout << " -> " << *it;
		cout << endl;
	}

	cout << "\nOptimized:\n";
	for (auto [id, route] : routing.route) {
		cout << "Plane[" << id << "]: " << route.front();
		for (auto it = next(route.begin()); it != route.end(); it++) cout << " -> " << *it;
		cout << endl;
	}

	return 0;
}
