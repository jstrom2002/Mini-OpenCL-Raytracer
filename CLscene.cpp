#include "CLScene.h"

namespace Glaze3D
{
	void CLscene::init()
	{
		m_Camera = std::make_shared<CLCamera>();
		m_Light = std::make_shared<CLLight>();

		isInitialized = true;
	}

	void CLscene::load(std::string filename)
	{

	}
}