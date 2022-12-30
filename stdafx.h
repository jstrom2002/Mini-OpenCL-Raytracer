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


#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libxml2.lib")
#pragma comment(lib, "charset.lib")
#pragma comment(lib, "iconv.lib")
#pragma comment(lib, "turbojpeg.lib")
#pragma comment(lib, "poly2tri.lib")
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "minizip.lib")
#pragma comment(lib, "kubazip.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "cuda.lib")
#pragma comment(lib, "webp.lib")
#pragma comment(lib, "lzma.lib")
#pragma comment(lib, "pugixml.lib")
#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "Cfgmgr32.lib")


#ifdef _DEBUG
#pragma comment(lib, "bz2d.lib")
#pragma comment(lib, "assimp-vc143-mtd.lib")
#pragma comment(lib, "glew32d.lib")
//#pragma comment(lib, "IlmImf-3_1_d.lib")
#pragma comment(lib, "Iex-3_1_d.lib")
#pragma comment(lib, "IlmThread-3_1_d.lib")
#pragma comment(lib, "Imath-3_1_d.lib")
//#pragma comment(lib, "Half-3_1_d.lib")
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "tiffd.lib")
#pragma comment(lib, "libszip_D.lib")
#pragma comment(lib, "libhdf5_D.lib")
#pragma comment(lib, "lz4d.lib")
#pragma comment(lib, "libpng16d.lib")
#pragma comment(lib, "opencv_core4d.lib")
#pragma comment(lib, "opencv_imgcodecs4d.lib")
#pragma comment(lib, "opencv_imgproc4d.lib")
#else
#pragma comment(lib, "bz2.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "assimp-vc143-mt.lib")
#pragma comment(lib, "glew32.lib")
//#pragma comment(lib, "IlmImf-3_5.lib")
#pragma comment(lib, "Iex-3_1.lib")
#pragma comment(lib, "IlmThread-3_5.lib")
#pragma comment(lib, "Imath-3_5.lib")
//#pragma comment(lib, "Half-3_5.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "tiff.lib")
#pragma comment(lib, "lz4.lib")
#pragma comment(lib, "libpng16.lib")
#pragma comment(lib, "libszip.lib")
#pragma comment(lib, "libhdf5.lib")
#pragma comment(lib, "opencv_core4.lib")
#pragma comment(lib, "opencv_imgcodecs4.lib")
#pragma comment(lib, "opencv_imgproc4.lib")
#endif

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

namespace Glaze3D
{
	class CLEngineBase;
	extern std::shared_ptr<CLEngineBase> eng;
}
