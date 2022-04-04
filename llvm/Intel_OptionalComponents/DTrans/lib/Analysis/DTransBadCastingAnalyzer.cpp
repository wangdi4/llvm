//===-----DTransBadCastingAnalyzer.cpp - Specialized bad casting analyzer--===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransBadCastingAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransAllocAnalyzer.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"

#define DEBUG_TYPE "dtransanalysis"

using namespace llvm;
using namespace dtrans;

// Debug type for verbose bad casting analysis output.
#define DTRANS_BCA "dtrans-bca"

//
// Returns the Type of the structure referenced by the 'GEPI'.
//
// For example, in:
//   %8 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     %struct.lzma_next_coder_s* %0, i64 0, i32 2
// where:
//   %struct.lzma_next_coder_s = type { i8*, i64, i64, ..
// the last type is %struct.lzma_next_coder_s.
//
// The function is more interesting and useful when a GEP with more than
// 2 indices is specified. For example:
//   %30 = getelementptr inbounds %struct.lzma_coder_s.187, \
//     %struct.lzma_coder_s.187* %28, i64 0, i32 2, i32 0
// where:
//   %struct.lzma_coder_s.187 = type { %struct.lzma_dict, \
//     %struct.lzma_lz_decoder, %struct.lzma_next_coder_s, i8, i8, \
//     %struct.anon.186 }
// and the last type is %struct.lzma_next_coder_s.
//
llvm::Type *DTransBadCastingAnalyzer::getLastType(GetElementPtrInst *GEPI) {
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
// which are function pointers.
//
bool DTransBadCastingAnalyzer::analyzeBeforeVisit() {
  DEBUG_WITH_TYPE(DTRANS_BCA,
                  { dbgs() << "dtrans-bca: Begin bad casting analysis\n"; });
  unsigned BestCount = 0;
  for (StructType *Ty : M.getIdentifiedStructTypes()) {
    unsigned E = Ty->getNumElements();
    if (E <= CandidateVoidField)
      continue;
    if (Ty->getElementType(CandidateVoidField) != Int8PtrTy)
      continue;
    unsigned Count = 0;
    for (unsigned I = 0; I != E; ++I) {
      if (I == CandidateVoidField)
        continue;
      llvm::Type *FieldTy = Ty->getElementType(I);
      if (FieldTy->isPointerTy() &&
          FieldTy->getPointerElementType()->isFunctionTy())
        ++Count;
    }
    if (Count > BestCount) {
      CandidateRootType = Ty;
      BestCount = Count;
    }
  }
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Candidate Root Type: ";
    dbgs() << (CandidateRootType ? dtrans::getStructName(CandidateRootType)
                                 : "<<NONE>>")
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
//   define internal void @lz_decoder_end(i8*, %struct.lzma_allocator*) #7 {
//     %3 = bitcast i8* %0 to %struct.lzma_coder_s.187*
// The function lz_decoder_end has the argument at index 0 cast to
//   %struct.lzma_coder_s.187*
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzer::findSpecificArgType(Function *F, unsigned Index) {
  if (Index > F->arg_size() - 1)
    return std::make_pair(false, nullptr);
  Argument *ArgIndex = F->arg_begin() + Index;
  unsigned UserCount = 0;
  llvm::Type *ResultType = nullptr;
  for (auto *U : ArgIndex->users()) {
    // Give up if there are more than 2 uses. It's possible this could
    // be generalized, but no need to do that unless it is necessary.
    if (UserCount > 2)
      return std::make_pair(false, nullptr);
    // Look for a bitcast to a specific type.  This will be the result
    // type if it is unique.
    auto BC = dyn_cast<BitCastInst>(U);
    if (BC) {
      if (ResultType)
        return std::make_pair(false, nullptr);
      ResultType = BC->getDestTy();
      UserCount++;
      continue;
    }
    // Tolerate use in a call to a free-like function.
    auto CI = dyn_cast<CallInst>(U);
    if (CI) {
      const TargetLibraryInfo &TLI = GetTLI(*CI->getFunction());
      if (!dtrans::isFreeFn(CI, TLI) && !DTAA.isFreePostDom(CI))
        return std::make_pair(false, nullptr);
      UserCount++;
      continue;
    }
    return std::make_pair(false, nullptr);
  }
  // Jumped through all of the hoops. Return the ResultType.
  return std::make_pair(true, ResultType);
}

//
// Find the single bit cast instruction that is used to cast the value
// assigned to the store instruction 'STI'.  Return nullptr if there is none.
//
// For example, in:
//   %18 = tail call fastcc i8* @lzma_alloc(i64 %17) #4
//   store i8* %18, i8** %11, align 8, !tbaa !34
//   %19 = icmp eq i8* %18, null
//   %20 = bitcast i8* %18 to %struct.lzma_coder_s.260*
// we are supplying the store as an argument and expecting to get %20.
//
BitCastInst *DTransBadCastingAnalyzer::findSingleBitCastAlloc(StoreInst *STI) {
  auto CI = dyn_cast<CallInst>(STI->getValueOperand());
  if (!CI)
    return nullptr;
  BitCastInst *RBC = nullptr;
  const TargetLibraryInfo &TLI = GetTLI(*CI->getFunction());
  if (dtrans::getAllocFnKind(CI, TLI) == dtrans::AK_NotAlloc &&
      !DTAA.isMallocPostDom(CI))
    return nullptr;
  unsigned UserCount = 0;
  for (auto *U : CI->users()) {
    // Don't expect more than three users. This can be generalized if
    // necessary.
    if (UserCount > 3)
      return nullptr;
    // The store should be one of the uses.
    if (U == STI) {
      UserCount++;
      continue;
    }
    // One of the uses can be an optional test against a constant null pointer.
    // This indicates that the allocation can be conditional.
    auto CmpI = dyn_cast<ICmpInst>(U);
    if (CmpI) {
      auto CT = U->getOperand(0) == CI ? U->getOperand(1) : U->getOperand(0);
      if (!isa<ConstantPointerNull>(CT))
        return nullptr;
      UserCount++;
      continue;
    }
    // One use should bit cast the pointer to the allocated memory to the
    // result type.
    auto BC = dyn_cast<BitCastInst>(U);
    if (!BC) {
      // Tolerate a single PHINode with one use, which we can skip past
      // to get to the bit cast.
      auto PHI = dyn_cast<PHINode>(U);
      if (PHI && PHI->hasOneUse())
        BC = dyn_cast<BitCastInst>(*(PHI->user_begin()));
    }
    if (BC) {
      if (RBC)
        return nullptr;
      RBC = BC;
      UserCount++;
      continue;
    }

    return nullptr;
  }
  return RBC;
}

//
// Return the Type to which 'TI' is bit cast, if it is store to a field
// of a structure type specified by 'GEPI'.  If there is no such unique
// type, return nullptr.
//
// For example, in:
//   %11 = getelementptr inbounds %struct.lzma_next_coder_s, \
//       %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %18 = tail call fastcc i8* @lzma_alloc(i64 %17) #4
//   store i8* %18, i8** %11, align 8, !tbaa !34
//   %19 = icmp eq i8* %18, null
//   %20 = bitcast i8* %18 to %struct.lzma_coder_s.260*
// If the GetElementPtrInst in %11 matches 'GEPI' and 'TI' is the indicated
// store, we return the type %struct.lzma_coder_s.260*.
//
llvm::Type *DTransBadCastingAnalyzer::foundStoreType(Instruction *TI,
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
  // Expect it to be stored from a malloc like function through a single
  // bit cast.
  auto BC = findSingleBitCastAlloc(STI);
  if (!BC)
    return nullptr;
  return BC->getDestTy();
}

//
// Starting with instruction before 'TI', walk backward to find a store
// instruction which stores to the structure indicated by 'GEPI', and
// return the Type to which that store instruction is bit cast. If no
// such store is found, return nullptr.
//
llvm::Type *
DTransBadCastingAnalyzer::findStoreTypeBack(Instruction *TI,
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
//       %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %12 = load i8*, i8** %11, align 8, !tbaa !34
//   %13 = icmp eq i8* %12, null
//   %14 = bitcast i8* %12 to %struct.lzma_coder_s.260*
//   br i1 %13, label %15, label %44
// we return the GEPI in %11.
//
GetElementPtrInst *
DTransBadCastingAnalyzer::getRootGEPIFromConditional(BasicBlock *BB) {
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
// Return 'true' if the 'GEPI' is accessing the candidate field.
//
bool DTransBadCastingAnalyzer::gepiMatchesCandidate(GetElementPtrInst *GEPI) {
  llvm::Type *IndexedTy = getLastType(GEPI);
  auto IndexedStructTy = dyn_cast<StructType>(IndexedTy);
  if (!IndexedStructTy)
    return false;
  if (IndexedStructTy != CandidateRootType)
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
//       %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %12 = load i8*, i8** %11, align 8, !tbaa !34
//   %13 = icmp eq i8* %12, null
//   %14 = bitcast i8* %12 to %struct.lzma_coder_s.260*
//   br i1 %13, label %15, label %44
// and the basic block returned will be the one starting with label 15.
//
BasicBlock *DTransBadCastingAnalyzer::getTakenPathOfSpecialGuardConditional(
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
//       %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %12 = load i8*, i8** %11, align 8, !tbaa !34
//   %13 = icmp eq i8* %12, null
//   %14 = bitcast i8* %12 to %struct.lzma_coder_s.260*
//   br i1 %13, label %15, label %44
// and the basic block returned will be the one starting with label 44.
//
BasicBlock *DTransBadCastingAnalyzer::getNotTakenPathOfSpecialGuardConditional(
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
bool DTransBadCastingAnalyzer::isSpecialGuardConditional(BasicBlock *BB) {
  GetElementPtrInst *GEPI = getRootGEPIFromConditional(BB);
  if (!GEPI)
    return false;
  return gepiMatchesCandidate(GEPI);
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
DTransBadCastingAnalyzer::getStoreForwardAltNextBB(BasicBlock *BB,
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
// of which is the type to which the store is bit cast. In this case, the
// first element will be a bool which indicates whether the search required
// taking the 'true' path of a conditional which tests the candidate
// field against nullptr.  If no such store instruction is found, return
// std::make_pair(false, nullptr).
//
// For example, in:
//  %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     %struct.lzma_next_coder_s* %0, i64 0, i32 7
//  store i32 (i8*, %struct.lzma_allocator*, %struct.lzma_filter*, \
//     %struct.lzma_filter*)* @delta_encoder_update, i32 (i8*, \
//     %struct.lzma_allocator*, %struct.lzma_filter*, \
//     %struct.lzma_filter*)** %5, align 8, !tbaa !37
//  %6 = tail call fastcc i32 @lzma_delta_coder_init( \
//     %struct.lzma_next_coder_s* %0, %struct.lzma_allocator* %1, \
//     %struct.lzma_filter_info_s* %2) #4
// We would walk forward through the GetElementPtrInst and StoreInst
// and get to the call.  At the call, we will start with the GEPI at %5
// with  pointer operand %0, and then continue through the CallInst %6
// through the zeroth argument of @lzma_delta_coder_init:
//   define internal fastcc i32 @lzma_delta_coder_init( \
//     %struct.lzma_next_coder_s* nocapture, %struct.lzma_allocator*, \
//     %struct.lzma_filter_info_s*) unnamed_addr #7 {
//   %4 = alloca { i32 (i8*, %struct.lzma_allocator*, i8*, i64*, i64, i8*, \
//     i64*, i64, i32)*, void (i8*, %struct.lzma_allocator*)*, i32 (i8*)*, \
//     i32 (i8*, i64*, i64*, i64)*, i32 (i8*, %struct.lzma_allocator*, \
//     %struct.lzma_filter*, %struct.lzma_filter*)* }, align 8
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %6 = load i8*, i8** %5, align 8, !tbaa !34
//   %7 = icmp eq i8* %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8:                                      ; preds = %3
//   %9 = tail call fastcc i8* @lzma_alloc(i64 336) #4
//   store i8* %9, i8** %5, align 8, !tbaa !34
//   %10 = icmp eq i8* %9, null
//   %11 = bitcast i8* %9 to %struct.lzma_coder_s.243*
// We would continue the search at %5 take the "true" conditional branch
// to %9, and find the type %struct.lzma_coder_s.243*. We would return
// std::make_pair(true, %struct.lzma_coder_s.243*).
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzer::findStoreTypeForwardCall(CallInst *CI,
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
// If one is found, return a pair, the second element of which is the type
// to which the store is bit cast. In this case, the first element will be
// a bool which indicates whether the search required taking the 'true' path
// of a conditional which tests the candidate field against nullptr.  If no
// such store instruction is found, return std::make_pair(false, nullptr).
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzer::findStoreTypeForward(Instruction *TI,
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
// forward, to find a store instruction which stores to the structure
// indicated by 'GEPI'.  If one is found, return a pair, the second element
// of which is the type to which the store is bit cast. In this case, the
// first element will be a bool which indicates whether the search required
// taking the 'true' path of a conditional which tests the candidate field
// against nullptr.  If no such store instruction is found, return
// std::make_pair(false, nullptr).
//
std::pair<bool, llvm::Type *>
DTransBadCastingAnalyzer::findStoreType(Instruction *TI,
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
//     %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %6 = load i8*, i8** %5, align 8, !tbaa !34
//   %7 = icmp eq i8* %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8:                                      ; preds = %3
//   %9 = tail call fastcc i8* @lzma_alloc(i64 336) #4
//   store i8* %9, i8** %5, align 8, !tbaa !34
// The basic block in <label>:8 is conditionally executed under a test of
// the candidate field against nullptr, and so we would return BCCondSpecial
// if it were passed to this function.
//

DTransBadCastingAnalyzer::BCCondType
DTransBadCastingAnalyzer::isConditionalBlock(BasicBlock *SBB) {
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
void DTransBadCastingAnalyzer::recordAllocStore(StoreInst *SI,
                                                llvm::Type *StType) {
  DTransBadCastingAnalyzer::BCCondType Result;
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
// because a violation (bad casting or unsafe pointer store) is found.
// In this case, setFoundViolation() is called to record that the violation
// has been observed.  Otherwise, we return 'true' and continue with the
// analysis.
//
// For example, we are looking to analyze stores like:
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %6 = load i8*, i8** %5, align 8, !tbaa !34
//   %7 = icmp eq i8* %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8:                                      ; preds = %3
//   %9 = tail call fastcc i8* @lzma_alloc(i64 336) #4
//   store i8* %9, i8** %5, align 8, !tbaa !34
// and like:
//   %13 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     %struct.lzma_next_coder_s* %0, i64 0, i32 4
//   store void (i8*, %struct.lzma_allocator*)* @delta_coder_end, \
//     void (i8*, %struct.lzma_allocator*)** %13, align 8, !tbaa !33
//
bool DTransBadCastingAnalyzer::analyzeStore(dtrans::FieldInfo &FI,
                                            Instruction &I) {

  // Lambda function which returns 'true' if 'V' is a GEPI of the
  // indicated 'TargetType'. 'Index' is used for printing the index of
  // the GEPI in traces.
  auto handleGEPI = [this](Value *V, llvm::Type *TargetType,
                           uint64_t Index) -> bool {
    auto SGEPI = dyn_cast<GetElementPtrInst>(V);
    if (!SGEPI) {
      DEBUG_WITH_TYPE(DTRANS_BCA, {
        dbgs() << "dtrans-bca: (SV) [" << Index;
        dbgs() << "] Non-GEP bitcast target:";
        V->dump();
      });
      setFoundViolation(true);
      return false;
    }
    assert(SGEPI->getSourceElementType() == TargetType);
    return true;
  };

  if (foundViolation())
    return false;
  // Examine stores only from a GEPI.
  auto STI = cast<StoreInst>(&I);
  auto GEPI = dyn_cast<GetElementPtrInst>(STI->getPointerOperand());
  if (!GEPI)
    return true;
  // Find the llvm::Type being indexed and see if it is a CandidateRootType.
  auto AV = STI->getValueOperand();
  llvm::Type *AVType = AV->getType();
  Type *IndexedTy = getLastType(GEPI);
  auto IndexedStructTy = dyn_cast<StructType>(IndexedTy);
  if (!IndexedStructTy)
    return false;
  if (IndexedTy != CandidateRootType)
    return true;
  // Find the field being stored
  auto ConstIndex = GEPI->getOperand(GEPI->getNumOperands() - 1);
  auto *LastArg = cast<ConstantInt>(ConstIndex);
  uint64_t Index = LastArg->getLimitedValue();
  if (Index == CandidateVoidField) {
    // Check if the store is from an allocation bit cast to a specific type.
    BitCastInst *BCI = findSingleBitCastAlloc(STI);
    if (BCI) {
      // The bit cast better be a pointer type, since it comes from an
      // allocation.
      llvm::Type *TargetType = BCI->getDestTy();
      assert(TargetType->isPointerTy());
      TargetType = TargetType->getPointerElementType();
      // Any uses should be GEPs from the same bit cast type.  This can
      // be generalized, but this is sufficient for now.
      for (auto *V : BCI->users()) {
        auto PHIN = dyn_cast<PHINode>(V);
        if (PHIN) {
          // A PHINode may join an access to stored value with other
          // "fake" sources that will never actually reach to the PHINode.
          // For now, it is enough to check that such a "fake" source
          // can only come from the special guard conditional basic block.
          // This can be generalized if it is found useful.
          for (unsigned I = 0; I < PHIN->getNumIncomingValues(); ++I) {
            BasicBlock *BB = PHIN->getIncomingBlock(I);
            if (!isSpecialGuardConditional(BB) && BB != STI->getParent()) {
              DEBUG_WITH_TYPE(DTRANS_BCA, {
                dbgs() << "dtrans-bca: (SV) [" << Index;
                dbgs() << "] Improper incoming block for PHI node:";
                V->dump();
              });
              setFoundViolation(true);
              return false;
            }
          }
          for (auto *VV : PHIN->users())
            if (!handleGEPI(VV, TargetType, Index))
              return false;
        } else if (!handleGEPI(V, TargetType, Index)) {
          return false;
        }
      }
      recordAllocStore(STI, TargetType);
      return true;
    }
    // We can also tolerate a store of a nullptr value.
    if (isa<ConstantPointerNull>(AV))
      return true;
    // But anything else is not permissible.
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: (SV) [" << Index;
      dbgs() << "] Store not from alloc or nullptr: ";
      I.dump();
      GEPI->dump();
      AV->dump();
    });
    setFoundViolation(true);
    return false;
  }
  if (!isa<ConstantPointerNull>(AV) && AVType->isPointerTy() &&
      AVType->getPointerElementType()->isFunctionTy()) {
    // For the case of a function pointer field being stored, ensure that the
    // VoidArgumentIndex of that function can be determined ...
    Function *F = cast<Function>(AV);
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
      auto StReturn = findStoreType(&I, GEPI);
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
  }
  // No special rules for any other fields.
  return true;
}

//
// If 'SI' needs to be an alloc store to validate the removal of the bad
// casting and unsafe pointer store safety violations, check if it is
// already determined to be one, and if not, save it as a pending store
// that will be checked later.
//
void DTransBadCastingAnalyzer::handlePotentialAllocStore(StoreInst *SI) {
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
//      %struct.lzma_next_coder_s* %25, i64 0, i32 3
//    %27 = load i32 (i8*, %struct.lzma_allocator*, i8*, i64*, i64, i8*, \
//      i64*, i64, i32)*, i32 (i8*, %struct.lzma_allocator*, i8*, i64*, i64, \
//      i8*, i64*, i64, i32)** %26, align 8, !tbaa !48
//    ...
//    %112 = getelementptr inbounds %struct.lzma_next_coder_s, \
//      %struct.lzma_next_coder_s* %25, i64 0, i32 0
//    %113 = load i8*, i8** %112, align 8, !tbaa !61
//    %114 = getelementptr inbounds %struct.lzma_stream, \
//      %struct.lzma_stream* %0, i64 0, i32 6
//    ...
//    %120 = call i32 %27(i8* %113, %struct.lzma_allocator* %115, i8* %6, \
//      i64* nonnull %3, i64 %117, i8* %14, i64* nonnull %4, i64 %119, \
//      i32 %1) #4
// %113 is an innocuous load of the indirect call type.
//
bool DTransBadCastingAnalyzer::isInnocuousLoadOfCall(CallInst *CI, LoadInst *LI,
                                                     GetElementPtrInst *GEPI) {
  Function *F = CI->getCalledFunction();
  if (F) {
    const TargetLibraryInfo &TLI = GetTLI(*CI->getFunction());
    if (!dtrans::isFreeFn(CI, TLI) && !DTAA.isFreePostDom(CI))
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
// Return 'true' if all of the uses of 'I' are in basic blocks that are
// either dominated by a potential alloc store or in a dead basic blcck
// when the special condition is applied to the Function in which
// 'I' appears.
//
// For example, consider:
//
// ; <label>:14:                                     ; preds = %12
//  %15 = getelementptr inbounds %struct.lzma_next_coder_s, \
//    %struct.lzma_next_coder_s* %0, i64 0, i32 0
//  %16 = load i8*, i8** %15, align 8, !tbaa !34
//  %17 = icmp eq i8* %16, null
//  %18 = bitcast i8* %16 to %struct.lzma_coder_s.92*
//  br i1 %17, label %22, label %19
//
// ; <label>:19:                                     ; preds = %14
//  %20 = bitcast %struct.lzma_next_coder_s* %0 to %struct.lzma_coder_s.92**
//  %21 = getelementptr inbounds %struct.lzma_coder_s.92, \
//    %struct.lzma_coder_s.92* %18, i64 0, i32 6
//  br label %49
//
//; <label>:22:                                     ; preds = %14
//  %23 = tail call fastcc i8* @lzma_alloc(i64 1472) #4
//  store i8* %23, i8** %15, align 8, !tbaa !34
//  %24 = icmp eq i8* %23, null
//  %25 = bitcast i8* %23 to %struct.lzma_coder_s.92*
//  br i1 %24, label %78, label %26
//
//; <label>:26:                                     ; preds = %22
//...
//  br label %49
//
//; <label>:49:                                     ; preds = %26, %19
//  %50 = phi %struct.lzma_index_s** [ %21, %19 ], [ %48, %26 ]
//  %51 = phi %struct.lzma_coder_s.92** [ %20, %19 ], [ %30, %26 ]
//  %52 = phi %struct.lzma_coder_s.92* [ %18, %19 ], [ %47, %26 ]
//
// Let 'I' be %18. It has uses in basic blocks <label>:19 and <label>:49.
//
// Note that <label>:14 is a special guard conditional basic block. Its
// true branch targets basic block <label>:22, which contains an alloc
// store. Its false branch targets basic block <label>:19 which is
// therefore conditionally dead.
//
bool DTransBadCastingAnalyzer::allUseBBsConditionallyDead(Instruction *I) {

  //
  // Return a StoreInst which is a potential alloc store, if the
  // alloc store dominates 'BB'.
  //
  auto dominatingAllocStore = [this](BasicBlock *BB) -> StoreInst * {
    for (auto PBB = BB; PBB; PBB = PBB->getUniquePredecessor()) {
      for (Instruction &I : *PBB) {
        auto SI = dyn_cast<StoreInst>(&I);
        if (!SI)
          continue;
        auto GEPI = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
        if (!GEPI)
          continue;
        auto CI = dyn_cast<CallInst>(SI->getValueOperand());
        if (!CI)
          continue;
        const TargetLibraryInfo &TLI = GetTLI(*CI->getFunction());
        if (gepiMatchesCandidate(GEPI) &&
            (dtrans::getAllocFnKind(CI, TLI) != dtrans::AK_NotAlloc ||
             DTAA.isMallocPostDom(CI)))
          return SI;
      }
    }
    return nullptr;
  };

  //
  // Return 'true' if 'BB', which is a predecessor of 'OBB', is dominated
  // by a conditionally dead basic block.
  //
  auto dominatedByConditionallyDeadBlock = [this](BasicBlock *BB,
                                                  BasicBlock *OBB) -> bool {
    auto TBB = OBB;
    for (auto PBB = BB; PBB; TBB = PBB, PBB = PBB->getUniquePredecessor())
      if (isSpecialGuardConditional(PBB) &&
          getNotTakenPathOfSpecialGuardConditional(PBB) == TBB)
        return true;
    return false;
  };

  for (auto *U : I->users()) {
    auto II = dyn_cast<Instruction>(U);
    if (!II)
      continue;
    auto BB = II->getParent();
    for (BasicBlock *PBB : predecessors(BB)) {
      if (!dominatedByConditionallyDeadBlock(PBB, BB)) {
        auto SI = dominatingAllocStore(PBB);
        if (!SI)
          return false;
        handlePotentialAllocStore(SI);
      }
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
// because a violation (bad casting or unsafe pointer store) is found.
// In this case, setFoundViolation() is called to record that the violation
// has been observed.  Otherwise, we return 'true' and continue with the
// analysis.
//
// For example, in:
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %6 = load i8*, i8** %5, align 8, !tbaa !34
//   %7 = icmp eq i8* %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8: ; preds = %3
//   %9 = tail call fastcc i8* @lzma_alloc(i64 336) #4
//   store i8* %9, i8** %5, align 8, !tbaa !34
// The load above is one of the kind of loads we are analyzing here. The
// other type is the innocuous load, for which an example appears in the
// relevant function.
//
bool DTransBadCastingAnalyzer::analyzeLoad(dtrans::FieldInfo &FI,
                                           Instruction &I) {
  if (foundViolation())
    return false;
  auto LDI = cast<LoadInst>(&I);
  auto GEPI = dyn_cast<GetElementPtrInst>(LDI->getPointerOperand());
  if (!GEPI)
    return true;
  Type *IndexedTy = getLastType(GEPI);
  auto IndexedStructTy = dyn_cast<StructType>(IndexedTy);
  if (!IndexedStructTy)
    return false;
  if (IndexedTy != CandidateRootType)
    return true;
  // Find the field being loaded
  auto ConstIndex = GEPI->getOperand(GEPI->getNumOperands() - 1);
  auto *LastArg = cast<ConstantInt>(ConstIndex);
  uint64_t Index = LastArg->getLimitedValue();
  // Only care about the candidate field.
  if (Index != CandidateVoidField)
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
    // are dominated, at least conditionally, by a potential alloc store.
    if (allUseBBsConditionallyDead(I)) {
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
// in the Function 'F'.
//
void DTransBadCastingAnalyzer::pruneCondLoadFunctions() {
  for (auto &IT : AllocStores) {
    Function *MyF = IT.first->getFunction();
    CondLoadFunctions.remove_if([&](Function *F) { return F == MyF; });
  }
}

//
// If any of the potentially bad cast instructions are from anything other
// than an alloc store, record a safety violation.
//
void DTransBadCastingAnalyzer::processPotentialBitCastsOfAllocStores() {
  for (auto *BCI : BadCastOperators) {
    auto SI = dyn_cast<StoreInst>(BCI->getOperand(0));
    if (SI && (AllocStores.find(SI) != AllocStores.end())) {
      setFoundViolation(true);
      return;
    }
  }
}

//
// If any potential unsafe pointer store is not an alloc store to the
// indicated alias type, record a safety violation.
//
void DTransBadCastingAnalyzer::processPotentialUnsafePointerStores() {
  for (auto &IT : UnsafePtrStores) {
    StoreInst *SI = IT.first;
    llvm::Type *StPtrType = IT.second;
    if (!StPtrType->isPointerTy()) {
      setFoundViolation(true);
      return;
    }
    llvm::Type *StType = StPtrType->getPointerElementType();
    auto ASIT = AllocStores.find(SI);
    if (ASIT == AllocStores.end() || ASIT->second.second != StType) {
      setFoundViolation(true);
      return;
    }
  }
}

//
// Return 'true' if the safety check violations noted can be removed by
// inserting appropriate conditional tests of the candidate field against
// nullptr.  Return 'false' if no such conditionals must be inserted.
//
bool DTransBadCastingAnalyzer::violationIsConditional() {
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
void DTransBadCastingAnalyzer::applySafetyCheckToCandidate(
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
//   (2) Any potentially problematic bit cast and store instructions are
//       not identified as alloc stores.
// During the visiting of loads and stores, we mark types affected by
// potential bad casting and unsafe pointer stores with BadCastingPending
// and UnsafePointerStorePending safety violations. One of three things
// will happen with these:
//   (1) If there is no way to remove the safety violation, these will
//       be converted to BadCasting and UnsafePointerStore.
//   (2) If the safety violations can be removed by adding conditionals,
//       these safety violations are converted to BadCastingConditional
//       and UnsafePointerStoreConditional.
//   (3) If adding conditionals is not necessary, these pending safety
//       simply removed.
// After this, any type with both BadCasting and BadCastingConditional has
// BadCastingConditional removed.  Similarly, if any type has both
// UnsafePointerStore and UnsafePointerStoreConditional,
// UnsafePointerStoreConditional is removed.
//
bool DTransBadCastingAnalyzer::analyzeAfterVisit() {
  // Lambda to convert pending safety violations to unconditional
  auto convertPendingToUnconditional = [this]() -> void {
    applySafetyCheckToCandidate(dtrans::BadCastingPending,
                                dtrans::BadCastingPending, dtrans::BadCasting);
    applySafetyCheckToCandidate(dtrans::UnsafePointerStorePending,
                                dtrans::UnsafePointerStorePending,
                                dtrans::UnsafePointerStore);
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
  // Check if there are potential bit casts and stores that may be problematic.
  // If there are any, call setFoundViolation(true), and change pending
  // safety violations into unconditional ones.
  processPotentialBitCastsOfAllocStores();
  processPotentialUnsafePointerStores();
  if (foundViolation()) {
    convertPendingToUnconditional();
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: Found unconditional bad cast or unsafe "
             << "pointer store\n"
             << "dtrans-bca: End bad casting analysis: (NOT OK)\n";
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
    applySafetyCheckToCandidate(
        dtrans::BadCasting, dtrans::BadCastingConditional, dtrans::NoIssues);
    applySafetyCheckToCandidate(dtrans::UnsafePointerStore,
                                dtrans::UnsafePointerStoreConditional,
                                dtrans::NoIssues);
    DEBUG_WITH_TYPE(DTRANS_BCA, {
      dbgs() << "dtrans-bca: Found conditional bad cast or unsafe "
             << "pointer store\n"
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
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Removed pending bad cast and unsafe"
           << " pointer stores\n"
           << "dtrans-bca: End bad casting analysis: (OK)\n";
  });
  return true;
}

//
// Return true if the 'Type' and 'Index' are for the BadCastingAnalysis
// root type and candidate field.
//
bool DTransBadCastingAnalyzer::isBadCastTypeAndFieldCandidate(llvm::Type *Type,
                                                              unsigned Index) {
  if (!Type->isPointerTy())
    return false;
  llvm::Type *PTy = Type->getPointerElementType();
  auto StPTy = dyn_cast<StructType>(PTy);
  if (StPTy != CandidateRootType)
    return false;
  return Index == CandidateVoidField;
}

//
// Return 'true' if 'BCI' is a bit cast of store instruction which stores
// into the candidate field.
//
// For example, in:
//   %5 = getelementptr inbounds %struct.lzma_next_coder_s, \
//     %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %6 = load i8*, i8** %5, align 8, !tbaa !34
//   %7 = icmp eq i8* %6, null
//   br i1 %7, label %8, label %20
//   ; <label>:8: ; preds = %3
//   %9 = tail call fastcc i8* @lzma_alloc(i64 336) #4
//   store i8* %9, i8** %5, align 8, !tbaa !34
//   %10 = icmp eq i8* %9, null
//   %11 = bitcast i8* %9 to %struct.lzma_coder_s.243*
// %11 is a bit cast of the desired form.
//
bool DTransBadCastingAnalyzer::isPotentialBitCastOfAllocStore(
    BitCastOperator *BCI) {
  auto CI = dyn_cast<CallInst>(BCI->getOperand(0));
  if (!CI)
    return false;
  StoreInst *SI = nullptr;
  for (User *U : CI->users())
    if (auto LSI = dyn_cast<StoreInst>(U))
      if (LSI->getValueOperand() == CI)
        if (!SI)
          SI = LSI;
        else
          return false;
  if (!SI)
    return false;
  const TargetLibraryInfo &TLI = GetTLI(*CI->getFunction());
  if (dtrans::getAllocFnKind(CI, TLI) == dtrans::AK_NotAlloc &&
      !DTAA.isMallocPostDom(CI))
    return false;
  auto GEPI = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
  if (!GEPI)
    return false;
  return gepiMatchesCandidate(GEPI);
}

//
// Return true if 'BCI' is a bit cast of a load of the candidate field.
//
// For example, in:
//   %23 = getelementptr inbounds %struct.lzma_next_coder_s,
//     %struct.lzma_next_coder_s* %0, i64 0, i32 0
//   %24 = load i8*, i8** %23, align 8, !tbaa !34
//   %25 = icmp eq i8* %24, null
//   br i1 %25, label %29, label %26
//   ; <label>:26: ; preds = %22
//   %27 = bitcast i8* %24 to %struct.lzma_coder_s.39*
// %27 is a bit cast of the desired form.
//
bool DTransBadCastingAnalyzer::isBitCastFromBadCastCandidate(
    BitCastOperator *BCI) {
  auto BCV = BCI->getOperand(0);
  if (BCV->getType() != Int8PtrTy)
    return false;
  auto LI = dyn_cast<LoadInst>(BCV);
  if (!LI)
    return false;
  auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEPI)
    return false;
  return gepiMatchesCandidate(GEPI);
}

//
// Add the instruction 'I' to the list of bad cast instructions which
// could lead to a bad casting safety violation.
//
void DTransBadCastingAnalyzer::setSawBadCastBitCast(BitCastOperator *I) {
  BadCastOperators.insert(I);
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Bad casting (pending) -- "
           << "unknown pointer cast to type of interest:\n"
           << "  " << *I << "\n";
  });
}

//
// Add the instruction 'SI' on the map of unsafe pointer stores and the
// the corresponding alias types which could lead to an unsafe pointer
// store safety violation.
//
void DTransBadCastingAnalyzer::setSawUnsafePointerStore(StoreInst *SI,
                                                        llvm::Type *AliasType) {
  UnsafePtrStores.insert(std::make_pair(SI, AliasType));
  DEBUG_WITH_TYPE(DTRANS_BCA, {
    dbgs() << "dtrans-bca: Unsafe pointer store -- "
           << " (pending) "
           << "  Unmatch store of aliased value:\n";
  });
}

void DTransBadCastingAnalyzer::getConditionalFunctions(
    SetVector<Function *> &Funcs) const {
  Funcs.clear();

  for (auto *F : CondLoadFunctions) {
    Funcs.insert(F);
  }

  for (auto &Entry : AllocStores) {
    // If store is conditional, it requires a value validation (should be NULL).
    if (Entry.second.first) {
      Funcs.insert(Entry.first->getParent()->getParent());
    }
  }
}

void DTransBadCastingAnalyzer::noteUnsafeCastOfAliasedPtr(BitCastOperator *I) {
  DEBUG_WITH_TYPE(DTRANS_BCA, {
  dbgs() << "dtrans-bca: Bad casting -- "
         << "unsafe cast of aliased pointer:\n"
         << "  " << *I << "\n";
  });
}
