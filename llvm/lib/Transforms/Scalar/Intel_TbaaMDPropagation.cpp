//===-- TbaaMDPropagation.cpp - TBAA recovery for return pointers implementation -===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass recovers the tbaa information for the return pointers and
// the corresponding intrinsic are cleaned up.
//
// Here is one example.
//
// struct S {
//   int a[4];
//   int b[4];
//
//   int& geta(int i) {
//     return a[i];
//   }
//   int& getb(int i) {
//     return b[i];
//   }
// };
//
// int foo(S& s, int i, int j) {
//   s.geta(i) = 0;
//   s.getb(j) = 1;
//   return s.geta(i);
// }
//
// This optmization is expected to generate the struct path tbaa
// for s.geta(i) and s.getb(j) as follows. 
//
// store i32 0, i32* %arrayidx.i, align 4, !tbaa !1
// store i32 1, i32* %arrayidx.i9, align 4, !tbaa !7
//
// !0 = !{!"clang version 4.0.0 (trunk 17977)"}
// !1 = !{!2, !4, i64 0}
// !2 = !{!"struct@_ZTS1S", !3, i64 0, !3, i64 16}
// !3 = !{!"array@_ZTSA4_i", !4, i64 0}
// !4 = !{!"int", !5, i64 0}
// !5 = !{!"omnipotent char", !6, i64 0}
// !6 = !{!"Simple C++ TBAA"}
// !7 = !{!2, !4, i64 16}
//
// The front end inserts intel.fakeload intrinsics that wrap the value
// passed to return statements in order to allow us to propagate the
// TBAA metadata after the functions have been inlined.  In order for
// this to work correctly, this pass cannot remove the fakeload intrinsic
// until after a function has been inlined.  However, we would like to
// run the pass before SROA (which is a pre-inlining function simplification
// pass) because the fakeload intrinsic blocks SROA.
//
// To solve this problem, tha TbaaMDPropagation pass only operates on intrinsics
// that have uses that are not return instructions, which is an indication that
// the function containing the intrinsic has been inlined.  The CleanupFakeLoads
// pass should be run after all inlining is complete to remove remaining
// intrinsics.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_TbaaMDPropagation.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h" // INTEL
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "tbaa-prop"

