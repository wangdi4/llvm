#ifndef __CL_BASE__
#define __CL_BASE__

#include <iostream>
#include <gtest/gtest.h>
#include "CL/cl.h"
#include "test_utils.h"
#include "CL/cl_platform.h"
#include "cl_config.h"
#include "common_utils.h"

using Intel::OpenCL::Utils::OPENCL_VERSION;

extern cl_device_type gDeviceType;

class CL_base : public testing::Test
{
    static cl_platform_id m_platform;
    static cl_device_id m_device;
    static cl_context m_context;
    static cl_command_queue m_queue;
    static bool m_hasFailure;
    static OPENCL_VERSION m_version;

protected:

    static void SetUpTestCase();

    cl_platform_id   GetPlatform()   { return m_platform; }

    cl_device_id     GetDeviceID()   { return m_device;   }

    cl_context       GetContext()    { return m_context;  }

    cl_command_queue GetQueue()      { return m_queue;    }

    OPENCL_VERSION   GetOCLVersion() { return m_version;  }

    virtual void SetUp() final;

    virtual void TearDown() final;

    virtual void Init() = 0;

    static void TearDownTestCase()
    {
        // Do not release OpenCL stub intantionally, want to check shutdown mechanism.
    }

    void GetSimpleSPIRV(std::vector<char>& spirv) const
    {
        std::string filename = get_exe_dir() + "test.spv";
        std::fstream spirv_file(filename, std::fstream::in | std::fstream::binary | std::fstream::ate);
        ASSERT_TRUE((bool)(spirv_file.is_open())) << " Error while opening " << filename << " file. ";

        size_t length = spirv_file.tellg();
        spirv_file.seekg(0, spirv_file.beg);

        spirv.resize(length, 0);
        spirv_file.read(&spirv[0], length);
    }
};

#endif /*__CL_BASE__*/
