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

File Name:  BackendConfiguration.h

\*****************************************************************************/
#pragma once
#include "CompilerConfig.h"
#if defined(INCLUDE_MIC_DEVICE)
#include "MICCompilerConfig.h"
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ICLDevBackendOptions;

//*****************************************************************************
// Represents the global backend configuration.
// It is a singletinon that must be initialized explicitly.
class BackendConfiguration
{
public:
    /**
     * Status initialization. Must be called once, in single threaded
     * environment.
     */
    static void Init();
    /**
     * Termination. Must be called once, in single threaded environment
     */
    static void Terminate();
    /**
     * Singleton instance getter.
     */
    static const BackendConfiguration& GetInstance();
    /**
     * Returns the global compiler configuration.
     */
    GlobalCompilerConfig GetGlobalCompilerConfig( const ICLDevBackendOptions* pBackendOptions ) const;
    /**
     * Returns the CPU compiler instance configuration.
     */
    CompilerConfig GetCPUCompilerConfig(const ICLDevBackendOptions* pBackendOptions) const;
#if defined(INCLUDE_MIC_DEVICE)
    /**
     * Returns the MIC compiler instance configuration.
     */
    MICCompilerConfig GetMICCompilerConfig(const ICLDevBackendOptions* pBackendOptions) const;
#endif

private:
    BackendConfiguration(){}
    ~BackendConfiguration(){}

    static BackendConfiguration* s_pInstance;
};

}}}
