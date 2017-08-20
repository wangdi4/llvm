/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  BackendConfiguration.cpp

\*****************************************************************************/

#include "BackendConfiguration.h"
#include "cl_dev_backend_api.h"

#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

BackendConfiguration* BackendConfiguration::s_pInstance = nullptr;

void BackendConfiguration::Init()
{
    assert(!s_pInstance);
    s_pInstance = new BackendConfiguration();
}

void BackendConfiguration::Terminate()
{
    if( nullptr != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

const BackendConfiguration& BackendConfiguration::GetInstance()
{
    assert(s_pInstance);
    return *s_pInstance;
}

GlobalCompilerConfig BackendConfiguration::GetGlobalCompilerConfig( const ICLDevBackendOptions* pBackendOptions ) const
{
    GlobalCompilerConfig config;
    config.LoadDefaults();
    config.LoadConfig();
    config.ApplyRuntimeOptions(pBackendOptions);
    return config;
}

CompilerConfig BackendConfiguration::GetCPUCompilerConfig(const ICLDevBackendOptions* pBackendOptions ) const
{
    CompilerConfig config;
    config.LoadDefaults();
    config.LoadConfig();
    config.ApplyRuntimeOptions(pBackendOptions);
    return config;
}
#if defined(INCLUDE_MIC_DEVICE)
MICCompilerConfig BackendConfiguration::GetMICCompilerConfig(const ICLDevBackendOptions* pBackendOptions ) const
{
    MICCompilerConfig config;
    config.LoadDefaults();
    config.LoadConfig();
    config.ApplyRuntimeOptions(pBackendOptions);
    return config;
}
#endif

}}}
