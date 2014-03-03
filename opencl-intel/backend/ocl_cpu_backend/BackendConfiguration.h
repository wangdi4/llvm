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
#include "CPUDetect.h"
#include "CompilerConfig.h"
#if defined(INCLUDE_MIC_DEVICE)
#include "IMICCompilerConfig.h"
#endif
#include "ServiceFactory.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ICLDevBackendOptions;

namespace Utils
{
    DEVICE_TYPE SelectDevice(const char* cpuArch);
}

#if defined(INCLUDE_MIC_DEVICE)
class MICCompilerConfig: public CompilerConfig, public IMICCompilerConfig
{
public:
    // MIC CompilerConfiguration methods
    void LoadConfig();
    void ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions);

    std::string GetCpuArch() const     { return CompilerConfig::GetCpuArch(); }
    std::string GetCpuFeatures() const { return CompilerConfig::GetCpuFeatures(); }
    ETransposeSize GetTransposeSize() const   { return CompilerConfig::GetTransposeSize(); }
    bool  GetUseVTune() const                 { return CompilerConfig::GetUseVTune(); }
    bool  GetLoadBuiltins() const             { return CompilerConfig::GetLoadBuiltins(); }
    std::vector<int> GetIRDumpOptionsAfter() const { return CompilerConfig::GetIRDumpOptionsAfter(); }
    std::vector<int> GetIRDumpOptionsBefore() const { return CompilerConfig::GetIRDumpOptionsBefore(); }
    std::string GetDumpIRDir() const { return CompilerConfig::GetDumpIRDir(); }
    bool GetDumpHeuristicIRFlag() const { return CompilerConfig::GetDumpHeuristicIRFlag(); }

    const TargetDescription& GetTargetDescription() const { return m_TargetDescription; }

protected:
    TargetDescription m_TargetDescription;

};
#endif

//*****************************************************************************************
// Represents the global backend configuration.
// It is a singletinon that must be initialized explicitly
//
class BackendConfiguration
{
private:
    BackendConfiguration();
    ~BackendConfiguration();

public:
    /**
     * Statis initialization. Must be called once, in single threaded
     * environment
     */
    static void Init();
    /**
     * Termination. Must be called once, in single threaded
     * environment
     */
    static void Terminate();
    /**
     * Singleton instance getter
     */
    static const BackendConfiguration& GetInstance();
    /**
     * Returns the global compiler configuration
     */
    GlobalCompilerConfig GetGlobalCompilerConfig( const ICLDevBackendOptions* pBackendOptions ) const;
    /**
     * Returns the compiler instance configuration
     */
    CompilerConfig GetCPUCompilerConfig(const ICLDevBackendOptions* pBackendOptions) const;
#if defined(INCLUDE_MIC_DEVICE)
    /**
     * Returns the compiler instance configuration
     */
    MICCompilerConfig GetMICCompilerConfig(const ICLDevBackendOptions* pBackendOptions) const;
#endif

private:
    static BackendConfiguration* s_pInstance;
};

}}}
