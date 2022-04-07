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
  if (Reflection::BuiltinKeeper::instance()->isBuiltin(Name.str())) {
    F.setName("__userlib" + Name);
  }
}

bool ClangFECompilerMaterializeSPIRVTask::MaterializeSPIRV(llvm::Module *&pM) {
  if (m_opts.getDesiredBIsRepresentation() ==
      SPIRV::BIsRepresentation::SPIRVFriendlyIR) {
    // Rename those user functions conflicting with OpenCL builtins
    std::for_each(pM->begin(), pM->end(), renameUserFunctionConflictingWithBI);
  }

  return true;
}

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
