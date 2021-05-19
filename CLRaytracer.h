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
        void         Init();
        void         RenderFrame();

        // Generic helper for setting CL kernel arguments.
        template <class T> bool SetUniform(int i, T& val);

        void SetupBuffers();
    
        std::shared_ptr<CLContext>  m_CLContext = nullptr;
        std::shared_ptr<CLKernel>   m_RenderKernel = nullptr;
        std::shared_ptr<CLBVHScene> m_Scene = nullptr;

        // Renderer values.
        unsigned int m_FrameCount = 1;
        int lightBounces = 9;
        int dofOn = 0;
        float skyboxIntensity = 1.0f;

        // Vars for output data.
        std::vector<float3> pixels;
        cl::Buffer m_OutputBuffer;
        unsigned int outputTexture = 0;
    };
}