#include "quaternion.h"
#include <ostream>
#include <cmath>

using std::ostream;

Quaternion::Quaternion(double w_, double x_, double y_, double z_) : w(w_), x(x_), y(y_), z(z_) {}

Quaternion Quaternion::operator*(const Quaternion& q) const {
	return {
		w * q.w - x * q.x - y * q.y - z * q.z,
		w * q.x + x * q.w + y * q.z - z * q.y,
		w * q.y - x * q.z + y * q.w + z * q.x,
		w * q.z + x * q.y - y * q.x + z * q.w
	};
}

Quaternion Quaternion::conjugate() const // Equivalent to inverse for unit-length quat
{
	return { w, -x, -y, -z };
}

void Quaternion::normalise()
{
	float mag = sqrt(w * w + x * x + y * y + z * z);
	if (mag == 0.0f) return; // Avoid division by zero

	w /= mag;
	x /= mag;
	y /= mag;
	z /= mag;
}

ostream& operator<<(ostream& os, const Quaternion& q)
{
	return os << "[" << q.w << " " << q.x << " " << q.y << " " << q.z << "]";
}

