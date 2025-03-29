#ifndef ROUTING_H
#define ROUTING_H

#include <math/aviation.h>
#include <math/geospatial.h>
#include <utils/Graph.h>
#include <utils/JSONStruct.h>
#include <utils/concepts.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "dijkstra.h"
#include "fw.h"

namespace arro {
namespace algo {
namespace data {
struct Runway : public arro::JSONStruct {
	arro::JSONStruct::Field<std::string> name;
	arro::JSONStruct::Field<double> length;
	arro::JSONStruct::Field<double> width;
	arro::JSONStruct::Field<bool> lighted;

	Runway(const nlohmann::json& obj) : arro::JSONStruct(obj), name(this, "name"), length(this, "length"), width(this, "width"), lighted(this, "lighted") {}
	Runway(const Runway& other) : arro::JSONStruct(other), name(this, "name"), length(this, "length"), width(this, "width"), lighted(this, "lighted") {}
};

struct AirportLatLng : public arro::JSONStruct {
	arro::JSONStruct::Field<std::string> id;
	arro::JSONStruct::Field<std::string> city;
	arro::JSONStruct::Field<std::string> type;
	arro::JSONStruct::Field<double> lat;
	arro::JSONStruct::Field<double> lng;
	arro::JSONStruct::Field<double> fuel;
	arro::JSONStruct::Field<nlohmann::json> runways;

	AirportLatLng(const nlohmann::json& obj)
		: arro::JSONStruct(obj),
		  id(this, "id"),
		  city(this, "city"),
		  type(this, "type"),
		  lat(this, "lat"),
		  lng(this, "lng"),
		  fuel(this, "fuel"),
		  runways(this, "runways") {}
	AirportLatLng(const AirportLatLng& other)
		: arro::JSONStruct(other),
		  id(this, "id"),
		  city(this, "city"),
		  type(this, "type"),
		  lat(this, "lat"),
		  lng(this, "lng"),
		  fuel(this, "fuel"),
		  runways(this, "runways") {}

	static nlohmann::json stringify(const AirportLatLng& airport);
};

struct CityLatLng : public arro::JSONStruct {
	arro::JSONStruct::Field<std::string> id;
	arro::JSONStruct::Field<std::string> name;
	arro::JSONStruct::Field<double> lat;
	arro::JSONStruct::Field<double> lng;

	CityLatLng(const nlohmann::json& obj) : arro::JSONStruct(obj), id(this, "name"), name(this, "name"), lat(this, "lat"), lng(this, "lng") {}
	CityLatLng(const CityLatLng& other) : arro::JSONStruct(other), id(this, "name"), name(this, "name"), lat(this, "lat"), lng(this, "lng") {}
};

struct RouteReq : public arro::JSONStruct {
	arro::JSONStruct::Field<std::string> from;
	arro::JSONStruct::Field<std::string> to;

	RouteReq(const nlohmann::json& obj) : arro::JSONStruct(obj), from(this, "from"), to(this, "to") {}
	RouteReq(const RouteReq& other) : arro::JSONStruct(other), from(this, "from"), to(this, "to") {}
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
}  // namespace algo
}  // namespace arro

#endif