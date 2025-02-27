#ifndef AVIATION_H
#define AVIATION_H

#include <math/Vector3D.h>

namespace arro {
namespace aviation {
extern const double CRUISING_ALTITUDE;
extern const double TAKEOFF_PITCH;
extern const double LANDING_PITCH;
extern const double CLIMB_LATERAL;
extern const double CLIMB_DISTANCE;
extern const double DESCENT_LATERAL;
extern const double DESCENT_DISTANCE;
extern const double FULL_CLIMB_LATERAL;

double flightDistance(const Vector3D& from, const Vector3D& to);
}  // namespace aviation
}  // namespace arro

#endif