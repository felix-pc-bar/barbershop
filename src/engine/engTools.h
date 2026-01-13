#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <cstdint>
#include <unordered_map>

using std::vector, std::ostream, std::string;

class Quaternion;

class Position3d // Stores 3D positions ONLY. Nearly always as used as part of a bigger part (e.g. vert). Doubles as a vector.
{
private:
public:
	double x, y, z;
	Position3d(double xPos, double yPos, double zPos);
	Position3d();
	Position3d cameraspace(Quaternion* camInvRot = nullptr) const;
	void rotateQuat(const Quaternion& q);

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

class Camera
{
public:
	Position3d pos;
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
};

class Colour // Store colour as decimal fractions of RGB
{
public:
	Colour();
	Colour(float r, float g, float b);
	Colour(float r, float g, float b, float a);
	Colour(uint32_t col);
	Colour(const std::string& gruvName); // Gruv colour from name
	uint32_t raw() const;
	friend Colour operator*(const Colour& c1, const float val);
	Colour& operator*=(const float val);

	
	float red;
	float green;
	float blue;
	float alpha;
private:
	static const std::unordered_map<std::string, Colour>& gruvCols() {
		static const std::unordered_map<std::string, Colour> table = {
			//bg
			{"darkred", Colour(0xFFcc241d)},
			{"darkgreen", Colour(0xFF98971a)},
			{"darkyellow", Colour(0xFFd79921)},
			{"darkblue", Colour(0xFF458588)},
			{"darkpurple", Colour(0xFFb16286)},
			{"darkaqua", Colour(0xFF689d6a)},
			{"gray5", Colour(0xFFa89984)},
			{"gray6", Colour(0xFF928374)},
			{"red", Colour(0xFFfb4934)},
			{"green", Colour(0xFFb8bb26)},
			{"yellow", Colour(0xFFfabd2f)},
			{"blue", Colour(0xFF83a598)},
			{"purple", Colour(0xFFd3869b)},
			{"aqua", Colour(0xFF8ec07c)},
			{"fg", Colour(0xFFebdbb2)},
			{"black", Colour(0xFF1d2021)}, //bg0_h
			{"grey", Colour(0xFF282828)},
			{"grey1", Colour(0xFF3c3836)},
			{"grey2", Colour(0xFF504945)},
			{"grey3", Colour(0xFF665c54)},
			{"grey4", Colour(0xFF7c6f64)},
			// gray5
			{"darkorange", Colour(0xFFd65d0e)},
			{"orange", Colour(0xFFfe8019)},
			// bg0_s
			{"white4", Colour(0xFFa89984)},
			{"white3", Colour(0xFFbdae93)},
			{"white2", Colour(0xFFd5c4a1)},
			{"white1", Colour(0xFFebdbb2)},
			{"white", Colour(0xFFfbf1c7)}
		};
		return table;
	};
};
class Material
{
public:
	Material();
	Material(Colour col, int pointSize = 0, bool shade = true, bool allowDebugVis = true, uint16_t ditherValue = 128);
	int pointWidth; //Size of squares to draw for points in scene
	Colour colour;
	bool shadeMat; //Whether to shade this material
	bool omitDbg; //Don't change shading in debug
	// Dither stuff
	uint16_t ditherValue; // set value offset for dithering (0-255, 128 by default)
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
