#pragma once
#include "stdafx.h"
#include "CLutils.h"
#include "CLBVHnode.h"
#include "CLmathlib.hpp"
#include <memory>

namespace Glaze3D
{
    class CLContext;
    class CLKernel;
    class CLBVHScene;

    class CLRaytracer
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


        void         Init();
        void         RenderFrame();

        // Generic helper for setting CL kernel arguments.
        template <class T> bool SetUniform(int i, T& val);

        void SetupBuffers();
    
        std::shared_ptr<CLContext>  m_CLContext = nullptr;
        std::shared_ptr<CLKernel>   m_RenderKernel = nullptr;
        std::shared_ptr<BVHScene>   m_Scene = nullptr;

        unsigned int m_FrameCount = 1;
        unsigned int lightBounces = 3;

        // Vars for output data.
        std::vector<float3> pixels;
        cl::Buffer m_OutputBuffer;
        unsigned int outputTexture = 0;
    };
}