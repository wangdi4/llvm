#if INTEL_COLLAB // -*- C++ -*-
//===--------- Intel_GeneralUtils.h - Class definition -*- C++ -*----------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// General purpose set of utilities that are visible within Intel_VPO and to
/// the community.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_UTILS_INTEL_GENERALUTILS_H
#define LLVM_TRANSFORM_UTILS_INTEL_GENERALUTILS_H

#include "llvm/ADT/SmallVector.h"

namespace llvm {

class Constant;
class Loop;
class LoopInfo;
class BasicBlock;
class Type;
class LLVMContext;
class Instruction;
class DbgDeclareInst;
class DbgValueInst;
class DbgInfoIntrinsic;
class ConstantExpr;
class Value;
class Function;
class DominatorTree;

/// \brief This class provides a set of general utility functions that can be
/// used for a variety of purposes.
class IntelGeneralUtils {

public:

  /// \brief Returns a floating point or integer constant depending on Ty.
  template <typename T>
  static Constant* getConstantValue(Type *Ty, LLVMContext &Context, T Val);

  /// \brief Returns Loop in LoopInfo corresponding to the WRN.  The initial
  /// call to this recursive DFS function should pass in the WRN's EntryBB and
  /// ExitBB to prevent searching for the loop header outside of the region.
  static Loop *getLoopFromLoopInfo(LoopInfo *LI, DominatorTree *DT,
                                   BasicBlock *EntryBB, BasicBlock *ExitBB);

  /// \brief Generates BB set in sub CFG for a given WRegionNode.
  /// The entry basic bblock 'EntryBB' and the exit basic
  /// block 'ExitBB' are the inputs, and 'BBSet' is the output containing all
  /// the basic blocks that belong to this region. It guarantees that the
  /// first item in BBSet is 'EntryBB' and the last item is 'ExitBB'.
  static void collectBBSet(BasicBlock *EntryBB, BasicBlock *ExitBB,
                           SmallVectorImpl<BasicBlock *> &BBSet);
  /// \brief Breaks up the instruction recursively for all the constant
  /// expression operands. If NewInstArr is not null, put the newly created
  /// instructions in *NewInstArr.
  static void breakExpressions(Instruction *Inst,
                           SmallVectorImpl<Instruction *> *NewInstArr=nullptr);

  /// \brief Breaks up the instruction recursively for the gvien constant
  /// expression operand. If NewInstArr is not null, put the newly created
  /// instructions in *NewInstArr.
  static void breakExpressionsHelper(ConstantExpr* Expr, unsigned OperandIndex,
                           Instruction* User,
                           SmallVectorImpl<Instruction *> *NewInstArr=nullptr);

  /// \brief Returns false if I's next instruction is terminator instruction.
  /// Otherwise returns true.
  static bool hasNextUniqueInstruction(Instruction *I);

  /// \brief Returns instruction I's next instruction in the same basic block.
  static Instruction* nextUniqueInstruction(Instruction *I);

#if INTEL_CUSTOMIZATION
  /// \brief Returns true if the value V escapes.
  static bool isEscaped(const Value *V);
#endif // INTEL_CUSTOMIZATION

  /// \brief Return the size_t type for 32/64 bit architecture
  static Type *getSizeTTy(Function *F);
};

} // end llvm namespace

#endif // LLVM_TRANSFORM_UTILS_INTEL_GENERALUTILS_H
#endif // INTEL_COLLAB
