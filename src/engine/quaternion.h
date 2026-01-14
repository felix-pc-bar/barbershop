#pragma once

#include <ostream>

class Position3d;

class Quaternion // fml
{
public:
	float w, x, y, z;
	Quaternion(); // 0 rotation
	Quaternion(float angle, const Position3d& axis);// Use position3d vector as axis
	Quaternion(double w_, double x_, double y_, double z_);

	Quaternion operator*(const Quaternion& q) const;
	Quaternion conjugate() const;
	void normalise();

	friend std::ostream& operator<<(std::ostream& os, const Quaternion& q);
};