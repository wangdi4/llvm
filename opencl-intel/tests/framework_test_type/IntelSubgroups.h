#ifndef __CL_NATIVE_SUBGROUPS_
#define __CL_NATIVE_SUBGROUPS_

#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include "CL/cl.h"
#include "test_utils.h"
#include "CL/cl_platform.h"
#include "CL_BASE.h"

extern cl_device_type gDeviceType;

class NativeSubgroups : public ::testing::Test
{
    static cl_platform_id m_platform;
    static cl_device_id m_device;
    static cl_context m_context;
    static cl_command_queue m_queue;

protected:

    static void SetUpTestCase();

    cl_platform_id   GetPlatform()   { return m_platform; }

    cl_device_id     GetDeviceID()   { return m_device;   }

    cl_context       GetContext()    { return m_context;  }

    cl_command_queue GetQueue()      { return m_queue;    }

    virtual void SetUp() final;

    virtual void TearDown() final;

    virtual void Init();

    static void TearDownTestCase(); 

    void GetDummySubgroupKernel(cl_kernel& kern) const;

    // Tests

    void NativeSubgroups_MAX_SB_SIZE() const;

    void NativeSubgroups_SG_COUNT() const;

    void NativeSubgroups_LOCAL_SIZE_FOR_SG_COUNT() const;
};

#endif /*NativeSubgroups*/
