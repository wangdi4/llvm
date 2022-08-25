#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===--------- GeneralUtils.cpp - General set of utilities for VPO --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file includes a set of utilities that are generally useful for many
/// purposes.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h" // INTEL
#include <queue>

#define DEBUG_TYPE "general-utils"

using namespace llvm;

#if INTEL_CUSTOMIZATION
bool Usei1MaskForSimdFunctions;
cl::opt<bool, true> Usei1MaskForSimdFunctionsOpt(
    "use-i1-mask-for-simd-funcs", cl::location(Usei1MaskForSimdFunctions),
    cl::init(false), cl::desc("Use vector of i1 as mask for simd functions"));
#endif // INTEL_CUSTOMIZATION

template Constant *
GeneralUtils::getConstantValue<int>(Type *Ty, LLVMContext &Context, int Val);
template Constant *GeneralUtils::getConstantValue<float>(Type *Ty,
                                                         LLVMContext &Context,
                                                         float Val);
template Constant *GeneralUtils::getConstantValue<double>(Type *Ty,
                                                          LLVMContext &Context,
                                                          double Val);
template <typename T>
Constant *GeneralUtils::getConstantValue(Type *Ty, LLVMContext &Context,
                                         T Val) {
  Constant *ConstVal = nullptr;

  if (Ty->isIntegerTy()) {
    ConstVal = ConstantInt::get(Ty, Val);
#if INTEL_CUSTOMIZATION
  } else if (Ty->isFloatingPointTy()) {
#else
  } else if (Ty->isFloatTy()) {
#endif
    ConstVal = ConstantFP::get(Ty, Val);
  }

  assert(ConstVal && "Could not generate constant for type");

  return ConstVal;
}

Loop *GeneralUtils::getLoopFromLoopInfo(LoopInfo *LI, DominatorTree *DT,
                                        BasicBlock *BB, BasicBlock *ExitBB) {
  Loop *Lp = nullptr;

  // If DFS reached the region's ExitBB, go back
  if (BB == ExitBB)
    return nullptr;

  // The first BB with 2 predecessors found in the DFS is the loop header.
  // Return the loop associated with this BB.
  if (std::distance(pred_begin(BB), pred_end(BB))==2) {

    auto IT = pred_begin(BB);
    BasicBlock *FirstPred = *IT;
    IT++;
    BasicBlock *SecondPred = *IT;

    if (DT->dominates(BB, FirstPred) ||
        DT->dominates(BB, SecondPred)) {
      Lp = LI->getLoopFor(BB);
      // assert(Lp &&
      //   "The first BB with 2 predecessors should be the loop header");
      //
      // dbgs() << "\n=== getLoopFromLoopInfo found loop : " << *Lp << "\n";
      return Lp;
    }
    else
      return nullptr;
  }

  // BB is not a loop header; continue DFS with BB's successors recursively
  for (succ_iterator I = succ_begin(BB), E = succ_end(BB); I != E; ++I) {
    Lp = getLoopFromLoopInfo(LI, DT, *I, ExitBB);
    if (Lp)
      return Lp;
  }

  return nullptr;
}

/// \brief This function ensures that EntryBB is the first item in BBSet and
/// ExitBB is the last item in BBSet.
void GeneralUtils::collectBBSet(BasicBlock *EntryBB, BasicBlock *ExitBB,
                                SmallVectorImpl<BasicBlock *> &BBSet) {

  assert(EntryBB && "no EntryBB");
  assert(ExitBB && "no ExitBB");

  assert(EntryBB != ExitBB && "Entry and Exit BBs should be different");

  std::queue<BasicBlock *> BBlockQueue;
  BBlockQueue.push(EntryBB);
  BBSet.push_back(EntryBB);

  while (!BBlockQueue.empty()) {
    BasicBlock *Front = BBlockQueue.front();
    BBlockQueue.pop();

    if (Front != ExitBB) {
      for (succ_iterator I = succ_begin(Front), E = succ_end(Front); I != E;
           ++I)
        // Push the successor to the queue only if it is not in the BBSet.
        // Also add it to the BBSet here if it isn't ExitBB so that it won't be
        // added to the queue twice: otherwise, the queue size grows
        // exponentially for certain CFG patterns.
        if (std::find(BBSet.begin(), BBSet.end(), *I) == BBSet.end()) {
          BBlockQueue.push(*I);
          if (*I != ExitBB)
            BBSet.push_back(*I);
        }
    }
  }

  BBSet.push_back(ExitBB);

  assert((BBSet.front() == EntryBB) &&
         "The first element of BBSet is not EntryBB");
  assert((BBSet.back() == ExitBB) && "The last element of BBSet is not ExitBB");
}

