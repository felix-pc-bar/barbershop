#include <stdexcept>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

#include "engTools.h"
#include "import3d.h"

using namespace std;

Object3D importObj(string filepath) // Objects should be exported as Forward = +Y, Up = +Z
{
	string line;
	string inxStr;
	Object3D result;
	Mesh* mesh = new Mesh(); // allocate to heap so it's not zapped when we leave importObj()
	if (!std::filesystem::exists(filepath))
	{
		filepath = "content/obj/nomesh.obj";
	}
	ifstream objfile(filepath);
	//if (!objfile.is_open()) { objfile = ifstream("content/obj/nomesh.obj"); }
	if (objfile.is_open())
	{
		std::filesystem::path p(filepath);
		result.name = p.stem().string();
		while (getline(objfile, line))
		{
			if (!line.compare(0, 2, "v "))
			{
				istringstream substr(line.substr(2));
				double xin, yin, zin;
				substr >> xin >> yin >> zin;
				mesh->addVertex(Position3d(xin, yin, zin));
			}
			if (!line.compare(0, 2, "f "))
			{
				istringstream ss(line.substr(2));
				string token;
				vector<int> faceIndices;

				while (ss >> token)
				{
					istringstream tokenStream(token);
					string vStr;
					getline(tokenStream, vStr, '/');  // Get the position index only
					int vIndex = stoi(vStr) - 1;      // OBJ is 1-based; convert to 0-based
					faceIndices.push_back(vIndex);
				}

				mesh->addFace(faceIndices);
				mesh->matIndices.push_back(0); // For now, assign material 0 to every tri

			}
		}
		std::cout << "Imported " << filepath << " - " << mesh->vertices.size() << " vertices" << std::endl;
		result.mesh = mesh;
		return result;
	}
	std::cout << "Import error" << endl;
	string fp(filepath);
	throw std::invalid_argument(fp + " doesn't exist");
	return result;
}
