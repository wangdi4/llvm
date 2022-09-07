//==-DTransBadCastingAnalyzerOP.cpp - Specialized OP bad casting analyzer--===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransBadCastingAnalyzerOP.h"
#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include <queue>

#define DEBUG_TYPE "dtransanalysis"

using namespace llvm;
using namespace dtransOP;

// Debug type for verbose bad casting analysis output.
#define DTRANS_BCA "dtrans-bca"

//
// Returns the Type of the structure referenced by the 'GEPI'.
//
// For example, in:
//   %8 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     ptr %0, i64 0, i32 2
// where:
//   %struct.lzma_next_coder_s = type { i8*, i64, i64, ..
// the last type is %struct.lzma_next_coder_s.
//
// The function is more interesting and useful when a GEP with more than
// 2 indices is specified. For example:
//   %30 = getelementptr inbounds %struct.lzma_coder_s.187, \
//     ptr %28, i64 0, i32 2, i32 0
// where:
//   %struct.lzma_coder_s.187 = type { %struct.lzma_dict, \
//     %struct.lzma_lz_decoder, %struct.lzma_next_coder_s, i8, i8, \
//     %struct.anon.186 }
// and the last type is %struct.lzma_next_coder_s.
//
llvm::Type *
DTransBadCastingAnalyzerOP::getLastType(GetElementPtrInst *GEPI) {
  SmallVector<Value *, 4> Ops(GEPI->idx_begin(), GEPI->idx_end() - 1);
  return GetElementPtrInst::getIndexedType(GEPI->getSourceElementType(), Ops);
}

//
// Bad casting analysis which is performed before the instructions of the
// module are visited. This analysis walks the list of structure types to
// identify a single candidate root type.  The candidate root type will
// have a field which is a void* (in LLVM IR i8*) field in field position
// CandidateVoidField and contains the maximum number of fields which are
// function pointers.
//
// For example:
//  %struct.lzma_next_coder_s = type { i8*, i64, i64, \
//   i32 (i8*, %struct.lzma_allocator*, i8*, i64*, i64, i8*, i64*, i64, i32)*,
//   void (i8*, %struct.lzma_allocator*)*, \
//   i32 (i8*)*, i32 (i8*, i64*, i64*, i64)*, \
//   i32 (i8*, %struct.lzma_allocator*, %struct.lzma_filter*, \
//     %struct.lzma_filter*)*
//  }
// Could be a good candidate, as its zeroth field is i8* and it has 5 fields
// which are function pointers. (Not that the types of the pointers can only
// be determined by using the metadata.)
//
bool
DTransBadCastingAnalyzerOP::analyzeBeforeVisit() {
  DEBUG_WITH_TYPE(DTRANS_BCA,
                  { dbgs() << "dtrans-bca: Begin bad casting analysis\n"; });
  unsigned BestCount = 0;
  for (auto *StTy : TM.getIdentifiedStructTypes()) {
      unsigned E = StTy->getNumFields();
      if (E <= CandidateVoidField)
        continue;
      DTransType *DFTyC = StTy->getFieldType(CandidateVoidField);
      if (DFTyC != getDTransI8PtrType())
        continue;
      unsigned Count = 0;
      for (unsigned I = 0; I != E; ++I) {
        if (I == CandidateVoidField)
          continue;
        DTransType *DFTy = StTy->getFieldType(I);
        if (DFTy->isPointerTy() &&
            DFTy->getPointerElementType()->isFunctionTy())
          ++Count;
      }
      if (Count > BestCount) {
        CandidateRootType = StTy;
        BestCount = Count;
      }
  }
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Candidate Root Type: ";
    dbgs() << (CandidateRootType ?
        dtrans::getStructName(CandidateRootType->getLLVMType()) : "<<NONE>>")
        << "\n";
  });
  if (!CandidateRootType || BestCount == 0)
    setFoundViolation(true);
  return CandidateRootType;
}

//
// If the Index-th argument of the Function F is of a single specific type,
// return a pair of the form std::make_pair(true, Type), where Type is that
// single specific type.  If that argument is not used, return the pair
// std::make_pair(true, nullptr). Otherwise, return std::make_pair(false,
// nullptr).
//
// For example, in
//  define internal void @lz_decoder_end(ptr, ptr) #7 {
//    %4 = getelementptr inbounds %struct.lzma_coder_s.187, ptr %0, i64 0, i32 2
//    %5 = getelementptr inbounds %struct.lzma_coder_s.187, ptr %0, i64 0, i32 0
//    %8 = getelementptr inbounds %struct.lzma_coder_s.187, ptr %0, i64 0, i32 1
// The function lz_decoder_end has the argument at index 0 "virtually" cast to
//    %struct.lzma_coder_s.187*
// and so we will return (true, %struct.lzma_coder_s.187).
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzerOP::findSpecificArgType(Function *F, unsigned Index) {
  if (Index > F->arg_size() - 1)
    return std::make_pair(false, nullptr);
  Argument *ArgIndex = F->arg_begin() + Index;
  llvm::Type *ResultType = nullptr;
  for (auto *U : ArgIndex->users()) {
    // Tolerate use in a call to a free-like function.
    if (auto CI = dyn_cast<CallInst>(U)) {
      if (PTA.getFreeCallKind(CI) == dtrans::FK_NotFree)
        return std::make_pair(false, nullptr);
      continue;
    }
    auto GEPI = dyn_cast<GetElementPtrInst>(U);
    if (GEPI) {
      if (!ResultType)
        ResultType = GEPI->getSourceElementType();
      else if (ResultType != GEPI->getSourceElementType())
        return std::make_pair(false, nullptr);
      continue;
    } 
    return std::make_pair(false, nullptr);
  }
  if (!ResultType)
    return std::make_pair(true, nullptr);
  if (!isa<StructType>(ResultType))
    return std::make_pair(false, nullptr);
  // Jumped through all of the hoops. Return the ResultType.
  return std::make_pair(true, ResultType);
}

