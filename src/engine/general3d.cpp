#include <cmath>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "general3d.h"
#include "material.h"
#include "globals.h"
#include "quaternion.h"
#include "render/render.h"

using std::vector, std::cout, std::endl, std::sin, std::cos;

Scene* currentScene = nullptr;


Vertex3d::Vertex3d(Position3d pos)
{
	this->position = pos;
}


Position3d operator+(const Position3d& p1, const Position3d& p2) { return Position3d(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z); }
Position3d operator-(const Position3d& p1, const Position3d& p2) { return Position3d(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z); }
Position3d operator*(const Position3d& p1, const Position3d& p2) { return Position3d(p1.x * p2.x, p1.y * p2.y, p1.z * p2.z); }
Position3d operator*(const Position3d& p1, const float mpcand) { return Position3d(p1.x * mpcand, p1.y * mpcand, p1.z * mpcand); }
Position3d operator/(const Position3d& p1, const float div) { return Position3d(p1.x / div, p1.y / div, p1.z / div); }
bool operator==(const Position3d& p1, const Position3d& p2) { return (p1.x == p2.x && p1.y == p2.y && p1.z == p2.z) ? true : false; }
bool operator<(const Position3d& p1, const Position3d& p2) { return (p1.x < p2.x && p1.y < p2.y && p1.z < p2.z) ? true : false; }
bool operator>(const Position3d& p1, const Position3d& p2) { return (p1.x > p2.x && p1.y > p2.y && p1.z > p2.z) ? true : false; }

