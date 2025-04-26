#ifndef ROUTING_H
#define ROUTING_H

#include <math/aviation.h>
#include <math/geospatial.h>
#include <utils/Graph.h>
#include <utils/bench.h>
#include <utils/concepts.h>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

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

	AirportLatLng() {}
	AirportLatLng(const nlohmann::json& obj);

	static nlohmann::json stringify(const AirportLatLng& airport);
};

struct AirportWithRunways : public AirportLatLng {
	std::vector<Runway> runways;

	AirportWithRunways(const nlohmann::json& obj);

	static nlohmann::json stringify(const AirportWithRunways& airport);
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
	unsigned int passengers;
	double approxCost;

	// approximate cost used for A* routing, default of 0 reverts to Dijkstra's
	RouteReq(const nlohmann::json& obj) : from(obj["from"]), to(obj["to"]), passengers(obj["passengers"]), approxCost(0) {}
};

struct AirwayData {
	aviation::FlightData data;
	double fuelPrice;

	double cost(unsigned int passengers) const;

	static nlohmann::json stringify(const AirwayData& airway);
};

struct ReducedAirwayData {
	double minFuel;
	double fuelPrice;
	double time;
	double lateral;

	double cost(unsigned int passengers) const;

	static nlohmann::json stringify(const ReducedAirwayData& airway);
};
}  // namespace data

namespace __routing {
struct PlaneLoc {
	aviation::Plane plane;
	const typename arro::Graph<data::AirportLatLng, data::ReducedAirwayData>::Node* loc;
	double time;
};

bool operator>(const PlaneLoc& a, const PlaneLoc& b);

struct RoutePlan {
	std::map<std::string, std::list<const typename arro::Graph<data::AirportLatLng, data::ReducedAirwayData>::Node*>> route;
	std::priority_queue<PlaneLoc, std::vector<PlaneLoc>, std::greater<PlaneLoc>> planeOrder;
	std::vector<data::RouteReq> remaining;
	double cost;
	double reqCost;
};

struct PotentialFlight {
	aviation::FlightData data;
	std::string from;
	std::string to;
	double costApprox;
};

bool operator>(const RoutePlan& a, const RoutePlan& b);

bool operator>(const PotentialFlight& a, const PotentialFlight& b);

struct PlannedFlight {
	std::string plane;

	static nlohmann::json stringify(const PlannedFlight& flight);
};
}  // namespace __routing

struct Routing {
	std::map<std::string, std::list<std::string>> route;
	std::map<std::string, std::list<std::string>> baseline;

	double baselineCost;
	double routeCost;
};

Routing findRoute(const std::vector<data::CityLatLng>& cities, const std::vector<data::RouteReq>& requestedRoutes,
				  const std::vector<aviation::Plane>& planes);
}  // namespace algo
}  // namespace arro

#endif