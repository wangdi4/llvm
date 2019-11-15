#if INTEL_COLLAB // -*- C++ -*-
//===-- VPORestoreOperands.h - Restore renamed VPO  operands --*- C++ -*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file declares the VPO Restore Operands pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VPORESTOREOPERANDS_H
#define LLVM_TRANSFORMS_VPO_VPORESTOREOPERANDS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Undo the renaming of OpenMP clause operands done in vpo-paropt-prepare pass.
/// The addresses of renamed operands are expected to be in a list on an operand
/// bundle titled "QUAL.OMP.OPERAND.ADDR".
///
/// This pass goes through the list, and does the following transformation:
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   store i32* %y, i32** %y.addr      |
///   %1 = begin_region[... %y...       |   %1 = begin_region[... %y...]
///                     "OPND.ADDR"     |
///                     (i32* %y,       |
///                     (i32** %y.addr)]|
///   %y1 = load i32*, i32** %y.addr    |
///                                     |
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
/// \endcode
///
/// The pass looks at the OPND.ADDR pair, and replaces `%y1`, which is a load
/// from `%y.addr`, with `%y`, which it gets from the first member of the
/// OPND.ADDR pair. This pass is expected to run before the vpo-paropt
/// transformation pass.
/// \see VPOParoptTransform::renameOperandsUsingStoreThenLoad() for details on
/// how the renaming is done.
/// \see VPOUtils::restoreOperands() for details on how the original operands
/// are restored.
class VPORestoreOperandsPass : public PassInfoMixin<VPORestoreOperandsPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  bool runImpl(Function &F);
};

namespace vpo {

class VPORestoreOperands : public FunctionPass {
public:
  static char ID; // Pass identification

  VPORestoreOperands();

  StringRef getPassName() const override { return "VPO Restore Operands"; }
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  VPORestoreOperandsPass Impl;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VPORESTOREOPERANDS_H

#endif // INTEL_COLLAB
