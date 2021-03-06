#pragma once
#include <string>
#include <vector>

//A class to store texture data.
//A texture is a data buffer that contains values with relate to pixel colours.

class Texture
{
public:
	//Constructor.
	Texture();
	//Destructor.
	~Texture();

	//Function to load a texture from file.
	bool Load(std::string a_fileName);
	void Unload();
	//Get file name.
	const std::string& GetFileName() const { return m_fileName; }
	unsigned int GetTextureID() const { return m_textureID; }
	void GetDimensions(unsigned int& a_w, unsigned int& a_h) const;

private:
	std::string m_fileName;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_textureID;
};

inline void Texture::GetDimensions(unsigned int& a_w, unsigned int& a_h) const
{
	a_w = m_width; a_h = m_height;
}

class CubeMap {
public:
	CubeMap();
	~CubeMap();

	//Getters.
	unsigned int GetCubeMapTexture() { return m_cubemapTexture; }

private:
	//Cubemap Functions.
	unsigned int LoadCubeMap(std::vector<std::string> faces);

	//Cubemap variables.
	std::vector<std::string> m_skyboxFaces;
	unsigned int m_cubemapTexture;
};