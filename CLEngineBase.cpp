#include "CLEngineBase.h"
#include "stdafx.h"
#include "CLRaytracer.h"
#include "CLBVHnode.h"
#include "CLOBJloader.h"
#include "CLmathlib.hpp"
#include <exception>

namespace Glaze3D
{
    CLEngineBase::CLEngineBase()
	{
		render = std::make_shared<CLRaytracer>();
	}

    void CLEngineBase::initialize()
    {
        if (isInitialized)
            return;

        // Create context and window.
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        // Get fullscreen width/height.
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        window_width = mode->width;
        window_height = mode->height;

        // Set hints to allow AA samples and double buffering.
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
        glfwWindowHint(GLFW_AUTO_ICONIFY, false);
        glfwWindowHint(GLFW_RESIZABLE, false);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//Declaring these hints will limit version to v4.5, which we may not want
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);//<== wont work with renderdoc
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        glfwWindowHint(GLFW_DECORATED, true);
        glfwSwapInterval(0); // Enable vsync.

        // Create window.
        window = glfwCreateWindow(window_width, window_height, "", nullptr, nullptr);
        glfwMakeContextCurrent(window);
        if (window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwSetWindowPos(window, 0, 0);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glewExperimental = true;
        glewInit();
        glfwMakeContextCurrent(eng->window);
        glfwGetWindowSize(window, &window_width, &window_height);

        // Init shader.
        const char* vertex_shader =
            "#version 450\n"
            "layout(location = 0) in vec3 Position;\n"
            "layout(location = 1) in vec2 UV;\n"
            "out vec2 Frag_UV;\n"
            "void main()\n"
            "{\n"
            "	Frag_UV = UV;\n"
            "	gl_Position = vec4(Position.xy,0,1);\n"
            "}\n";
        const char* fragment_shader =
            "#version 450\n"
            "layout(binding = 0) uniform sampler2D tx;\n"
            "in vec2 Frag_UV;\n"
            "out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "	Out_Color = texture(tx, Frag_UV);\n"
            "}\n";     

        GLint success;

        // vertex shader
        GLuint vertHandle = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertHandle, 1, &vertex_shader, NULL);
        glCompileShader(vertHandle);
        glGetShaderiv(vertHandle, GL_COMPILE_STATUS, &success);
        if (!success) 
        {
            char infoLog[1024];
            glGetShaderInfoLog(vertHandle, 1024, NULL, infoLog);
            MessageBox(0, infoLog, 0, 0);
        }

        // fragment Shader
        GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragHandle, 1, &fragment_shader, NULL);
        glCompileShader(fragHandle);
        glGetShaderiv(fragHandle, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            glGetShaderInfoLog(fragHandle, 1024, NULL, infoLog);
            MessageBox(0, infoLog, 0, 0);
        }

        shaderID = glCreateProgram();
        glAttachShader(shaderID, vertHandle);
        glAttachShader(shaderID, fragHandle);
        glLinkProgram(shaderID);
        glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            glGetProgramInfoLog(shaderID, 1024, NULL, infoLog);
            MessageBox(0, infoLog, 0, 0);
        }

        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertHandle);
        glDeleteShader(fragHandle);

        glUseProgram(shaderID);

        isInitialized = true;
    }

    void CLEngineBase::processInput()
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            windowClose = true;        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            eng->camera.position += eng->camera.front;
            eng->render->m_FrameCount = 1;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            eng->camera.position -= eng->camera.front;
            eng->render->m_FrameCount = 1;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            //eng->camera.position -= Cross(eng->camera.front, eng->camera.up);
            eng->render->m_FrameCount = 1;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            //eng->camera.position += Cross(eng->camera.front, eng->camera.up);
            eng->render->m_FrameCount = 1;
        }

    }

    void CLEngineBase::renderLoop()
    {
        initialize();

        try 
        {
            render->Init();
            eng->render->m_Scene = std::make_shared<CLBVHScene>();
            CLOBJloader obj;
            int maxPrimitives = 4;
            obj.Load(
                "cornell.obj"
                //"C:\\assets\\obj\\sibenik\\sibenik.obj"
                //"C:\\Users\\--\\source\\repos\\OpenCL-Raytracer2\\OpenCL-Raytracer2\\meshes\\city.obj"
                //"C:\\Users\\--\\source\\repos\\OpenCL-Raytracer2\\OpenCL-Raytracer2\\meshes\\dragon.obj"
                , maxPrimitives);
            eng->render->m_Scene->CreateBVHTrees(maxPrimitives);
        }
        catch (std::exception& ex) 
        {
            MessageBox(0, std::string("Caught exception: " + std::string(ex.what())).c_str(), 0, 0);
            return;
        }

        while (!eng->windowClose){
            float currentFrame = clock();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            glfwPollEvents();
            processInput();

            try 
            {
                render->RenderFrame();
            }
            catch (const std::exception& ex) 
            {
                MessageBox(0, std::string("Caught exception: " + std::string(ex.what())).c_str(), 0, 0);
                return;
            }
        }
    }
}