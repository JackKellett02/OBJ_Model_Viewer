#include "ShaderUtil.h"
#include "Utilities.h"
#include <glad/glad.h>
#include <iostream>

//Static Instance of ShaderUtil.
ShaderUtil* ShaderUtil::mInstance = nullptr;
//Singleton creation, fetch and destroy functionality.
ShaderUtil* ShaderUtil::GetInstance()
{
	if (mInstance == nullptr)
	{
		return ShaderUtil::CreateInstance();
	}
	return mInstance;
}

ShaderUtil* ShaderUtil::CreateInstance()
{
	if (mInstance == nullptr)
	{
		mInstance = new ShaderUtil();
	}
	else
	{
		//Print to console that attempt made to create multiple instances of ShaderUtil.
		std::cout << "\nAttempt to create multiple instances of ShaderUtil!!!" << std::endl;
	}
	return mInstance;
}

void ShaderUtil::DestroyInstance()
{
	if (mInstance != nullptr)
	{
		delete mInstance;
		mInstance = nullptr;
	}
	else
	{
		//Print to console that attempt made to destroy null instance of ShaderUtil.
		std::cout << "\nAttempt to destroy null instance of ShaderUtil." << std::endl;
	}
}

ShaderUtil::ShaderUtil()
{
}

ShaderUtil::~ShaderUtil()
{
	//Delete any shaders that have not been unloaded.
	for(auto iter = mShaders.begin(); iter != mShaders.end(); iter++)
	{
		glDeleteShader(*iter);
	}
	//Destroy any programs that are still dangling about.
	for(auto iter = mPrograms.begin(); iter != mPrograms.end(); iter++)
	{
		glDeleteProgram(*iter);
	}
}

unsigned int ShaderUtil::LoadShader(const char* a_filename, unsigned int a_type)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	return instance->LoadShaderInternal(a_filename, a_type);
}

unsigned int ShaderUtil::LoadShaderInternal(const char* a_filename, unsigned int a_type)
{
	//Integer to test for shader creation success.
	int success = GL_FALSE;
	//Grab the shader source from the file.
	char* source = Utility::FileToBuffer(a_filename);
	unsigned int shader = glCreateShader(a_type);
	//Set the source buffer for the shader.
	glShaderSource(shader, 1, (const char**)&source, 0);
	glCompileShader(shader);
	//As the buffer from fileToBuffer was allocated this needs to be destroyed.
	delete[] source;

	//Test shader compilation for any errors and display them to console.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(GL_FALSE == success)//Shader compilation failed, get logs and display them to console.
	{
		int infoLogLength = 0;//Variable to store the length of the error log.
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];//Allocate buffer to hold data.
		glGetShaderInfoLog(shader, infoLogLength, 0, infoLog);
		std::cout << "\nUnable to compile: " << a_filename << std::endl;
		std::cout << infoLog << std::endl;
		delete[] infoLog;
		return 0;
	}
	//Success - add shader to mShaders vector.
	mShaders.push_back(shader);
	return shader;
}

void ShaderUtil::DeleteShader(unsigned int a_shaderID)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	instance->DeleteShaderInternal(a_shaderID);
}

void ShaderUtil::DeleteShaderInternal(unsigned int a_shaderID)
{
	//Loop through the shaders vector.
	for(auto iter = mShaders.begin(); iter != mShaders.end(); iter++)
	{
		if(*iter == a_shaderID)//If we find the shader we are looking for.
		{
			glDeleteShader(*iter);//Delete the shader.
			mShaders.erase(iter);//Remove this item from the shaders vector.
			break;
		}
	}
}

unsigned int ShaderUtil::CreateProgram(const int& a_vertexShader, const int& a_fragmentShader)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	return instance->CreateProgramInternal(a_vertexShader, a_fragmentShader);
}

unsigned int ShaderUtil::CreateProgramInternal(const int& a_vertexShader, const int& a_fragmentShader)
{
	//Boolean value to test for shader program linkage success.
	int success = GL_FALSE;

	//Create a shader program and attach the shaders to it.
	unsigned int handle = glCreateProgram();
	glAttachShader(handle, a_vertexShader);
	glAttachShader(handle, a_fragmentShader);
	//Link the shaders together into one shader program.
	glLinkProgram(handle);
	//Test to see if the program was successfully created.
	glGetProgramiv(handle, GL_LINK_STATUS, &success);
	if(GL_FALSE == success)//If something has gone wrong then execute the following.
	{
		int infoLogLength = 0; //Integer value to tell us the length of the error log.
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
		//Allocate enough space in a buffer for the error message.
		char* infoLog = new char[infoLogLength];
		//Fill the buffer with data.
		glGetProgramInfoLog(handle, infoLogLength, 0, infoLog);
		//print log message to console.
		std::cout << "\nShader Linker Error" << std::endl;
		std::cout << infoLog << std::endl;

		//Delete the char buffer now we have displayed it.
		delete[] infoLog;
		return 0;//Return 0, programID 0 is a null program.
	}
	//Add the program to the shader program vector.
	mPrograms.push_back(handle);
	return handle; //Return the program ID.
}

void ShaderUtil::DeleteProgram(unsigned int a_program)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	instance->DeleteProgramInternal(a_program);
}

void ShaderUtil::DeleteProgramInternal(unsigned int a_program)
{
	//Loop through the shaders vector.
	for(auto iter = mPrograms.begin(); iter != mPrograms.end(); iter++)
	{
		if(*iter == a_program)//If we find the program we are looking for.
		{
			glDeleteProgram(*iter);//Delete the program.
			mPrograms.erase(iter);//Remove this item from the shaders vector.
			break;
		}
	}
}