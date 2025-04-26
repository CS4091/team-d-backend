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
#include <iomanip>
#include <iostream>
#include <list>
#include <numbers>
#include <queue>
#include <string>
#include <vector>

using namespace std;
using namespace numbers;
using json = nlohmann::json;

template <JSONConstructable T>
vector<T> readArray(const string& path) {
	ifstream in(path);
	vector<json> arr = json::parse(in);
	in.close();

	vector<T> out;
	copy(arr.begin(), arr.end(), back_inserter(out));

	return out;
}

int main(int argc, char* argv[]) {
	vector<arro::algo::data::CityLatLng> cities = readArray<arro::algo::data::CityLatLng>("cities.json");
	vector<arro::algo::data::RouteReq> routes = readArray<arro::algo::data::RouteReq>("routes.json");
	vector<arro::aviation::Plane> planes = readArray<arro::aviation::Plane>("planes.json");
	vector<arro::algo::data::AirportWithRunways> airports = readArray<arro::algo::data::AirportWithRunways>("airports.json");

	for (auto& req : routes) {
		auto from = find_if(airports.begin(), airports.end(), [&req](const arro::algo::data::AirportWithRunways& airport) { return airport.id == req.from; }),
			 to = find_if(airports.begin(), airports.end(), [&req](const arro::algo::data::AirportWithRunways& airport) { return airport.id == req.to; });

		auto fromLoc = arro::geospatial::llToRect(from->lat, from->lng), toLoc = arro::geospatial::llToRect(to->lat, to->lng);

		req.approxCost =
			((toLoc - fromLoc).magnitude()) * arro::aviation::FUEL_ECONOMY * from->fuel + req.passengers * arro::aviation::FE_PER_PASSENGER * from->fuel;
	}

	arro::algo::Routing routing = arro::algo::findRoute(cities, routes, planes);

	cout << "Baseline:\n";
	for (auto [id, route] : routing.baseline) {
		cout << "Plane[" << id << "]: " << route.front();
		for (auto it = next(route.begin()); it != route.end(); it++) cout << " -> " << *it;
		cout << endl;
	}

	cout << "cost: $" << put_money(routing.baselineCost) << endl;

	cout << "\nOptimized:\n";
	for (auto [id, route] : routing.route) {
		cout << "Plane[" << id << "]: " << route.front();
		for (auto it = next(route.begin()); it != route.end(); it++) cout << " -> " << *it;
		cout << endl;
	}

	cout << "cost: $" << put_money(routing.routeCost) << endl;

	return 0;
}
