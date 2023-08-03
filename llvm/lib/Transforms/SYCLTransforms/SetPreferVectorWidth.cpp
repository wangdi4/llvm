//===- SetPreferVectorWidth.cpp - Set prefer vector width -----------------===//
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

#include "llvm/Transforms/SYCLTransforms/SetPreferVectorWidth.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-set-prefer-vector-width"

extern cl::opt<VFISAKind> IsaEncodingOverride;

static cl::opt<unsigned> ForcedVecWidth("sycl-force-prefer-vector-width",
                                        cl::init(0), cl::Hidden);

static StringRef PreferVecWidth = "prefer-vector-width";

SetPreferVectorWidthPass::SetPreferVectorWidthPass(VFISAKind ISA)
    : ISA(ISA) {
  if (IsaEncodingOverride.getNumOccurrences())
    this->ISA = IsaEncodingOverride.getValue();
}

PreservedAnalyses SetPreferVectorWidthPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

static bool setPreferVectorWidth(Module &M, VFISAKind ISA) {
  bool Changed = false;
  unsigned VecWidth;
  if (ForcedVecWidth)
    VecWidth = ForcedVecWidth;
  else {
    switch (ISA) {
    case VFISAKind::AVX512:
      VecWidth = 512;
      break;
    case VFISAKind::AVX2:
      VecWidth = 256;
      break;
    default:
      VecWidth = 128;
    }
  }

  for (Function &F : M) {
    if (F.isIntrinsic())
      continue;
    if (F.hasFnAttribute(PreferVecWidth))
      continue;
    F.addFnAttr(PreferVecWidth, utostr(VecWidth));
    Changed = true;
  }
  return Changed;
}

bool SetPreferVectorWidthPass::runImpl(Module &M) {
  return setPreferVectorWidth(M, ISA);
}
