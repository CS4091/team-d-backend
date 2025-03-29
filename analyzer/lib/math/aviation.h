#ifndef AVIATION_H
#define AVIATION_H

#include <math/Vector3D.h>
#include <utils/JSONStruct.h>

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

struct Plane : public arro::JSONStruct {
	arro::JSONStruct::Field<std::string> id;
	arro::JSONStruct::Field<std::string> homeBase;
	arro::JSONStruct::Field<std::string> model;
	arro::JSONStruct::Field<double> range;
	arro::JSONStruct::Field<double> takeoffRunway;
	arro::JSONStruct::Field<double> landingRunway;
	arro::JSONStruct::Field<double> climb5kAirspeed;
	arro::JSONStruct::Field<double> climb15kAirspeed;
	arro::JSONStruct::Field<double> climb24kAirspeed;
	arro::JSONStruct::Field<double> cruiseAirspeed;
	arro::JSONStruct::Field<double> desc24kAirspeed;
	arro::JSONStruct::Field<double> desc10kAirspeed;
	arro::JSONStruct::Field<double> approachAirspeed;

	Plane(const nlohmann::json& obj)
		: arro::JSONStruct(obj),
		  id(this, "id"),
		  homeBase(this, "homeBase"),
		  model(this, "model"),
		  range(this, "range"),
		  takeoffRunway(this, "takeoffRunway"),
		  landingRunway(this, "landingRunway"),
		  climb5kAirspeed(this, "climb5kAirspeed"),
		  climb15kAirspeed(this, "climb15kAirspeed"),
		  climb24kAirspeed(this, "climb24kAirspeed"),
		  cruiseAirspeed(this, "cruiseAirspeed"),
		  desc24kAirspeed(this, "desc24kAirspeed"),
		  desc10kAirspeed(this, "desc10kAirspeed"),
		  approachAirspeed(this, "approachAirspeed") {}
	Plane(const Plane& other)
		: arro::JSONStruct(other),
		  id(this, "id"),
		  homeBase(this, "homeBase"),
		  model(this, "model"),
		  range(this, "range"),
		  takeoffRunway(this, "takeoffRunway"),
		  landingRunway(this, "landingRunway"),
		  climb5kAirspeed(this, "climb5kAirspeed"),
		  climb15kAirspeed(this, "climb15kAirspeed"),
		  climb24kAirspeed(this, "climb24kAirspeed"),
		  cruiseAirspeed(this, "cruiseAirspeed"),
		  desc24kAirspeed(this, "desc24kAirspeed"),
		  desc10kAirspeed(this, "desc10kAirspeed"),
		  approachAirspeed(this, "approachAirspeed") {}
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