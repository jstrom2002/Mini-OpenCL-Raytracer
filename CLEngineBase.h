#pragma once
#include "stdafx.h"
#include <memory>

namespace Glaze3D
{
    class CLRaytracer;

    class CLEngineBase 
    {
    public:
        bool isInitialized = false;
        bool windowClose = false;
        int window_width = 1280;
        int window_height = 720;
        float lastFrame = 0;
        float deltaTime = 0;
        GLFWwindow* window = nullptr;
        std::shared_ptr<CLRaytracer> render = nullptr;

        double m_StartFrameTime;
        double m_PreviousFrameTime;
        HGLRC m_GLContext;

        CLEngineBase();
        void initialize();
        void processInput();
        void renderLoop();

        // Shader For rendering output.
        GLuint shaderID = 0;
    };
}