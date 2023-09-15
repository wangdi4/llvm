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

#include "BuiltinModuleManager.h"
#include "BuiltinModules.h"
#include "CPUBuiltinLibrary.h"
#include "FPGAEmuBuiltinLibrary.h"
#include <assert.h>
#include <memory>
#include <string>

llvm::Error RegisterCPUBIFunctions(bool isFPGAEmuDev,
                                   llvm::orc::LLJIT *LLJIT = nullptr);

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

BuiltinModuleManager *BuiltinModuleManager::s_pInstance = nullptr;

BuiltinModuleManager::BuiltinModuleManager() {}

BuiltinModuleManager::~BuiltinModuleManager() {}

void BuiltinModuleManager::Init(bool isFPGAEmuDev) {
  assert(!s_pInstance);
  s_pInstance = new BuiltinModuleManager();
  // TODO: need to move this function from the Manager Initialization
  llvm::consumeError(RegisterCPUBIFunctions(isFPGAEmuDev));
}

void BuiltinModuleManager::Terminate() {
  if (nullptr != s_pInstance) {
    delete s_pInstance;
    s_pInstance = nullptr;
  }
}

BuiltinModuleManager *BuiltinModuleManager::GetInstance() {
  assert(s_pInstance);
  return s_pInstance;
}

template <typename DeviceBuiltinLibrary>
BuiltinLibrary *
BuiltinModuleManager::GetOrLoadDeviceLibrary(const CPUDetect *cpuId) {
  // Load device libary according to CPU arch.
  // If we make pair using CPUDetect pointer, we may load wrong device library,
  // since we are always getting same address of CPUDetect instance. When
  // CL_CONFIG_CPU_TARGET_ARCH env is set, CPUDetect instance is updated but its
  // address remains unchanged, so we will get a iterator with previous CPU arch
  TIdCpuId key = std::make_pair(std::this_thread::get_id(), cpuId->GetCPU());
  BuiltinsMap::iterator it = m_BuiltinLibs.find(key);
  if (it != m_BuiltinLibs.end()) {
    return it->second.get();
  }

  std::unique_ptr<BuiltinLibrary> pLibrary(new DeviceBuiltinLibrary(cpuId));
  pLibrary->Load();

  auto *ptr = pLibrary.get();
  m_BuiltinLibs[key] = std::move(pLibrary);
  return ptr;
}

// TODO: Make this method re-entrable
BuiltinLibrary *
BuiltinModuleManager::GetOrLoadCPULibrary(const CPUDetect *cpuId) {
  return GetOrLoadDeviceLibrary<CPUBuiltinLibrary>(cpuId);
}

// TODO: Make this method re-entrable
BuiltinLibrary *
BuiltinModuleManager::GetOrLoadFPGAEmuLibrary(const CPUDetect *cpuId) {
  return GetOrLoadDeviceLibrary<FPGAEmuBuiltinLibrary>(cpuId);
}

llvm::Error
BuiltinModuleManager::RegisterCPUBIFunctionsToLLJIT(bool isFPGAEmuDev,
                                                    llvm::orc::LLJIT *LLJIT) {
  return RegisterCPUBIFunctions(isFPGAEmuDev, LLJIT);
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
