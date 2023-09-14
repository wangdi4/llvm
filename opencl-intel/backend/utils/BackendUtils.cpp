//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//

#include "BackendUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/IR/CallingConv.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

using namespace llvm;

namespace BackendUtils {

OptimizationLevel getOptLevel(bool HasDisableOptFlag, llvm::Module &M) {
  bool AllKernelsHasOptNone = llvm::all_of(M, [](Function &F) {
    return F.getCallingConv() != CallingConv::SPIR_KERNEL || F.hasOptNone();
  });
  return (HasDisableOptFlag || AllKernelsHasOptNone) ? OptimizationLevel::O0
                                                     : OptimizationLevel::O3;
}

static void recordCtorDtors(iterator_range<orc::CtorDtorIterator> CtorDtors,
                            std::vector<std::string> &CtorDtorNames) {
  if (CtorDtors.empty())
    return;

  std::map<unsigned, std::vector<const Function *>> CtorDtorsByPriority;
  for (auto CtorDtor : CtorDtors) {
    assert(CtorDtor.Func && CtorDtor.Func->hasName() &&
           "Ctor/Dtor must be a named function");
    if (CtorDtor.Data && cast<GlobalValue>(CtorDtor.Data)->isDeclaration())
      continue;

    if (CtorDtor.Func->hasLocalLinkage()) {
      CtorDtor.Func->setLinkage(GlobalValue::ExternalLinkage);
      CtorDtor.Func->setVisibility(GlobalValue::HiddenVisibility);
    }

    CtorDtorsByPriority[CtorDtor.Priority].push_back(CtorDtor.Func);
  }

  for (auto &KV : CtorDtorsByPriority) {
    for (auto &Func : KV.second)
      CtorDtorNames.push_back(Func->getName().str());
  }
}

void recordGlobalCtorDtors(Module &M, std::vector<std::string> &CtorNames,
                           std::vector<std::string> &DtorNames) {
  recordCtorDtors(orc::getConstructors(M), CtorNames);
  recordCtorDtors(orc::getDestructors(M), DtorNames);
}

} // namespace BackendUtils

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
