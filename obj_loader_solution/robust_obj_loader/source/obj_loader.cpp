#include "obj_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>

void OBJModel::Unload()
{
	m_meshes.clear();
}

bool OBJModel::Load(const char* a_filename, float a_scale)
{
	std::cout << "Attempting to open file: " << a_filename << std::endl;
	//Get an fstream to read in the file data.
	std::fstream file;
	file.open(a_filename, std::ios_base::in | std::ios_base::binary);
	//test to see if the file has opened in correctly.
	if (file.is_open())
	{
		std::cout << "\nSuccessfully Opened file: " << a_filename << std::endl;
		//Success file has been opened, verify contents of the file -- ie check that file is not zero length.
		file.ignore(std::numeric_limits<std::streamsize>::max());//Attempt to read the highest number of bytes from the file.
		std::streamsize fileSize = file.gcount(); //gCount will have reach EOF marker, letting us know number of bytes.
		file.clear(); //Clear EOF marker from being read.
		file.seekg(0, std::ios_base::beg); //Seek back to begging of the file.
		if (fileSize == 0)
		{
			std::cout << "File contains no data, closing file!!" << std::endl;
			file.close();
		}
		std::cout << "File Size: " << fileSize / (float)1024 << "KB" << std::endl;

		OBJMesh* currentMesh = nullptr;
		std::string fileLine;
		std::vector<glm::vec4> vertexData;
		std::vector<glm::vec4> normalData;
		std::vector<glm::vec2> UVData;
		//Store out material in a string as face data is not generated prior to material assignment and may not have a mesh.
		OBJMaterial* currentMtl = nullptr;
		//Set up reading in chunks of a file at a time.
		std::cout << "\nPlease wait, processing file may take time!!!" << std::endl;
		std::string lastDataType = "";
		while (!file.eof())
		{
			if (std::getline(file, fileLine))
			{
				if (fileLine.size() > 0)
				{
					std::string dataType = LineType(fileLine);
					//If datatype has a 0 length then skip all tests and continue to next line.
					if (dataType.length() == 0) { continue; }
					std::string data = LineData(fileLine);

					if (dataType == "#") //This is a comment line.
					{
						std::cout << data << std::endl;
						continue;
					}
					if (dataType == "mtllib")
					{
						std::cout << "Material File: " << data << std::endl;
						//Load in Material file so that materials can be used as required.
						LoadMaterialLibrary(data);
						continue;
					}
					if (dataType == "g" || dataType == "o")
					{
						std::cout << "OBJ Group Found: " << data << std::endl;
						//We can use group tags to split our model up into smaller mesh components.
						if (currentMesh != nullptr)
						{
							m_meshes.push_back(currentMesh);
						}
						currentMesh = new OBJMesh();
						currentMesh->m_name = data;
						if (currentMtl != nullptr) //If we have a material name.
						{
							currentMesh->m_material = currentMtl;
							currentMtl = nullptr;
						}
					}
					if (dataType == "v")
					{
						glm::vec4 vertex = ProcessVectorString(data);
						vertex *= a_scale; //Multiply by passed in vector to allow scaling of the model.
						vertex.w = 1.0f; //As this is a position data ensure the w comp is set to 1.
						vertexData.push_back(vertex);
						continue;
					}
					if (dataType == "vt")
					{
						glm::vec4 uvCoordv4 = ProcessVectorString(data);
						glm::vec2 uvCoord = glm::vec2(uvCoordv4.x, uvCoordv4.y);
						UVData.push_back(uvCoord);
						continue;
					}
					if (dataType == "vn")
					{
						glm::vec4 normal = ProcessVectorString(data);
						normal.w = 0.0f;
						normalData.push_back(normal);
						continue;
					}
					if (dataType == "f")
					{
						if (currentMesh == nullptr) //We have entered processing faces without having hit a '0' or 'g' tag.
						{
							currentMesh = new OBJMesh();
							if (currentMtl != nullptr) //If we have a material name.
							{
								currentMesh->m_material = currentMtl;
								currentMtl = nullptr;
							}
						}
						//Process face data.
						//Face consists of 3 -> More vertices split at ' ' then at '/' characters.
						std::vector<std::string> faceData = SplitStringAtCharacter(data, ' ');
						unsigned int ci = currentMesh->m_vertices.size();
						for (auto iter = faceData.begin(); iter != faceData.end(); iter++)
						{
							//Process face triplet.
							obj_face_triplet triplet = ProcessTriplet(*iter);
							//Triplet processed now set Vertex Data from position/normal/texture data.
							OBJVertex currentVertex;
							currentVertex.position = vertexData[triplet.v - 1];
							if (triplet.vn != 0)
							{
								currentVertex.normal = normalData[triplet.vn - 1];
							}
							if (triplet.vt != 0)
							{
								currentVertex.uvcoord = UVData[triplet.vt - 1];
							}
							currentMesh->m_vertices.push_back(currentVertex);
						}
						//All face information for the tri/quad/fan have been collected.
						//Time to index these into the current mesh.
						//Test to see if OBJ file contains normal data if normalData is empty then there are no normals.
						//normalData.clear(); //ONLY TO TEST. REMOVE LATER TO FIX.
						//std::cout << "Normal data buffer size: " << normalData.size() << std::endl;
						bool calcNormals = normalData.empty();
						for (unsigned int offset = 1; offset < (faceData.size() - 1); offset++)
						{
							currentMesh->m_indices.push_back(ci);
							currentMesh->m_indices.push_back(ci + offset);
							currentMesh->m_indices.push_back(ci + 1 + offset);
							if (calcNormals)//If we need to calculate the normals we can do that here.
							{
								glm::vec4 normal = currentMesh->CalculateFaceNormal(
									ci,
									ci + offset,
									ci + offset + 1);
								currentMesh->m_vertices[ci].normal = normal;
								currentMesh->m_vertices[ci + offset].normal = normal;
								currentMesh->m_vertices[ci + offset + 1].normal = normal;
							}
						}
					}
					if (dataType == "usemtl")
					{
						//We have a material to use on the current mesh.
						OBJMaterial* mtl = GetMaterialByName(data.c_str());
						if (mtl != nullptr)
						{
							currentMtl = mtl;
							if (currentMesh != nullptr)
							{
								currentMesh->m_material = currentMtl;
							}
						}
					}
					lastDataType = dataType;
				}
			}
		}
		if (currentMesh != nullptr)
		{
			m_meshes.push_back(currentMesh);
		}
		file.close();
		return true;
	}
	return false;
}