//
// Find the single source element type used in GetElementPtrInsts that access
// pointer stored in the StoreInst 'STI'.  Return nullptr if there is not a
// single source element type. If 'CheckPHIInputs', check the inputs to any
// PHINode which has the value operand of 'STI' as one of its inputs to ensure
// that they only come from conditionally dead basic blocks.
//
// For example, in:
//   %18 = tail call fastcc ptr @lzma_alloc(i64 %17) #4
//   store ptr %18, ptr %11, align 8
//   %19 = icmp eq ptr %18, null
//   ...
//   %28 = getelementptr inbounds %struct.lzma_coder_s.260, \
//      ptr %18, i64 0, i32 0, i32 0
//   store ptr null, ptr %28, align 8
//   %29 = getelementptr inbounds %struct.lzma_coder_s.260, \
//     ptr %18, i64 0, i32 0, i32 1
//   %30 = getelementptr inbounds %struct.lzma_coder_s.260, \
//     ptr %18, i64 0, i32 0, i32 2
// we are supplying the store as an argument and expecting to get the
// source element type of the GetElementPtrInst instructions which have %18
// as their pointer operand, which is %struct.lzma_coder_s.260.
//
llvm::Type *
DTransBadCastingAnalyzerOP::findSingleGEPISourceElementType(StoreInst *STI,
    bool CheckPHIInputs) {
  auto CI = dyn_cast<CallInst>(STI->getValueOperand());
  if (!CI)
    return nullptr;
  llvm::Type *Result = nullptr;
  if (PTA.getAllocationCallKind(CI) == dtrans::AK_NotAlloc)
    return nullptr;
  unsigned UserCount = 0;
  for (auto *U : CI->users()) {
    // The store should be one of the uses.
    if (U == STI) {
      UserCount++;
      continue;
    }
    // One of the uses can be an optional test against a constant null pointer.
    // This indicates that the allocation can be conditional.
    if (auto CmpI = dyn_cast<ICmpInst>(U)) {
      auto CT = U->getOperand(0) == CI ? U->getOperand(1) : U->getOperand(0);
      if (!isa<ConstantPointerNull>(CT))
        return nullptr;
      UserCount++;
      continue;
    }
    if (auto PHIN = dyn_cast<PHINode>(U)) {
      if (CheckPHIInputs) {
        // A PHINode may join an access to stored value with other
        // "fake" sources that will never actually reach to the PHINode.
        // For now, it is enough to check that such a "fake" source
        // can only come from the special gaurd conditional basic block.
        // This can be generalized if it is found useful.
        for (unsigned I = 0; I < PHIN->getNumIncomingValues(); ++I) {
          if (PHIN->getIncomingValue(I) == CI)
            continue;
          BasicBlock *BB = PHIN->getIncomingBlock(I);
          if (!isSpecialGuardConditional(BB) && BB != STI->getParent()) {
            DEBUG_WITH_TYPE(DTRANS_BCA, {
              dbgs() << "dtrans-bca: (SV) [" << CandidateVoidField;
              dbgs() << "] Improper incoming block for PHI node:";
              PHIN->dump();
            });
            return nullptr;
          }
        }
      }
      for (auto *UU : PHIN->users()) {
        if (auto GEPI = dyn_cast<GetElementPtrInst>(UU)) {
          if (!Result)
            Result = GEPI->getSourceElementType();
          else if (Result !=  GEPI->getSourceElementType())
            return nullptr;
        } else {
          return nullptr;
        }
      }
      continue;
    }
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      if (!Result)
        Result = GEPI->getSourceElementType();
      else if (Result !=  GEPI->getSourceElementType())
        return nullptr;
      continue;
    }
    return nullptr;
  }
  return Result;
}

//
// Return the Type to which 'TI' is bit cast, if it is store to a field
// of a structure type specified by 'GEPI'.  If there is no such unique
// type, return nullptr.
//
// For example, in:
//   %11 = getelementptr inbounds %struct.lzma_next_coder_s, \
//       ptr %0, i64 0, i32 0
//   %18 = tail call fastcc ptr @lzma_alloc(i64 %17) #4
//   store ptr %18, ptr %11, align 8
//   %19 = icmp eq ptr %18, null
//   ...
//   %53 = getelementptr inbounds %struct.lzma_coder_s.260, \
//       ptr %19, i64 0, i32 5
//   ...
//   %60 = getelementptr inbounds %struct.lzma_coder_s.260, \
//       ptr %19, i64 0, i32 2
//   ...
// If the GetElementPtrInst in %11 matches 'GEPI' and 'TI' is the indicated
// store, we return the type %struct.lzma_coder_s.260.
//
llvm::Type *
DTransBadCastingAnalyzerOP::foundStoreType(Instruction *TI,
                                           GetElementPtrInst *GEPI) {
  // Look for a store to a matching GEP
  auto STI = dyn_cast<StoreInst>(TI);
  if (!STI)
    return nullptr;
  auto AltGEPI = dyn_cast<GetElementPtrInst>(STI->getPointerOperand());
  if (!AltGEPI)
    return nullptr;
  if (getLastType(AltGEPI) != getLastType(GEPI))
    return nullptr;
  // Expect it to be stored from a malloc like function and to be used as
  // a single type.
  return findSingleGEPISourceElementType(STI, false);
}

//
// Starting with instruction before 'TI', walk backward to find a store
// instruction which stores to the structure indicated by 'GEPI', and
// return the unique type of the value operands of the store instruction.
// If no such store with a unique type is found, return nullptr.
//
llvm::Type *
DTransBadCastingAnalyzerOP::findStoreTypeBack(Instruction *TI,
                                            GetElementPtrInst *GEPI) {
  // Starting with the Instruction above 'TI' search for a store of
  // allocated memory to the same location as 'GEPI'.
  BasicBlock::reverse_iterator RIT(TI);
  BasicBlock *BB = TI->getParent();
  ++RIT;
  while (RIT != BB->rend()) {
    Instruction &I = *RIT;
    llvm::Type *Ty = foundStoreType(&I, GEPI);
    if (Ty)
      return Ty;
    ++RIT;
  }
  // If one is not found in the immediate basic block, traverse the blocks
  // in the path immediately above it, as long as each block we search is
  // a single predecessor of the last block that we traversed.
  for (BB = BB->getSinglePredecessor(); BB != nullptr;
       BB = BB->getSinglePredecessor()) {
    for (auto RS = BB->rbegin(), RE = BB->rend(); RS != RE; ++RS) {
      Instruction &I = *RS;
      llvm::Type *Ty = foundStoreType(&I, GEPI);
      if (Ty)
        return Ty;
    }
  }
  return nullptr;
}

//
// Return the GEPI that is tested to be equal to a nullptr in the terminating
// condition of 'BB', if there is one. Otherwise, return nullptr.
//
// For example, in the basic block:
//   %11 = getelementptr inbounds %struct.lzma_next_coder_s, \
//       ptr %0, i64 0, i32 0
//   %12 = load ptr, ptr %11, align 8
//   %13 = icmp eq ptr %12, null
//   br i1 %13, label %15, label %44
// we return the GEPI in %11.
//
GetElementPtrInst *
DTransBadCastingAnalyzerOP::getRootGEPIFromConditional(BasicBlock *BB) {
  auto BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI)
    return nullptr;
  if (!BI || BI->isUnconditional())
    return nullptr;
  if (BI->getNumSuccessors() != 2)
    return nullptr;
  auto ICI = dyn_cast<ICmpInst>(BI->getCondition());
  if (!ICI)
    return nullptr;
  if (ICI->getPredicate() != ICmpInst::ICMP_EQ &&
      ICI->getPredicate() != ICmpInst::ICMP_NE)
    return nullptr;
  Value *V = nullptr;
  if (isa<ConstantPointerNull>(ICI->getOperand(0)))
    V = ICI->getOperand(1);
  else if (isa<ConstantPointerNull>(ICI->getOperand(1)))
    V = ICI->getOperand(0);
  if (V == nullptr)
    return nullptr;
  auto LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return nullptr;
  auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  return GEPI;
}

