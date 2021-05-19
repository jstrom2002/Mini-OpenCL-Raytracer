#pragma once
#include <glm/glm.hpp>

namespace Glaze3D
{
	struct CLLight
	{
		glm::vec3 position = glm::vec3(0.0f, -10.0f, 16.0f);
		glm::vec3 direction = glm::vec3(-0.5f, 0.4f, -0.1f);
		int type = 0;//0 == directional, 1 == point, 2 == spot.
		float intensity = 1.0f;
		float attenuation = 0.6f;
	};
}