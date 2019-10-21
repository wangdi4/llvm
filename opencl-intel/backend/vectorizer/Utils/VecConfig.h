// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __VEC_OPTIONS__H__
#define __VEC_OPTIONS__H__

#include "TargetArch.h"
#include "ICLDevBackendOptions.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace llvm {
class TargetMachine;
}

namespace intel {

class OptimizerConfig
{
public:
    OptimizerConfig( const Intel::CPUId &cpuId, ETransposeSize tranposeSize,
            std::vector<int> dumpIROptionAfter,
            std::vector<int> dumpIROptionBefore,
            std::string dumpIRDir,
            llvm::TargetMachine* machine,
            bool debugInfo,
            bool useNativeDebugger,
            bool profiling,
            bool disableOpt,
            bool relaxedMath,
            bool uniformWGSize,
            bool fpgaEmulator,
            bool eyeqEmulator,
            bool heuristicIR,
            int  APFLevel,
            int rtLoopUnrollFactor):
      m_cpuId(cpuId),
      m_transposeSize(tranposeSize),
      m_dumpIROptionsAfter(dumpIROptionAfter),
      m_dumpIROptionsBefore(dumpIROptionBefore),
      m_dumpIRDir(dumpIRDir),
      m_targetMachine(machine),
      m_debugInfo(debugInfo),
      m_useNativeDebugger(useNativeDebugger),
      m_profiling(profiling),
      m_disableOpt(disableOpt),
      m_relaxedMath(relaxedMath),
      m_uniformWGSize(uniformWGSize),
      m_fpgaEmulator(fpgaEmulator),
      m_eyeqEmulator(eyeqEmulator),
      m_dumpHeuristicIR(heuristicIR),
      m_APFLevel(APFLevel),
      m_rtLoopUnrollFactor(rtLoopUnrollFactor)
    {}

    const Intel::CPUId &GetCpuId() const { return m_cpuId; }
    ETransposeSize GetTransposeSize() const { return m_transposeSize; }

    const std::vector<int>* GetIRDumpOptionsAfter() const{ return &m_dumpIROptionsAfter; }
    const std::vector<int>* GetIRDumpOptionsBefore() const{ return &m_dumpIROptionsBefore; }
    const std::string& GetDumpIRDir() const{ return m_dumpIRDir; }
    llvm::TargetMachine* GetTargetMachine() const { return m_targetMachine; }
    bool GetDisableOpt()    const { return m_disableOpt; }
    bool GetDebugInfoFlag() const { return m_debugInfo; }
    bool GetUseNativeDebuggerFlag() const { return m_useNativeDebugger; }
    bool GetProfilingFlag() const { return m_profiling; }
    bool GetRelaxedMath()   const { return m_relaxedMath; }
    bool GetUniformWGSize() const { return m_uniformWGSize; }
    bool isFpgaEmulator()   const { return m_fpgaEmulator; }
    bool isEyeQEmulator()   const { return m_eyeqEmulator; }
    int GetRTLoopUnrollFactor() const { return m_rtLoopUnrollFactor; }
    bool GetDumpHeuristicIRFlag() const {return m_dumpHeuristicIR; }
    int  GetAPFLevel() const { return m_APFLevel; }

private:
    Intel::CPUId m_cpuId;
    ETransposeSize m_transposeSize;

    std::vector<int> m_dumpIROptionsAfter;
    std::vector<int> m_dumpIROptionsBefore;
    const std::string m_dumpIRDir;
    llvm::TargetMachine* m_targetMachine;
    bool m_debugInfo;
    bool m_useNativeDebugger;
    bool m_profiling;
    bool m_disableOpt;
    bool m_relaxedMath;
    bool m_uniformWGSize;
    // Sets whether we are working as fpga emulator
    bool m_fpgaEmulator;
    // Sets whether we are working as EyeQ emulator
    bool m_eyeqEmulator;
    // Sets whether the vectorize should output heuristic LL IR inputs
    bool m_dumpHeuristicIR;
    // Auto prefetch disable options
    int  m_APFLevel;

    int m_rtLoopUnrollFactor;
};


}

#endif