//
// Return 'true' if the 'GEPI' is accessing the candidate structure.
//
bool
DTransBadCastingAnalyzerOP::gepiMatchesCandidateStruct(
    GetElementPtrInst *GEPI) {
  llvm::Type *IndexedTy = getLastType(GEPI);
  auto IndexedStructTy = dyn_cast<StructType>(IndexedTy);
  if (!IndexedStructTy)
    return false;
  return IndexedStructTy == CandidateRootType->getLLVMType();
}

//
// Return 'true' if the 'GEPI' is accessing the candidate field.
//
bool
DTransBadCastingAnalyzerOP::gepiMatchesCandidateField(
    GetElementPtrInst *GEPI) {
  if (!CandidateRootType)
    return false;
  if (!gepiMatchesCandidateStruct(GEPI))
    return false; 
  auto ConstIndex = GEPI->getOperand(GEPI->getNumOperands() - 1);
  auto *LastArg = dyn_cast<ConstantInt>(ConstIndex);
  if (!LastArg)
    return false;
  uint64_t LastIndex = LastArg->getLimitedValue();
  return LastIndex == CandidateVoidField;
}

//
// Assuming 'BB' is a special guard conditional basic block, return the
// basic block which is the target of the taken path.
//
// For example, this is a special guard conditional basic block:
//   %11 = getelementptr inbounds %struct.lzma_next_coder_s, \
//       ptr %0, i64 0, i32 0
//   %12 = load ptr, ptr %11, align 8
//   %13 = icmp eq ptr %12, null
//   br i1 %13, label %15, label %44
// and the basic block returned will be the one starting with label 15.
//
BasicBlock *
DTransBadCastingAnalyzerOP::getTakenPathOfSpecialGuardConditional(
    BasicBlock *BB) {
  auto BI = cast<BranchInst>(BB->getTerminator());
  auto ICI = cast<ICmpInst>(BI->getCondition());
  assert(ICI->getPredicate() == ICmpInst::ICMP_EQ ||
         ICI->getPredicate() == ICmpInst::ICMP_NE);
  return ICI->getPredicate() == ICmpInst::ICMP_EQ ? BI->getSuccessor(0)
                                                  : BI->getSuccessor(1);
}

//
// Assuming 'BB' is a special guard conditional basic block, return the
// basic block which is the target of the NOT taken path.
//
// For example, this is a special guard coniditonal basic block:
//   %11 = getelementptr inbounds %struct.lzma_next_coder_s, \
//       ptr %0, i64 0, i32 0
//   %12 = load ptr, ptr %11, align 8
//   %13 = icmp eq ptr %12, null
//   br i1 %13, label %15, label %44
// and the basic block returned will be the one starting with label 44.
//
BasicBlock *
DTransBadCastingAnalyzerOP::getNotTakenPathOfSpecialGuardConditional(
    BasicBlock *BB) {
  auto BI = cast<BranchInst>(BB->getTerminator());
  auto ICI = cast<ICmpInst>(BI->getCondition());
  assert(ICI->getPredicate() == ICmpInst::ICMP_EQ ||
         ICI->getPredicate() == ICmpInst::ICMP_NE);
  return ICI->getPredicate() == ICmpInst::ICMP_EQ ? BI->getSuccessor(1)
                                                  : BI->getSuccessor(0);
}

//
// Return 'true' if 'BB' is terminated with an instruction representing the
// a test of whether the candidate field is equal (or not equal) to nullptr.
//
bool
DTransBadCastingAnalyzerOP::isSpecialGuardConditional(BasicBlock *BB) {
  GetElementPtrInst *GEPI = getRootGEPIFromConditional(BB);
  if (!GEPI)
    return false;
  return gepiMatchesCandidateField(GEPI);
}

//
// Given that we have searched 'BB', find the next BasicBlock to search in
// a forward search for a store to a structure indexed by 'GEPI'.  If 'BB'
// is terminated by a test of the candidate field against nullptr, take the
// path that assumes that the candidate field is nullptr. Otherwise, the
// returned BasicBlock should be the unique successor to 'BB', or nullptr,
// if there is none.
//
BasicBlock *
DTransBadCastingAnalyzerOP::getStoreForwardAltNextBB(BasicBlock *BB,
                                                     GetElementPtrInst *GEPI) {
  GetElementPtrInst *AltGEPI = getRootGEPIFromConditional(BB);
  if (GEPI != AltGEPI)
    return nullptr;
  return getTakenPathOfSpecialGuardConditional(BB);
}

//
// Walk through 'CI' into its calling function to continue the search for
// a store instruction which stores to the same structure type as that
// indicated by 'GEPI'.  If one is found, return a pair, the second element
// of which is the unique type of the store's value operand. In this case, the
// first element will be a bool which indicates whether the search required
// taking the 'true' path of a conditional which tests the candidate
// field against nullptr.  If no such store instruction with a value operand
// which has a unique type is found, return std::make_pair(false, nullptr).
//
// For example, in:
//  %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     ptr %0, i64 0, i32 7
//  store ptr @delta_encoder_update, ptr %5, align 8
//  %6 = tail call fastcc i32 @lzma_delta_coder_init( \
//     ptr %0, ptr %1, ptr %2) #4
// We would walk forward through the GetElementPtrInst and StoreInst
// and get to the call.  At the call, we will start with the GEPI at %5
// with  pointer operand %0, and then continue through the CallInst %6
// through the zeroth argument of @lzma_delta_coder_init:
//   define internal fastcc i32 @lzma_delta_coder_init( \
//     ptr, ptr, ptr) unnamed_addr #7 {
//   %4 = alloca { ptr, ptr, ptr, ptr, ptr }
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     ptr %0, i64 0, i32 0
//   %6 = load ptr, ptr %5, align 8, !tbaa !34
//   %7 = icmp eq ptr %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8:                                      ; preds = %3
//   %9 = tail call fastcc ptr @lzma_alloc(i64 336) #4
//   store ptr %9, ptr %5, align 8
//   %10 = icmp eq ptr %9, null
//   ...
//   %15 = getelementptr inbounds %struct.lzma_coder_s.243, \
//     ptr %9, i64 0, i32 0, i32 0
//   store ptr null, ptr %15, align 8
//   %16 = getelementptr inbounds %struct.lzma_coder_s.243, \
//     ptr %9, i64 0, i32 0, i32 1
//   store i64 -1, ptr %16, align 8
//   %17 = getelementptr inbounds %struct.lzma_coder_s.243, \
//     ptr %9, i64 0, i32 0, i32 2
// We would continue the search at %5 take the "true" conditional branch
// to %9, and find the GetElementPtrInsts's source element type
// %struct.lzma_coder_s.243. We would return
// std::make_pair(true, %struct.lzma_coder_s.243).
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzerOP::findStoreTypeForwardCall(CallInst *CI,
                                                     GetElementPtrInst *GEPI) {
  Function *F = CI->getCalledFunction();
  if (F == nullptr)
    return std::make_pair(false, nullptr);
  // Exclude varargs and other cases like legacy Fortran and C code where
  // we can have more actual arguments than formal arguments.
  if (CI->arg_size() != F->arg_size())
    return std::make_pair(false, nullptr);
  unsigned ArgIndex = 0;
  bool ArgIndexFound = false;
  for (unsigned I = 0; I < CI->arg_size(); ++I)
    if (CI->getArgOperand(I) == GEPI->getPointerOperand()) {
      ArgIndex = I;
      ArgIndexFound = true;
      break;
    }
  if (!ArgIndexFound)
    return std::make_pair(false, nullptr);
  Argument *FormalArg = F->arg_begin() + ArgIndex;
  GetElementPtrInst *RootGEPI = nullptr;
  for (auto &I : F->getEntryBlock()) {
    auto AltGEPI = dyn_cast<GetElementPtrInst>(&I);
    if (AltGEPI && AltGEPI->getPointerOperand() == FormalArg &&
        getLastType(GEPI) == getLastType(AltGEPI)) {
      RootGEPI = AltGEPI;
      break;
    }
  }
  if (!RootGEPI)
    return std::make_pair(false, nullptr);
  return findStoreTypeForward(RootGEPI, RootGEPI);
}