namespace {

struct TbaaMDPropagationImpl : public InstVisitor<TbaaMDPropagationImpl> {
public:
  void visitInstruction(Instruction &I) { return; }
  void visitLoad(LoadInst &LI);
  void visitStore(StoreInst &SI);
  void visitIntrinsicInst(IntrinsicInst &II);
  void removeBadArgMD(Function *F);

private:
  friend class InstVisitor<TbaaMDPropagationImpl>;
};

/// \Returns combined !intel-tbaa metadata for a chain of GEPs starting at \p
/// GEP.
MDNode *getGepChainTBAA(const GetElementPtrInst *GEP) {
  MDNode *GepMD = GEP->getMetadata(LLVMContext::MD_intel_tbaa);
  if (!GepMD)
    return nullptr;

  const auto *BaseGEP = dyn_cast<GetElementPtrInst>(GEP->getPointerOperand());
  if (!BaseGEP)
    return GepMD;

  return mergeIntelTBAA(getGepChainTBAA(BaseGEP), GepMD);
}

// TODO: merge with InstCombineCalls, TypeBasedAliasAnalysis....
static bool isStructPathTBAA(const MDNode *MD) {
  return isa<MDNode>(MD->getOperand(0)) && MD->getNumOperands() >= 3;
}

// The tbaa information is retrieved from the fakeload intrinsic
// and attached to the pointer's dereference sites.
void TbaaMDPropagationImpl::visitIntrinsicInst(IntrinsicInst &II) {
  if (II.getIntrinsicID() != Intrinsic::intel_fakeload)
    return;

  MDNode *FakeloadTBAA = dyn_cast<MDNode>(
      cast<MetadataAsValue>(II.getArgOperand(1))->getMetadata());

  if (const auto *GEP = dyn_cast<GetElementPtrInst>(II.getArgOperand(0))) {
    // Try to refine TBAA info of the GEP chain feeding our fakeload.
    auto *GEPChainTBAA = getGepChainTBAA(GEP);
    if (GEPChainTBAA) {
      // intel-tbaa may have access types that are not scalars (pointers to
      // arrays or structs). If it's not valid, use the fakeload's TBAA.
      // The fakeload TBAA is checked below.
      auto *AccessType = dyn_cast<MDNode>(GEPChainTBAA->getOperand(1));
      if (AccessType && TBAAVerifier::isValidScalarTBAANodeStatic(AccessType)) {
        auto *MergedTBAA = getMostSpecificTBAA(GEPChainTBAA, FakeloadTBAA);
        if (MergedTBAA != FakeloadTBAA)
          II.setArgOperand(1,
                           MetadataAsValue::get(GEP->getContext(), MergedTBAA));
      }
    }
  }

  // If the only user is a return instruction, we aren't at the right level
  // of inlining yet.
  if (II.hasOneUse() && isa<ReturnInst>(II.user_back()))
    return;

  // Reload the TBAA in case we changed it above.
  FakeloadTBAA = dyn_cast<MDNode>(
      cast<MetadataAsValue>(II.getArgOperand(1))->getMetadata());

  // Check it for validity.
  if (FakeloadTBAA && isStructPathTBAA(FakeloadTBAA)) {
    auto *AccessType = dyn_cast<MDNode>(FakeloadTBAA->getOperand(1));
    if (!AccessType || !TBAAVerifier::isValidScalarTBAANodeStatic(AccessType)) {
      // Functions that return type pointer-to-struct may have fakeloads that
      // refer to a struct type as if it were a scalar:
      // !0 = !{!"struct", !1, i64 0, !2, i64 4}
      // !x = !{!0, !0, i64 0}
      // With opaque pointers, the pointer-to-struct may be loaded as an actual
      // scalar type.
      // We should avoid propagating the fakeload MD in this case.
      // In most (if not all) of these cases, the load will have TBAA MD
      // already.
      FakeloadTBAA = nullptr;
    }
  }

  // Check for loads with different types, we can't use the same
  // TBAA info on them.
  if (FakeloadTBAA) {
    Type *LoadType = nullptr;
    for (auto *User : II.users()) {
      if (auto *LI = dyn_cast<LoadInst>(User)) {
        if (!LoadType) {
          LoadType = LI->getType();
        } else if (LI->getType() != LoadType) {
          FakeloadTBAA = nullptr;
          break;
        }
      }
    }
  }

  // Propagate the fakeload info if it's valid. Also check to see if
  // we can delete the fakeload (no return users).
  bool HasRetUser = false;
  for (auto *User : II.users()) {
    LoadInst *LI = dyn_cast<LoadInst>(User);
    if (FakeloadTBAA && LI && LI->getPointerOperand() == &II) {
      LI->setMetadata(LLVMContext::MD_tbaa, FakeloadTBAA);
      continue;
    }
    StoreInst *SI = dyn_cast<StoreInst>(User);
    if (FakeloadTBAA && SI && SI->getPointerOperand() == &II) {
      SI->setMetadata(LLVMContext::MD_tbaa, FakeloadTBAA);
      continue;
    }
    // If there is a return instruction user but it wasn't the only user we
    // need to keep the fakeload intrinsic to prepare for the next level of
    // inlining.
    if (isa<ReturnInst>(User))
      HasRetUser = true;
  }
  // If the value passed through the intrinsic is not used by a return
  // instruction, we can remove the intrinsic now.
  if (!HasRetUser) {
    II.replaceAllUsesWith(II.getArgOperand(0));
    II.eraseFromParent();
  }
}

// Helper function to unify processing of both Load/Store.
/// Refine and propagate TBAA information from the \p MemInst's \p PointerOp
/// pointer operand.
void propagateIntelTBAAToMemInst(Instruction &MemInst, Value *PointerOp) {
  const auto *GEP = dyn_cast<GetElementPtrInst>(PointerOp);
  if (!GEP)
    return;

  MDNode *MemInstTBAA = MemInst.getMetadata(LLVMContext::MD_tbaa);

  // MemInst's TU was compiled without TBAA support, can't legally propagate
  // information from the GEP.
  if (!MemInstTBAA)
    return;
  
  // In opaque pointers mode, there may be a "missing" bitcast, causing the
  // GEP result type not to match the memory access type. We cannot propagate
  // the TBAA info in this case.
  //
  //   %addr = gep %struct.s, ptr %p, i64 0... ; struct field type
  //   %res = load i8, %addr ; implicit cast of %addr to i8*
  //
  // The GEP indexes into the struct, which can result in any type. If the
  // result type does not match the i8 load type, the TBAA info cannot be used
  // for the load.
  if (GEP->getPointerOperandType()->isOpaquePointerTy()) {
    auto *GEPType = GEP->getResultElementType();
    // If the GEP result itself is a "ptr" type, we can't tell if it is
    // actually compatible with any other "ptr" types. This may be overly
    // conservative as C++ rules not allow dereferences of an object
    // from incompatible pointer types.
    if (GEPType->isOpaquePointerTy()) {
      return;
    }
    if (auto *LI = dyn_cast<LoadInst>(&MemInst)) {
      if (LI->getType() != GEPType)
        return;
    }
    else if (auto *SI = dyn_cast<StoreInst>(&MemInst)) {
      if (SI->getValueOperand()->getType() != GEPType)
        return;
    }
  }

  MDNode *MergedTBAA = getMostSpecificTBAA(getGepChainTBAA(GEP), MemInstTBAA);
  if (MemInstTBAA != MergedTBAA)
    MemInst.setMetadata(LLVMContext::MD_tbaa, MergedTBAA);
}

void TbaaMDPropagationImpl::visitLoad(LoadInst &LI) {
  propagateIntelTBAAToMemInst(LI, LI.getPointerOperand());
}

void TbaaMDPropagationImpl::visitStore(StoreInst &SI) {
  propagateIntelTBAAToMemInst(SI, SI.getPointerOperand());
}

// Recognize this pattern:
// define internal ... @callee(ptr p1, ptr p2) {
//    ... load ptr, ptr %p2 !tbaa !0
//   store ptr ..., ptr %p2 !tbaa !0
// }
//
// define ... @caller(...) {
//   %actual = getelementptr .... !intel-tbaa !1
//   call ... callee(... , %actual)
// }
// !0 = {!2, !2, i64 ... }
// !1 = {!3, !4, i64 ... }
// !2 = {"pointer@type1", ...}
// !3 = {"struct@s1", ... }
// !4 = {"pointer@type2", ...}
//
// The caller is typecasting the address of a pointer type struct field,
// to a different pointer type. This creates a possible aliasing error.
//
// void callee(...*p1, type1 **p2) {
//   *p2 = ...
//   ... = p2->...
// }
//
// callee(..., (type1**)&s->actual) // "actual" is *type2, addr is **type2
//
// For this situation, remove the TBAA information on the 2 callee accesses.
// The "F" arg is the callee. There are multiple callers (users of F).
void TbaaMDPropagationImpl::removeBadArgMD(Function *F) {
  // For the most part, we want to trust TBAA, so we restrict this code to
  // the exact pattern above.
  // Check internal (static) linkage.
  if (!F->hasInternalLinkage())
    return;

  // Check for 2 args, both opaque pointer type.
  if (F->arg_size() != 2)
    return;
  if (!F->getArg(0)->getType()->isOpaquePointerTy() ||
      !F->getArg(1)->getType()->isOpaquePointerTy())
    return;

  // This function takes an instruction with intel-tbaa or tbaa MD, and
  // looks for this pattern:
  //   ![intel-]tbaa = !{xxx, !at, xxx}
  //   !at = !{"pointer@something", ...}
  //
  // The TBAA node is a "struct path node" with access type !at.
  // This can be either a struct field {!0, !at, ...}
  // or a scalar type {!at, !at, ...}.
  // If the access type has a name "pointer@XXX", returns the name string
  // as an Optional<StringRef> (llvm::None if no match).
  auto getTBAAPtrTypeAccess = [=](Instruction *I) -> Optional<StringRef> {
    MDNode *TBAA = I->getMetadata(LLVMContext::MD_intel_tbaa);
    if (!TBAA)
      TBAA = I->getMetadata(LLVMContext::MD_tbaa);
    if (!TBAA)
      return llvm::None;
    if (TBAA->getNumOperands() != 3)
      return llvm::None;
    // Get the access type node which is the 2nd operand (operand 1).
    MDNode *FieldMD = dyn_cast<MDNode>(TBAA->getOperand(1));
    if (!FieldMD || FieldMD->getNumOperands() != 3)
      return llvm::None;

    // Check the access type name string (operand 0) for "pointer@".
    MDString *Tag = dyn_cast<MDString>(FieldMD->getOperand(0));
    if (!Tag)
      return llvm::None;
    if (Tag->getString().contains("pointer@"))
      return Optional<StringRef>(Tag->getString());
    return llvm::None;
  };

  // We are only interested in the 2nd arg of the callee.

  // Check that the 2nd arg has 1 load use, 1 store use.
  auto *Formal1 = F->getArg(1);
  unsigned numLoads = 0, numStores = 0;
  for (auto *U : Formal1->users()) {
    if (isa<LoadInst>(U))
      numLoads++;
    else if (isa<StoreInst>(U))
      numStores++;
    else
      return;
    if (numLoads > 1 || numStores > 1)
      return;
  }
  if (numLoads != 1 || numStores != 1)
    return;

  // Get the **pointer type name string from the first use (a load or store).
  Instruction *FirstUse = cast<Instruction>(*Formal1->user_begin());
  Optional<StringRef> FormalArgType = getTBAAPtrTypeAccess(FirstUse);
  if (!FormalArgType)
    return;

  // Now check the callers of F, for a GEP parameter whose type name string
  // does not match the formal's type name.
  bool MismatchedTypes = false;
  for (auto *U : F->users()) {
    auto *Call = dyn_cast<CallBase>(U);
    if (!Call)
      return;
    // callee name + 2 args = 3 ops
    if (Call->getNumOperands() != 3)
      return;
    // 2nd arg is GEP
    auto *GEP = dyn_cast<GetElementPtrInst>(Call->getOperand(1));
    if (!GEP)
      return;
    // GEP is **ptr type
    Optional<StringRef> CallArgType = getTBAAPtrTypeAccess(GEP);
    if (!CallArgType)
      return;

    // Check the actual's typename against the formal's typename.
    if (CallArgType.value() != FormalArgType.value()) {
      MismatchedTypes = true;
      break;
    }
  }
  if (!MismatchedTypes)
    return;

  // If any caller mismatched types, remove the TBAA on the uses of
  // the formal parameter.
  for (auto *U : Formal1->users()) {
    if (auto *I = dyn_cast<Instruction>(U)) {
      if (I->getMetadata(LLVMContext::MD_tbaa)) {
        LLVM_DEBUG(dbgs() << "Removing TBAA info on: " << *I << "\n");
        I->setMetadata(LLVMContext::MD_tbaa, nullptr);
      }
    }
  }
}

bool runTbaaMDPropagation(Function &F) {
  TbaaMDPropagationImpl impl;
  for (BasicBlock *BB : depth_first(&F.getEntryBlock())) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE;) {
      // Because we might be erasing the instruction, we need to get the
      // instruction reference first and then increment the iterator before
      // visiting the instruction.
      Instruction &I = *II;
      ++II;
      impl.visit(&I);
    }
  }
  impl.removeBadArgMD(&F);
  return false; // FIXME: Should this return true when something changes?
}