Position3d& Position3d::operator+=(const Position3d& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Position3d& Position3d::operator-=(const Position3d& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

void Vertex3d::offsetPosition(Position3d offset) 
{
	this->position = this->position + offset;
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

inline void Position3d::rotateQuat(const Quaternion& q) 
{
	// q assumed normalized
	const float ux = q.x;
	const float uy = q.y;
	const float uz = q.z;
	const float w = q.w;

	// t = 2 * (u * v)
	const float tx = 2.0f * (uy * z - uz * y);
	const float ty = 2.0f * (uz * x - ux * z);
	const float tz = 2.0f * (ux * y - uy * x);

	// v' = v + w * t + (u * t)
	x += w * tx + (uy * tz - uz * ty);
	y += w * ty + (uz * tx - ux * tz);
	z += w * tz + (ux * ty - uy * tx);
}

Point2d Position3d::project(Camera* cam, const cRenderer* renderer)
{
	Point2d result = Point2d(-99999, -99999);
	Position3d cs = this->cameraspace(&cam->camRotInv);
	float z = cs.z;
	if (z <= 0.00001f) { return result; }

	// Perspective projection
	// float perspScale =  z / cam->invTanHalfFov;
	float perspScale =  (z / (renderer == nullptr ? globScreenheight : renderer->height)) / cam->invTanHalfFov;

	result.x = cs.x / perspScale + (renderer == nullptr ? globScreenwidth : renderer->width) * 0.5f;
	result.y = cs.y / perspScale + (renderer == nullptr ? globScreenheight : renderer->height) * 0.5f;
	return result;
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
	return x * operand.x + y * operand.y + z * operand.z;
}

Position3d& Position3d::normalise()
{
	float magnitude = std::sqrt(x * x + y * y + z * z); // find magnitude by pythaagoras
	if (magnitude > 0.0f)
	{
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}
	return *this;
}

void Position3d::flip()
{
	this->x = -this->x;
	this->y = -this->y;
	this->z = -this->z;
}

Position3d Position3d::cameraspace(Quaternion* camRotInv) const
{	
	Position3d cs;
	if (currentScene->currentCam != nullptr)
	{
		cs = *this - currentScene->currentCam->pos; // Set cameraspace position by subtracting camera pos from this pos
		Quaternion cri = Quaternion();
		if (camRotInv == nullptr)
		{
			cri = currentScene->currentCam->quatIdentity.conjugate();
		}
		else { cri = *camRotInv; }

		cs.rotateQuat(cri);
	}
	return cs;
}

float Position3d::lengthSquared() const {
	return x * x + y * y + z * z; }



ostream& operator<< (ostream& os, Position3d pos)
{
	return os << "[" << pos.x << " " << pos.y << " " << pos.z << "]";
}
Mesh::Mesh() 
{ 
	this->position = { 0,0,0 };
	this->quatIdentity = Quaternion(); // Default identity quaternion
	this->calcBaseVecs(); // Calculate base vectors
	this->quatIdentity.normalise(); // Normalise the identity quaternion	
}

string Scene::getName(string candidate = "New mesh") const
{
	int x = 0;
	string original = candidate; // Store the original name
	bool nameTaken = false;
	// Check if the name is already taken
	for (const Object3D& ob : objects)
	{
		if (ob.name == candidate)
		{
			nameTaken = true;
			x += 1;
			candidate = original + "." + std::to_string(x); // Append a number to the name
		}
	}
	if (nameTaken) { cout << "Warning: name " << original << " wasn't available." << endl; }
	return candidate;
}

Object3D* Scene::objectByName(std::string name)
{
	auto ob = std::find_if(this->objects.begin(), this->objects.end(), [&name](const Object3D& tOb)
		{
			return tOb.name == name;
		});
	return ob != this->objects.end() ? &(*ob) : nullptr;
}

//void Mesh::instanceOnMesh(Mesh& instancer)
//{
//	int numVerts = this->vertices.size();
//	for (Vertex3d iV : instancer.vertices)
//	{
//		for (Vertex3d sV : this->vertices)
//		{
//			this->addVertex(sV.position + iV.position);
//		}
//	}
//	// Add face indices
//	vector<int> tmpInds = this->indices;
//	for (int& i : tmpInds) { i += numVerts; }
//	this->indices.insert(this->indices.end(), tmpInds.begin(), tmpInds.end());
//}

Object3D::Object3D()
{
	this->name = "New object"; // Default name
	this->mesh = nullptr;
}

Object3D::Object3D(Mesh* meshin)
{

	this->name = "New mesh object"; // Default name
	this->mesh = meshin;
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
	//for (Mesh* m : children) { m->move(offset); }
}

void Mesh::setPos(Position3d pos)
{
	Position3d offset = pos - this->position; // find the offset that will change the current pos to the new one
	this->move(offset);
}

void Mesh::rotateQuat(const Quaternion& q)
{
	Position3d center = this->position;
	for (Vertex3d& vert : this->vertices)
	{
		Position3d relPos = vert.position - center;
		relPos.rotateQuat(q);
		vert.position = relPos + center;
	}
}

void Mesh::rotateQuat(const Quaternion& q, const Position3d& pivot)
{
	for (Vertex3d& vert : this->vertices)
	{
		Position3d relPos = vert.position - pivot;
		relPos.rotateQuat(q);
		vert.position = relPos + pivot;
	}
}

void Mesh::rotateAxis(float angle, const Position3d& axis)
{
	Quaternion qDelta(angle, axis);
	this->quatIdentity =  qDelta * this->quatIdentity;
	this->quatIdentity.normalise();
	this->rotateQuat({ angle, axis });
}

void Mesh::rotateAxis(float angle, const Position3d& axis, const Position3d& pivot)
{
	Quaternion qDelta(angle, axis);
	this->quatIdentity = qDelta * this->quatIdentity;
	this->quatIdentity.normalise();
	this->rotateQuat(qDelta, pivot);
}

void Mesh::setRotationQuat(const Quaternion& qTarget) {
	Quaternion qDelta = qTarget * this->quatIdentity.conjugate();
	this->rotateQuat(qDelta);
}

void Mesh::calcBaseVecs() // (re)calculate forward/right/up vectors
{
	this->forward = { 0,0,1 }; // World forward
	this->forward.rotateQuat(this->quatIdentity);
	this->up = { 0,1,0 }; // World up
	this->up.rotateQuat(this->quatIdentity);
	this->right = { 1,0,0 }; // World right
	this->right.rotateQuat(this->quatIdentity);
}

bb3d::bb3d() : p1(), p2() {}

bb3d::bb3d(Position3d p1_, Position3d p2_) : p1(p1_), p2(p2_) {}

bool bb3d::containsMesh(Mesh m) const
{
	Position3d p = m.position;
	float minX = std::min(p1.x, p2.x);
	float maxX = std::max(p1.x, p2.x);
	float minY = std::min(p1.y, p2.y);
	float maxY = std::max(p1.y, p2.y);
	float minZ = std::min(p1.z, p2.z);
	float maxZ = std::max(p1.z, p2.z);

	return (p.x >= minX && p.x <= maxX) &&
		(p.y >= minY && p.y <= maxY) &&
		(p.z >= minZ && p.z <= maxZ);
}

Camera::Camera() 
{
	this->aspect = (float)globScreenwidth / (float)globScreenheight; //Fix this later? not priority
	this->setFov(globFOVrads);
}

void Camera::setFov(float newFovRads)
{
	this->fov = newFovRads;
	this->invTanHalfFov = 1.0f / std::tanf(fov * 0.5f);
}

void Camera::rotateCam(float angle, const Position3d& axis) // Axis is in global space!
{
	Quaternion qDelta(angle, axis);
	this->quatIdentity = qDelta * this->quatIdentity;
	this->quatIdentity.normalise();
}

void Camera::calcCamData() 
{
	this->forward = { 0,0,1 }; // World forward
	this->forward.rotateQuat(this->quatIdentity);
	this->up = { 0,1,0 }; // World up
	this->up.rotateQuat(this->quatIdentity);
	this->right = { 1,0,0 }; // World right
	this->right.rotateQuat(this->quatIdentity);
	this->camRotInv = this->quatIdentity.conjugate();
}

void Scene::addObject(Object3D ob)
{
	ob.name = this->getName(ob.name);
	this->objects.emplace_back(ob);
}

float lerp(float a, float b, float ratio) 
{
	return (ratio * (b - a)) + a;
}
