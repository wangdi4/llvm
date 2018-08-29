// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "crt_config.h"
#include "crt_registry.h"

CrtConfig::CrtConfig()
{
}

CrtConfig::~CrtConfig()
{
}


cl_uint CrtConfig::getNumPlatforms()
{
    return (cl_uint)m_libraryNames.size();
}

std::string& CrtConfig::getPlatformLibName(cl_uint index)
{
    return m_libraryNames[index];
}

crt_err_code CrtConfig::Init()
{
    std::string cpuRuntimeLibName = "intelocl";
    std::string gpuRuntimeLibName = "igdrcl";

    // CPU-runtime library:
    std::string libName;
    std::string valueName;

    if (emulatorEnabled())
    {
        valueName = "cpu_2_1_emulator_path";
    }
    else
    {
        valueName = "cpu_path";
    }

    if( OCLCRT::Utils::GetCpuPathFromRegistry( valueName, libName ) )
    {
        // full path library name
        libName = libName + "\\" + OCLCRT::Utils::FormatLibNameForOS( cpuRuntimeLibName );
    }
    else
    {
        libName = OCLCRT::Utils::FormatLibNameForOS( cpuRuntimeLibName );
    }
    m_libraryNames.push_back( libName );

    if (!emulatorEnabled())
    {
        // GPU-runtime library:
        m_libraryNames.push_back( OCLCRT::Utils::FormatLibNameForOS( gpuRuntimeLibName ) );
    }

    return CRT_SUCCESS;
}

bool CrtConfig::emulatorEnabled()
{
#if defined( _WIN32 ) && defined( BUILD_OPENCL_21 )
    return true;
#endif // defined( _WIN32 )
    return false;
}