// Breaks up the instruction recursively for all the constant expression
// operands.
void GeneralUtils::breakExpressions(
    Instruction *Inst, SmallVectorImpl<Instruction *> *NewInstArr,
    SmallPtrSetImpl<ConstantExpr *> *ExprsToBreak) {
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Breaking constant exprs in '" << *Inst
                    << "'.\n");
  if (DbgDeclareInst *DbgDclInst = dyn_cast<DbgDeclareInst>(Inst)) {
    // For DbgDeclareInst, the operand is a metadata that might
    // contain a constant expression.
    Value* Op = DbgDclInst->getAddress();
    // If the debug adress is a constant expression, recursively break it up.
    if (ConstantExpr* Expr = dyn_cast_or_null<ConstantExpr>(Op))
      breakExpressionsHelper(Expr, 0, Inst, NewInstArr, ExprsToBreak);
  }
  else if (DbgValueInst *DbgValInst = dyn_cast<DbgValueInst>(Inst)) {
    // For DbgValueInst, the operand is a metadata that might
    // contain a constant expression.
    Value* Op = DbgValInst->getValue();
    // If the debug value operand is a constant expression, recursively break it up.
    if (ConstantExpr* Expr = dyn_cast_or_null<ConstantExpr>(Op))
      breakExpressionsHelper(Expr, 0, Inst, NewInstArr, ExprsToBreak);
  }
  else {
    // And all the operands of each instruction
    for( unsigned I = 0; I < Inst->getNumOperands(); ++I )
    {
      Value* Op = Inst->getOperand(I);

      // If the operand is a constant expression, recursively break it up.
      if (ConstantExpr* Expr = dyn_cast<ConstantExpr>(Op))
        breakExpressionsHelper(Expr, I, Inst, NewInstArr, ExprsToBreak);
    }
  }
}

// Breaks up the instruction recursively for the gvien constant
// expression operand.
void GeneralUtils::breakExpressionsHelper(
    ConstantExpr *Expr, unsigned OperandIndex, Instruction *User,
    SmallVectorImpl<Instruction *> *NewInstArr,
    SmallPtrSetImpl<ConstantExpr *> *ExprsToBreak) {
  if (ExprsToBreak && !ExprsToBreak->count(Expr))
    return;

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Breaking Expr '" << *Expr << "'.\n");

  // Create a new instruction, and insert it at the appropriate point.
  Instruction *NewInst = Expr->getAsInstruction();
  NewInst->setDebugLoc(User->getDebugLoc());
  if (NewInstArr != nullptr)
    NewInstArr->push_back(NewInst);

  if (PHINode *Phi = dyn_cast<PHINode>(User)) {
    NewInst->insertBefore(Phi->getIncomingBlock(OperandIndex)->getTerminator());
    User->setOperand(OperandIndex, NewInst);
  } else if (isa<DbgInfoIntrinsic>(User))
    NewInst->insertBefore(User);
  else {
    NewInst->insertBefore(User);
    User->replaceUsesOfWith(Expr, NewInst);
  }
  if (Expr->use_empty())
    Expr->destroyConstant();
  // Thew new instruction may itself reference constant expressions.
  // So, recursively process all of its arguments.
  for (unsigned I = 0; I < NewInst->getNumOperands(); ++I) {
    Value *Op = NewInst->getOperand(I);
    ConstantExpr *InnerExpr = dyn_cast<ConstantExpr>(Op);
    if (InnerExpr)
      breakExpressionsHelper(InnerExpr, I, NewInst, NewInstArr, ExprsToBreak);
  }
}

