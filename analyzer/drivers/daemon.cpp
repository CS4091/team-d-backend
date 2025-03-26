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

struct AirportLatLng : public arro::JSONStruct {
	arro::JSONStruct::Field<string> id;
	arro::JSONStruct::Field<string> city;
	arro::JSONStruct::Field<double> lat;
	arro::JSONStruct::Field<double> lng;

	AirportLatLng(const json& obj) : arro::JSONStruct(obj), id(this, "id"), city(this, "city"), lat(this, "lat"), lng(this, "lng") {}
};

struct CityLatLng : public arro::JSONStruct {
	arro::JSONStruct::Field<string> id;
	arro::JSONStruct::Field<string> name;
	arro::JSONStruct::Field<double> lat;
	arro::JSONStruct::Field<double> lng;

	CityLatLng(const json& obj) : arro::JSONStruct(obj), id(this, "name"), name(this, "name"), lat(this, "lat"), lng(this, "lng") {}
};

struct RouteReq : public arro::JSONStruct {
	arro::JSONStruct::Field<string> from;
	arro::JSONStruct::Field<string> to;

	RouteReq(const json& obj) : arro::JSONStruct(obj), from(this, "from"), to(this, "to") {}
};

struct Plane : public arro::JSONStruct {
	arro::JSONStruct::Field<string> id;
	arro::JSONStruct::Field<string> homeBase;
	arro::JSONStruct::Field<double> takeoffRunway;
	arro::JSONStruct::Field<double> landingRunway;

	Plane(const json& obj)
		: arro::JSONStruct(obj), id(this, "id"), homeBase(this, "homeBase"), takeoffRunway(this, "takeoffRunway"), landingRunway(this, "landingRunway") {}
};

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
	vector<AirportLatLng> airports = arro::JSONStruct::parseArray<AirportLatLng>(id / "airports.json");
	vector<CityLatLng> cities = arro::JSONStruct::parseArray<CityLatLng>(id / "cities.json");
	vector<RouteReq> routes = arro::JSONStruct::parseArray<RouteReq>(id / "routes.json");
	vector<Plane> planes = arro::JSONStruct::parseArray<Plane>(id / "planes.json");
}