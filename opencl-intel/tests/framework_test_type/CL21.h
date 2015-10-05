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

    void Init()
    {
        ASSERT_LE(OPENCL_VERSION::OPENCL_VERSION_2_1, ::CL_base::GetOCLVersion()) <<
            "Test required OpenCL2.1 version at least";

        m_platform = ::CL_base::GetPlatform();
        m_device   = ::CL_base::GetDeviceID();
        m_context  = ::CL_base::GetContext();
        m_queue    = ::CL_base::GetQueue();
    }

};
#endif /*__CL21__*/