//
// Starting with instruction after 'TI', walk forward to find a store
// instruction which stores to the structure indicated by 'GEPI'.
// If one is found, return a pair, the second element of which is the
// pointer element type of a StoreInst, which serves as the pointer operand
// of one or more GetElementPtrInsts. In this case, the first element will be
// a bool which indicates whether the search required taking the 'true' path
// of a conditional which tests the candidate field against/ nullptr.  If no
// such StoreInst found, return std::make_pair(false, nullptr).
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzerOP::findStoreTypeForward(Instruction *TI,
                                                 GetElementPtrInst *GEPI) {
  BasicBlock::iterator IT(TI);
  BasicBlock *BB = TI->getParent();
  for (++IT; IT != BB->end(); ++IT) {
    Instruction &I = *IT;
    llvm::Type *Ty = foundStoreType(&I, GEPI);
    if (Ty)
      return std::make_pair(false, Ty);
    auto CI = dyn_cast<CallInst>(&I);
    if (CI) {
      auto CallReturn = findStoreTypeForwardCall(CI, GEPI);
      if (CallReturn.second)
        return CallReturn;
    }
  }
  bool IsConditional = false;
  auto BBS = BB->getSingleSuccessor();
  IsConditional = !BBS;
  BB = BBS ? BBS : getStoreForwardAltNextBB(BB, GEPI);
  while (BB != nullptr) {
    for (auto &I : *BB) {
      llvm::Type *Ty = foundStoreType(&I, GEPI);
      if (Ty)
        return std::make_pair(IsConditional, Ty);
    }
    if (IsConditional)
      return std::make_pair(false, nullptr);
    auto BBS = BB->getSingleSuccessor();
    IsConditional = !BBS;
    BB = BBS ? BBS : getStoreForwardAltNextBB(BB, GEPI);
  }
  return std::make_pair(false, nullptr);
}

//
// Starting with instruction after 'TI', walk backward, and if necessary,
// forward, to find a StoreInst which stores to the structure
// indicated by 'GEPI'.  If one is found, return a pair, the second element
// of which is the pointer element type of the value operand of the StoreInst.
// In this case, the first element of the return value will be a bool which
// indicates whether the search required taking the 'true' path of a
// conditional which tests the candidate field against nullptr.  If no such
// store instruction is found, return std::make_pair(false, nullptr).
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzerOP::findStoreType(Instruction *TI,
                                          GetElementPtrInst *GEPI) {
  llvm::Type *TypeBack = findStoreTypeBack(TI, GEPI);
  if (TypeBack)
    return std::make_pair(false, TypeBack);
  auto TypeForwardReturn = findStoreTypeForward(TI, GEPI);
  if (TypeForwardReturn.second)
    return TypeForwardReturn;
  return std::make_pair(false, nullptr);
}

//
// Return BCCondTop, if 'SBB' is an unconditionally executed basic block.
// Return BCCondSpecial, if 'SBB' is executed under a special test of the
//   candidate field against nullptr.
// Return BCCondBottom, if we can't determine either of the two above
//   conditions.
//
// For example,
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     ptr %0, i64 0, i32 0
//   %6 = load ptr, ptr %5, align 8, !tbaa !34
//   %7 = icmp eq ptr %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8:                                      ; preds = %3
//   %9 = tail call fastcc ptr @lzma_alloc(i64 336) #4
//   store ptr %9, ptr %5, align 8
// The basic block in <label>:8 is conditionally executed under a test of
// the candidate field against nullptr, and so we would return BCCondSpecial
// if it were passed to this function.
//

DTransBadCastingAnalyzerOP::BCCondType
DTransBadCastingAnalyzerOP::isConditionalBlock(BasicBlock *SBB) {
  SmallPtrSet<BasicBlock *, 20> VisitedBlocks;
  std::queue<BasicBlock *> PendingBlocks;
  for (BasicBlock *BB : predecessors(SBB))
    PendingBlocks.push(BB);
  while (!PendingBlocks.empty()) {
    BasicBlock *BB = PendingBlocks.front();
    PendingBlocks.pop();
    if (VisitedBlocks.find(BB) != VisitedBlocks.end())
      continue;
    VisitedBlocks.insert(BB);
    auto BI = dyn_cast<BranchInst>(BB->getTerminator());
    if (!BI)
      return BCCondBottom;
    if (BI->isUnconditional()) {
      for (BasicBlock *PBB : predecessors(BB))
        PendingBlocks.push(PBB);
      continue;
    }
    if (isSpecialGuardConditional(BB))
      return BCCondSpecial;
    for (BasicBlock *PBB : predecessors(BB))
      PendingBlocks.push(PBB);
  }
  return BCCondTop;
}

//
// Record that 'SI' is an alloc store to the type 'StType'.
//
void
DTransBadCastingAnalyzerOP::recordAllocStore(StoreInst *SI,
                                             llvm::Type *StType) {
  DTransBadCastingAnalyzerOP::BCCondType Result;
  Result = isConditionalBlock(SI->getParent());
  bool IsConditional = Result == BCCondSpecial;
  AllocStores.insert(std::make_pair(SI, std::make_pair(IsConditional, StType)));
  PendingStores.erase(SI);
}

