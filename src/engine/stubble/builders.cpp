#include "builders.h"
#include "types.h"

#include "../import3d.h"

Colour* _colRGB(std::vector<extendedValue> args)
{
	return new Colour(std::get<float>(args[0]),
			   std::get<float>(args[1]),
			   std::get<float>(args[2])
			   );
}

Colour* _colRGBA(std::vector<extendedValue> args)
{
	return new Colour(std::get<float>(args[0]),
		   std::get<float>(args[1]),
		   std::get<float>(args[2]),
		   std::get<float>(args[3])
		   );
}

Material* _matColPtShd(std::vector<extendedValue> args)
{
	return new Material(*std::get<Colour*>(args[0]), std::get<int>(args[1]), std::get<bool>(args[2]));
}

Object3D* _objPathpp(std::vector<extendedValue> args)
{
	Object3D* result = new Object3D(importObj(std::get<std::string>(args[0])));
	result->materials[0] = *std::get<Material*>(args[1]);
	return result;
}

Position3d* _posXYZf(std::vector<extendedValue> args);

Quaternion* _quatAngleAxis(std::vector<extendedValue> args);

Camera* _camPosQuatFov(std::vector<extendedValue> args);

Scene* _scnFull(std::vector<extendedValue> args);
