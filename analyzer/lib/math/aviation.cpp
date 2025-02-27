#include "aviation.h"

#include <cmath>
#include <numbers>

using namespace std;
using namespace arro;

const double aviation::CRUISING_ALTITUDE = 30'000 * 0.3048;		// flight level 3000
const double aviation::TAKEOFF_PITCH = 10 * numbers::pi / 180;	// 10deg takeoff climb
const double aviation::LANDING_PITCH = -3 * numbers::pi / 180;	// 3deg final approach

// lateral distance covered on a climb to flight level 3000 (also stdlib doesn't have cotangent for some reason??)
const double aviation::CLIMB_LATERAL = CRUISING_ALTITUDE * (1 / tan(TAKEOFF_PITCH));
const double aviation::CLIMB_DISTANCE = CRUISING_ALTITUDE * (1 / sin(TAKEOFF_PITCH));
// lateral distance covered on a descent from flight level 3000
const double aviation::DESCENT_LATERAL = -CRUISING_ALTITUDE * (1 / tan(LANDING_PITCH));
const double aviation::DESCENT_DISTANCE = -CRUISING_ALTITUDE * (1 / sin(LANDING_PITCH));

// lateral distance covered assuming a full climb to flight level 3000 followed by descent
const double aviation::FULL_CLIMB_LATERAL = CLIMB_LATERAL + DESCENT_LATERAL;

double aviation::flightDistance(const Vector3D& from, const Vector3D& to) {
	double lateralDistance = (to - from).magnitude();

	if (lateralDistance < FULL_CLIMB_LATERAL) {
		// a/sin(A) = b/sin(B) = c/sin(C)
		double lateralRatio = lateralDistance / sin((180 - 10 - 3) * numbers::pi / 180);

		double climbDistance = lateralRatio * sin(TAKEOFF_PITCH), descentDistance = lateralRatio * sin(LANDING_PITCH);

		return climbDistance + descentDistance;
	} else {
		double cruiseDistance = lateralDistance - CLIMB_LATERAL - DESCENT_LATERAL;

		return CLIMB_DISTANCE + cruiseDistance + DESCENT_DISTANCE;
	}
}