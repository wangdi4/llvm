/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2014).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  MICCompilerConfig.h

\*****************************************************************************/

#ifndef MIC_COMPILER_CONFIG_H
#define MIC_COMPILER_CONFIG_H

#include "IMICCompilerConfig.h"
#include "CompilerConfig.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
class ICLDevBackendOptions;

class MICCompilerConfig: public CompilerConfig, public IMICCompilerConfig
{
public:
    // MIC CompilerConfiguration methods
    void LoadConfig();
    void ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions);

    std::string GetCpuArch() const     { return CompilerConfig::GetCpuArch(); }
    std::string GetCpuFeatures() const { return CompilerConfig::GetCpuFeatures(); }
    ETransposeSize GetTransposeSize() const   { return CompilerConfig::GetTransposeSize(); }
    int GetRTLoopUnrollFactor() const         { return 1; } // not used for MIC
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
}}}
#endif //MIC_COMPILER_CONFIG_H