// Checks instruction I has unique next instruction or not.
// If I is terminator instruction, checks whether it has unique succ BB.
bool GeneralUtils::hasNextUniqueInstruction(Instruction *I) {
  if (!I->isTerminator())
    return true;

  BasicBlock *nextBB = I->getParent()->getUniqueSuccessor();
  return nextBB && (nextBB->getUniquePredecessor() != nullptr);
}

// Returns I's next unique instruciton, which could be in the same
// basic block or the first instruciotn of the unique succ BB.
Instruction *GeneralUtils::nextUniqueInstruction(Instruction *I) {
  assert(hasNextUniqueInstruction(I) &&
         "first check if there is a next instruction!");

  if (I->isTerminator())
    return &I->getParent()->getUniqueSuccessor()->front();
  return &*++I->getIterator();
}

#if INTEL_CUSTOMIZATION
// This code uses class AddressInst, which is an INTEL_CUSTOMIZATION code in
// include/llvm/IR/IntrinsicInst.h, so we have to guard this util as well

// Recursively checks whether V escapes or not.
static bool analyzeEscapeAux(const Value *V,
                             SmallPtrSetImpl<const PHINode *> &PhiUsers) {
  if (isa<GlobalVariable>(V))
    return true;

  const ConstantExpr *CE;
  for (const Use &U : V->uses()) {
    const User *UR = U.getUser();
    CE = dyn_cast<ConstantExpr>(UR);
    if (CE) {

      if (!isa<PointerType>(CE->getType()))
        return true;

      if (analyzeEscapeAux(CE, PhiUsers))
        return true;
    } else if (const Instruction *I = dyn_cast<Instruction>(UR)) {
      if (const LoadInst *LI = dyn_cast<LoadInst>(I)) {
        if (LI->isVolatile())
          return true;
        CE = dyn_cast<ConstantExpr>(V);
        if (LI->getOperand(0) == V && CE) {
          if (analyzeEscapeAux(LI, PhiUsers))
            return true;
        }
      } else if (const StoreInst *SI = dyn_cast<StoreInst>(I)) {
        if (SI->getOperand(0) == V)
          return true;

        if (SI->isVolatile())
          return true;

      } else if (isa<BitCastInst>(I) || isa<GetElementPtrInst>(I) ||
                 isa<AddressInst>(I) ||
                 isa<SelectInst>(I)) {
        if (analyzeEscapeAux(I, PhiUsers))
          return true;
      } else if (const PHINode *PN = dyn_cast<PHINode>(I)) {
        // The return value of the insert() method is std::pair.If the second
        // element of the pair is true, it means that the incoming PN is new. If
        // it is false, it means that the PN eixsts in the set PhiUses. By this
        // way it avoids cycling issue.
        if (PhiUsers.insert(PN).second)
          if (analyzeEscapeAux(I, PhiUsers))
            return true;
      } else if (const MemTransferInst *MTI = dyn_cast<MemTransferInst>(I)) {
        if (MTI->isVolatile())
          return true;
      } else if (const MemSetInst *MSI = dyn_cast<MemSetInst>(I)) {
        assert(MSI->getArgOperand(0) == V && "Memset only takes one pointer!");
        if (MSI->isVolatile())
          return true;
      } else if (auto C = dyn_cast<CallBase>(I)) {
        if (!C->isCallee(&U))
          return true;
      } else
        return true;
    } else
      return true;
  }

  return false;
}

// Returns true if the value V escapes.
bool GeneralUtils::isEscaped(const Value *V) {
  SmallPtrSet<const PHINode *, 16> PhiUsers;
  return analyzeEscapeAux(V, PhiUsers);
}
#endif // INTEL_CUSTOMIZATION

// Return the size_t type for 32/64 bit architecture
Type *GeneralUtils::getSizeTTy(Module *M) {
  LLVMContext &C = M->getContext();

  IntegerType *IntTy;
  const DataLayout &DL = M->getDataLayout();

  if (DL.getIntPtrType(Type::getInt8PtrTy(C))->getIntegerBitWidth() == 64)
    IntTy = Type::getInt64Ty(C);
  else
    IntTy = Type::getInt32Ty(C);
  return IntTy;
}

Type *GeneralUtils::getSizeTTy(Function *F) {
  return getSizeTTy(F->getParent());
}

