#pragma once
//#include <SDL.h>
#include <vector>
//#include <cstdint>
#include <iostream>
#include <string>

using std::vector, std::ostream, std::string;

class Rotation3d;
class Quaternion;

class Position3d // Stores 3D positions ONLY. Nearly always as used as part of a bigger part (e.g. vert). Doubles as a vector.
{
private:
public:
	double x, y, z;
	Position3d(double xPos, double yPos, double zPos);
	Position3d();
	Position3d cameraspace() const;
	void rotateAroundPoint(const Rotation3d& rotation, const Position3d& pivot);
	void rotateQuat(const Quaternion& q);

	// Vector stuff
	Position3d cross(const Position3d& operand) const;
	float dot(const Position3d& operand) const;
	void normalise();
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

class Rotation3d
{
public:
	float pitch, yaw, roll;
	Rotation3d();
	Rotation3d(float x_, float y_, float z_);
	friend Rotation3d operator+(const Rotation3d& p1, const Rotation3d& p2);
	friend Rotation3d operator-(const Rotation3d& p1, const Rotation3d& p2);
	friend Rotation3d operator*(const Rotation3d& p1, const Rotation3d& p2);
	Rotation3d& operator+=(const Rotation3d& other);
};

class Quaternion // fml
{
public:
	float w, x, y, z;
	Quaternion(); // 0 rotation
	Quaternion(float angle, const Position3d& axis);// Use position3d vector as axis
	Quaternion(double w_, double x_, double y_, double z_);

	Quaternion operator*(const Quaternion& q) const;
	Quaternion inverse() const;
	void normalise();

	friend std::ostream& operator<<(std::ostream& os, const Quaternion& q);
};

class Camera
{
public:
	Position3d pos;
	Rotation3d rot;
	Quaternion quatIdentity; // Stores the "identity" of the cam orientation, for base vecs

	Position3d up;
	Position3d right;
	Position3d forward;

	void rotateCam(float angle, const Position3d& axis);
	void calcBaseVecs(); // (re)calculate forward/right/up vectors
};

class Vertex3d
{
public:
	Vertex3d(Position3d pos);
	Position3d position; 
	void offsetPosition(Position3d offset);
	void rotatePosition(const Rotation3d& rot, const Position3d& pivot);
};

class Colour // Store colour as decimal fractions of RGB
{
public:
	Colour();
	Colour(float r, float g, float b);
	Colour(float r, float g, float b, float a);
	uint32_t raw() const;
	friend Colour operator*(const Colour& c1, const float val);
	Colour& operator*=(const float val);

	
	float red;
	float green;
	float blue;
	float alpha;
};
class Material
{
public:
	Material();
	Material(float r, float g, float b);
	Material(float r, float g, float b, float a);
	Colour colour;
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

class Mesh
{
public:
	Mesh();
	void addVertex(Position3d pos);
	void addFace(vector<int> &ind);

	//void instanceOnMesh(Mesh& instancer); //Duplicate this mesh on every vertex of the instancer (permanent)

	void move(Position3d offset); 
	void setPos(Position3d pos);

	void rotate(const Rotation3d& rot, const Position3d& pivot); 
	void setRotation(const Rotation3d& rot, const Position3d& pivot);

	void rotate(const Rotation3d& rot); // Depracated, use quaternion instead
	void setRotation(const Rotation3d& rot); // Depracated, use quaternion instead

	void calcBaseVecs(); // (re)calculate forward/right/up vectors

	void rotateAxis(float angle, const Position3d& axis);
	void rotateAxis(float angle, const Position3d& axis, const Position3d& pivot);
	void rotateQuat(const Quaternion& q);
	void rotateQuat(const Quaternion& q, const Position3d& pivot);

	void setRotationQuat(const Quaternion& q); // Faulty

	string name; // Name of the mesh

	vector<Vertex3d> vertices; //vector of type Vertex
	vector<int> indices; //stores tri indices as 3-tuple
	vector<Mesh*> children;

	Position3d position;
	Rotation3d rotation;
	Quaternion quatIdentity;

	vector<Material> materials;
	vector<int> matIndices; // Stores an index of materials corresponding to each tri

	Position3d up;
	Position3d right;
	Position3d forward;

};


class Scene
{
public:
	Camera* currentCam = nullptr;
	vector<Mesh> meshes;
	vector<Camera> cams;
	void addMesh(Mesh& mesh); // Add a mesh to the scene
	string getName(string candidate) const; // Get a name for a mesh
	string getName() const; // Generate default name
};


extern Scene* currentScene;

const float pi = 3.14159f; // looks like a good place to me 
