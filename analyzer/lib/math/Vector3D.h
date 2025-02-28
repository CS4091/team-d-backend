#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <cmath>

namespace arro {
class Vector3D;
}  // namespace arro

arro::Vector3D operator+(const arro::Vector3D& a, const arro::Vector3D& b);
arro::Vector3D operator-(const arro::Vector3D& a, const arro::Vector3D& b);
arro::Vector3D operator*(const arro::Vector3D& vec, double scale);
double operator*(const arro::Vector3D& a, const arro::Vector3D& b);
arro::Vector3D operator/(const arro::Vector3D& vec, double scale);
// vector projection of a onto b
arro::Vector3D operator%(const arro::Vector3D& a, const arro::Vector3D& b);

namespace arro {
class Vector3D {
public:
	Vector3D(double x, double y, double z) : _x(x), _y(y), _z(z) {}

	Vector3D(const Vector3D& other) : _x(other._x), _y(other._y), _z(other._z) {}
	Vector3D& operator=(const Vector3D& other);

	double magnitude() const { return std::sqrt(_x * _x + _y * _y + _z * _z); }

	friend Vector3D(::operator+)(const Vector3D& a, const Vector3D& b);

	Vector3D& operator+=(const Vector3D& other);

	friend Vector3D(::operator-)(const Vector3D& a, const Vector3D& b);

	Vector3D& operator-=(const Vector3D& other);

	friend Vector3D(::operator*)(const Vector3D& vec, double scale);
	friend double(::operator*)(const Vector3D& a, const Vector3D& b);

	Vector3D& operator*=(double scale);

	friend Vector3D(::operator/)(const Vector3D& vec, double scale);

	Vector3D& operator/=(double scale);

	// vector projection of a onto b
	friend Vector3D(::operator%)(const Vector3D& a, const Vector3D& b);

	Vector3D& operator%=(const Vector3D& other);

	Vector3D operator()() const;

	double operator[](int idx) const;

	// scalar projection of *this onto other
	double operator[](const Vector3D& other) const;

	static Vector3D midpoint(const Vector3D& a, const Vector3D& b);

	static Vector3D cross(const Vector3D& a, const Vector3D& b);

private:
	double _x, _y, _z;
};
}  // namespace arro

#endif