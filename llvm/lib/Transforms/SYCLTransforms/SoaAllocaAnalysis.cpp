//===- SoaAllocaAnalysis.cpp - SOA alloca analysis --------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SoaAllocaAnalysis.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/Utils/VectorizerUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-soa-alloca-analysis"

AnalysisKey SoaAllocaAnalysis::Key;

SoaAllocaInfo::SoaAllocaInfo(const Function &F) : F(F) { compute(); }

SoaAllocaInfo SoaAllocaAnalysis::run(Function &F, FunctionAnalysisManager &) {
  return SoaAllocaInfo(F);
}

void SoaAllocaInfo::compute() {
  AllocaSOA.clear();
  SmallSet<const Value *, 32> Visited;

  // Compute the first iteration of the WI-dep according to ordering
  // instructions. This ordering is generally good (as it usually correlates
  // well with dominance).
  for (auto &I : instructions(F)) {
    auto *AI = dyn_cast<AllocaInst>(&I);
    if (!AI)
      continue;

    Type *AllocaTy = AI->getAllocatedType();
    unsigned ArrayNestedLevel = 0;
    while (auto *ArrayTy = dyn_cast<ArrayType>(AllocaTy)) {
      AllocaTy = ArrayTy->getElementType();
      ArrayNestedLevel++;
    }

    // At this point AllocaTy is not an array.
    if (!(AllocaTy->isIntOrIntVectorTy() || AllocaTy->isFPOrFPVectorTy())) {
      LLVM_DEBUG(dbgs() << "Unsupported base type: " << *AllocaTy << "\n");
      continue;
    }

    bool IsVectorBasedTy = AllocaTy->isVectorTy();
    unsigned Width = IsVectorBasedTy
                         ? (cast<FixedVectorType>(AllocaTy))->getNumElements()
                         : 0;
    // At this point the alloca type is supported for SOA-alloca. Need to check
    // all the derived usages of the alloca pointer are allowed.
    if (isSupportedAlloca(AI, IsVectorBasedTy, ArrayNestedLevel, Visited)) {
      // We have an alloca instruction that can be converted to SOA-alloca.
      // Mark it and all derived usages to be SOA-alloca related.
      for (const auto *V : Visited) {
        assert(AllocaSOA.count(V) == 0 &&
               "Visited value is already in the alloca list");
        AllocaSOA[V] = Width;
      }
    }
  }
}

bool SoaAllocaInfo::isSoaAllocaRelated(const Value *V) const {
  return AllocaSOA.count(V);
}

bool SoaAllocaInfo::isSoaAllocaScalarRelated(const Value *V) const {
  const auto It = AllocaSOA.find(V);
  return It != AllocaSOA.end() && It->second == 0;
}

bool SoaAllocaInfo::isSoaAllocaVectorRelated(const Value *V) const {
  const auto It = AllocaSOA.find(V);
  return It != AllocaSOA.end() && It->second != 0;
}

bool SoaAllocaInfo::isSoaAllocaRelatedPointer(const Value *V) const {
  return isSoaAllocaRelated(V) &&
         (isa<AllocaInst>(V) || isa<GetElementPtrInst>(V));
}

unsigned SoaAllocaInfo::getSoaAllocaVectorWidth(const Value *V) const {
  assert(isSoaAllocaVectorRelated(V) &&
         "V is not related to supported SOA alloca with vector base type");
  return AllocaSOA.find(V)->second;
}

