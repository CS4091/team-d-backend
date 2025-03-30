#include <algo/routing.h>
#include <sys/ioctl.h>
#include <utils/Graph.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

using namespace std;
using namespace filesystem;
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
	vector<arro::algo::data::AirportLatLng> airports = readArray<arro::algo::data::AirportLatLng>(id / "airports.json");
	vector<arro::algo::data::CityLatLng> cities = readArray<arro::algo::data::CityLatLng>(id / "cities.json");
	vector<arro::algo::data::RouteReq> routes = readArray<arro::algo::data::RouteReq>(id / "routes.json");
	vector<arro::aviation::Plane> planes = readArray<arro::aviation::Plane>(id / "planes.json");
}