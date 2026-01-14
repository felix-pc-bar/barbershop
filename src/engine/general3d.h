#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <cstdint>
#include <unordered_map>

#include "general2d.h"
#include "quaternion.h"
#include "material.h"

using std::vector, std::ostream, std::string;

class Quaternion;

class Camera;
class cRenderer;

class Position3d // Stores 3D positions ONLY. Nearly always as used as part of a bigger part (e.g. vert). Doubles as a vector.
{
private:
public:
	double x, y, z;
	Position3d(double xPos, double yPos, double zPos);
	Position3d();
	Position3d cameraspace(Quaternion* camInvRot = nullptr) const;
	void rotateQuat(const Quaternion& q);
	Point2d project(Camera* cam, const cRenderer* renderer = nullptr); // Don't like this much
	
	// Vector stuff
	Position3d cross(const Position3d& operand) const;
	float dot(const Position3d& operand) const;
	Position3d& normalise();
	void flip();
	float lengthSquared() const;

	friend ostream& operator<< (ostream& os, Position3d pos);
	friend Position3d operator+(const Position3d& p1, const Position3d& p2);
	friend Position3d operator-(const Position3d& p1, const Position3d& p2);
	friend Position3d operator*(const Position3d& p1, const Position3d& p2);
	friend Position3d operator/(const Position3d& p1, const float div);
	friend Position3d operator*(const Position3d& p1, const float mpcand);

	Position3d& operator+=(const Position3d& other);
	Position3d& operator-=(const Position3d& other);


	friend bool operator==(const Position3d& p1, const Position3d& p2);
	friend bool operator<(const Position3d& p1, const Position3d& p2);
	friend bool operator>(const Position3d& p1, const Position3d& p2);
};



class Camera
{
public:
	Camera();
	Position3d pos;
	Quaternion quatIdentity; // Stores the "identity" of the cam orientation, for base vecs
	Quaternion camRotInv; //Used for cameraspace calcs
	float invTanHalfFov; //NEEDS TO BE RECALCULATED WITH FOV CHANGE
	float fov; // Don't set manually!
	float aspect; //Ratio of screen

	Position3d up;
	Position3d right;
	Position3d forward;

	void rotateCam(float angle, const Position3d& axis);
	void calcCamData(); // (re)calculate base vectors, inverse rotation
	void setFov(float fov);
};

class Vertex3d
{
public:
	Vertex3d(Position3d pos);
	Position3d position; 
	void offsetPosition(Position3d offset);
};


class Mesh;

class bb3d // 3D bounding box
{
public:
	bb3d();
	bb3d(Position3d p1_, Position3d p2_);

	bool containsMesh(Mesh m) const;
	
	Position3d p1, p2;
};

class Object3D
{
public:
	Object3D();
	Object3D(Mesh* meshin);
	Mesh* mesh;
	std::vector<Position3d> points;
	vector<Material> materials;
	string name; // Name of the object
};

class Mesh
{
public:
	Mesh();
	void addVertex(Position3d pos);
	void addFace(vector<int> &ind);

	//void instanceOnMesh(Mesh& instancer); //Duplicate this mesh on every vertex of the instancer (permanent)

	void move(Position3d offset); 
	void setPos(Position3d pos);

	void calcBaseVecs(); // (re)calculate forward/right/up vectors

	void rotateAxis(float angle, const Position3d& axis);
	void rotateAxis(float angle, const Position3d& axis, const Position3d& pivot);
	void rotateQuat(const Quaternion& q);
	void rotateQuat(const Quaternion& q, const Position3d& pivot);

	void setRotationQuat(const Quaternion& q); // Faulty


	vector<Vertex3d> vertices; //vector of type Vertex
	vector<int> indices; //stores tri indices as 3-tuple
	vector<Mesh*> children;

	Position3d position;
	Quaternion quatIdentity;

	vector<int> matIndices; // Stores an index of materials corresponding to each tri

	Position3d up;
	Position3d right;
	Position3d forward;
};


class Scene
{
public:
	Camera* currentCam = nullptr;
	vector<Object3D> objects;
	vector<Camera> cams;
	void addObject(Object3D ob); // Add an object to the scene
	string getName(string candidate) const; // Get a name for a object
	Object3D* objectByName(std::string name);
};


extern Scene* currentScene;

const float pi = 3.14159f; // looks like a good place to me 

float lerp(float a, float b, float ratio);