bool SoaAllocaInfo::isSupportedAlloca(const AllocaInst *AI,
                                      bool IsVectorBasedType,
                                      unsigned ArrayNestedLevel,
                                      SmallSet<const Value *, 32> &Visited) {
  SmallVector<const Value *, 16> Users(AI->user_begin(), AI->user_end());
  Visited.clear();
  Visited.insert(AI);
  while (!Users.empty()) {
    const Value *U = Users.pop_back_val();
    // Insert into visited list if it is not already visited.
    if (!Visited.insert(U).second)
      continue;
    if (const auto *GEP = dyn_cast<GetElementPtrInst>(U)) {
      // One index for pointer and other indices for arrays.
      if (GEP->getNumIndices() <= (ArrayNestedLevel + 1)) {
        // These are allowed instructions that result in pointers, so need to
        // check their users too.
        Users.insert(Users.end(), U->user_begin(), U->user_end());
        LLVM_DEBUG(dbgs() << "Supported GEP user: " << *U << "\n");
        continue;
      }
      // Cannot support GEP with last index for vector type.
      LLVM_DEBUG(dbgs() << "Unsupported GEP user: " << *U << "\n");
      return false;
    }
    if (isa<LoadInst>(U)) {
      // Load is allowed instruction that does not result in a pointer.
      continue;
    }
    if (const auto *SI = dyn_cast<StoreInst>(U)) {
      if (SI->getValueOperand() == AI) {
        LLVM_DEBUG(dbgs() << "Unsupported StoreInst user: " << *U << "\n");
        return false;
      }
      continue;
    }
    if (isa<BitCastInst>(U) || isa<AddrSpaceCastInst>(U)) {
      const auto *CastI = cast<CastInst>(U);
      // BitCast on alloca with vector based type is not supported. Only Bitcast
      // of alloca instruction is supported.
      if (!IsVectorBasedType && CastI->getOperand(0) == AI) {
        for (const auto *CU : CastI->users()) {
          // Only CastInst with users that are memset or llvm instrinsics
          // declared in isSafeInstrinsic are supported.
          const auto *CI = dyn_cast<CallInst>(CU);
          Function *Callee;
          if (!CI || !(Callee = CI->getCalledFunction()) ||
              !Callee->isIntrinsic() ||
              (!VectorizerUtils::isSafeIntrinsic(Callee->getIntrinsicID()) &&
               !isSupportedMemset(CI))) {
            LLVM_DEBUG(dbgs() << "Unsupported CastInst user " << *U
                              << " which is used in " << *CU << "\n");
            return false;
          }
        }
        Users.insert(Users.end(), U->user_begin(), U->user_end());
        LLVM_DEBUG(dbgs() << "Supported CastInst user: " << *U << "\n");
        continue;
      }
      LLVM_DEBUG(dbgs() << "Unsupported CastInst user: " << *U << "\n");
      return false;
    }
    if (const auto *CI = dyn_cast<CallInst>(U)) {
      Function *Callee = CI->getCalledFunction();
      assert(Callee && "Unexpected indirect function call");
      if (Callee->isIntrinsic()) {
        if (VectorizerUtils::isSafeIntrinsic(Callee->getIntrinsicID()) ||
            isSupportedMemset(CI)) {
          LLVM_DEBUG(dbgs() << "Supported intrinsic user: " << *U << "\n");
          continue;
        }
        LLVM_DEBUG(dbgs() << "Unsupported intrinsic user: " << *U << "\n");
        return false;
      }
    }

    // Reaching here means we have user of unsupported instruction.
    LLVM_DEBUG(dbgs() << "Unhandled and unsupported user: " << *U << "\n");
    return false;
  }
  // Reaching here means alloca instruction is SOA supported.
  LLVM_DEBUG(dbgs() << "Alloca is supported: " << *AI << "\n");
  return true;
}

bool SoaAllocaInfo::isSupportedMemset(const CallInst *CI) {
  assert(CI && "invalid CI");

  Function *Callee = CI->getCalledFunction();
  assert(Callee && "Unexpected indirect function call");

  if (!Callee || !Callee->isIntrinsic() ||
      Callee->getIntrinsicID() != Intrinsic::memset)
    return false;

  assert(CI->getType()->isVoidTy() &&
         "llvm.memset function should return void");
  // Only support that value to set is constant, i.e. cannot support non-uniform
  // set value to SOA alloca.
  return isa<Constant>(CI->getArgOperand(1));
}

void SoaAllocaInfo::print(raw_ostream &OS) const {
  OS << "SoaAllocaAnalysis for function " << F.getName() << ":\n";
  for (auto &I : instructions(F))
    if (isSoaAllocaRelated(&I))
      OS << "  " << I << " SR:[" << isSoaAllocaScalarRelated(&I) << "] VR:["
         << isSoaAllocaVectorRelated(&I) << "] PR:["
         << isSoaAllocaRelatedPointer(&I) << "]\n";
}

PreservedAnalyses SoaAllocaAnalysisPrinter::run(Function &F,
                                                FunctionAnalysisManager &AM) {
  AM.getResult<SoaAllocaAnalysis>(F).print(OS);
  return PreservedAnalyses::all();
}