glm::vec4 OBJModel::ProcessVectorString(const std::string a_data)
{
	//Split the line data at each space character and store this as a float value within a glm::vec4.
	std::stringstream iss(a_data);
	glm::vec4 vecData = glm::vec4(0.0f);
	int i = 0;
	for (std::string val; iss >> val; i++)
	{
		float fVal = std::stof(val);
		vecData[i] = fVal;
	}
	return vecData;
}

std::vector<std::string> OBJModel::SplitStringAtCharacter(std::string data, char a_character)
{
	std::vector<std::string> lineData;
	//Split the line data at each space character and store this as a float value withing a glm::vec4.
	std::stringstream iss(data);
	std::string lineSegment;
	while (std::getline(iss, lineSegment, a_character))
	{
		lineData.push_back(lineSegment);
	}
	return lineData;
}

void OBJModel::LoadMaterialLibrary(std::string a_mtllib)
{
	std::string matFile = m_path + a_mtllib;
	std::cout << "Attempting to load material file: " << matFile << std::endl;
	//get an fstream to read in the file data.
	std::fstream file;
	file.open(matFile, std::ios_base::in | std::ios_base::binary);
	//test to see if the file has opened in correctly.
	if (file.is_open())
	{
		std::cout << "Material Library SuccessFully Opened!!!" << std::endl;
		//Success file has been opened, verify contents of the file -- ie check that file is not zero length.
		file.ignore(std::numeric_limits<std::streamsize>::max()); //Attempt to read highest num bytes.
		std::streamsize fileSize = file.gcount(); //gCount will reach EOF marker, letting us know num bytes.
		file.clear(); //Clear EOF marker from being read.
		file.seekg(0, std::ios_base::beg); //Seek back to beginning of file.
		if (fileSize == 0)
		{
			std::cout << "File contains no data, closing file!!" << std::endl;
			file.close();
		}
		std::cout << "Material File Size: " << fileSize / (float)1024 << "KB" << std::endl;

		//Variable to store file data as it is read line by line.
		std::string fileLine;
		OBJMaterial* currentMaterial = nullptr;
		while (!file.eof())
		{
			if (std::getline(file, fileLine))
			{
				if (fileLine.size() > 0)
				{
					std::string dataType = LineType(fileLine);
					//If datatype has a 0 length then skip all tests and continue to next line.
					if (dataType.length() == 0) { continue; }
					std::string data = LineData(fileLine);

					if (dataType == "#") //This is a comment line.
					{
						std::cout << data << std::endl;
						continue;
					}
					if (dataType == "newmtl")
					{
						std::cout << "New Material Found: " << data << std::endl;
						if (currentMaterial != nullptr)
						{
							m_materials.push_back(currentMaterial);
						}
						currentMaterial = new OBJMaterial();
						currentMaterial->name = data;
						continue;
					}
					if (dataType == "Ns")
					{
						if (currentMaterial != nullptr)
						{
							//NS is guaranteed to be a single float value.
							currentMaterial->kS.a = std::stof(data);
						}
						continue;
					}
					if (dataType == "Ka")
					{
						if (currentMaterial != nullptr)
						{
							//Process kA as vector string.
							float kAd = currentMaterial->kA.a; //Store alpha channel as may contain refractive index.
							currentMaterial->kA = ProcessVectorString(data);
							currentMaterial->kA.a = kAd;
						}
						continue;
					}
					if (dataType == "Kd")
					{
						if (currentMaterial != nullptr)
						{
							//Process kD as vector string.
							float kDa = currentMaterial->kD.a; //Store alpha channel as may contain dissolve value.
							currentMaterial->kD = ProcessVectorString(data);
							currentMaterial->kD.a = kDa;
						}
						continue;
					}
					if (dataType == "Ks")
					{
						if (currentMaterial != nullptr)
						{
							//Process Ks as vector string.
							float kSa = currentMaterial->kS.a; //Store alpha as may contain specular component.
							currentMaterial->kS = ProcessVectorString(data);
							currentMaterial->kS.a = kSa;
						}
						continue;
					}
					if (dataType == "Ke")
					{
						//KE is for emissive properties.
						//We will not need to support this for our purposes.
						continue;
					}
					if (dataType == "Ni")
					{
						if (currentMaterial != nullptr)
						{
							//This is the refractive index of the mesh (how light bends as it passes through
							//the material). We will store this in the alpha component of the ambient light (kA).
							currentMaterial->kA.a = std::stof(data);
						}
						continue;
					}
					if (dataType == "d" || dataType == "Tr") //Transparency/Opacity Tr = 1 - d.
					{
						if (currentMaterial != nullptr)
						{
							//This is the dissolve or alpha value of the material, we will store this in the kD alpha channel.
							currentMaterial->kD.a = std::stof(data);
							if (dataType == "Tr")
							{
								currentMaterial->kD.a = 1.0f - currentMaterial->kD.a;
							}
						}
						continue;
					}
					if (dataType == "illum")
					{
						//Illum describes the illumination model used to light the model.
						//Ignore this for now as we will light the scene ourselves.
						continue;
					}
					if (dataType == "map_Kd") //Diffuse texture.
					{
						std::vector<std::string> mapData = SplitStringAtCharacter(data, ' ');
						currentMaterial->textureFileNames[OBJMaterial::TextureTypes::DiffuseTexture] =
							m_path + mapData[mapData.size() - 1]; //We are only interested in the file name.
						//and other data is hot garbage as far as our loader is concerned.
						continue;
					}
					if (dataType == "map_Ks") //Specular texture.
					{
						std::vector<std::string> mapData = SplitStringAtCharacter(data, ' ');
						currentMaterial->textureFileNames[OBJMaterial::TextureTypes::SpecularTexture] =
							m_path + mapData[mapData.size() - 1]; //We are only interested in the file name
						//and other data is hot garbage as far as our loader is concerned.
						continue;
					}
					if (dataType == "map_bump" || dataType == "bump") //Normal map texture.
					{
						std::vector<std::string> mapData = SplitStringAtCharacter(data, ' ');
						currentMaterial->textureFileNames[OBJMaterial::TextureTypes::NormalTexture] =
							m_path + mapData[mapData.size() - 1]; //We are only interested in the file name
						//and other data is hot garbage as far as our loader is concerned.
						continue;
					}
				}
			}
		}
		if (currentMaterial != nullptr)
		{
			m_materials.push_back(currentMaterial);
		}
		file.close();
	}
	else
	{
		std::cout << "Could not open material file: " << a_mtllib << std::endl;
	}
}