//
// Analyze the store instruction 'I' to the field 'FI' for special bad
// casting analysis. In particular, we are analyzing stores to the fields
// of the CandidateRootType.  If it is to the CandidateVoidField, show
// that the stored value is either nullptr or from allocated memory which
// is cast to be of a particular structure type, and all references to that
// memory are references to the fields of that structure type. If it is to
// a field which is a function pointer field, ensure that the
// VoidArgumentIndex of that function can be determined and if the
// VoidArgumentIndex is used in the function, it is used as a pointer to
// the type of the structure being allocated in this store.
//
// This function will return 'false' if the analysis must terminate
// because a violation (bad casting, unsafe pointer store, or mismatched
// element access) is found. In this case, setFoundViolation() is called to
// record that the violation has been observed.  Otherwise, we return 'true'
// and continue with the analysis.
//
// For example, we are looking to analyze stores like:
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     ptr %0, i64 0, i32 0
//   %6 = load ptr, ptr %5, align 8, !tbaa !34
//   %7 = icmp eq ptr %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8:                                      ; preds = %3
//   %9 = tail call fastcc ptr @lzma_alloc(i64 336) #4
//   store ptr %9, ptr %5, align 8, !tbaa !34
// and like:
//   %13 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     ptr %0, i64 0, i32 4
//   store ptr @delta_coder_end, ptr %13, align 8
//
bool
DTransBadCastingAnalyzerOP::analyzeStore(dtrans::FieldInfo &FI,
                                         Instruction &I) {

  auto HandleCandidateField = [this](StoreInst *STI,
                                     GetElementPtrInst *GEPI) -> bool {
    // We can tolerate a store of a nullptr value.
    if (isa<ConstantPointerNull>(STI->getValueOperand()))
      return true;
    // Check if the store used as the pointer operand of one or more
    // GetElementPtrInsts of a specific source element type.
    llvm::Type *TargetType = findSingleGEPISourceElementType(STI, true);
    if (!TargetType) {
      DEBUG_WITH_TYPE(DTRANS_BCA, {
        dbgs() << "dtrans-bca: (SV) [" << CandidateVoidField;
        dbgs() << "] Non-GEP target\n";
      });
      setFoundViolation(true);
      return false;
    }
    recordAllocStore(STI, TargetType);
    return true;
  };

  auto HandleCandidateRootFxnPtrField = [this](StoreInst *STI,
                                               GetElementPtrInst *GEPI,
                                               unsigned Index) {
    // For the case of a function pointer field being stored, ensure that the
    // VoidArgumentIndex of that function can be determined ...
    Function *F = cast<Function>(STI->getValueOperand());
    auto SpecificArgResult = findSpecificArgType(F, VoidArgumentIndex);
    if (!SpecificArgResult.first) {
      DEBUG_WITH_TYPE(DTRANS_BCA, {
        dbgs() << "dtrans-bca: ";
        dbgs() << "(SF) [" << Index << "] Fxn ptr arg not a single type: ";
        F->printAsOperand(dbgs());
        dbgs() << "\n";
      });
      setFoundViolation(true);
      return false;
    }
    // ... and if the VoidArgumentIndex is used in the function, it is used
    // as a pointer to the type of the structure being allocated in this store.
    if (SpecificArgResult.second) {
      auto StReturn = findStoreType(STI, GEPI);
      if (StReturn.second != SpecificArgResult.second) {
        DEBUG_WITH_TYPE(DTRANS_BCA, {
          dbgs() << "dtrans-bca: ";
          dbgs() << "(SF) [" << Index << "] ";
          dbgs() << "Store type != fxn ptr arg type:\n";
          if (StReturn.second) {
            dbgs() << "Store Type: ";
            StReturn.second->dump();
            dbgs() << "\n";
          }
          if (SpecificArgResult.second) {
            dbgs() << "Fxn ptr arg Type: ";
            SpecificArgResult.second->dump();
            dbgs() << "\n";
          }
        });
        setFoundViolation(true);
        return false;
      }
    }
    return true;
  };

  if (foundViolation())
    return false;
  // Examine stores only from a GEPI.
  auto STI = cast<StoreInst>(&I);
  auto GEPI = dyn_cast<GetElementPtrInst>(STI->getPointerOperand());
  if (!GEPI)
    return true;
  if (!gepiMatchesCandidateStruct(GEPI))
    return true; 
  // Find the field being stored
  auto ConstIndex = GEPI->getOperand(GEPI->getNumOperands() - 1);
  auto *LastArg = cast<ConstantInt>(ConstIndex);
  uint64_t Index = LastArg->getLimitedValue();
  auto AV = STI->getValueOperand();
  if (isa<ConstantPointerNull>(AV))
    return true;
  if (Index == CandidateVoidField)
    return HandleCandidateField(STI, GEPI);
  if (isa<Function>(AV))
    return HandleCandidateRootFxnPtrField(STI, GEPI, Index);
  // No special rules for any other fields.
  return true;
}

//
// If 'SI' needs to be an alloc store to validate the removal of the bad
// casting, unsafe pointer store, and mismatched element access safety
// violations, check if it is already determined to be one, and if not,
// save it as a pending store that will be checked later.
//
void
DTransBadCastingAnalyzerOP::handlePotentialAllocStore(StoreInst *SI) {
  if (AllocStores.find(SI) == AllocStores.end())
    PendingStores.insert(SI);
}

//
// Return 'true' if 'CI' is an innocuous call, which is uses the load
// instruction 'LI' of a field indicated by 'GEPI'.
//
// A load is innocuous if its only uses are as an argument to a "free" like
// function or if it passed to as the VoidArgumentIndex of an indirect call
// whose function pointer comes from the same structure instance as that
// referenced by 'GEPI'.
//
// For example, in:
//    %26 = getelementptr inbounds %struct.lzma_next_coder_s, \
//      ptr %25, i64 0, i32 3
//    %27 = load ptr, ptr %26, align 8
//    ...
//    %112 = getelementptr inbounds %struct.lzma_next_coder_s, \
//      ptr %25, i64 0, i32 0
//    %113 = load ptr, ptr %112, align 8
//    %114 = getelementptr inbounds %struct.lzma_stream, \
//      ptr %0, i64 0, i32 6
//    ...
//    %120 = call i32 %27(ptr %113, ptr %115, ptr %6, \
//      ptr nonnull %3, i64 %117, ptr %14, ptr nonnull %4, i64 %119, \
//      i32 %1) #4
// %113 is an innocuous load of the indirect call type.
//
bool
DTransBadCastingAnalyzerOP::isInnocuousLoadOfCall(CallInst *CI, LoadInst *LI,
                                                  GetElementPtrInst *GEPI) {
  Function *F = CI->getCalledFunction();
  if (F) {
    if (PTA.getFreeCallKind(CI) == dtrans::FK_NotFree)
      return false;
  } else {
    auto LI2 = dyn_cast<LoadInst>(CI->getCalledOperand());
    if (!LI2)
      return false;
    if (CI->arg_size() < VoidArgumentIndex + 1 ||
        CI->getOperand(VoidArgumentIndex) != LI)
      return false;
    auto GEPI2 = dyn_cast<GetElementPtrInst>(LI2->getPointerOperand());
    if (!GEPI2 || (GEPI2->getPointerOperand() != GEPI->getPointerOperand()))
      return false;
  }
  return true;
}

