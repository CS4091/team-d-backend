#include "Vector3D.h"

using namespace arro;

Vector3D& Vector3D::operator=(const Vector3D& other) {
  _x = other._x;
	_y = other._y;
	_z = other._z;
  return *this;
}

Vector3D& Vector3D::operator+=(const Vector3D& other) {
	_x += other._x;
	_y += other._y;
	_z += other._z;

	return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D& other) {
	_x -= other._x;
	_y -= other._y;
	_z -= other._z;

	return *this;
}

Vector3D& Vector3D::operator*=(double scale) {
  _x *= scale;
	_y *= scale;
	_z *= scale;
  return *this;
}

Vector3D& Vector3D::operator/=(double scale) {
  _x /= scale;
	_y /= scale;
	_z /= scale;
  return *this;
}
