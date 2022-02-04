#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

/// <summary>
/// An OBJ Material.
///	Materials have properties such as lights, textures, roughness.
/// </summary>
class OBJMaterial
{
public:
	OBJMaterial() : name(), kA(0.0f), kD(0.0f), kS(0.0f) {};
	~OBJMaterial() {};

	std::string name;
	//Colour and illumination variables.
	glm::vec4 kA; //Ambient light colour - alpha comp stores optical density (Ni)(Refraction index 0.001 - 10).
	glm::vec4 kD; //Diffuse Light Colour - alpha comp stores dissolve (d)(0-1).
	glm::vec4 kS; //Specular Light Colour - (exponent stored in alpha).

	//enum for the texture our OBJ model will support.
	enum TextureTypes
	{
		DiffuseTexture = 0,
		SpecularTexture,
		NormalTexture,

		TextureTypes_Count
	};
	//Texture will have filenames for loading, once loaded ID's store in ID array.
	std::string textureFileNames[TextureTypes_Count];
	unsigned int textureIDs[TextureTypes_Count];
};

//A basic vertex class for an OBJ file, supports vertex position, vertex normal, vertex uv coord.
class OBJVertex
{
public:
	enum VertexAttributeFlags
	{
		POSITION = (1 << 0), //The position of the vertex.
		NORMAL = (1 << 1), //The normal for the vertex.
		UVCOORD = (1 << 2), //The UV coordinates for the vertex.
	};

	enum Offssets
	{
		PositionOffset = 0,
		NormalOffset = PositionOffset + sizeof(glm::vec4),
		UVCoordOffset = NormalOffset + sizeof(glm::vec4),
	};
	OBJVertex();
	~OBJVertex();

	glm::vec4 position;
	glm::vec4 normal;
	glm::vec2 uvcoord;

	bool operator == (const OBJVertex& a_rhs) const;
	bool operator < (const OBJVertex& a_rhs) const;
};
//Inline constructor destructor for OBJVertex class.
inline OBJVertex::OBJVertex() : position(0, 0, 0, 1), normal(0, 0, 0, 0), uvcoord(0, 0) {}
inline OBJVertex::~OBJVertex() {}
//Inline comparitor methods for OBJVertex.
inline bool OBJVertex::operator==(const OBJVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(OBJVertex)) == 0;
}

inline bool OBJVertex::operator<(const OBJVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(OBJVertex)) < 0;
}

//An OBJ Model can be composed of many meshes. Much like any 3D model
//lets us use a class to store individual mesh data.
class OBJMesh
{
public:
	OBJMesh();
	~OBJMesh();

	glm::vec4 CalculateFaceNormal(const unsigned int& a_indexA, const unsigned int& a_indexB, const unsigned int& a_indexC)const;
	void CalculateFaceNormals();

	std::string                m_name;
	std::vector<OBJVertex>     m_vertices;
	std::vector<unsigned int>  m_indices;
	OBJMaterial* m_material = nullptr;
};
//Inline constructor destructor -- to be expanded upon as required.
inline OBJMesh::OBJMesh() {}
inline OBJMesh::~OBJMesh() {}

class OBJModel
{
public:
	OBJModel(std::string a_modelName, const char* a_texturePath) : m_worldMatrix(glm::mat4(1.0f)), m_path(a_texturePath), m_modelName(a_modelName), m_meshes(), m_materials() {};
	~OBJModel()
	{
		Unload(); //Function to unload any data loaded in from file.
	};

	//Load from file function.
	bool Load(const char* a_filename, float a_scale = 1.0f);
	//Function to unload and free memory.
	void Unload();
	//Functions to retrieve path, number of meshes and world matrix of model.
	const char* GetPath()              const { return m_path.c_str(); }
	unsigned int       GetMeshCount()         const { return m_meshes.size(); }
	unsigned int GetMaterialCount() const { return m_materials.size(); }
	const glm::mat4& GetWorldMatrix()       const { return m_worldMatrix; }
	const char* GetModelName() const { return m_modelName.c_str(); }
	//Functions to retrieve mesh by name or index for models that contain multiple meshes.
	OBJMesh* GetMeshByName(const char* a_name);
	OBJMesh* GetMeshByIndex(unsigned int a_index);
	OBJMaterial* GetMaterialByName(const char* a_name);
	OBJMaterial* GetMaterialByIndex(unsigned int a_index);

private:
	//Function to process line data read in from file.
	std::string LineType(const std::string& a_in);
	std::string LineData(const std::string& a_in);
	glm::vec4 ProcessVectorString(const std::string a_data);
	std::vector<std::string> SplitStringAtCharacter(std::string data, char a_character);

	void LoadMaterialLibrary(std::string a_mtllib);
	//OBJ face triplet struct.
	typedef struct obj_face_triplet
	{
		unsigned int v;
		unsigned int vt;
		unsigned int vn;
	}obj_face_triplet;
	//Function to extract triplet data from OBJ file.
	obj_face_triplet ProcessTriplet(std::string a_triplet);

	//Vector to store mesh data.
	std::vector<OBJMesh*> m_meshes;
	//Vector to store materials.
	std::vector<OBJMaterial*> m_materials;
	//Path to model data - useful for things like texture lookups.
	std::string m_path;
	//Filename.
	std::string m_modelName;
	//Root Mat4 (World Matrix);
	glm::mat4 m_worldMatrix;
};