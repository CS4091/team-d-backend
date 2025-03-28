#include "aviation.h"

#include <cmath>
#include <numbers>

using namespace std;
using namespace arro;
using json = nlohmann::json;

const double aviation::FUEL_ECONOMY = 3e-3;
const double aviation::FE_PER_PASSENGER = 2.5e-5;

const double aviation::CRUISING_ALTITUDE = 30'000 * 0.3048;
const double aviation::TAKEOFF_PITCH = 10 * numbers::pi / 180;
const double aviation::DESCENT_PITCH = -3 * numbers::pi / 180;

optional<aviation::FlightData> aviation::planFlight(const Plane& plane, const Vector3D& from, const Vector3D& to) {
	FlightData data;

	double lateralDistance = (to - from).magnitude();
	data.totalLateral = lateralDistance;
	data.minFuel = lateralDistance * FUEL_ECONOMY;
	data.time = 0;

	// initial climb to 5kft
	data.c5k.altitude = 5'000 * 0.3048;
	data.c5k.lateral = data.c5k.altitude / tan(TAKEOFF_PITCH);
	if (data.c5k.lateral > lateralDistance) return nullopt;
	lateralDistance -= data.c5k.lateral;
	data.c5k.distance = data.c5k.altitude / sin(TAKEOFF_PITCH);
	data.time += data.c5k.distance / plane.climb5kAirspeed();

	// initial climb to flight level 150 (15kft)
	data.cfl150.altitude = 10'000 * 0.3048;
	data.cfl150.lateral = data.cfl150.altitude / tan(TAKEOFF_PITCH);
	if (data.cfl150.lateral > lateralDistance) return nullopt;
	lateralDistance -= data.cfl150.lateral;
	data.cfl150.distance = data.cfl150.altitude / sin(TAKEOFF_PITCH);
	data.time += data.cfl150.distance / plane.climb15kAirspeed();

	// climb to FL 240 (24kft)
	data.cfl240.altitude = 9'000 * 0.3048;
	data.cfl240.lateral = data.cfl240.altitude / tan(TAKEOFF_PITCH);
	if (data.cfl240.lateral > lateralDistance) return nullopt;
	lateralDistance -= data.cfl240.lateral;
	data.cfl240.distance = data.cfl240.altitude / sin(TAKEOFF_PITCH);
	data.time += data.cfl240.distance / plane.climb24kAirspeed();

	// skip climbing to FL 300 for now (only calculate final climb/cruise if necessary)
	// descent to FL 100 (10kft)
	data.dfl100.altitude = -14'000 * 0.3048;
	data.dfl100.lateral = data.dfl100.altitude / tan(DESCENT_PITCH);
	if (data.dfl100.lateral > lateralDistance) return nullopt;
	lateralDistance -= data.dfl100.lateral;
	data.dfl100.distance = data.dfl100.altitude / sin(DESCENT_PITCH);
	data.time += data.dfl100.distance / plane.desc10kAirspeed();

	// landing
	data.land.altitude = -10'000 * 0.3048;
	data.land.lateral = data.land.altitude / tan(DESCENT_PITCH);
	if (data.land.lateral > lateralDistance) return nullopt;
	lateralDistance -= data.land.lateral;
	data.land.distance = data.land.altitude / sin(DESCENT_PITCH);
	data.time += data.land.distance / plane.approachAirspeed();

	double cruiseClimbDescLateral = (CRUISING_ALTITUDE - 24'000) * 0.3048 / tan(TAKEOFF_PITCH) + (24'000 - CRUISING_ALTITUDE) * 0.3048 / tan(DESCENT_PITCH);
	if (lateralDistance < cruiseClimbDescLateral) {
		// just cruise at FL 240
		data.cCruise = {0, 0, 0};
		data.dfl240 = {0, 0, 0};
		data.cruise = {lateralDistance, 0, lateralDistance};
		data.time += lateralDistance / plane.cruiseAirspeed();
	} else {
		// climb up to target crusing altitude (FL 300)
		data.cCruise.altitude = (CRUISING_ALTITUDE - 24'000) * 0.3048 / tan(TAKEOFF_PITCH);
		data.cCruise.lateral = data.cCruise.altitude / tan(TAKEOFF_PITCH);
		lateralDistance -= data.cCruise.lateral;
		data.cCruise.distance = data.cCruise.altitude / sin(TAKEOFF_PITCH);
		data.time +=
			data.cCruise.distance / plane.climb24kAirspeed();  // FIXME: should be "mach climb" airspeed field, but didn't scrape that, maybe fix later

		// initial descent from cruising to FL 240
		data.dfl240.altitude = (24'000 - CRUISING_ALTITUDE) * 0.3048 / tan(DESCENT_PITCH);
		data.dfl240.lateral = data.dfl240.altitude / tan(DESCENT_PITCH);
		lateralDistance -= data.dfl240.lateral;
		data.dfl240.distance = data.dfl240.altitude / sin(DESCENT_PITCH);
		data.time += data.dfl240.distance / plane.desc24kAirspeed();

		if (lateralDistance > plane.range()) return nullopt;
		data.cruise = {lateralDistance, 0, lateralDistance};
		data.time += lateralDistance / plane.cruiseAirspeed();
	}

	return data;
}
