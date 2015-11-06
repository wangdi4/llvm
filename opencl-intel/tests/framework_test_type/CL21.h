#ifndef __CL21__
#define __CL21__

#include <iostream>
#include <gtest/gtest.h>
#include "CL/cl.h"
#include "test_utils.h"
#include "CL/cl_platform.h"
#include "CL_BASE.h"

class CL21 : public ::CL_base
{
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;

protected:

    // clEnqueueSVMMigrateMem

    void EnqueueSVMMigrateMem_Positive() const;

    void EnqueueSVMMigrateMem_Negative() const;

    // clSetDefaultDeviceCommandQueue

    void SetDefaultDeviceCommandQueueOOO() const;

    void SetDefaultDeviceCommandQueue_Negative() const;

    void SetDefaultDeviceCommandQueueOOO_Profiling() const;

    void SetDefaultDeviceCommandQueueOOO_SubDevice() const;

    void SetDefaultDeviceCommandQueue_Get_Default_Queue_Query() const;

    // clGetKernelSubGroupInfo

    void GetKernelSubGroupInfo_MAX_SB_SIZE() const;

    void GetKernelSubGroupInfo_Negative() const;

    void GetKernelSubGroupInfo_SG_COUNT() const;

    void GetKernelSubGroupInfo_LOCAL_SIZE_FOR_SG_COUNT() const;

    void GetKernelSubGroupInfo_COMPILE_NUM_SUB_GROUPS() const;

    void GetKernelSubGroupInfo_MAX_NUM_SUB_GROUPS() const;

    // clGetDeviceInfo

    void GetDeviceInfo_INDEPENDENT_PROGRESS() const;

    void GetDeviceInfo_CL_DEVICE_MAX_NUM_SUB_GROUPS() const;

    // zero sized enqueue

    void ZeroSized_clEnqueueReadBuffer() const;

    void ZeroSized_clEnqueueWriteBuffer() const;

    void ZeroSized_clEnqueueNDRangeKernel() const;

    void ZeroSized_clEnqueueCopyBuffer() const;

    void ZeroSized_clEnqueueSVMFree() const;

    void ZeroSized_clEnqueueSVMFree_Negative() const;

    void ZeroSized_clEnqueueSVMMemcpy() const;

    void ZeroSized_clEnqueueSVMMemFill() const;

    void Init()
    {
        ASSERT_LE(OPENCL_VERSION::OPENCL_VERSION_2_1, ::CL_base::GetOCLVersion()) <<
            "Test required OpenCL2.1 version at least";

        m_platform = ::CL_base::GetPlatform();
        m_device   = ::CL_base::GetDeviceID();
        m_context  = ::CL_base::GetContext();
        m_queue    = ::CL_base::GetQueue();
    }

    void GetDummyKernel(cl_kernel& kern) const
    {
        const char* kernel = "\
            __kernel void dummy_kernel()\
            {\
                return;\
            }\
            ";
        const size_t kernel_size = strlen(kernel);
        cl_int iRet = CL_SUCCESS;

        cl_program program = clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

        iRet = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
        if( CL_SUCCESS != iRet )
        {
            std::string log("", 1000);
            clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, log.size(), &log[0], nullptr);
            std::cout << log << std::endl;
        }
        ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed. ";

        kern = clCreateKernel(program, "dummy_kernel", &iRet);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed. ";
    }


};
#endif /*__CL21__*/
