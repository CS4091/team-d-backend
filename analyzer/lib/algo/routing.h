#ifndef ROUTING_H
#define ROUTING_H

#include <math/aviation.h>
#include <math/geospatial.h>
#include <utils/Graph.h>
#include <utils/concepts.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "dijkstra.h"
#include "fw.h"

namespace arro {
namespace algo {
namespace data {
struct Runway {
	std::string name;
	double length;
	double width;
	bool lighted;

	Runway(const nlohmann::json& obj) : name(obj["name"]), length(obj["length"]), width(obj["width"]), lighted(obj["lighted"]) {}
};

struct AirportLatLng {
	std::string id;
	std::string city;
	std::string type;
	double lat;
	double lng;
	double fuel;
	std::vector<Runway> runways;

	AirportLatLng(const nlohmann::json& obj);

	static nlohmann::json stringify(const AirportLatLng& airport);
};

struct CityLatLng {
	std::string id;
	std::string name;
	double lat;
	double lng;

	CityLatLng(const nlohmann::json& obj) : id(obj["name"]), name(obj["name"]), lat(obj["lat"]), lng(obj["lng"]) {}
};

struct RouteReq {
	std::string from;
	std::string to;

	RouteReq(const nlohmann::json& obj) : from(obj["from"]), to(obj["to"]) {}
};

struct AirwayData {
	aviation::FlightData data;
	double fuelPrice;

	double cost(unsigned int passengers) const;

	static nlohmann::json stringify(const AirwayData& airway);
};
}  // namespace data

namespace __routing {
struct RoutePlan {
	std::map<std::string, std::list<const typename arro::Graph<data::AirportLatLng, data::AirwayData>::Node*>> route;
	std::vector<data::RouteReq> remaining;
	double cost;
};

struct PotentialFlight {
	aviation::FlightData data;
	std::string from;
	std::string to;
	double costApprox;
};

struct PlaneLoc {
	aviation::Plane plane;
	const typename arro::Graph<data::AirportLatLng, data::AirwayData>::Node* loc;
	double time;
};

bool operator>(const RoutePlan& a, const RoutePlan& b);

bool operator>(const PotentialFlight& a, const PotentialFlight& b);

bool operator>(const PlaneLoc& a, const PlaneLoc& b);
}  // namespace __routing

struct Routing {
	std::map<std::string, std::list<std::string>> route;
	std::map<std::string, std::list<std::string>> baseline;
};

Routing findRoute(const std::vector<data::AirportLatLng>& airports, const std::vector<data::CityLatLng>& cities,
				  const std::vector<data::RouteReq>& requestedRoutes, const std::vector<aviation::Plane>& planes);

std::map<std::string, arro::Graph<data::AirportLatLng, data::AirwayData>> mapFlights(const std::vector<data::AirportLatLng>& airports,
																					 const std::vector<aviation::Plane>& planes);

void recluster(arro::Graph<data::AirportLatLng, data::AirwayData>& graph, const aviation::Plane& plane);
}  // namespace algo
}  // namespace arro

#endif