#pragma once
#include <GL/glew.h>
#ifdef _WIN32
#include <Windows.h>
#else
//PUT CROSS-PLATFORM STUFF HERE
#endif
#define CL_HPP_TARGET_OPENCL_VERSION 210
#include <CL/cl2.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define USE_IMGUI_TABLES
#include "imgui.h"
#include "imconfig.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Glaze3D
{
	class CLEngineBase;
	extern std::shared_ptr<CLEngineBase> eng;
}
