#include "stdafx.h"
#include "CLEngineBase.h"

namespace Glaze3D { std::shared_ptr<CLEngineBase> eng = nullptr; }
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Glaze3D::eng = std::make_shared<Glaze3D::CLEngineBase>();
	Glaze3D::eng->renderLoop();
}