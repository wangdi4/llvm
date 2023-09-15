// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#pragma once
#include "cl_cpu_detect.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <map>
#include <memory>
#include <thread>

using CPUDetect = Intel::OpenCL::Utils::CPUDetect;
using ECPU = Intel::OpenCL::Utils::ECPU;

namespace llvm {
class LLVMContext;
}

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class BuiltinModules;
class BuiltinLibrary;

//******************************************************************************
// Responsible for loading builtin modules in a lazy fashion
//
class BuiltinModuleManager {
private:
  BuiltinModuleManager();
  ~BuiltinModuleManager();

public:
  /**
   * Static singleton intialization
   */
  static void Init(bool isFPGAEmuDev);
  /**
   * Static singleton termination
   */
  static void Terminate();
  /**
   * Singleton instance
   */
  static BuiltinModuleManager *GetInstance();

  /**
   * Returns the \see BuiltinsLibrary for the given cpu. Loads it if necessary
   */
  BuiltinLibrary *GetOrLoadCPULibrary(const CPUDetect *cpuId);

  /**
   * Returns the \see BuiltinsLibrary for the FPGA emu. Loads it if necessary
   */
  BuiltinLibrary *GetOrLoadFPGAEmuLibrary(const CPUDetect *cpuId);

  /**
   * Creates the builtins module for the given cpu using the given LLVMContext
   */
  BuiltinModules *CreateBuiltinModule(int cpuId, llvm::LLVMContext *pContext);

  /**
   * Register symbols of CPU builtins to LLJIT.
   */
  llvm::Error RegisterCPUBIFunctionsToLLJIT(bool isFPGAEmuDev,
                                            llvm::orc::LLJIT *LLJIT);

private:
  template <typename DeviceBuiltinLibrary>
  BuiltinLibrary *GetOrLoadDeviceLibrary(const CPUDetect *cpuId);

  typedef std::pair<std::thread::id, ECPU> TIdCpuId;
  typedef std::map<TIdCpuId, std::unique_ptr<BuiltinLibrary>> BuiltinsMap;
  static BuiltinModuleManager *s_pInstance;
  BuiltinsMap m_BuiltinLibs;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
