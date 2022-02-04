#include "3DRenderingFramework.h"
#include "ShaderUtil.h"
#include "Utilities.h"
#include "Dispatcher.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "TextureManager.h"
#include "obj_loader.h"
#include "Skybox.h"
#include <iostream>
#include <imgui.h>

_3DRenderingFramework::_3DRenderingFramework()
{

}

_3DRenderingFramework::~_3DRenderingFramework()
{
	if (m_skybox)
	{
		delete m_skybox;
		m_skybox = nullptr;
	}
}

void GlobalWindowResizeEventHandler(WindowResizeEvent* e)
{
	std::cout << "Global Event handler called!!" << std::endl;
	e->Handled();
}

bool _3DRenderingFramework::OnCreate(std::string a_modelToLoad, float a_modelScale)
{
	m_lightStrength = 100.0f;
	Dispatcher* dp = Dispatcher::GetInstance();
	if (dp)
	{
		dp->subscribe(this, &_3DRenderingFramework::onWindowResize);
		dp->subscribe(&GlobalWindowResizeEventHandler);
	}

	//Get an instance of the texture manager.
	TextureManager::CreateInstance();

	//Set the clear colour and enable depth testing and backface culling.
	glClearColor(0.25f, 0.45f, 0.75f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//Set up the shaders and vertex data for the model viewer's grid lines.
	SetUpGridLines();

	//Create a world-space matrix for a camera.
	m_cameraMatrix =
		glm::inverse(
			glm::lookAt(glm::vec3(10, 10, 10),
				glm::vec3(0, 0, 0),
				glm::vec3(0, 1, 0))
		);

	CreateProjectionMatrix();

	//Create and setup the skybox.
	m_skybox = new Skybox();
	m_skybox->SetUpSkybox();

	//Load the model data for specified obj file.
	LoadObjModelData(a_modelToLoad, a_modelScale);

	//Set default model colour.
	m_defaultMaterialColour = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);

	return true;
}

