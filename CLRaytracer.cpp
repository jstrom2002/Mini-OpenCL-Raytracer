#include "CLRaytracer.h"
#include "stdafx.h"
#include "CLEngineBase.h"
#include <glm/gtc/type_ptr.hpp>
#include "CLBVHnode.h"
#include "CLutils.h"

namespace Glaze3D
{
    void CLRaytracer::RenderFrame()
    {
        if (outputTexture == 0) {
            glGenTextures(1, &outputTexture);
            glBindTexture(GL_TEXTURE_2D, outputTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, eng->window_width,
                eng->window_height, 0, GL_RGBA, GL_FLOAT, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Update uniforms.
        unsigned int seed = rand();
        eng->render->SetUniform<unsigned int>((int)RenderKernelArgument_t::FRAME_COUNT, m_FrameCount);
        eng->render->SetUniform<unsigned int>((int)RenderKernelArgument_t::FRAME_SEED, seed);

        // Execute kernel code, copy output to viewport pixels.
        unsigned int globalWorksize = eng->window_width * eng->window_height;
        if (pixels.size() != globalWorksize)
            pixels.resize(globalWorksize);
        eng->render->m_CLContext->ExecuteKernel(eng->render->m_RenderKernel, globalWorksize);
        eng->render->m_CLContext->ReadBuffer(m_OutputBuffer, pixels.data(), sizeof(float3) * globalWorksize);
        eng->render->m_CLContext->Finish();

        // Update output texture.
        glBindTexture(GL_TEXTURE_2D, outputTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, eng->window_width,
            eng->window_height, GL_RGBA, GL_FLOAT, pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        // Draw framebuffer to default frame.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, eng->window_width, eng->window_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(eng->shaderID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputTexture);
        basicShapes.drawQuad();
        glBindTexture(GL_TEXTURE_2D, 0);

        // Swap buffers, end render.
        glfwSwapBuffers(eng->window);
        ++m_FrameCount;
    }

    void CLRaytracer::Init()
    {
        std::vector<cl::Platform> all_platforms;
        cl::Platform::get(&all_platforms);
        if (all_platforms.empty())
        {
            throw std::exception("No OpenCL platforms found");
        }

        m_CLContext = std::make_shared<CLContext>(all_platforms[0]);
        std::vector<cl::Device> platform_devices;
        all_platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &platform_devices);
        m_RenderKernel = std::make_shared<CLKernel>("kernel_bvh.cl", platform_devices);
        
        SetupBuffers();
    }

    void CLRaytracer::SetupBuffers()
    {
        cl_int errCode;

        // Set output frame dimensions.
        eng->render->SetUniform<int>((int)RenderKernelArgument_t::WIDTH,eng->window_width);
        eng->render->SetUniform<int>((int)RenderKernelArgument_t::HEIGHT,eng->window_height);
        pixels.resize(eng->window_width * eng->window_height);

        // Set output buffer for frame image -- should use 'img' image for output data.
        m_OutputBuffer = cl::Buffer(eng->render->m_CLContext->GetContext(),
            CL_MEM_WRITE_ONLY, eng->window_width * eng->window_height * sizeof(float3), 0, &errCode);
        if (errCode)
            throw CLException("Failed to create output buffer", errCode);
        eng->render->SetUniform<cl::Buffer>((int)RenderKernelArgument_t::BUFFER_OUT, m_OutputBuffer);

        // Set max light bounces.
        eng->render->SetUniform<unsigned int>((int)RenderKernelArgument_t::LIGHT_BOUNCES, lightBounces);
    }

    template <class T>
    bool CLRaytracer::SetUniform(int i, T& val)
    {
        return eng->render->m_RenderKernel->SetArgument((RenderKernelArgument_t)i, &val, sizeof(T));
    }
    template bool CLRaytracer::SetUniform<cl::Buffer>(int i, cl::Buffer& val);
    template bool CLRaytracer::SetUniform<float>(int i, float& val);
    template bool CLRaytracer::SetUniform<float3>(int i, float3& val);
    template bool CLRaytracer::SetUniform<int>(int i, int& val);
    template bool CLRaytracer::SetUniform<unsigned int>(int i, unsigned int& val);
}