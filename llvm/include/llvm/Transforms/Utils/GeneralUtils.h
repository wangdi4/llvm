#if INTEL_COLLAB // -*- C++ -*-
//===------------ GeneralUtils.h - Class definition -*- C++ -*-------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// General purpose set of utilities
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_UTILS_GENERALUTILS_H
#define LLVM_TRANSFORM_UTILS_GENERALUTILS_H

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
class Module;

/// \brief This class provides a set of general utility functions that can be
/// used for a variety of purposes.
class GeneralUtils {

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
  static Type *getSizeTTy(Module *M);
};

} // end llvm namespace

#endif // LLVM_TRANSFORM_UTILS_GENERALUTILS_H
#endif // INTEL_COLLAB