//
// If there is one, return a StoreInst in 'BB' which stores the result
// of a recognized alloc function to the candidate field. Otherwise,
// return 'nullptr'. Also indicate that the StoreInst is a potential
// alloc store.
//
StoreInst *
DTransBadCastingAnalyzerOP::allocStoreInst(BasicBlock *BB) {
  StoreInst *RSI = nullptr;
  for (Instruction &I : *BB) {
    auto SI = dyn_cast<StoreInst>(&I);
    if (!SI)
      continue;
    auto GEPI = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
    if (!GEPI)
      continue;
    auto CI = dyn_cast<CallInst>(SI->getValueOperand());
    if (!CI)
      continue;
    if (gepiMatchesCandidateField(GEPI) &&
        PTA.getAllocationCallKind(CI) != dtrans::AK_NotAlloc) {
      RSI = SI;
      break;
    }
  }
  if (!RSI)
    return nullptr;
  handlePotentialAllocStore(RSI);
  return RSI;
}

//
// Return 'true' if 'BB' is either conditionally dead or is dominated
// by a potential alloc store. If it may be conditionally dead, set
// 'IsCondDead'.
//
bool
DTransBadCastingAnalyzerOP::condDeadOrAllocStoreDominated(BasicBlock *BB,
                                                          bool &IsCondDead) {
  std::function<bool(BasicBlock *, BasicBlock *, bool &,
                     SmallPtrSetImpl<BasicBlock *> &)>
      CondDeadOrAllocStoreBlock = [this, &CondDeadOrAllocStoreBlock](
                                      BasicBlock *PBB, BasicBlock *BB,
                                      bool &IsCondDead,
                                      SmallPtrSetImpl<BasicBlock *> &Visited) {
    if (Visited.count(PBB))
      return true; 
    Visited.insert(PBB);
    if (isSpecialGuardConditional(PBB) &&
        getNotTakenPathOfSpecialGuardConditional(PBB) == BB) {
      IsCondDead = true;
      return true;
    }
    if (allocStoreInst(BB))
      return true;
    for (BasicBlock *NPBB : predecessors(PBB))
      if (!CondDeadOrAllocStoreBlock(NPBB, BB, IsCondDead, Visited))
        return false;
    return true;
  };

  SmallPtrSet<BasicBlock *, 10> Visited;
  Visited.insert(BB);
  for (BasicBlock *PBB : predecessors(BB))
    if (!CondDeadOrAllocStoreBlock(PBB, BB, IsCondDead, Visited))
      return false;
  return true;
}


//
// Return 'true' if all of the uses of 'I' are in basic blocks that are
// either dominated by a potential alloc store or in a dead basic blcck
// when the special condition is applied to the Function in which
// 'I' appears.
//
// For example, consider:
//
// ; <label>:14:                                     ; preds = %12
//  %15 = getelementptr inbounds %struct.lzma_next_coder_s, \
//    ptr %0, i64 0, i32 0
//  %16 = load ptr, ptr %15, align 8
//  %17 = icmp eq ptr %16, null
//  br i1 %17, label %22, label %19
//
// ; <label>:19:                                     ; preds = %14
//  %21 = getelementptr inbounds %struct.lzma_coder_s.92, \
//    ptr %16, i64 0, i32 6
//  br label %49
//
//; <label>:22:                                     ; preds = %14
//  %23 = tail call fastcc ptr @lzma_alloc(i64 1472) #4
//  store ptr %23, ptr %15, align 8, !tbaa !34
//  %24 = icmp eq ptr %23, null
//  br i1 %24, label %78, label %26
//
//; <label>:26:                                     ; preds = %22
//...
//  br label %49
//
//; <label>:49:                                     ; preds = %26, %19
//  %50 = phi ptr [ %21, %19 ], [ %48, %26 ]
//  %51 = phi ptr [ %0, %19 ], [ %30, %26 ]
//  %52 = phi ptr [ %16, %19 ], [ %47, %26 ]
//
// Let 'I' be %16. It has uses in basic blocks <label>:19 and <label>:49.
//
// Note that <label>:14 is a special guard conditional basic block. Its
// true branch targets basic block <label>:22, which contains an alloc
// store. Its false branch targets basic block <label>:19 which is
// therefore conditionally dead.
//
bool
DTransBadCastingAnalyzerOP::allUseBBsCondDeadOrAllocStoreDominated(
    Instruction *I, bool &IsCondDead) {
  for (auto *U : I->users()) {
    auto II = dyn_cast<Instruction>(U);
    if (!II)
      continue;
    auto BB = II->getParent();
    if (!condDeadOrAllocStoreDominated(BB, IsCondDead))
      return false;
  }
  return true;
}

//
// Return 'true' if 'LDI' is a valid zero element ptr access load for
// bad casting analysis. This is a special check needed when we have
// opaque pointers. 'LDI' is valid if the type accessed is consistent
// in all of its uses, as determined by the source type if the use is a
// GEP or by the uses within called function if the use is an operand
// of a CallBase.
//
bool
DTransBadCastingAnalyzerOP::isValidZeroElementPtrAccess(LoadInst *LDI) {
  auto GEPI = cast<GetElementPtrInst>(LDI->getPointerOperand());
  StoreInst *SI = nullptr;
  for (User *U : GEPI->users())
    if (auto LSI = dyn_cast<StoreInst>(U)) {
      if (SI || LSI->getPointerOperand() != GEPI)
        return false;
      SI = LSI;
    }
  if (!SI)
    return false;
  handlePotentialAllocStore(SI);
  llvm::Type *GEPIType = nullptr;
  for (User *U : SI->getValueOperand()->users()) {
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      GEPIType = GEPI->getSourceElementType();
      break;
    }
  }
  if (!GEPIType)
    return false;
  for (User *U : LDI->users()) {
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      if (GEPI->getSourceElementType() != GEPIType)
        return false;
    } else if (auto CB = dyn_cast<CallBase>(U)) {
      Function *Callee = CB->getCalledFunction();
      if (!Callee)
        return false;
      bool FoundUniqueIndex = false;
     unsigned UniqueIndex = 0;
      for (unsigned I = 0; I < CB->arg_size(); ++I)
        if (CB->getArgOperand(I) == LDI) {
          if (FoundUniqueIndex)
            return false;
          FoundUniqueIndex = true;
          UniqueIndex = I;
        }
      if (!FoundUniqueIndex || UniqueIndex != VoidArgumentIndex)
        return false;
      auto Result = findSpecificArgType(Callee, VoidArgumentIndex);
      if (!Result.first || Result.second != GEPIType)
        return false;
    } else {
      return false;
    }
  }
  return true;
}

