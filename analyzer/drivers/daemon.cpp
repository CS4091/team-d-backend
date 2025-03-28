#include <algo/routing.h>
#include <sys/ioctl.h>
#include <utils/Graph.h>
#include <utils/JSONStruct.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

using namespace std;
using namespace filesystem;
using json = nlohmann::json;

void processRouting(path id);

int main() {
	string id;

	while (true) {
		getline(cin, id);

		if (id.length() != 12) {
			cerr << "Received bad opid '" << id << "'";
			return 1;
		}

		thread worker(processRouting, id);

		worker.detach();
	}

	return 0;
}

void processRouting(path id) {
	vector<arro::algo::data::AirportLatLng> airports = arro::JSONStruct::parseArray<arro::algo::data::AirportLatLng>(id / "airports.json");
	vector<arro::algo::data::CityLatLng> cities = arro::JSONStruct::parseArray<arro::algo::data::CityLatLng>(id / "cities.json");
	vector<arro::algo::data::RouteReq> routes = arro::JSONStruct::parseArray<arro::algo::data::RouteReq>(id / "routes.json");
	vector<arro::aviation::Plane> planes = arro::JSONStruct::parseArray<arro::aviation::Plane>(id / "planes.json");
}