#include "engTools.h"
#include <cmath>

using std::vector, std::cout, std::endl, std::sin, std::cos;

Scene* currentScene = nullptr;


Vertex3d::Vertex3d(Position3d pos)
{
	this->position = pos;
}


Position3d operator+(const Position3d& p1, const Position3d& p2) { return Position3d(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z); }
Position3d operator-(const Position3d& p1, const Position3d& p2) { return Position3d(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z); }
Position3d operator*(const Position3d& p1, const Position3d& p2) { return Position3d(p1.x * p2.x, p1.y * p2.y, p1.z * p2.z); }
Position3d operator/(const Position3d& p1, const float div) { return Position3d(p1.x / div, p1.y / div, p1.z / div); }
bool operator==(const Position3d& p1, const Position3d& p2) { return (p1.x == p2.x && p1.y == p2.y && p1.z == p2.z) ? true : false; }

Position3d& Position3d::operator+=(const Position3d& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

void Vertex3d::offsetPosition(Position3d offset) 
{
	this->position = this->position + offset;
}

void Vertex3d::rotatePosition(const Rotation3d& rot, const Position3d& pivot)
{
	this->position.rotateAroundPoint(rot, pivot);
}

Position3d::Position3d() //if not passed a position, we just put it at origin
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

Position3d::Position3d(double xPos, double yPos, double zPos)
{
	this->x = xPos;
	this->y = yPos;
	this->z = zPos;
}

// just the rotation matrices hardcoded (it's not like we're gonna want skew later on, right?)
void Position3d::rotateAroundPoint(const Rotation3d& rotation, const Position3d& pivot)
{
	// Translate to origin relative to pivot
	float x = this->x - pivot.x;
	float y = this->y - pivot.y;
	float z = this->z - pivot.z;

	// Rotate around Z
	float cosZ = cos(rotation.z);
	float sinZ = sin(rotation.z);
	float x1 = cosZ * x - sinZ * y;
	float y1 = sinZ * x + cosZ * y;

	// Rotate around Y
	float cosY = cos(rotation.y);
	float sinY = sin(rotation.y);
	float x2 = cosY * x1 + sinY * z;
	float z1 = -sinY * x1 + cosY * z;

	// Rotate around X
	float cosX = cos(rotation.x);
	float sinX = sin(rotation.x);
	float y2 = cosX * y1 - sinX * z1;
	float z2 = sinX * y1 + cosX * z1;

	// Translate back from origin
	this->x = x2 + pivot.x;
	this->y = y2 + pivot.y;
	this->z = z2 + pivot.z;
}


Position3d Position3d::cross(const Position3d& operand) const
{
	return Position3d(
		y * operand.z - z * operand.y,
		z * operand.x - x * operand.z,
		x * operand.y - y * operand.x
	);
}
float Position3d::dot(const Position3d& operand) const
{
	return x * operand.x * y * operand.y + z * operand.z;
}

void Position3d::normalise()
{
	float magnitude = std::sqrt(x * x + y * y + z * z); // find magnitude by pythaagoras
	if (magnitude > 0.0f)
	{
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}
}

Position3d Position3d::cameraspace()
{	
	Position3d cs;
	if (currentScene->currentCam != nullptr)
	{
		cs = *this - currentScene->currentCam->pos;
	}
	return cs;
}

//float Position3d::lengthSquared() const { return this->dot(*this); }
float Position3d::lengthSquared() const {
	return x * x + y * y + z * z; }

ostream& operator<< (ostream& os, Position3d pos)
{
	return os << "[" << pos.x << " " << pos.y << " " << pos.z << "]";
}


Mesh::Mesh() { 
	//this->addVertex(Position3d{ 0.0f, 1.0f, 0.5f });
	//this->addVertex(Position3d{ -0.5f, 1.0f, -0.5f });
	//this->addVertex(Position3d{ 0.5f, 1.0f, -0.5f });

	//indices = { 0, 1, 2 };
}

void Mesh::addVertex(Position3d pos)
{
	vertices.emplace_back(Vertex3d{pos});
}


void Mesh::addFace(vector<int>& ind)
{
	this->indices.insert(this->indices.end(), ind.begin(), ind.end());
}

void Mesh::move(Position3d offset)
{
	for (Vertex3d& vert : this->vertices)
	{
		vert.offsetPosition(offset);
	}
	this->position += offset;
}

void Mesh::setPos(Position3d pos)
{
	Position3d offset = pos - this->position; // find the offset that will change the current pos to the new one
	this->move(offset);
}

Rotation3d operator+(const Rotation3d& p1, const Rotation3d& p2) { return Rotation3d(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z); }
Rotation3d operator-(const Rotation3d& p1, const Rotation3d& p2) { return Rotation3d(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z); }
Rotation3d operator*(const Rotation3d& p1, const Rotation3d& p2) { return Rotation3d(p1.x * p2.x, p1.y * p2.y, p1.z * p2.z); }

Rotation3d& Rotation3d::operator+=(const Rotation3d& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

void Mesh::rotate(const Rotation3d& rot, const Position3d& pivot)
{
	for (Vertex3d& vert : this->vertices)
	{
		vert.rotatePosition(rot, pivot);
	}
	this->rotation += rot;
}

void Mesh::setRotation(const Rotation3d& rot, const Position3d& pivot)
{
	Rotation3d offset = rot - this->rotation; // find the offset that will change the current pos to the new one
	this->rotate(offset);
}

void Mesh::rotate(const Rotation3d& rot)
{
	for (Vertex3d& vert : this->vertices)
	{
		vert.rotatePosition(rot, this->position);
	}
	this->rotation += rot;
}

void Mesh::setRotation(const Rotation3d& rot)
{
	Rotation3d offset = rot - this->rotation; // find the offset that will change the current pos to the new one
	this->rotate(offset);
}

Rotation3d::Rotation3d() { x = 0; y = 0; z = 0; }

Rotation3d::Rotation3d(float x_, float y_, float z_)
{
	this->x = x_;
	this->y = y_;
	this->z = z_;
}

Camera* currentCam = nullptr;
