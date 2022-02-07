#include "Texture.h"
#include <stb_image.h>
#include <iostream>
#include <glad/glad.h>

Texture::Texture() : m_fileName(), m_width(0), m_height(0), m_textureID(0)
{
}

Texture::~Texture()
{
	Unload();
}

bool Texture::Load(std::string a_fileName)
{
	int width = 0, height = 0, channels = 0;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imageData = stbi_load(a_fileName.c_str(), &width, &height, &channels, 4);
	if(imageData != nullptr)
	{
		m_fileName = a_fileName;
		m_width = width;
		m_height = height;
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(imageData);
		std::cout << "Successfully loaded Image File: " << a_fileName << std::endl;
		return true;
	}
	std::cout << "Failed to open Image File: " << a_fileName << std::endl;
	return false;
}

void Texture::Unload()
{
	glDeleteTextures(1, &m_textureID);
}


CubeMap::CubeMap()
{
	//Set up cube map variables.
	m_skyboxFaces = std::vector<std::string>{
			"resource/models/skybox/right.jpg",
			"resource/models/skybox/left.jpg",
			"resource/models/skybox/top.jpg",
			"resource/models/skybox/bottom.jpg",
			"resource/models/skybox/front.jpg",
			"resource/models/skybox/back.jpg"
	};

	m_cubemapTexture = LoadCubeMap(m_skyboxFaces);
}

CubeMap::~CubeMap()
{
	glDeleteTextures(1, &m_cubemapTexture);
}

unsigned int CubeMap::LoadCubeMap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		const char* fileName = (faces[i]).c_str();
		unsigned char* data = stbi_load(fileName, &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}