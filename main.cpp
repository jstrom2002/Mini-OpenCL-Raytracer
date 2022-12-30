#include "stdafx.h"
#include "CLEngineBase.h"
#include <locale>
#include <codecvt>
namespace Glaze3D { std::shared_ptr<CLEngineBase> eng = nullptr; }
int CALLBACK WinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       pCmdLine,
	int         nCmdShow
){
	Glaze3D::eng = std::make_shared<Glaze3D::CLEngineBase>();
	std::string fileToRender = "cornell.obj";
	Glaze3D::eng->renderLoop(fileToRender);
	return 0;
}