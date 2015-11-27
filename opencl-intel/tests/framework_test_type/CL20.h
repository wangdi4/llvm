#ifndef __CL20_
#define __CL20_

#include <iostream>
#include <gtest/gtest.h>
#include "CL/cl.h"
#include "test_utils.h"
#include "CL/cl_platform.h"
#include "CL_BASE.h"

class CL20 : public ::CL_base
{
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;

protected:

    void DoubleSetDefaultCommandQueue();

    void clSVMMap_FINE_GRAIN();

    void clSVMUnmap_FINE_GRAIN();

    void Init()
    {
//        ASSERT_LE(OPENCL_VERSION::OPENCL_VERSION_2_0, ::CL_base::GetOCLVersion()) <<
//            "Test required OpenCL2.0 version at least";

        m_platform = ::CL_base::GetPlatform();
        m_device   = ::CL_base::GetDeviceID();
        m_context  = ::CL_base::GetContext();
        m_queue    = ::CL_base::GetQueue();
    }

};
#endif /*__CL20_*/
