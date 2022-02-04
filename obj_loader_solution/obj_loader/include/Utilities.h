#pragma once
#include <glm/glm.hpp>

//A utility class with static helper methods.
class Utility
{
public:
	//Utilities for timing; Get() uppdates timers and returns time since last Get call.
	static void ResetTimer();
	static float TickTimer();
	static float GetDeltaTime();
	static float GetTotalTime();

	//Helper function for loading shader code into memory.
	static char* FileToBuffer(const char* a_szPath);

	//Utility for mouse / keyboard movement of a matrix transform (suitable for camera).
	static void FreeMovement(glm::mat4& a_transform,
		float a_deltaTime,
		float a_speed,
		const glm::vec3& a_up = glm::vec3(0, 1, 0));
};