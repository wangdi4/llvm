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
#include "llvm/SYCLLowerIR/SYCLUtils.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

using namespace llvm;

namespace BackendUtils {

OptimizationLevel getOptLevel(bool HasDisableOptFlag, llvm::Module &M) {
  if (HasDisableOptFlag)
    return OptimizationLevel::O0;

  unsigned OptLevel = 0;
  for (const Function &F : M) {
    if (F.getCallingConv() != CallingConv::SPIR_KERNEL || F.hasOptNone())
      continue;
    if (!F.hasFnAttribute(sycl::utils::ATTR_SYCL_OPTLEVEL)) {
      OptLevel = 3;
      break;
    }
    unsigned Level;
    // getAsInteger returns true on error.
    if (!F.getFnAttribute(sycl::utils::ATTR_SYCL_OPTLEVEL)
             .getValueAsString()
             .getAsInteger(10, Level))
      OptLevel = std::max(OptLevel, Level);
  }
  switch (OptLevel) {
  case 0:
    return OptimizationLevel::O0;
  case 1:
    return OptimizationLevel::O1;
  case 2:
    return OptimizationLevel::O2;
  case 3:
    return OptimizationLevel::O3;
  default:
    llvm_unreachable("invalid optimization level");
  }
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
