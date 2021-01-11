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

#include "SPIRVMaterializer.h"

#include "BuiltinKeeper.h"

#include "LLVMSPIRVLib.h"
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

// Function functor, to be applied for every function in the module when it's
// BIsRepresentation == SPIRVFriendlyIR.
// User may define their own function of the same name as OpenCL builtins
// in SYCL (but in different cpp namespace). User function and OpenCL BI has
// different representation in SPIRV-Friendly-IR.
// For example,
// user function: _Z8isfinitef(float)
// OpenCL BI in SPIRV-Friendly-IR: _Z16__spirv_IsFinitef(float)
//
// We need to translate _Z16__spirv_IsFinitef to _Z8isfinitef to get oclopt
// work properly. So we rename those user define function with a prefix
// "__userlib" to avoid name clashing problems.
static void renameUserFunctionConflictingWithBI(Function &F) {
  if (F.isDeclaration())
    return;

  StringRef Name = F.getName();
  if (reflection::BuiltinKeeper::instance()->isBuiltin(Name.str())) {
    F.setName("__userlib" + Name);
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

bool ClangFECompilerMaterializeSPIRVTask::MaterializeSPIRV(llvm::Module *&pM) {
  if (ifOptEnable()) {
    std::for_each(pM->begin(), pM->end(), removeNoInlineAttr);
  }

  if (m_opts.getDesiredBIsRepresentation() ==
      SPIRV::BIsRepresentation::SPIRVFriendlyIR) {
    // Rename those user functions conflicting with OpenCL builtins
    std::for_each(pM->begin(), pM->end(), renameUserFunctionConflictingWithBI);

    // SPV-IR --> SPV
    std::stringstream spv_ss;
    std::string err;
    bool success = writeSpirv(pM, spv_ss, err);
    if (!success) {
      errs() << "[MaterializeSPIRV] Error during SPV-IR --> SPV translation: "
             << err << '\n';
      return success;
    }

    // SPV --> CL20-IR
    m_opts.setDesiredBIsRepresentation(SPIRV::BIsRepresentation::OpenCL20);
    success &= readSpirv(pM->getContext(), m_opts, spv_ss, pM, err);
    if (!success)
      errs() << "[MaterializeSPIRV] Error during SPV --> CL20-IR translation: "
             << err << '\n';

    return success;
  }

  return true;
}

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
