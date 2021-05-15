#include "CLutils.h"
#include "CLEngineBase.h"
#include "CLRaytracer.h"
#include <algorithm>
#include <fstream>

namespace Glaze3D
{
    CLContext::CLContext(const cl::Platform& platform)
    {
        cl_context_properties props[] =
        {
            CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
            CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
            CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
            0
        };

        std::vector<cl::Device> platform_devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &platform_devices);
        if (platform_devices.empty())        
            throw std::exception("No devices found!");        
        cl_int errCode;
        m_Context = cl::Context(platform_devices, props, 0, 0, &errCode);
        if (errCode)        
            throw CLException("Failed to create context", errCode);        
        m_Queue = cl::CommandQueue(m_Context, platform_devices[0], 0, &errCode);
        if (errCode)        
            throw CLException("Failed to create queue", errCode);        
    }

    void CLContext::ReadBuffer(const cl::Buffer& buffer, void* data, size_t size) const
    {
        cl_int errCode = m_Queue.enqueueReadBuffer(buffer, false, 0, size, data);
        if (errCode)
            throw CLException("Failed to read buffer", errCode);
    }

    void CLContext::ExecuteKernel(std::shared_ptr<CLKernel> kernel, size_t workSize) const
    {
        cl_int errCode = m_Queue.enqueueNDRangeKernel(
            kernel->GetKernel(), cl::NullRange, cl::NDRange(workSize), cl::NullRange, 0);
        if (errCode)
            throw CLException("Failed to enqueue kernel", errCode);
    }

    CLKernel::CLKernel(const char* filename, const std::vector<cl::Device>& devices)
    {
        std::ifstream input_file(filename);
        if (!input_file)        
            throw std::exception("Failed to load kernel file!");        
        std::string source((std::istreambuf_iterator<char>(input_file)), (std::istreambuf_iterator<char>()));
        cl::Program program(eng->render->m_CLContext->GetContext(), source);
        cl_int errCode = program.build(devices, " -I . ");
        if (errCode != CL_SUCCESS)       
            throw CLException("Error building" + program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]), errCode);        
        m_Kernel = cl::Kernel(program, "KernelEntry", &errCode);
        if (errCode)        
            throw CLException("Failed to create kernel", errCode);       
    }

    bool CLKernel::SetArgument(RenderKernelArgument_t argIndex, void* data, size_t size)
    {
        cl_int errCode = m_Kernel.setArg(static_cast<unsigned int>(argIndex), size, data);
        if (errCode)
        {
            throw CLException("Failed to set kernel argument", errCode);
            return false;
        }
        return true;
    }
}