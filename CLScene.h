#pragma once
#include "stdafx.h"
#include <memory>
#include "CLcamera.h"
#include "CLLight.h"

namespace Glaze3D
{
	class CLscene
	{
	public:		
		std::shared_ptr<CLCamera> m_Camera;
		std::shared_ptr<CLLight> m_Light;

		bool isInitialized = false;

		CLscene(){}
		void init();
		void load(std::string filename);
	};
}