struct TbaaMDPropagationLegacyPass : public FunctionPass {
public:
  static char ID;
  TbaaMDPropagationLegacyPass() : FunctionPass(ID) {
    initializeTbaaMDPropagationLegacyPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  StringRef getPassName() const override { return "TBAAPROP"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};

} // end anonymous namespace

char TbaaMDPropagationLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(TbaaMDPropagationLegacyPass, "tbaa-prop",
                      "Propagate the TbaaMD through intrinsic", false, false)
INITIALIZE_PASS_END(TbaaMDPropagationLegacyPass, "tbaa-prop",
                    "Propagate the TbaaMD through intrinsic", false, false)

FunctionPass *llvm::createTbaaMDPropagationLegacyPass() {
  return new TbaaMDPropagationLegacyPass();
}

bool TbaaMDPropagationLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  return runTbaaMDPropagation(F);
}

PreservedAnalyses TbaaMDPropagationPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  if (!runTbaaMDPropagation(F))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<GlobalsAA>();
  return PA;
}

static bool runCleanupFakeLoads(Function &F) {
  for (BasicBlock *BB : depth_first(&F.getEntryBlock())) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE;) {
      // Because we might be erasing the instruction, we need to get the
      // instruction reference first and then increment the iterator before
      // processing the instruction.
      Instruction &I = *II;
      ++II;
      if (auto *Intrin = dyn_cast<IntrinsicInst>(&I)) {
        if (Intrin->getIntrinsicID() == Intrinsic::intel_fakeload) {
          I.replaceAllUsesWith(I.getOperand(0));
          I.eraseFromParent();
        }
      }
    }
  }
  return false; // FIXME: Should this return true when something changes?
}

PreservedAnalyses CleanupFakeLoadsPass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  if (!runCleanupFakeLoads(F))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<GlobalsAA>();
  return PA;
}

namespace {
struct CleanupFakeLoadsLegacyPass : public FunctionPass {
public:
  static char ID;
  CleanupFakeLoadsLegacyPass() : FunctionPass(ID) {
    initializeCleanupFakeLoadsLegacyPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  StringRef getPassName() const override { return "Cleanup fake loads"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};
} // end anonymous namespace

char CleanupFakeLoadsLegacyPass::ID = 0;
INITIALIZE_PASS(CleanupFakeLoadsLegacyPass, "cleanup-fakeloads",
                "Remove intel.fakeload intrinsics", false, false)

FunctionPass *llvm::createCleanupFakeLoadsPass() {
  return new CleanupFakeLoadsLegacyPass();
}

bool CleanupFakeLoadsLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  return runCleanupFakeLoads(F);
}
