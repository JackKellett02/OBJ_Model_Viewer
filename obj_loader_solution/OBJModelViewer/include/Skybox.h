#pragma once
//Includes.
#include <string>
#include <vector>
#include <glm/fwd.hpp>

class CubeMap;
class Skybox
{
public:
	Skybox();
	~Skybox();

	//Getters

	//Setters.
	void SetUpSkybox();

	//Render.
	void RenderSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, float lightStrength);

private:
	//Cubemap variables.
	CubeMap* m_SkyboxTexture;

	//Skybox Functions.

	//Skybox Variables.
	unsigned int m_SkyboxShader;
	unsigned int m_skyboxVAO;
	unsigned int m_skyboxVBO;
};