//
// Analyze the load instruction 'I' to the field 'FI' for special bad
// casting analysis. In particular, we are analyzing loads to the
// candidate field. The users of the loads of the candidate field
// must be alloc stores or innocuous loads.
//
// This function will return 'false' if the analysis must terminate
// because a violation (bad casting, unsafe pointer store, or mismatched
// element access) is found. In this case, setFoundViolation() is called to
// record that the violation has been observed.  Otherwise, we return 'true'
// and continue with the analysis.
//
// For example, in:
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     ptr %0, i64 0, i32 0
//   %6 = load ptr, ptr %5, align 8
//   %7 = icmp eq ptr %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8: ; preds = %3
//   %9 = tail call fastcc ptr @lzma_alloc(i64 336) #4
//   store ptr %9, ptr %5, align 8
// The load above is one of the kind of loads we are analyzing here. The
// other type is the innocuous load, for which an example appears in the
// relevant function.
//
bool
DTransBadCastingAnalyzerOP::analyzeLoad(dtrans::FieldInfo &FI,
                                        Instruction &I) {
  if (foundViolation())
    return false;
  auto LDI = cast<LoadInst>(&I);
  auto GEPI = dyn_cast<GetElementPtrInst>(LDI->getPointerOperand());
  if (!GEPI)
    return true;
  if (!gepiMatchesCandidateStruct(GEPI))
    return true;
  // Find the field being loaded
  auto ConstIndex = GEPI->getOperand(GEPI->getNumOperands() - 1);
  auto *LastArg = cast<ConstantInt>(ConstIndex);
  uint64_t Index = LastArg->getLimitedValue();
  // Only care about the candidate field.
  if (Index != CandidateVoidField)
    return true;
  if (isValidZeroElementPtrAccess(LDI))
    return true;
  // Check the uses of the load.
  for (auto *U : LDI->users()) {
    // If it is a store, it should be an alloc store.
    auto SI = dyn_cast<StoreInst>(U);
    if (SI) {
      handlePotentialAllocStore(SI);
      continue;
    }
    // If it is a call, 'LDI' should be an innocuous load of a call.
    auto CI = dyn_cast<CallInst>(U);
    if (CI) {
      if (isInnocuousLoadOfCall(CI, LDI, GEPI))
        continue;
      DEBUG_WITH_TYPE(DTRANS_BCA, {
        dbgs() << "dtrans-bca: (L) Unmatched call: ";
        CI->dump();
      });
      setFoundViolation(true);
      return false;
    }
    // If it is an ICmp, if should be used to test the candidate load
    // against nullptr.  If it does, record that the function in which
    // it appears should get the special conditional expression test
    // inserted.
    auto ICI = dyn_cast<ICmpInst>(U);
    if (ICI) {
      if (isSpecialGuardConditional(ICI->getParent())) {
        CondLoadFunctions.insert(ICI->getFunction());
        continue;
      }
      DEBUG_WITH_TYPE(DTRANS_BCA, {
        dbgs() << "dtrans-bca: (L) ICmp not in spec guard cond basic block: ";
        ICI->dump();
      });
      setFoundViolation(true);
      return false;
    }
    // If it is a PHINode, check each incoming value which is the load being
    // analyzed, and see if its incoming basic block will be determined to be
    // dead if the special conditional expression test is added, or if it is
    // dominated by a potential alloc store. If the former, this is OK, but
    // record that the function the load appears in needs the conditional test.
    if (auto PHIN = dyn_cast<PHINode>(U)) {
      for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I !=E; ++I) {
        if (PHIN->getIncomingValue(I) == LDI) {
          auto BBP = PHIN->getIncomingBlock(I);
          bool IsCondDead = false;
          if (condDeadOrAllocStoreDominated(BBP, IsCondDead)) {
            if (IsCondDead)
              CondLoadFunctions.insert(PHIN->getFunction());
          } else {
            DEBUG_WITH_TYPE(DTRANS_BCA, {
              dbgs() << "dtrans-bca: (L) Not conditionally dead: ";
              ICI->dump();
            });
            setFoundViolation(true);
            return false;
          }
        }
      }
      continue;
    }
    // If it is an instruction in a basic block that will be determined to be
    // dead if the special conditional expression test is added, this is OK,
    // but record that the function it appears in needs the conditional test.
    auto I = cast<Instruction>(U);
    auto BB = I->getParent();
    auto BBP = BB->getUniquePredecessor();
    if (BBP && isSpecialGuardConditional(BBP) &&
        getNotTakenPathOfSpecialGuardConditional(BBP) == BB) {
      CondLoadFunctions.insert(I->getFunction());
      continue;
    }
    // Now check all of the uses of the instruction to ensure that they
    // are either conditionally dead or dominated by a potential alloc store.
    bool IsCondDead = false;
    if (allUseBBsCondDeadOrAllocStoreDominated(I, IsCondDead)) {
      if (IsCondDead)
        CondLoadFunctions.insert(I->getFunction());
      continue;
    }
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: (L) Not conditionally dead: ";
      I->dump();
    });
    setFoundViolation(true);
    return false;
  }
  return true;
}

//
// Remove 'F' from the set 'CondLoadFunctions' if there is an alloc store

//
void
DTransBadCastingAnalyzerOP::pruneCondLoadFunctions() {
  for (auto &IT : AllocStores) {
    Function *MyF = IT.first->getFunction();
    CondLoadFunctions.remove_if([&](Function *F) { return F == MyF; });
  }
}

//
// Return 'true' if the safety check violations noted can be removed by
// inserting appropriate conditional tests of the candidate field against
// nullptr.  Return 'false' if no such conditionals must be inserted.
//
bool
DTransBadCastingAnalyzerOP::violationIsConditional() {
  bool SawConditional = false;
  for (auto &IT : AllocStores) {
    bool IsConditional = IT.second.first;
    if (IsConditional) {
      SawConditional = true;
      DEBUG_WITH_TYPE(DTRANS_BCA, {
        dbgs() << "dtrans-bca: (CS) " << IT.first->getFunction()->getName();
        dbgs() << " " << IT.second.second->getStructName() << "\n";
        IT.first->dump();
      });
    }
  }
  return SawConditional;
}

//
// Walk through all of the recorded types of interest, and if a type has
// safety violation 'SC1', remove safety violation 'SC2' (if it is set), and
// set safety violation 'SC3'.
//
void
DTransBadCastingAnalyzerOP::applySafetyCheckToCandidate(
    dtrans::SafetyData FindCondition, dtrans::SafetyData RemoveCondition,
    dtrans::SafetyData ReplaceByCondition) {
  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    // Unhandled use is already worst case
    if (TI->testSafetyData(FindCondition) &&
        !TI->testSafetyData(dtrans::UnhandledUse)) {
      TI->resetSafetyData(RemoveCondition);
      TI->setSafetyData(ReplaceByCondition);
    }
  }
}

