//===- RelaxedMath.cpp - Relaxed math builtin substitution ----------------===//
//
// Copyright (C) 2022 Intel Corporation
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
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/RelaxedMath.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-relaxed-math"

static bool runImpl(Module &M) {
  static const StringSet<> NativeBuiltins{
      "cos", "exp", "exp10", "exp2", "fdim", "fmax", "fmin", "fmod", "fract",
      "hypot", "ilogb", "log", "log10", "log2", "logb", "powr", "rsqrt", "sin",
      "sqrt", "tan",

      // non-spec native-BI
      "acos", "acosh", "acospi", "asin", "asinh", "asinpi", "atan", "atanh",
      "atanpi", "atan2", "atan2pi", "cbrt", "cospi", "erfc", "erf", "expm1",
      "log1p", "pow", "sinpi", "tanpi"};

  // OpenCL 2.0 rev 08 Table 7.2
  static const StringSet<> RelaxedBuiltins{"cos", "exp",  "exp2",  "exp10",
                                           "log", "log2", "pow",   "sincos",
                                           "sin", "tan",  "divide"};

  bool HasOcl20 = hasOcl20Support(M);
  const StringSet<> &Builtins = HasOcl20 ? RelaxedBuiltins : NativeBuiltins;

  // Use work list since we don't need to iterate over newly inserted functions.
  SmallVector<Function *, 16> WorkList;
  for (auto &F : M) {
    StringRef FName = F.getName();
    if (F.isDeclaration() && NameMangleAPI::isMangledName(FName) &&
        Builtins.contains(NameMangleAPI::stripName(FName)))
      WorkList.push_back(&F);
  }

  for (auto *F : WorkList) {
    StringRef FName = F->getName();
    reflection::FunctionDescriptor FD = NameMangleAPI::demangle(FName);
    if (HasOcl20)
      FD.Name += "_rm";
    else
      FD.Name = "native_" + FD.Name;
    std::string NewName = NameMangleAPI::mangle(FD);
    LLVM_DEBUG(dbgs() << "F: " << FName << ", FD.Name: " << FD.Name
                      << ", NewName: " << NewName << "\n");

    Function *NewF = dyn_cast<Function>(
        M.getOrInsertFunction(NewName, F->getFunctionType(), F->getAttributes())
            .getCallee());
    assert(NewF && "failed to get or insert relaxed function");
    F->replaceAllUsesWith(NewF);
    F->eraseFromParent();
  }

  return !WorkList.empty();
}

PreservedAnalyses RelaxedMathPass::run(Module &M, ModuleAnalysisManager &) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
