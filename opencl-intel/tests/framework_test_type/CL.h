#ifndef __CL_GENERAL__
#define __CL_GENERAL__

#include <iostream>
#include <gtest/gtest.h>
#include "CL/cl.h"
#include "test_utils.h"
#include "CL/cl_platform.h"
#include "CL_BASE.h"

class CL : public ::CL_base
{
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;

protected:

    void Init()
    {
        m_platform = ::CL_base::GetPlatform();
        m_device   = ::CL_base::GetDeviceID();
        m_context  = ::CL_base::GetContext();
        m_queue    = ::CL_base::GetQueue();
    }

    void CheckExtensions();

    void QueryInvalidParamNameFromCQueue_Negative();

    void ZeroLocalSize();
};
#endif /*__CL_GENERAL__*/
