#include <algo/routing.h>
#include <sys/ioctl.h>
#include <utils/Graph.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;
using namespace filesystem;
using json = nlohmann::json;

mutex streamMutex;

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

		if (id.length() != 24) {
			cerr << "Received bad opid '" << id << "'";
			return 1;
		}

		thread worker(processRouting, id);

		worker.detach();
	}

	return 0;
}

void processRouting(path id) {
	vector<arro::algo::data::CityLatLng> cities = readArray<arro::algo::data::CityLatLng>(id / "cities.json");
	vector<arro::algo::data::RouteReq> routes = readArray<arro::algo::data::RouteReq>(id / "routes.json");
	vector<arro::aviation::Plane> planes = readArray<arro::aviation::Plane>(id / "planes.json");

	arro::algo::Routing routing = arro::algo::findRoute(cities, routes, planes);

	json baseline = routing.baseline;
	json optimized = routing.route;

	ofstream bOut(id / "baseline.json");
	bOut << json{{"routing", baseline}, {"cost", routing.baselineCost}};
	bOut.close();

	ofstream oOut(id / "routing.json");
	oOut << json{{"routing", optimized}, {"cost", routing.routeCost}};
	oOut.close();

	lock_guard lock(streamMutex);
	cout << string(id) << endl;
}