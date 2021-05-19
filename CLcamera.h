#pragma once
#include <glm/glm.hpp>

namespace Glaze3D
{
    struct CLCamera
    {
        glm::vec3 position = glm::vec3(0.0f, -25.0f, 8.5f);
        glm::vec3 front = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
        float pitch = 1.571f;
        float yaw = 1.571f;       

        void Update()
        {
            right = glm::cross(front, up);
            front = glm::vec3(std::cosf(yaw) * std::sinf(pitch), 
                std::sinf(yaw) * std::sinf(pitch), 
                std::cosf(pitch));
        }
    };
}