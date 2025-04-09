#ifndef AVIATION_H
#define AVIATION_H

#include <math/Vector3D.h>

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace arro {
namespace aviation {
extern const double FUEL_ECONOMY;		// [kg/m] ~3kg/km according to various aircraft manufacturers, according to Wikipedia
extern const double FE_PER_PASSENGER;	// [kg/mP] ~0.025kg/kmPassenger according to various aircraft manufacturers, according to Wikipedia
extern const double CRUISING_ALTITUDE;	// [m] flight level 3000 (30,000ft)
extern const double TAKEOFF_PITCH;		// [rad] 10deg takeoff + climb pitch
extern const double DESCENT_PITCH;		// [rad] 3deg final approach (but also used for main descent because no better numbers)

struct FlightPhase {
	double distance;
	double altitude;
	double lateral;
};

struct Plane {
	std::string id;
	std::string homeBase;
	std::string model;
	double range;
	double takeoffRunway;
	double landingRunway;
	double climb5kAirspeed;
	double climb15kAirspeed;
	double climb24kAirspeed;
	double cruiseAirspeed;
	double desc24kAirspeed;
	double desc10kAirspeed;
	double approachAirspeed;

	Plane(const nlohmann::json& obj)
		: id(obj.count("id") ? obj["id"] : ""),					   // needs to be optional for mapgen
		  homeBase(obj.count("homeBase") ? obj["homeBase"] : ""),  // needs to be optional for mapgen
		  model(obj["model"]),
		  range(obj["range"]),
		  takeoffRunway(obj["takeoffRunway"]),
		  landingRunway(obj["landingRunway"]),
		  climb5kAirspeed(obj["climb5kAirspeed"]),
		  climb15kAirspeed(obj["climb15kAirspeed"]),
		  climb24kAirspeed(obj["climb24kAirspeed"]),
		  cruiseAirspeed(obj["cruiseAirspeed"]),
		  desc24kAirspeed(obj["desc24kAirspeed"]),
		  desc10kAirspeed(obj["desc10kAirspeed"]),
		  approachAirspeed(obj["approachAirspeed"]) {}
};

struct FlightData {
	FlightPhase c5k;
	FlightPhase cfl150;
	FlightPhase cfl240;
	FlightPhase cCruise;
	FlightPhase cruise;
	FlightPhase dfl240;
	FlightPhase dfl100;
	FlightPhase land;
	double totalLateral;
	double minFuel;
	double time;
};

std::optional<FlightData> planFlight(const Plane& plane, const Vector3D& from, const Vector3D& to);
}  // namespace aviation
}  // namespace arro

#endif