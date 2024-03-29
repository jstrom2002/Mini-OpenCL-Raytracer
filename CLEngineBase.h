#pragma once
#include "stdafx.h"
#include <memory>
#include "CLCamera.h"
#include "CLLight.h"

namespace Glaze3D
{
    class CLRaytracer;
    class CLui;

    // Helper class for demo code, delete in release.
    class CLEngineBase 
    {
    public:
        class BasicShapes
        {
        public:
            BasicShapes() {}
            ~BasicShapes() {}

            void clear() {
                if (quadVAO) { glDeleteVertexArrays(1, &quadVAO); quadVAO = 0; }
                if (quadVBO) { glDeleteBuffers(1, &quadVBO); quadVBO = 0; }
            }

            void drawQuad() {
                if (this->quadVAO == 0) {
                    float quadVertices[] = {// dimensions equal to NDC dims = [-1,1], opposite face will be culled.
                        // positions         // texCoords
                        //Front side.
                        -1.0f,  1.0f,  0.001f,  0.0f, 1.0f,
                        -1.0f, -1.0f,  0.001f,  0.0f, 0.0f,
                         1.0f, -1.0f,  0.001f,  1.0f, 0.0f,

                        -1.0f,  1.0f,  0.001f,  0.0f, 1.0f,
                         1.0f, -1.0f,  0.001f,  1.0f, 0.0f,
                         1.0f,  1.0f,  0.001f,  1.0f, 1.0f,

                         //Back side.
                        -1.0f,  1.0f,  -0.001f,  0.0f, 1.0f,
                         1.0f, -1.0f,  -0.001f,  1.0f, 0.0f,
                        -1.0f, -1.0f,  -0.001f,  0.0f, 0.0f,

                        -1.0f,  1.0f,  -0.001f,  0.0f, 1.0f,
                         1.0f,  1.0f,  -0.001f,  1.0f, 1.0f,
                         1.0f, -1.0f,  -0.001f,  1.0f, 0.0f
                    };

                    // setup plane VAO
                    glGenVertexArrays(1, &quadVAO);
                    glGenBuffers(1, &quadVBO);
                    glBindVertexArray(quadVAO);

                    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
                    glBindVertexArray(0);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 12);
                glBindVertexArray(0);
            }
        private:
            unsigned int quadVAO = 0;
            unsigned int quadVBO = 0;
        }basicShapes;

        bool isInitialized = false;
        bool windowClose = false;
        float lastFrame = 0;
        float deltaTime = 0;
        float FPS = 0;
        GLFWwindow* window = nullptr;
        std::shared_ptr<CLRaytracer> render = nullptr;
        std::shared_ptr<CLui> ui = nullptr;
        CLCamera m_Camera;
        CLLight m_Light;

        HWND m_hWnd;//HWND is used by CLui
        HGLRC m_GLContext;

        CLEngineBase();
        void init();
        void processInput();
        void renderLoop();

        // Shader For rendering output frame.
        GLuint shaderID = 0;
    };
}