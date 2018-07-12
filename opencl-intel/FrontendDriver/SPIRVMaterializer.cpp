// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "frontend_api.h"
#include "SPIRVMaterializer.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include <sstream>

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace ClangFE {

// Function functor, to be applied for every function in the module.
// SPIRV translator is designed to consume LLVM IR compiled with O0
// optimization level, thus clang generates NoInline attributes
// for every function. It blocks inline optimizations.
// The function removes this attribute.
static void removeNoInlineAttr(Function &F) {
  F.removeFnAttr(Attribute::AttrKind::NoInline);
  for (auto *U : F.users()) {
    if (auto *CI = dyn_cast<CallInst>(U))
      CI->removeAttribute(AttributeList::FunctionIndex, Attribute::NoInline);
  }
}

// Checks if the program was compiled with optimization.
bool ClangFECompilerMaterializeSPIRVTask::ifOptEnable() {
  std::vector<std::string> BuildOptionsSeparated;
  std::stringstream OptionsStrstream(m_pProgDesc->pszOptions);
  std::copy(std::istream_iterator<std::string>(OptionsStrstream),
            std::istream_iterator<std::string>(),
            std::back_inserter(BuildOptionsSeparated));
  for (auto Option : BuildOptionsSeparated) {
    if (Option == "-g")
      return false;
    if (Option == "-cl-opt-disable")
      return false;
  }
  return true;
}

int ClangFECompilerMaterializeSPIRVTask::MaterializeSPIRV(llvm::Module &M) {
  if (ifOptEnable()) {
    std::for_each(M.begin(), M.end(), removeNoInlineAttr);
  }

  return 0;
}

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
