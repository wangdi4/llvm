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

#include "BuiltinModules.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

BuiltinModules::BuiltinModules(
    SmallVector<std::unique_ptr<Module>, 2> builtinsModules) {
  transform(builtinsModules, std::back_inserter(m_BuiltinsModules),
            [](std::unique_ptr<Module> &M) { return M.release(); });
}

BuiltinModules::~BuiltinModules() {
  for (auto *M : m_BuiltinsModules)
    delete M;
}

BuiltinLibrary::BuiltinLibrary(const Intel::OpenCL::Utils::CPUDetect *cpuId)
    : m_cpuId(cpuId), m_pRtlBuffer(nullptr), m_pRtlBufferSvmlShared(nullptr) {}

BuiltinLibrary::~BuiltinLibrary() {}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
