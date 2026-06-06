#include "types.h"
#include "../buffer.h"

template<typename T>
struct TypeInfo;

template<>
struct TypeInfo<int>
{
	static constexpr TypesEnum id = TypesEnum::Int;
	static constexpr std::string_view name = "int";
};

template<>
struct TypeInfo<float>
{
	static constexpr TypesEnum id = TypesEnum::Float;
	static constexpr std::string_view name = "float";
};

template<>
struct TypeInfo<bool>
{
	static constexpr TypesEnum id = TypesEnum::Bool;
	static constexpr std::string_view name = "bool";
};

template<>
struct TypeInfo<std::string>
{
	static constexpr TypesEnum id = TypesEnum::StdString;
	static constexpr std::string_view name = "std::string";
};

template<>
struct TypeInfo<Pyjama*>
{
	static constexpr TypesEnum id = TypesEnum::_Pyjama;
	static constexpr std::string_view name = "Pyjama";
};
template<>
struct TypeInfo<Colour*>
{
	static constexpr TypesEnum id = TypesEnum::_Colour;
	static constexpr std::string_view name = "Colour";
};

template<>
struct TypeInfo<Material*>
{
	static constexpr TypesEnum id = TypesEnum::_Material;
	static constexpr std::string_view name = "Material";
};

template<>
struct TypeInfo<Object3D*>
{
	static constexpr TypesEnum id = TypesEnum::_Object3D;
	static constexpr std::string_view name = "Objcet3D";
};

template<>
struct TypeInfo<Position3d*>
{
	static constexpr TypesEnum id = TypesEnum::_Position3D;
	static constexpr std::string_view name = "Position3d";
};

template<>
struct TypeInfo<Quaternion*>
{
	static constexpr TypesEnum id = TypesEnum::_Quaternion;
	static constexpr std::string_view name = "Quaternion";
};

template<>
struct TypeInfo<Camera*>
{
	static constexpr TypesEnum id = TypesEnum::_Camera;
	static constexpr std::string_view name = "Camera";
};

template<>
struct TypeInfo<Scene*>
{
	static constexpr TypesEnum id = TypesEnum::_Scene;
	static constexpr std::string_view name = "Scene";
};

template<>
struct TypeInfo<pixelBuffer*>
{
	static constexpr TypesEnum id = TypesEnum::_pixelBuffer;
	static constexpr std::string_view name = "pixelBuffer";
};