OBJMaterial* OBJModel::GetMaterialByName(const char* a_name)
{
	for (auto iter = m_materials.begin(); iter != m_materials.end(); iter++)
	{
		OBJMaterial* mat = (*iter);
		if (mat->name == a_name)
		{
			return mat;
		}
		return nullptr;
	}
	return nullptr;
}

OBJMaterial* OBJModel::GetMaterialByIndex(unsigned int a_index)
{
	unsigned int materialCount = m_materials.size();
	if (materialCount > 0 && a_index < materialCount)
	{
		return m_materials[a_index];
	}
	return nullptr;
}

OBJModel::obj_face_triplet OBJModel::ProcessTriplet(std::string a_triplet)
{
	std::vector<std::string> vertexIndices = SplitStringAtCharacter(a_triplet, '/');
	obj_face_triplet ft;
	ft.v = 0; ft.vn = 0; ft.vt = 0;
	ft.v = std::stoi(vertexIndices[0]);
	if (vertexIndices.size() >= 2)
	{
		if (vertexIndices[1].size() > 0)
		{
			ft.vt = std::stoi(vertexIndices[1]);
		}
		if (vertexIndices.size() >= 3)
		{
			ft.vn = std::stoi(vertexIndices[2]);
		}
	}
	return ft;
}

