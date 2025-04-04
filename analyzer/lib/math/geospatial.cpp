#include "geospatial.h"

using namespace std;
using namespace arro;
using namespace numbers;

const double geospatial::EARTH_R = 6.378e6;

Vector3D geospatial::llToRect(double lat, double lng) {
	double rlat = lat * pi / 180, rlng = lng * pi / 180;

	// assume earth is a sphere, and just perform spherical to rectangular conversion (even though we know the earth is obviously flat)
	return arro::Vector3D(EARTH_R * cos(rlat) * cos(rlng), EARTH_R * cos(rlat) * sin(rlng), EARTH_R * sin(rlat));
}

Vector3D geospatial::mercator(double lat, double lng) {
	double x = EARTH_R * lng * pi / 180;
	double y = EARTH_R * log(tan(pi / 4 + (lat * pi / 180) / 2));

	return arro::Vector3D(x, y, 0);
}