////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Filename:       main.cpp
// Author:         Jack Kellett
// Date Created:   27/09/2021
// Brief:          To load and render and OBJ model.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "3DRenderingFramework.h"
#include <iostream>

#pragma region Function Declarations.

#pragma endregion

#pragma region Function Definitions.
int main(int argc, char* argv[]) {
	//Renderer.
	_3DRenderingFramework* myApp = new _3DRenderingFramework();
	myApp->Run("3D Rendering Framework Assignment", 1600, 900, false);
	delete myApp;
	return EXIT_SUCCESS;
}
#pragma endregion