std::string OBJModel::LineType(const std::string& a_in)
{
	if (!a_in.empty())
	{
		size_t token_start = a_in.find_first_not_of(" \t");
		size_t token_end = a_in.find_first_of(" \t", token_start);
		//test to see if the start token is valid, test to see if the end token is valid.
		if (token_start != std::string::npos && token_end != std::string::npos)
		{
			return a_in.substr(token_start, token_end - token_start);
		}
		else if (token_start != std::string::npos)
		{
			return a_in.substr(token_start);
		}
	}
	return "";
}

std::string OBJModel::LineData(const std::string& a_in)
{
	//Get the token part of the line.
	size_t token_start = a_in.find_first_not_of(" \t");
	size_t token_end = a_in.find_first_of(" \t", token_start);
	//Fine the data part of the current line.
	size_t data_start = a_in.find_first_not_of(" \t", token_end);
	size_t data_end = a_in.find_last_not_of(" \t\n\r");

	if (data_start != std::string::npos && data_end != std::string::npos)
	{
		return a_in.substr(data_start, data_end - data_start + 1);
	}
	else if (data_start != std::string::npos)
	{
		return a_in.substr(data_start);
	}
	return "";
}

OBJMesh* OBJModel::GetMeshByIndex(unsigned int a_index)
{
	unsigned int meshCount = m_meshes.size();
	if (meshCount > 0 && a_index < meshCount)
	{
		return m_meshes[a_index];
	}
}

OBJMesh* OBJModel::GetMeshByName(const char* a_name)
{
	for (int i = 0; i < m_meshes.size(); i++)
	{
		if (m_meshes[i]->m_name == a_name)
		{
			return m_meshes[i];
		}
	}
}

glm::vec4 OBJMesh::CalculateFaceNormal(const unsigned int& a_indexA, const unsigned int& a_indexB, const unsigned int& a_indexC) const
{
	glm::vec3 a = m_vertices[a_indexA].position;
	glm::vec3 b = m_vertices[a_indexB].position;
	glm::vec3 c = m_vertices[a_indexC].position;

	glm::vec3 ab = glm::normalize(b - a);
	glm::vec3 ac = glm::normalize(c - a);
	return glm::vec4(glm::cross(ab, ac), 0.0f);
}

void OBJMesh::CalculateFaceNormals()
{
	//As our indexed triangle array contains a tri for each three points we can iterate through this vector and calculate a face normal.
	for (int i = 0; i < m_indices.size(); i += 3)
	{
		if (i < m_indices.size() - 100)
		{
			glm::vec4 normal = CalculateFaceNormal(i, i + 1, i + 2);
			//Set face normal to each vertex for the tri.
			m_vertices[i].normal = m_vertices[i + 1].normal - m_vertices[i + 2].normal - normal;
		}
		else
		{
			glm::vec4 normal = CalculateFaceNormal(i, i - 1, i - 2);
			//Set face normal to each vertex for the tri.
			m_vertices[i].normal = m_vertices[i - 1].normal - m_vertices[i - 2].normal - normal;
		}

	}
}