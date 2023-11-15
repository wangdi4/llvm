// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "ICLDevBackendOptions.h"
#include "cl_cpu_detect.h"

namespace llvm {
class TargetMachine;
}

namespace intel {

class OptimizerConfig {
public:
  OptimizerConfig(const Intel::OpenCL::Utils::CPUDetect *cpuId,
                  Intel::OpenCL::DeviceBackend::ETransposeSize tranposeSize,
                  llvm::TargetMachine *machine, bool profiling, bool disableOpt,
                  bool relaxedMath, bool coverage, bool fpgaEmulator,
                  bool heuristicIR, int rtLoopUnrollFactor,
                  bool streamingAlways, unsigned expensiveMemOpts,
                  int subGroupConstructionMode)
      : m_cpuId(cpuId), m_transposeSize(tranposeSize), m_targetMachine(machine),
        m_profiling(profiling), m_disableOpt(disableOpt),
        m_relaxedMath(relaxedMath), m_coverage(coverage),
        m_fpgaEmulator(fpgaEmulator), m_dumpHeuristicIR(heuristicIR),
        m_rtLoopUnrollFactor(rtLoopUnrollFactor),
        m_streamingAlways(streamingAlways),
        m_expensiveMemOpts(expensiveMemOpts),
        m_subGroupConstructionMode(subGroupConstructionMode) {}

  const Intel::OpenCL::Utils::CPUDetect *GetCpuId() const { return m_cpuId; }
  Intel::OpenCL::DeviceBackend::ETransposeSize GetTransposeSize() const {
    return m_transposeSize;
  }

  const std::string &GetDumpIRDir() const { return m_dumpIRDir; }
  llvm::TargetMachine *GetTargetMachine() const { return m_targetMachine; }
  bool GetDisableOpt() const { return m_disableOpt; }
  bool GetProfilingFlag() const { return m_profiling; }
  bool GetRelaxedMath() const { return m_relaxedMath; }
  bool GetCoverage() const { return m_coverage; }
  bool isFpgaEmulator() const { return m_fpgaEmulator; }
  int GetRTLoopUnrollFactor() const { return m_rtLoopUnrollFactor; }
  bool GetDumpHeuristicIRFlag() const { return m_dumpHeuristicIR; }
  bool GetStreamingAlways() const { return m_streamingAlways; }
  bool EnableOCLAA() const;
  int GetSubGroupConstructionMode() const { return m_subGroupConstructionMode; }

private:
  const Intel::OpenCL::Utils::CPUDetect *m_cpuId;
  Intel::OpenCL::DeviceBackend::ETransposeSize m_transposeSize;

  std::vector<int> m_dumpIROptionsAfter;
  std::vector<int> m_dumpIROptionsBefore;
  const std::string m_dumpIRDir = "";
  llvm::TargetMachine *m_targetMachine;
  bool m_profiling;
  bool m_disableOpt;
  bool m_relaxedMath;
  bool m_coverage;
  // Sets whether we are working as fpga emulator
  bool m_fpgaEmulator;
  // Sets whether the vectorize should output heuristic LL IR inputs
  bool m_dumpHeuristicIR;

  int m_rtLoopUnrollFactor;
  bool m_streamingAlways;
  unsigned m_expensiveMemOpts;
  int m_subGroupConstructionMode;
};

} // namespace intel

#endif
