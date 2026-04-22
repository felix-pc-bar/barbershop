#include "builders.h"
#include "types.h"

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