bool GeneralUtils::isOMPItemGlobalVAR(const Value *V) {
  if (isa<GlobalVariable>(V))
    return true;

  auto *CE = dyn_cast<ConstantExpr>(V);

  if (!CE)
    return false;

  if (CE->getOpcode() != Instruction::AddrSpaceCast)
    return false;

  if (!isa<GlobalVariable>(CE->getOperand(0)))
    return false;
#if !ENABLE_OPAQUEPOINTER
  // If this is an AddrSpaceCast constant expression of a GlobalVariable,
  // then assert that the AddrSpaceCast's type and the operand's
  // type are only different by the addrspace. We expect that we can deduce
  // the original type of the OpenMP clause's item VAR in both cases, i.e.
  // when VAR is represented directly with a GlobalVariable or with
  // GlobalVariable followed by AddrSpaceCast.
  // TODO: OPAQUEPOINTER: Remove this call. It will not be needed after opaque
  // pointers are introduced.
  assert(cast<PointerType>(CE->getType())
             ->isOpaqueOrPointeeTypeMatches(
                 cast<GlobalVariable>(CE->getOperand(0))->getValueType()) &&
         "isOMPItemGlobalVAR: Type mismatch for a GlobalVariable and "
         "its addrspacecast.");
#endif // !ENABLE_OPAQUEPOINTER
  return true;
}

bool GeneralUtils::isOMPItemLocalVAR(const Value *V) {
  // Filter out global variable references, first.
  // Everything else is classified for a local variable reference,
  // except that we run special verification for AddrSpaceCastInst's
  // below.
  if (isOMPItemGlobalVAR(V))
    return false;

  if (!isa<PointerType>(V->getType()))
    return false;
#if !ENABLE_OPAQUEPOINTER
  if (V->getType()->isOpaquePointerTy())
    return true;
#ifndef NDEBUG
  // Skip a sequence of AddrSpaceCastInst's in hope to reach
  // the AllocaInst.
  bool ASCIChangedType = false;
  while (const auto *ASCI = dyn_cast<AddrSpaceCastInst>(V)) {
    // We expect that the address space casts do not change
    // the original type of their pointer operand.
    // If we reach an AllocaInst by following AddrSpaceCastInst
    // chain, and the type changes along the way, then it is not
    // clear what object type was specified in the clause.
    // TODO: OPAQUEPOINTER: Delete the check below or change it appropriately.
    // There will only be an AddrSpaceCast, the pointer type cannot be
    // changed.
    V = ASCI->getPointerOperand();
    if (ASCI->getType()->getNonOpaquePointerElementType() !=
        V->getType()->getNonOpaquePointerElementType())
      ASCIChangedType = true;
  }

  (void)ASCIChangedType;
  assert(!isa<AllocaInst>(V) || !ASCIChangedType && "isItemLocalVAR: "
         "type changed between an alloca and the clause reference.");
#endif  // NDEBUG
#endif // !ENABLE_OPAQUEPOINTER
  return true;
}

std::tuple<Type *, Value *>
GeneralUtils::getOMPItemLocalVARPointerTypeAndNumElem(Value *V, Type *VElemTy) {
  if (!isOMPItemLocalVAR(V)) {
    llvm_unreachable("getItemLocalVARPointerType: expect isOMPItemLocalVAR().");
    return std::make_tuple(nullptr, nullptr);
  }

  // Skip a sequence of AddrSpaceCastInst's in hope to reach
  // the AllocaInst.
  while (auto *ASCI = dyn_cast<AddrSpaceCastInst>(V))
    V = ASCI->getPointerOperand();

  if (auto *AI = dyn_cast<AllocaInst>(V)) {
    // VLA allocas specify the number of allocated elements.
    Value *NumElements = AI->getArraySize();
    return std::make_tuple(AI->getAllocatedType(), NumElements);
  }

  // If we did not find an AllocaInst, then we treat this as a scalar
  // clause item.
  return std::make_tuple(VElemTy, cast<Value>(ConstantInt::get(
                                      Type::getInt32Ty(V->getContext()), 1)));
}
#endif // INTEL_COLLAB
