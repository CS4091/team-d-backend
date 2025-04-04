#ifndef GEOSPATIAL_H
#define GEOSPATIAL_H

#include <numbers>

#include "Vector3D.h"

namespace arro {
namespace geospatial {
extern const double EARTH_R;

Vector3D llToRect(double lat, double lng);

Vector3D mercator(double lat, double lng);
}  // namespace geospatial
}  // namespace arro

#endif