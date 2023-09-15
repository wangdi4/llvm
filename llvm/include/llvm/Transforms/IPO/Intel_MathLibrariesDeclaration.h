//=--- Intel_MathLibrariesDeclaration.h - Add math function declaration  -*--=//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass goes through each llvm math intrinsic and adds a declaration for
// the corresponding math function in the IR if we are compiling for LTO. The
// reason is that we want to have the math symbols available when the linker
// is processing the symbols resolution in order to prioritize Intel's math
// libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_MATHLIBRARIESDECLARATION_H
#define LLVM_TRANSFORMS_IPO_INTEL_MATHLIBRARIESDECLARATION_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class IntelMathLibrariesDeclarationPass
    : public PassInfoMixin<IntelMathLibrariesDeclarationPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_MATHLIBRARIESDECLARATION_H
