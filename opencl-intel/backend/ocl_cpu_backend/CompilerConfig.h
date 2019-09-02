// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#ifndef COMPILER_CONFIG_H
#define COMPILER_CONFIG_H

#include "ICompilerConfig.h"
#include "exceptions.h"

#include <algorithm>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

DEFINE_EXCEPTION(BadConfigException)

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
    std::string LLVMOptions() const { return m_LLVMOptions; }
    DeviceMode TargetDevice() const override { return m_targetDevice; }

private:
    bool m_enableTiming;
    bool m_disableStackDump;
    std::string m_infoOutputFile;
    std::string m_LLVMOptions;
    DeviceMode  m_targetDevice;
};



//*****************************************************************************************
// CompilerConfig implementation.
//
// Responsible for loading both default configuraiton as well as runtime
// configuration passed to the backend.
//
// This class is used as an adapter from ICLDevBackendOptions to ICompilerConfig
class CompilerConfig: public virtual ICompilerConfig
{
public:
    CompilerConfig():m_transposeSize(TRANSPOSE_SIZE_NOT_SET),
                     m_rtLoopUnrollFactor(1),
                     m_useVTune(false),
                     m_loadBuiltins(true),
                     m_DumpIROptionAfter(NULL),
                     m_DumpIROptionBefore(NULL),
                     m_dumpHeuristicIR(false) {}

    // CompilerConfiguration methods
    void LoadDefaults();
    virtual void LoadConfig();
    void SkipBuiltins() { m_loadBuiltins = false; }
    void ApplyRuntimeOptions(const ICLDevBackendOptions* pBackendOptions);

    std::string GetCpuArch() const     { return m_cpuArch; }
    std::string GetCpuFeatures() const { return m_cpuFeatures; }
    ETransposeSize GetTransposeSize() const   { return m_transposeSize; }
    int GetRTLoopUnrollFactor() const         { return m_rtLoopUnrollFactor; }
    bool GetUseVTune() const                  { return m_useVTune; }
    bool GetLoadBuiltins() const              { return m_loadBuiltins; }
    size_t GetForcedPrivateMemorySize() const { return m_forcedPrivateMemorySize; }
    size_t GetForcedWorkGroupSize() const { return m_forcedWorkGroupSize; }
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

    const std::string &GetStatFileBaseName() const { return m_statFileBaseName;}

    DeviceMode TargetDevice() const override { return m_targetDevice; }

protected:
    std::string m_cpuArch;
    std::string m_cpuFeatures;
    ETransposeSize m_transposeSize;
    int         m_rtLoopUnrollFactor;
    bool        m_useVTune;
    bool        m_loadBuiltins;
    const std::vector<IRDumpOptions>* m_DumpIROptionAfter;
    const std::vector<IRDumpOptions>* m_DumpIROptionBefore;
    std::string m_dumpIRDir;
    bool m_dumpHeuristicIR;
    std::string m_statFileBaseName;
    int m_forcedPrivateMemorySize;
    DeviceMode  m_targetDevice;
    int m_forcedWorkGroupSize;
};

}}}
#endif //COMPILER_CONFIG_H
