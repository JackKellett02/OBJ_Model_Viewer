#pragma once
#include "Application.h"
#include "ApplicationEvent.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
//Forward declare OBJ model.
class OBJModel;
class Skybox;

class _3DRenderingFramework : public Application
{
public:
	_3DRenderingFramework();
	virtual ~_3DRenderingFramework();

	void onWindowResize(WindowResizeEvent* e);
protected:
	virtual bool OnCreate(std::string a_modelToLoad, float a_modelScale);
	virtual void Update(float deltaTime);
	virtual void Draw();
	virtual void Destroy();
	virtual void CreateProjectionMatrix();
	virtual void HandleWindowResize(WindowResizeEvent* e);

private:
	//Functions to set up and render the grid lines.
	void SetUpGridLines();
	void RenderGridLines(glm::mat4 a_projectionViewMatrix);

	//Functions to set up and render all obj models.
	bool LoadObjModelData(std::string a_sFilename, float a_fModelScale);
	void RenderOBJModel(OBJModel* a_model, glm::mat4 a_projectionViewMatrix);
	std::vector<std::string> CheckFileNameForSubFolder(std::string a_sFilename);
	std::string CheckFilenameForOBJPrefix(std::string a_sFilename);

	//Structure for a simple vertex - interleaved (position, colour)
	typedef struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
	}Vertex;

	//Structure for a line.
	typedef struct Line
	{
		Vertex v0;
		Vertex v1;
	}Line;

	glm::mat4 m_cameraMatrix;
	glm::mat4 m_projectionMatrix;

	//Shader programs.
	unsigned int m_uiProgram;
	unsigned int m_objProgram;
	unsigned int m_lineVBO;
	unsigned int m_objModelBuffer[2];
	float m_lightStrength;

	//Model.
	std::vector<OBJModel*> m_objList;
	OBJModel* m_objModel;
	Line* m_lines;
	Skybox* m_skybox = nullptr;

	glm::vec4 m_defaultMaterialColour;
};