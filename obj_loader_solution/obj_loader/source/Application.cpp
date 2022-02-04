#include "Application.h"
#include "Utilities.h"
#include "ShaderUtil.h"
#include "Dispatcher.h"
#include "ApplicationEvent.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

//Include the OpenGL Header.
#include <glad/glad.h>
//Include GLFW header.
#include <GLFW/glfw3.h>

//Include iostream for console logging.
#include <iostream>

bool Application::Create(const char* a_applicationName, unsigned int a_windowWidth, unsigned int a_windowHeight, bool fullscreen)
{
	//Get User Input for which model they would like to load.
	std::string modelToLoad;
	std::cout << "Enter name of the obj model to load." << std::endl;
	std::cout << "Model must be located in the 'resource/models/' folder." << std::endl;
	std::cout << "Model Name Input: ";
	std::cin >> modelToLoad;

	float modelScale;
	std::cout << "\nEnter how much the model should be scaled by.\nE.G. 1 or 0.5" << std::endl;
	std::cout << "Scale Input: ";
	std::cin >> modelScale;
	std::cout << std::endl;

	//Initialise GLFW
	if (!glfwInit())
	{
		return false;
	}
	m_windowWidth = a_windowWidth;
	m_windowHeight = a_windowHeight;
	//create a windowed mode window and it's OpenGL context
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, a_applicationName,
		(fullscreen ? glfwGetPrimaryMonitor() : nullptr), nullptr);
	if (!m_window)
	{
		glfwTerminate();
		return false;
	}
	//make the window's context current
	glfwMakeContextCurrent(m_window);

	//Initialise GLAD - Load in GL Extensions.
	if (!gladLoadGL()) {
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return false;
	}

	//Get OpenGL version.
	int major = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
	int minor = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);
	int revision = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_REVISION);

	std::cout << "OpenGL Version: " << major << "." << minor << "." << revision << std::endl;

	//Set up glfw window resize callback function.
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow*, int w, int h)
		{
			//Call the global dispatcher to handle this function.
			Dispatcher* dp = Dispatcher::GetInstance();
			if (dp != nullptr)
			{
				dp->publish(new WindowResizeEvent(w, h), true);
			}
		});

	//Create dispatcher.
	Dispatcher::CreateInstance();

	//Set up IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//Set ImGui style.
	ImGui::StyleColorsDark();
	const char* glsl_version = "#version 150";
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	//Implement a call to the derived class onCreate function for any implementation specific code.
	bool result = OnCreate(modelToLoad, modelScale);
	if (result == false)
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
	return result;
}

void Application::Run(const char* a_applicationName, unsigned int a_windowWidth, unsigned int a_windowHeight, bool fullscreen)
{
	if (Create(a_applicationName, a_windowWidth, a_windowHeight, fullscreen))
	{
		Utility::ResetTimer();
		m_running = true;
		do
		{
			//Start the Imgui frame.
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//Calculate delta time.
			float deltaTime = Utility::TickTimer();

			//Show the frame data.
			ShowFrameData(true);

			//Update and render.
			Update(deltaTime);
			Draw();

			//Render imgui draw data.
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			//Swap front and back buffers.
			glfwSwapBuffers(m_window);
			//Poll for process events.
			glfwPollEvents();
		} while (m_running == true && glfwWindowShouldClose(m_window) == 0);

		Destroy();
	}

	ShaderUtil::DestroyInstance();
	Dispatcher::DestroyInstance();
	//Cleanup.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::ShowFrameData(bool a_bShowFrameData)
{
	const float DISTANCE = 10.0f;
	static int corner = 0;
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
	ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.3f);

	//Create imgui window.
	if (ImGui::Begin("Frame Data", &a_bShowFrameData, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Separator();
		ImGui::Text("Application Average: %.3f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);
		if (ImGui::IsMousePosValid())
		{
			ImGui::Text("Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);

		}
		else
		{
			ImGui::Text("Mouse Position: <invalid>");
		}
	}
	ImGui::End();
}
