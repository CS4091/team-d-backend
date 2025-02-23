#include "Vector3D.h"

using namespace arro;

Vector3D& Vector3D::operator=(const Vector3D& other) 
{
  _x = other._x;
	_y = other._y;
	_z = other._z;
  
  return *this;
}

Vector3D operator+(const Vector3D& a,  const Vector3D& b)
{
  return Vector3D(a._x + b._x, a._y + b._y, a._z + b._z);
}

Vector3D& Vector3D::operator+=(const Vector3D& other) 
{
	_x += other._x;
	_y += other._y;
	_z += other._z;

	return *this;
}

Vector3D operator-(const Vector3D& a,  const Vector3D& b)
{
  return Vector3D(a._x - b._x, a._y - b._y, a._z - b._z);
}

Vector3D& Vector3D::operator-=(const Vector3D& other) 
{
	_x -= other._x;
	_y -= other._y;
	_z -= other._z;

	return *this;
}

Vector3D operator*(const Vector3D& vec, double scale) 
{
	return Vector3D(vec._x * scale, vec._y * scale, vec._z * scale);
}

double operator*(const Vector3D& a, const Vector3D& b)
{
  return a._x * b._x + a._y * b._y + a._z * b._z;
}

Vector3D& Vector3D::operator*=(double scale) 
{
  _x *= scale;
	_y *= scale;
	_z *= scale;

  return *this;
}

Vector3D operator/(const Vector3D& vec, double scale) 
{
	return Vector3D(vec._x / scale, vec._y / scale, vec._z / scale);
}

Vector3D& Vector3D::operator/=(double scale) 
{
  _x /= scale;
	_y /= scale;
	_z /= scale;

  return *this;
}

Vector3D operator%(const Vector3D& a, const Vector3D& b)
{
  return b()  * (a * b)  / b.magnitude();
}

Vector3D& Vector3D::operator%=(const Vector3D& other) 
{
	return (*this = *this % other);
}

Vector3D Vector3D::operator()() const 
{
	return *this / magnitude();
}

double Vector3D::operator[](const Vector3D&  other) const
{
  return (*this * other) / other.magnitude();
}