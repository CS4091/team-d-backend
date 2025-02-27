#include "geospatial.h"

using namespace std;
using namespace arro;
using namespace numbers;

const double geospatial::EARTH_R = 6.378e6;

Vector3D geospatial::llToRect(double lat, double lng) {
	double rlat = lat * pi / 180, rlng = lng * pi / 180;

	// assume earth is a sphere, and just perform spherical to rectangular conversion (even though we know the earth is obviously flat)
	return arro::Vector3D(EARTH_R * cos(lat) * cos(lng), EARTH_R * cos(lat) * sin(lng), EARTH_R * sin(lat));
}