//
// Post processing work for the bad casting analysis. We will declare a
// safety violation if:
//   (1) There are still pending stores which have not been determined
//       to be alloc stores.
//   (2) There are conditional load functions which do not have an alloc store.
// During the visiting of loads and stores, we mark types affected by
// potential bad casting, unsafe pointer stores, and mismatched element
// accesses with BadCastingPending, UnsafePointerStorePending, and
// MismatchedElementAccessPending safety violations. One of three things
// will happen with these:
//   (1) If there is no way to remove the safety violation, these will
//       be converted to BadCasting, UnsafePointerStore, and
//       MismatchedElementAccess.
//   (2) If the safety violations can be removed by adding conditionals,
//       these safety violations are converted to BadCastingConditional,
//       UnsafePointerStoreConditional, and MismatchedElementAccessConditional.
//   (3) If adding conditionals is not necessary, these pending safety
//       simply removed.
// After this, any type with both BadCasting and BadCastingConditional has
// BadCastingConditional removed. UnsafePointerStore and
// MismatchedElementAccess are treated similarly.
//
bool
DTransBadCastingAnalyzerOP::analyzeAfterVisit() {
  // Lambda to convert pending safety violations to unconditional
  auto convertPendingToUnconditional = [this]() -> void {
    applySafetyCheckToCandidate(dtrans::BadCastingPending,
                                dtrans::BadCastingPending, dtrans::BadCasting);
    applySafetyCheckToCandidate(dtrans::UnsafePointerStorePending,
                                dtrans::UnsafePointerStorePending,
                                dtrans::UnsafePointerStore);
    applySafetyCheckToCandidate(dtrans::MismatchedElementAccessPending,
                                dtrans::MismatchedElementAccessPending,
                                dtrans::MismatchedElementAccess);
  };
  // If we saw a violation that could not be removed by conditionalization,
  // give up now.
  if (foundViolation()) {
    convertPendingToUnconditional();
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: Found safety violation\n"
             << "dtrans-bca: End bad casting analysis: (NOT OK)\n";
    });
    return false;
  }
  // If there are still pending stores, give up now.
  if (!PendingStores.empty()) {
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      for (auto *SI : PendingStores) {
        dbgs() << "dtrans-bca: Pending Store in ";
        SI->getFunction()->printAsOperand(dbgs());
        dbgs() << "\n";
        SI->dump();
      }
    });
    convertPendingToUnconditional();
    setFoundViolation(true);
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: End bad casting analysis: (NOT OK)\n";
    });
    return false;
  }
  // Remove conditional load functions in which an alloc store appear.
  // If there are any remaining, give up now.
  pruneCondLoadFunctions();
  if (!CondLoadFunctions.empty()) {
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      for (auto *F : CondLoadFunctions) {
        dbgs() << "dtrans-bca: Conditional load in ";
        F->printAsOperand(dbgs());
        dbgs() << "\n";
      }
    });
    convertPendingToUnconditional();
    setFoundViolation(true);
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: End bad casting analysis: (NOT OK)\n";
    });
    return false;
  }
  // The remaining safety violations might be removable by inserting
  // conditionals.  If so, change them to conditional safety violations.
  if (violationIsConditional()) {
    applySafetyCheckToCandidate(dtrans::BadCastingPending,
                                dtrans::BadCastingPending,
                                dtrans::BadCastingConditional);
    applySafetyCheckToCandidate(dtrans::UnsafePointerStorePending,
                                dtrans::UnsafePointerStorePending,
                                dtrans::UnsafePointerStoreConditional);
    applySafetyCheckToCandidate(dtrans::MismatchedElementAccessPending,
                                dtrans::MismatchedElementAccessPending,
                                dtrans::MismatchedElementAccessConditional);
    applySafetyCheckToCandidate(
        dtrans::BadCasting, dtrans::BadCastingConditional, dtrans::NoIssues);
    applySafetyCheckToCandidate(dtrans::UnsafePointerStore,
                                dtrans::UnsafePointerStoreConditional,
                                dtrans::NoIssues);
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: Found conditional bad cast, unsafe "
             << "pointer store, or mismatched element access\n"
             << "dtrans-bca: End bad casting analysis: (OK)\n";
    });
    return true;
  }
  // Otherwise, simply remove the pending safety violations.
  applySafetyCheckToCandidate(dtrans::BadCastingPending,
                              dtrans::BadCastingPending, dtrans::NoIssues);
  applySafetyCheckToCandidate(dtrans::UnsafePointerStorePending,
                              dtrans::UnsafePointerStorePending,
                              dtrans::NoIssues);
  applySafetyCheckToCandidate(dtrans::MismatchedElementAccessPending,
                              dtrans::MismatchedElementAccessPending,
                              dtrans::NoIssues);
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Removed pending bad cast, unsafe"
           << " pointer stores, and mismatched element accesses\n"
           << "dtrans-bca: End bad casting analysis: (OK)\n";
  });
  return true;
}

void
DTransBadCastingAnalyzerOP::getConditionalFunctions(
    SetVector<Function *> &Funcs) const {
  Funcs.clear();
  for (auto *F : CondLoadFunctions)
    Funcs.insert(F);
  // If store is conditional, it requires a value validation (should be NULL).
  for (auto &Entry : AllocStores)
    if (Entry.second.first)
      Funcs.insert(Entry.first->getParent()->getParent());
}

bool
DTransBadCastingAnalyzerOP::isAllocStore(Instruction *I) {
  auto SI = dyn_cast<StoreInst>(I);
  return SI && AllocStores.find(SI) != AllocStores.end();
}

bool
DTransBadCastingAnalyzerOP::isCandidateLoad(Instruction *I) {
  auto LI = dyn_cast<LoadInst>(I);
  if (!LI)
    return false;
  auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEPI)
    return false;
  return gepiMatchesCandidateField(GEPI);
}

void
DTransBadCastingAnalyzerOP::setSawBadCasting(Instruction *I) {
   DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Bad casting (pending) -- "
           << "unknown pointer virtually cast to type of interest:\n"
           << "  " << *I << "\n";
  });
} 

void
DTransBadCastingAnalyzerOP::setSawUnsafePointerStore(Instruction *I) {
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Unsafe pointer store (pending) -- "
           << "unmatched store of aliased value:\n"
           << "  " << *I << "\n";
  });
} 

void
DTransBadCastingAnalyzerOP::setSawMismatchedElementAccess(Instruction *I) {
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Mismatched element access (pending) -- "
           << "incompatible type for field load/store value:\n"
           << "  " << *I << "\n";
  });
} 


