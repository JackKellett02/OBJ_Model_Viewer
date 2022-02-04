#include "Skybox.h"

//Library includes.
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

//Custom includes.
#include "Texture.h"
#include "ShaderUtil.h"

Skybox::Skybox()
{
	//Create a cubemap and load it.
	m_SkyboxTexture = new CubeMap();
}

Skybox::~Skybox()
{
	delete m_SkyboxTexture;
}

void Skybox::SetUpSkybox()
{
	ShaderUtil* shaderUtilInstance = ShaderUtil::GetInstance();

	//Create shader program.
	unsigned int vertexShader = ShaderUtil::LoadShader("resource/shaders/skybox_vertex.glsl", GL_VERTEX_SHADER);
	unsigned int fragmentShader = ShaderUtil::LoadShader("resource/shaders/skybox_fragment.glsl", GL_FRAGMENT_SHADER);
	m_SkyboxShader = ShaderUtil::CreateProgram(vertexShader, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Set up skybox model.
	//Set up skybox variables.
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	glGenVertexArrays(1, &m_skyboxVAO);
	glGenBuffers(1, &m_skyboxVBO);
	glBindVertexArray(m_skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::RenderSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, float lightStrength)
{
	// draw skybox as last
	glDepthMask(GL_FALSE);
	glUseProgram(m_SkyboxShader);
	int skyboxLocation = glGetUniformLocation(m_SkyboxShader, "skybox");
	glUniform1i(skyboxLocation, 0);

	glm::mat4 viewMat = glm::mat4(glm::mat3(viewMatrix));//Remove translation from view matrix.

	//Pass the view and projection matices to the skybox shader.
	int projectionLocation = glGetUniformLocation(m_SkyboxShader, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	int viewLocation = glGetUniformLocation(m_SkyboxShader, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMat));

	//Set the light level for this shader.
	int lightLevelUniformLocation = glGetUniformLocation(m_SkyboxShader, "lightStrength");
	glUniform1f(lightLevelUniformLocation, lightStrength);

	//Skybox cube.
	glBindVertexArray(m_skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexture->GetCubeMapTexture());
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE); // set depth function back to default

	glUseProgram(0);
}
