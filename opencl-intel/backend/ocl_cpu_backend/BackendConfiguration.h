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
#include "MICCompilerConfig.h"
#include "ServiceFactory.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ICLDevBackendOptions;

namespace Utils
{
    OPERATION_MODE SelectOperationMode(const char* cpuArch);
}

//*****************************************************************************************
// CompilerConfig implementation. The main purpose of this class is to cut the dependancy
// between the compiler and compiler service on ICLDevBackendOptions interface
// 
class CompilerConfiguration: public CompilerConfig
{
public:
    // CompilerConfiguration methods
    void LoadDefaults();
    void LoadConfig();
    void SkipBuiltins();
    void ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions);
};

class MICCompilerConfiguration: public MICCompilerConfig
{
public:
    // MIC CompilerConfiguration methods
    void LoadDefaults();
    void LoadConfig();
    void SkipBuiltins();
    void ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions);
};


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
    static void Init(const ICLDevBackendOptions* pBackendOptions);
    /**
     * Termination. Must be called once, in single threaded
     * environment
     */
    static void Terminate();
    /**
     * Singleton instance getter
     */
    static const BackendConfiguration* GetInstance();
    /**
     * Returns the compiler specific configuration 
     */
    CompilerConfiguration GetCPUCompilerConfig() const ;
    MICCompilerConfiguration GetMICCompilerConfig() const ;
private:
    static BackendConfiguration* s_pInstance;
};

}}}