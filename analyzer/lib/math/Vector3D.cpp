#include "Vector3D.h"

#include <stdexcept>
#include <utility>

using namespace arro;
using namespace std;

Vector3D& Vector3D::operator=(const Vector3D& other) {
	_x = other._x;
	_y = other._y;
	_z = other._z;

	return *this;
}

Vector3D operator+(const Vector3D& a, const Vector3D& b) {
	return Vector3D(a._x + b._x, a._y + b._y, a._z + b._z);
}

Vector3D& Vector3D::operator+=(const Vector3D& other) {
	_x += other._x;
	_y += other._y;
	_z += other._z;

	return *this;
}

Vector3D operator-(const Vector3D& a, const Vector3D& b) {
	return Vector3D(a._x - b._x, a._y - b._y, a._z - b._z);
}

Vector3D& Vector3D::operator-=(const Vector3D& other) {
	_x -= other._x;
	_y -= other._y;
	_z -= other._z;

	return *this;
}

Vector3D operator*(const Vector3D& vec, double scale) {
	return Vector3D(vec._x * scale, vec._y * scale, vec._z * scale);
}

double operator*(const Vector3D& a, const Vector3D& b) {
	return a._x * b._x + a._y * b._y + a._z * b._z;
}

Vector3D& Vector3D::operator*=(double scale) {
	_x *= scale;
	_y *= scale;
	_z *= scale;

	return *this;
}

Vector3D operator/(const Vector3D& vec, double scale) {
	return Vector3D(vec._x / scale, vec._y / scale, vec._z / scale);
}

Vector3D& Vector3D::operator/=(double scale) {
	_x /= scale;
	_y /= scale;
	_z /= scale;

	return *this;
}

Vector3D operator%(const Vector3D& a, const Vector3D& b) {
	return b() * (a * b) / b.magnitude();
}

Vector3D& Vector3D::operator%=(const Vector3D& other) {
	return (*this = *this % other);
}

Vector3D Vector3D::operator()() const {
	return *this / magnitude();
}

double Vector3D::operator[](int idx) const {
	if (idx > 2) throw out_of_range("Vector element access out of range");

	switch (idx) {
		case 0:
			return _x;
		case 1:
			return _y;
		case 2:
			return _z;
		default:
			unreachable();
	}
}

double Vector3D::operator[](const Vector3D& other) const {
	return (*this * other) / other.magnitude();
}

Vector3D Vector3D::midpoint(const Vector3D& a, const Vector3D& b) {
	return (a + b) / 2;
}

Vector3D Vector3D::cross(const Vector3D& a, const Vector3D& b) {
	double x = a[1] * b[2] - a[2] * b[1], y = -(a[0] * b[2] - a[2] * b[0]), z = a[0] * b[1] - a[1] * b[0];

	return Vector3D(x, y, z);
}