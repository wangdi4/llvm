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
    DEVICE_TYPE SelectDevice(const char* cpuArch);
}

class GlobalCompilerConfig: public IGlobalCompilerConfig
{
public:
    void LoadDefaults();
    void LoadConfig();
    void SkipBuiltins();
    void ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions);

    bool EnableTiming() const { return m_enableTiming;  }
    bool DisableStackDump() const { return m_disableStackDump; }
    std::string InfoOutputFile() const { return m_infoOutputFile; }

private:
    bool m_enableTiming;
    bool m_disableStackDump;
    std::string m_infoOutputFile;
};



//*****************************************************************************************
// CompilerConfig implementation. 
// 
// Responsible for loading both default configuration as well as runtime
// configuration passed to the backend.
//
// This class is used as an adapter from ICLDevBackendOptions to ICompilerConfig 
class CompilerConfig: public virtual ICompilerConfig
{
public:
    CompilerConfig():m_transposeSize(TRANSPOSE_SIZE_AUTO),
                     m_useVTune(false),
                     m_loadBuiltins(true),
                     m_DumpIROptionAfter(NULL), 
                     m_DumpIROptionBefore(NULL),
                     m_dumpHeuristicIR(false) {}

    // CompilerConfiguration methods
    void LoadDefaults();
    virtual void LoadConfig();
    void SkipBuiltins();
    void ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions);
    bool IsValidTransposeSize();

    std::string GetCpuArch() const     { return m_cpuArch; }
    std::string GetCpuFeatures() const { return m_cpuFeatures; }
    ETransposeSize GetTransposeSize() const   { return m_transposeSize; }
    bool  GetUseVTune() const                 { return m_useVTune; }
    bool  GetLoadBuiltins() const             { return m_loadBuiltins; }
    std::vector<int> GetIRDumpOptionsAfter() const
    {
        if(!m_DumpIROptionAfter){
            std::vector<int> tempVecotr;
            return tempVecotr;
        }
        std::vector<int> optionsVector(m_DumpIROptionAfter->begin(), m_DumpIROptionAfter->end());
        //sort the vector for later use (binary_search)
        std::sort(optionsVector.begin(), optionsVector.end());
        return optionsVector;
    }
    std::vector<int> GetIRDumpOptionsBefore() const
    {
       if(!m_DumpIROptionBefore){
            std::vector<int> tempVecotr;
            return tempVecotr;
        }
        std::vector<int> optionsVector(m_DumpIROptionBefore->begin(), m_DumpIROptionBefore->end());
        //sort the vector for later use (binary_search)
        std::sort(optionsVector.begin(), optionsVector.end());
        return optionsVector;
    }
    std::string GetDumpIRDir() const { return m_dumpIRDir; }

    bool GetDumpHeuristicIRFlag() const { return m_dumpHeuristicIR; }

protected:
    std::string m_cpuArch;
    std::string m_cpuFeatures;
    ETransposeSize m_transposeSize;
    bool        m_useVTune;
    bool        m_loadBuiltins;
    const std::vector<IRDumpOptions>* m_DumpIROptionAfter;
    const std::vector<IRDumpOptions>* m_DumpIROptionBefore;
    std::string m_dumpIRDir;
    bool m_dumpHeuristicIR;

};

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


//*****************************************************************************************
// Represents the global backend configuration. 
// It is a singleton that must be initialized explicitly
// 
class BackendConfiguration
{
private:
    BackendConfiguration();
    ~BackendConfiguration();

public:
    /**
     * Static initialization. Must be called once, in single threaded
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
    /**
     * Returns the compiler instance configuration 
     */
    MICCompilerConfig GetMICCompilerConfig(const ICLDevBackendOptions* pBackendOptions) const;

private:
    static BackendConfiguration* s_pInstance;
};

}}}
