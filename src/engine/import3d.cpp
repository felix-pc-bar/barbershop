#include "import3d.h"
#include <stdexcept>
#include <string>
#include <filesystem>

using namespace std;

Object3D importObj(string filepath) // Objects should be exported as Forward = +Y, Up = +Z
{
	string line;
	string inxStr;
	ifstream objfile(filepath);
	Object3D result;
	Mesh* mesh = new Mesh(); // allocate to heap so it's not zapped when we leave importObj()
	if (objfile.is_open())
	{
		std::filesystem::path p(filepath);
		result.name = p.stem().string();
		while (getline(objfile, line))
		{
			if (line._Starts_with("v "))
			{
				istringstream substr(line.substr(2));
				double xin, yin, zin;
				substr >> xin >> yin >> zin;
				mesh->addVertex(Position3d(xin, yin, zin));
			}
			if (line._Starts_with("f "))
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