void _3DRenderingFramework::Update(float deltaTime)
{
	Utility::FreeMovement(m_cameraMatrix, deltaTime, 4.0f);

	//Set up an imgui window to control default material colour.
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 window_size = ImVec2(600.0f, 100.0f);
	ImVec2 window_pos = ImVec2((io.DisplaySize.x * 0.99f) - window_size.x, io.DisplaySize.y * 0.01f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
	if (ImGui::Begin("Edit Scene Viewing."))
	{
		ImGui::ColorEdit3("Default Material Colour: ", glm::value_ptr(m_defaultMaterialColour));
		ImGui::SliderFloat("Scene Lightin%", &m_lightStrength, 10.0f, 100.0f);
	}
	ImGui::End();
}

void _3DRenderingFramework::Draw()
{
	//Clear the back buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Draw code goes here.
	//Get the view matrix from the world-space camera matrix.
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	glm::mat4 projectionViewMatrix = m_projectionMatrix * viewMatrix;

	//Render the skybox.
	m_skybox->RenderSkybox(viewMatrix, m_projectionMatrix, m_lightStrength);

	//Render the grid lines.
	RenderGridLines(projectionViewMatrix);

	//Render the obj model.
	RenderOBJModel(m_objModel, projectionViewMatrix);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glUseProgram(0);
}

void _3DRenderingFramework::RenderOBJModel(OBJModel* a_model, glm::mat4 a_projectionViewMatrix)
{
	//Enable obj model shader.
	glUseProgram(m_objProgram);
	//Set the light level for this shader.
	int lightLevelUniformLocation = glGetUniformLocation(m_objProgram, "lightStrength");
	glUniform1f(lightLevelUniformLocation, m_lightStrength);
	//Set the projection view matrix for this shader.
	int projectionViewUniformLocation = glGetUniformLocation(m_objProgram, "ProjectionViewMatrix");
	//Send this location a pointer to our glm::mat4 (send across float data).
	glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(a_projectionViewMatrix));
	OBJMaterial* lastOkMaterial = nullptr;
	for (int i = 0; i < a_model->GetMeshCount(); i++)
	{
		//Get the model matrix location from the shader program.
		int modelMatrixUniformLocation = glGetUniformLocation(m_objProgram, "ModelMatrix");
		//Send the OBJ Model's world matrix data across to the shader program.
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, false, glm::value_ptr(a_model->GetWorldMatrix()));

		int cameraPositionUniformLocation = glGetUniformLocation(m_objProgram, "camPos");
		glUniform4fv(cameraPositionUniformLocation, 1, glm::value_ptr(m_cameraMatrix[3]));

		OBJMesh* pMesh = nullptr;
		pMesh = a_model->GetMeshByIndex(i);
		//pMesh->CalculateFaceNormals();
		//Send material data to shader.
		int kA_location = glGetUniformLocation(m_objProgram, "kA");
		int kD_location = glGetUniformLocation(m_objProgram, "kD");;
		int kS_location = glGetUniformLocation(m_objProgram, "kS");


		OBJMaterial* p_material = nullptr;
		if (pMesh != nullptr)
		{
			p_material = pMesh->m_material;
		}
		if (p_material != nullptr)
		{
			lastOkMaterial = p_material;
			//Send the OBJ Model's world matrix data across to the shader program.
			glUniform4fv(kA_location, 1, glm::value_ptr(p_material->kA));
			glUniform4fv(kD_location, 1, glm::value_ptr(p_material->kD));
			glUniform4fv(kS_location, 1, glm::value_ptr(p_material->kS));

			//Get the location of the diffuse texture.
			int texUniformLoc = glGetUniformLocation(m_objProgram, "DiffuseTexture");
			glUniform1i(texUniformLoc, 0); //Set diffuse texture to be GL_Texture0.

			glActiveTexture(GL_TEXTURE0);//Set the active texture unity to texture0.
			//Bind the texture for diffuse for this material to the texture0.
			glBindTexture(GL_TEXTURE_2D, p_material->textureIDs[OBJMaterial::TextureTypes::DiffuseTexture]);

			//Get the location of the diffuse texture.
			texUniformLoc = glGetUniformLocation(m_objProgram, "SpecularTexture");
			glUniform1i(texUniformLoc, 1); //Set diffuse texture to be GL_Texture1.

			glActiveTexture(GL_TEXTURE1); //Set the active texture unity to texture1.
			//Bind the texture for diffuse for this material to the texture0.
			glBindTexture(GL_TEXTURE_2D, p_material->textureIDs[OBJMaterial::TextureTypes::SpecularTexture]);

			//Get the location of the diffuse texture.
			texUniformLoc = glGetUniformLocation(m_objProgram, "NormalTexture");
			glUniform1i(texUniformLoc, 2); //Set diffuse texture to be GL_Texture2.

			glActiveTexture(GL_TEXTURE2); //Set the active texture unit to texture2.
			//Bind the texture for diffuse for this material to the texture0.
			glBindTexture(GL_TEXTURE_2D, p_material->textureIDs[OBJMaterial::TextureTypes::NormalTexture]);
		}
		else //No material to obtain lighting information from, use defaults.
		{
			//Check previous material to see if we can apply that texture data to the mesh instead.
			if (lastOkMaterial != nullptr)
			{
				//Send the OBJ Model's world matrix data across to the shader program.
				glUniform4fv(kA_location, 1, glm::value_ptr(lastOkMaterial->kA));
				glUniform4fv(kD_location, 1, glm::value_ptr(lastOkMaterial->kD));
				glUniform4fv(kS_location, 1, glm::value_ptr(lastOkMaterial->kS));

				//Get the location of the diffuse texture.
				int texUniformLoc = glGetUniformLocation(m_objProgram, "DiffuseTexture");
				glUniform1i(texUniformLoc, 0); //Set diffuse texture to be GL_Texture0.

				glActiveTexture(GL_TEXTURE0);//Set the active texture unity to texture0.
				//Bind the texture for diffuse for this material to the texture0.
				glBindTexture(GL_TEXTURE_2D, lastOkMaterial->textureIDs[OBJMaterial::TextureTypes::DiffuseTexture]);

				//Get the location of the diffuse texture.
				texUniformLoc = glGetUniformLocation(m_objProgram, "SpecularTexture");
				glUniform1i(texUniformLoc, 1); //Set diffuse texture to be GL_Texture1.

				glActiveTexture(GL_TEXTURE1); //Set the active texture unity to texture1.
				//Bind the texture for diffuse for this material to the texture0.
				glBindTexture(GL_TEXTURE_2D, lastOkMaterial->textureIDs[OBJMaterial::TextureTypes::SpecularTexture]);

				//Get the location of the diffuse texture.
				texUniformLoc = glGetUniformLocation(m_objProgram, "NormalTexture");
				glUniform1i(texUniformLoc, 2); //Set diffuse texture to be GL_Texture2.

				glActiveTexture(GL_TEXTURE2); //Set the active texture unit to texture2.
				//Bind the texture for diffuse for this material to the texture0.
				glBindTexture(GL_TEXTURE_2D, lastOkMaterial->textureIDs[OBJMaterial::TextureTypes::NormalTexture]);
			}
			else //If there's been no texture data to apply at all apply default material to the mesh.
			{
				//Send the OBJ Model's world matrix data across to the shader program.
				glUniform4fv(kA_location, 1, glm::value_ptr(m_defaultMaterialColour));
				glUniform4fv(kD_location, 1, glm::value_ptr(glm::vec4(m_defaultMaterialColour.x * 4, m_defaultMaterialColour.y * 4, m_defaultMaterialColour.z * 4, 1.0f)));
				glUniform4fv(kS_location, 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 64.0f)));
			}

		}
		glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(OBJVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_objModelBuffer[1]);
		glEnableVertexAttribArray(0); //Position.
		glEnableVertexAttribArray(1); //Normal.
		glEnableVertexAttribArray(2); //UV coord.

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), ((char*)0) + OBJVertex::PositionOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(OBJVertex), ((char*)0) + OBJVertex::NormalOffset);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(OBJVertex), ((char*)0) + OBJVertex::UVCoordOffset);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void _3DRenderingFramework::RenderGridLines(glm::mat4 a_projectionViewMatrix)
{
	//Enable grid line shaders.
	glUseProgram(m_uiProgram);

	//Send the projection matrix to the vertex shader.
	//Ask the shader program for the location of the projection-view-matrix uniform variable.
	int projectionViewUniformLocation = glGetUniformLocation(m_uiProgram, "ProjectionViewMatrix");

	//Send this location a pointer to our glm::mat4 (send across float data).
	glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(a_projectionViewMatrix));

	glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
	glBufferData(GL_ARRAY_BUFFER, 42 * sizeof(Line), m_lines, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//Specify where our vertex array is, how many components each vertex has,
	//The data type of each component and whether the data is normalised or not.
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

	glDrawArrays(GL_LINES, 0, 42 * 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
}

bool _3DRenderingFramework::LoadObjModelData(std::string a_sFilename, float a_fModelScale)
{
	//Initialise file path/name variables.
	std::string filePath = "resource/models/";
	std::string filename;//Local variable created as passed in filename may have subfolder.

	//Check if filename has a subfolder directory attached to it.
	std::vector<std::string> splitFilenamePath = CheckFileNameForSubFolder(a_sFilename);
	if (!splitFilenamePath[1].empty())//If the second part of this vector is not empty then that means a '/' was encountered in the filename so a subfolder was provided.
	{
		filePath.append(splitFilenamePath[0]);
		filename = splitFilenamePath[1];
	}
	else //If this else is triggered then that means no subfolder was encountered.
	{
		filename = splitFilenamePath[0];
	}

	filename = CheckFilenameForOBJPrefix(filename); //Add obj prefix to filename if required.

	//Create a new obj model and load it.
	m_objModel = new OBJModel(filename, filePath.c_str());
	filePath = filePath + filename;
	if (m_objModel->Load(filePath.c_str(), a_fModelScale))
	{
		TextureManager* pTM = TextureManager::GetInstance();
		//Load in texture for model if any are present.
		for (int i = 0; i < m_objModel->GetMaterialCount(); i++)
		{
			OBJMaterial* mat = m_objModel->GetMaterialByIndex(i);
			for (int n = 0; n < OBJMaterial::TextureTypes::TextureTypes_Count; n++)
			{
				if (mat->textureFileNames[n].size() > 0)
				{
					unsigned int textureID = pTM->LoadTexture(mat->textureFileNames[n].c_str());
					mat->textureIDs[n] = textureID;
				}
			}
		}
		//Set up shaders for OBJ model rendering.
		//Create obj shader program.
		unsigned int obj_vertexShader = ShaderUtil::LoadShader("resource/shaders/obj_vertex.glsl", GL_VERTEX_SHADER);
		unsigned int obj_fragmentShader = ShaderUtil::LoadShader("resource/shaders/obj_fragment.glsl", GL_FRAGMENT_SHADER);
		m_objProgram = ShaderUtil::CreateProgram(obj_vertexShader, obj_fragmentShader);
		//Set up vertex and index buffer for obj rendering.
		glGenBuffers(2, m_objModelBuffer);
		//Set up vertex buffer data.
		glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteShader(obj_vertexShader);
		glDeleteShader(obj_fragmentShader);
	}
	else
	{
		std::cout << "\nFailed to load model: " << a_sFilename << std::endl;
		std::cout << "Check that the filename was entered correctly." << std::endl;
		return false;
	}
}

std::string _3DRenderingFramework::CheckFilenameForOBJPrefix(std::string a_sFilename)
{
	//Initialise return variables.
	std::string returnString = a_sFilename;

	//Check each character in filename string.
	bool addOBJPrefix = true;
	for (int i = 0; i < a_sFilename.size(); i++)
	{
		if (a_sFilename[i] == '.')
		{
			//Check if the next 3 characters are the obj prefix and if they are don't add the obj prefix to the filename.
			if (i + 1 < a_sFilename.size() && i + 2 < a_sFilename.size() && i + 3 <= a_sFilename.size())//Ensure it is safe to check the next 3 characters after the full stop.
			{
				bool firstChar = a_sFilename[i + 1] == 'o' || a_sFilename[i + 1] == 'O';
				bool secondChar = a_sFilename[i + 2] == 'b' || a_sFilename[i + 2] == 'B';
				bool thirdChar = a_sFilename[i + 3] == 'j' || a_sFilename[i + 3] == 'J';
				if (firstChar && secondChar && thirdChar) //If all 3 characters are correct then we don't need to add the obj prefix.
				{
					addOBJPrefix = false;
				}
			}
		}
	}

	//Add obj prefix if required then return string.
	if (addOBJPrefix)
	{
		returnString.append(".obj");
	}

	return returnString;
}


std::vector<std::string> _3DRenderingFramework::CheckFileNameForSubFolder(std::string a_sFilename)
{
	//Initialse return variables.
	std::vector<std::string> tempStringList = std::vector<std::string>();
	std::string sSubfolder = "";
	std::string sFilename = "";

	//Split a_sFilename at "/" and check both sides if there is a "/".
	bool addToFilename = false;
	for (int i = 0; i < a_sFilename.size(); i++)
	{
		if (a_sFilename[i] == '/')
		{
			sSubfolder = sSubfolder + a_sFilename[i];
			addToFilename = true;
		}
		else if (!addToFilename)
		{
			sSubfolder = sSubfolder + a_sFilename[i];
		}
		else if (addToFilename)
		{
			sFilename = sFilename + a_sFilename[i];
		}
	}

	//Add the sub strings to the string list.
	tempStringList.push_back(sSubfolder);
	tempStringList.push_back(sFilename);

	//Return the list.
	return tempStringList;
}


void _3DRenderingFramework::SetUpGridLines()
{
	//Create shader program.
	unsigned int vertexShader = ShaderUtil::LoadShader("resource/shaders/vertex.glsl", GL_VERTEX_SHADER);
	unsigned int fragmentShader = ShaderUtil::LoadShader("resource/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	m_uiProgram = ShaderUtil::CreateProgram(vertexShader, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Create a grid of lines to be drawn during our update.
	//Create a 10x10 square grid.
	m_lines = new Line[42];

	for (int i = 0, j = 0; i < 21; i++, j += 2)
	{
		m_lines[j].v0.position = glm::vec4(-10 + i, 0.0f, 10.0f, 1.0f);
		m_lines[j].v0.colour = (i == 10) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_lines[j].v1.position = glm::vec4(-10 + i, 0.0f, -10.0f, 1.0f);
		m_lines[j].v1.colour = (i == 10) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		m_lines[j + 1].v0.position = glm::vec4(10.0f, 0.0f, -10.0f + i, 1.0f);
		m_lines[j + 1].v0.colour = (i == 10) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_lines[j + 1].v1.position = glm::vec4(-10.0f, 0.0f, -10.0f + i, 1.0f);
		m_lines[j + 1].v1.colour = (i == 10) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	//Create a vertex buffer to hold our line data.
	glGenBuffers(1, &m_lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
	//Fill vertex buffer with line data.
	glBufferData(GL_ARRAY_BUFFER, 42 * sizeof(Line), m_lines, GL_STATIC_DRAW);


	//Enable the vertex array state, since we're sending in an array of vertices.
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//Specify where our vertex array is, how many components each vertex has,
	//the data type of each component and whether the data is normalised or not.
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void _3DRenderingFramework::onWindowResize(WindowResizeEvent* e)
{
	std::cout << "Member event handler called!!" << std::endl;
	HandleWindowResize(e);
	e->Handled();
}

void _3DRenderingFramework::CreateProjectionMatrix()
{
	//Create a perspective projection matrix with a 90 degree fov and widescreen aspect ratio.
	m_projectionMatrix =
		glm::perspective(
			glm::pi<float>() * 0.25f,
			m_windowWidth / (float)m_windowHeight,
			0.1f, 1000.0f);
}

void _3DRenderingFramework::HandleWindowResize(WindowResizeEvent* e)
{
	m_windowWidth = e->GetWidth();
	m_windowHeight = e->GetHeight();
	if (m_windowWidth > 0 && m_windowHeight > 0)
	{
		glViewport(0, 0, m_windowWidth, m_windowHeight);
		CreateProjectionMatrix();
	}
}


void _3DRenderingFramework::Destroy()
{
	delete m_objModel;
	delete[] m_lines;
	glDeleteBuffers(1, &m_lineVBO);
	ShaderUtil::DeleteProgram(m_uiProgram);
	ShaderUtil::DeleteProgram(m_objProgram);
	TextureManager::DestroyInstance();
	ShaderUtil::DestroyInstance();
}