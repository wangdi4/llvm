//===- Intel_AggInlAA.cpp - Aggressive  Inlining AA                   -===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-------------------------------------------------------------------===//
//
// After doing aggressive Inlining, there is opportunity to disam
// pointers that are allocated using .malloc. calls even if those
// pointers are swapped in loops. This pass basically adds Meta data
// to show that the pointers are never aliased if it proves that
// the pointers point to different malloc addresses. 
//
// Ex: In the below IR example, this pass collects %84 and %92, which are
// base pointers, and are allocated using 'malloc' and proves that
// %1146 and %1147 (also %2432 and %2433) are swapped pointers and they
// never alias to each other. Then, it generates MetaData for load and
// store instructions using the following info:
//    %84 and %92 never overlap
//    %1146 and %1147 never overlap
//    %2432 and %2433 never overlap    
//
//   Example IR:
//        %77 = call noalias i8* @malloc(i64 1689600000) #4
//        %78 = ptrtoint i8* %77 to i64
//        %79 = icmp eq i8* %77, null
//        %83 = getelementptr inbounds i8, i8* %77, i64 12800000
//        %84 = ptrtoint i8* %83 to i64
//        ...
//        %85 = call noalias i8* @malloc(i64 1689600000) #4
//        %86 = ptrtoint i8* %85 to i64
//        %87 = icmp eq i8* %85, null
//        %91 = getelementptr inbounds i8, i8* %85, i64 12800000
//        %92 = ptrtoint i8* %91 to i64
//        ...
//        %1146 = phi i64 [ %84, %1143 ], [ %1147, %2428 ]
//        %1147 = phi i64 [ %92, %1143 ], [ %1146, %2428 ]
//        ...
//        %2432 = phi i64 [ %84, %1143 ], [ %2433, %3099 ]
//        %2433 = phi i64 [ %92, %1143 ], [ %2432, %3099 ]
//
//
// It uses alias.scope and no.alias metadata to mark pointers
// as never aliased like below:
//
// Before:
//  store double 0x3, %111, align 8, !tbaa !10
//  store double 0x3, %176, align 8, !tbaa !10
//
// After:
//  store double 0x3, %111, align 8, !tbaa !10, !alias.scope !12, !noalias !14
//  store double 0x3, %176, align 8, !tbaa !10, !alias.scope !14, !noalias !12
//
//
//===-------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_AggInlAA.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_AggInline.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Pass.h"


using namespace llvm;

#define DEBUG_TYPE "intel-agg-inl-aa"

// Map between pointers and scopes that are created for AA
typedef SmallDenseMap<Value *, MDNode *, 8> PtrScopeMap;

// Map between pointer and its corresponding never alaised pointer (i.e
// pair of disambugated pointers)
typedef SmallDenseMap<Value*, Value*, 8> DisamValuesMap;

// Number of pointers, which are allocated with malloc, allowed to
// apply aggressive Inline AA. 
static cl::opt<unsigned> AggInlAAAllowedTargets("agg-inl-aa-allowed-targets",
                      cl::ReallyHidden, cl::init(2));

// Option to trace Aggressive Inline AAA
static cl::opt<bool> AggInlAATrace("print-agg-inl-aa", cl::ReallyHidden);

// Minimum malloc size limit for Aggressive Inline Analysis. A  malloc
// call that allocates more than this limit is considered for this
// analysis. This is expected to be same as InlineAggressiveMallocLimit
// in Analysis/Intel_AggInline.cpp
//
static cl::opt<unsigned> AggInlAAMallocLimit("agg-inl-aa-malloc-limit",
                                   cl::init(0x60000000), cl::ReallyHidden);

//
// Number of memory instructions that are marked with Aggressive Inline AA.
STATISTIC(NumMemInstAggInlAA, "Number of Memory Inst marked with Agg Inl AA");

// Returns true if 'CI' is allocating huge memory with malloc.
//
static bool isMallocAllocatingHugeMemory(const CallInst *CI) {
  Function *Callee = CI->getCalledFunction();
  StringRef FnName = Callee->getName();
  if (FnName != "malloc")
    return false;
  Value *MallocArg = CI->getArgOperand(0);
  ConstantInt *ConstInt = dyn_cast<ConstantInt>(MallocArg);
  if (!ConstInt)
    return false;
  if (ConstInt->getValue().getZExtValue() < AggInlAAMallocLimit)
    return false;

  return true;
}

// Explore uses of pointer 'V' and add all real uses to 'PtrList' by
// ignoring trivial uses.
//
//  Ex: Ignore use of %77 in icmp and ignore of use of %77 in ptrtoint if
//  there is no use of %78.
//       %77 = call noalias i8* @malloc(i64 1689600000) #4
//       %78 = ptrtoint i8* %77 to i64
//       %79 = icmp eq i8* %77, null
//       %83 = getelementptr inbounds i8, i8* %77, i64 12800000
//       %84 = ptrtoint i8* %83 to i64
//
static void explorePointerUses(Value* V, SmallVector<Value*, 8>& PtrList) {
  PtrList.clear();
  for (Use &U : V->uses()) {

    User *I = U.getUser();

    // Just ignore use in Cmp inst
    if (isa<ICmpInst>(I))
      continue;

    // Ignore if there are no uses to PtrToInt Inst
    PtrToIntInst *P = dyn_cast<PtrToIntInst>(I); 
    if (P && P->getNumUses() == 0)
      continue;

    GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(I); 
    if (GEPI && GEPI->hasOneUse()) {
      if (PtrToIntInst *PI = dyn_cast<PtrToIntInst>(*GEPI->user_begin())) {
        PtrList.push_back(PI);
        continue;
      }
    }
    PtrList.push_back(I);
  }
}

// Collect pointers that are assigned with return address of "malloc" and 
// save them in 'MallocedPtrs' if uses of these pointers can be tracked
// easily.
//  Ex:
//       %77 = call noalias i8* @malloc(i64 1689600000) #4
//       %83 = getelementptr inbounds i8, i8* %77, i64 12800000
//       %84 = ptrtoint i8* %83 to i64
//       ...
//       %85 = call noalias i8* @malloc(i64 1689600000) #4
//       %91 = getelementptr inbounds i8, i8* %85, i64 12800000
//       %92 = ptrtoint i8* %91 to i64
//
static bool collectMallocAllocatedPtrs(Function& F,
                SmallVector<Value*, 8> & MallocedPtrs) {

  MallocedPtrs.clear();
  for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
    const CallInst *CI = dyn_cast<CallInst>(&*II);
    if (!CI || !isa<CallInst>(CI))
      continue;

    Function *Callee = CI->getCalledFunction();
    if (Callee == nullptr)
      return false;

    if (!isMallocAllocatingHugeMemory(CI))
      continue;

    CallSite CS = CallSite(&*II);
    Value* V = CS.getInstruction();

    SmallVector<Value*, 8> MallocRetUseList;
    explorePointerUses(V, MallocRetUseList); 

    // If there are more than real use, returns false.
    if (MallocRetUseList.size() != 1)
      return false;

    MallocedPtrs.push_back(MallocRetUseList.front());
  }
  return true;
}


// Returns base pointer for given .V. by ignoring BitCasts, PtrToInt etc. 
//
// Ex:
//   It returns %93 for given %100 as input for below example
//      
//  %93 = inttoptr i64 %84 to [208000000 x double]*
//  %99 = getelementptr inbounds [.double], [.double]* %93, i64 0, i64 %98
//  %100 = bitcast double* %99 to <4 x double>*
//
static Value* getBasePointer(Value* V) {
  if (Operator::getOpcode(V) == Instruction::BitCast) {
    V = cast<Operator>(V)->getOperand(0);
  }
  GEPOperator *GEPOp = dyn_cast<GEPOperator>(V);
  if (GEPOp) {
    V = GEPOp->getOperand(0);
  }
  if (Operator::getOpcode(V) == Instruction::BitCast ||
      Operator::getOpcode(V) == Instruction::PtrToInt ||
      Operator::getOpcode(V) == Instruction::IntToPtr) {
    V = cast<Operator>(V)->getOperand(0);
  }
  return V;
}

// It returns mapped value for given 'V' by treating 'AggInlAAPairs'
// as bi-directional Map.
//
static Value* getMappedValueInAAPairs(Value* V,
                              DisamValuesMap& AggInlAAPairs) { 
  for(auto I = AggInlAAPairs.begin(), E = AggInlAAPairs.end(); I != E; ++I) {
    if (I->first == V)
      return I->second;
    if (I->second == V)
      return I->first;
  } 
  return nullptr;
}

// Create newscopes for all pointer pairs collected in 'AggInlAAPairs'
// and save them in 'PointersScopes', which map between pointers and
// scopes. 
//
static void createNewScopesForSelectedPtrs(Function& F,
            DisamValuesMap& AggInlAAPairs,
            PtrScopeMap& PointersScopes) {

  MDBuilder MDB(F.getContext());
  MDNode *NewDomain = MDB.createAnonymousAliasScopeDomain();

  PointersScopes.clear();

  for (auto I = AggInlAAPairs.begin(), E = AggInlAAPairs.end(); I != E; ++I) {
      MDNode *NewScope = MDB.createAnonymousAliasScope(NewDomain);
      Value *V1 = I->first;
      PointersScopes[V1] = NewScope;

      NewScope = MDB.createAnonymousAliasScope(NewDomain);
      V1 = I->second;
      PointersScopes[V1] = NewScope;
  } 
}

// Set alias_scope MetaData for 'I' using  new scope that was created
// for 'V' pointer. It gets new scope for 'V' from PointersScopes map.  
// Ex:
//     !alias.scope !12,
//
static void generateScopeMD(Instruction* I, Value* V,
                            PtrScopeMap& PointersScopes) {
  MDNode* SM = I->getMetadata(LLVMContext::MD_alias_scope);
  MDNode* NewScope = PointersScopes.lookup(V);
  assert(NewScope != nullptr && "Expecting valid Scope");
  MDNode* SM1 = MDNode::concatenate(SM, NewScope);
  I->setMetadata(LLVMContext::MD_alias_scope, SM1);
}

// Set noAlias MetaData for 'I' using new scope that was created for
// disambiguated pointer pair of 'V'. It gets disambiguated pointer pair
// of 'V' from 'AggInlAAPairs' map and new scope from 'PointersScopes' map.
// Ex:
//      !noalias !14
//
static void generateNoAliasMD(Instruction* I, Value* V,
                          PtrScopeMap& PointersScopes,
                          DisamValuesMap& AggInlAAPairs) {
  Value* V1 = getMappedValueInAAPairs(V, AggInlAAPairs); 
  MDNode* AM = I->getMetadata(LLVMContext::MD_noalias);
  MDNode* NewScope = PointersScopes.lookup(V1);
  assert(NewScope != nullptr && "Expecting valid Scope");
  MDNode* AM1 = MDNode::concatenate(AM, NewScope);
  I->setMetadata(LLVMContext::MD_noalias, AM1);
}

// Set alias_scope and noAlias MetaData for 'I', which is using 'V' as
// base pointer, using 'PointersScopes' and 'AggInlAAPairs' maps.   
//
static void generateMDForInst(Instruction* I, Value* V,
                     PtrScopeMap& PointersScopes,
                     DisamValuesMap& AggInlAAPairs) {

  NumMemInstAggInlAA++;
  generateScopeMD(I, V, PointersScopes);
  generateNoAliasMD(I, V, PointersScopes, AggInlAAPairs);
}

// Returns true if 'V' is found in 'SmallVec'.
//
static bool isPtrInVector(Value* V, SmallVector<Value*, 8>& SmallVec) {
  for (auto I = SmallVec.begin(), E = SmallVec.end(); I != E; I++) {
    if (V == (*I)) {
      return true;
    }
  }  
  return false;
}
 
// It walks through all load/store instructions and generates Metadata
// if base pointer is selected for AggInlAA.
// Ex:
// %1209 = load double, double* %1208, align 8, !alias.scope !16, !noalias !17
//
static void generateMDForMemInsts(Function& F,
                  SmallVector<Value*, 8>& PtrsSelectedForAA,
                  PtrScopeMap& PointersScopes,
                  DisamValuesMap& AggInlAAPairs) {
  Value* V;
  for (inst_iterator I = inst_begin((F)), E = inst_end((F)); I != E; ++I) {
    V = nullptr;
    if (LoadInst *LI = dyn_cast<LoadInst>(&(*I))) {
      V = LI->getPointerOperand();
    } else if (StoreInst *SI = dyn_cast<StoreInst>(&(*I))) {
      V = SI->getPointerOperand();
    } else {
      continue;
    }

    V = getBasePointer(V);  

    if (!isPtrInVector(V, PtrsSelectedForAA))
      continue;

    if (AggInlAATrace)
      errs() << "    Before: " << (*I) << "\n";

    generateMDForInst(&(*I), V, PointersScopes, AggInlAAPairs);

    if (AggInlAATrace)
      errs() << "    After: " << (*I) << "\n";
  }
}

// It tries to find base pointers that are swapped and it proves that
// swapped pointers are also never aliased to each other.
//
// Ex:
//     %84 = ptrtoint i8* %83 to i64  
//     %92 = ptrtoint i8* %91 to i64
//     ...
//     %1146 = phi i64 [ %84, %1143 ], [ %1147, %2428 ]
//     %1147 = phi i64 [ %92, %1143 ], [ %1146, %2428 ]
//
// %1146 and %1147 also never aliased to each other since
//   1. They are defined in same basic block (Adjacent Insts)
//   2. Their true-operands never overlap (Base ptrs allocated using malloc)
//   3. Their false-operands never overlap
//
static void collectSwappedBasePtrs(SmallVector<Value*, 8>& FirstPtrUses,
           SmallVector<Value*, 8>& SecondPtrUses,
           DisamValuesMap& AggInlAAPairs,
           SmallVector<Value*, 8>& PtrsSelectedForAA,
           Value* FirstPtr, Value* SecondPtr) {

  for (auto I = FirstPtrUses.begin(), E = FirstPtrUses.end(); I != E; ++I) {
    PHINode *PN1 = cast<PHINode>(*I);

    if (PN1->getNumIncomingValues() != 2) {
      continue;
    }
    Value *L1 = PN1->getIncomingValue(0);
    if (L1 != FirstPtr)
      continue;

    Value *R1 = PN1->getIncomingValue(1);

    if (!isPtrInVector(R1, SecondPtrUses))
      continue;

    PHINode *PN2 =  cast<PHINode>(R1);
    assert(PN2 != nullptr && " Expecting PHI node");

    Value *L2 = PN2->getIncomingValue(0);
    if (L2 != SecondPtr)
      continue;
    Value *R2 = PN2->getIncomingValue(1);
    if (R2 != PN1)
      continue;
    if (PN1->getParent() != PN2->getParent())
      continue;

    if (PN1->getNextNode() != PN2 && PN2->getNextNode() != PN1)
      continue;

    AggInlAAPairs.insert(std::make_pair(PN1, PN2));
    PtrsSelectedForAA.push_back(PN1);
    PtrsSelectedForAA.push_back(PN2);
  }
}

static bool rumImpl(Function& F) {

  SmallVector<Value*, 8> MallocedPtrs;
  SmallVector<Value*, 8> FirstPtrUses;
  SmallVector<Value*, 8> SecondPtrUses;

  // Keeps all pointers that are selected for Agg Inl AA.
  SmallVector<Value*, 8> PtrsSelectedForAA;

  PtrScopeMap PointersScopes;
  DisamValuesMap AggInlAAPairs;

  // Collect pointers that are assigned with malloc return addresses.
  if (!collectMallocAllocatedPtrs(F, MallocedPtrs)) {
    if (AggInlAATrace) { 
      errs() << " AggInlAA: Not able to collect malloc allocated ptrs\n"; 
    }
    return false;
  }

  // Allow only limited number of pointers for now.
  if (MallocedPtrs.size() != AggInlAAAllowedTargets) {
    if (AggInlAATrace) { 
      errs() << " AggInlAA: Number of malloced ptrs excedded the target\n"; 
    }
    return false;
  }

  if (AggInlAATrace) { 
    errs() << " AggInlAA: Collected base pointers are ... \n"; 
    for (auto I = MallocedPtrs.begin(), E = MallocedPtrs.end();
         I != E; ++I) {
      Value* V = (*I);
      errs() << "    " <<  *V << " \n";
    }
  }

  // Get pointers using front() and back() since it has only 2 pointers.
  // Ex:
  //  %84 = ptrtoint i8* %83 to i64
  //  %92 = ptrtoint i8* %91 to i64
  //
  Value* FirstPtr = MallocedPtrs.front();
  Value* SecondPtr = MallocedPtrs.back();

  // Collect all uses in PHI nodes for 'FirstPtr'
  // Ex:
  //    %2432 = phi i64 [ %84, %1143 ], [ %2433, %3099 ]
  //    %1146 = phi i64 [ %84, %1143 ], [ %1147, %2428 ]
  //
  for (Use &U : FirstPtr->uses()) {
    User *I = U.getUser();
    if (!isa<PHINode>(I)) {
      continue;
    }
    FirstPtrUses.push_back(I);
  }

  // Collect all uses in PHI nodes for 'SecondPtr'
  // Ex:
  //   %2433 = phi i64 [ %92, %1143 ], [ %2432, %3099 ]
  //   %1147 = phi i64 [ %92, %1143 ], [ %1146, %2428 ]
  //
  for (Use &U : SecondPtr->uses()) {
    User *I = U.getUser();
    if (!isa<PHINode>(I)) {
      continue;
    }
    SecondPtrUses.push_back(I);
  }

  // Add FirstPtr and SecondPtr pair to AggInlAAPairs as never aliased
  // pointers since they are the base pointers that point to malloc return
  // addresses.
  //
  AggInlAAPairs.insert(std::make_pair(FirstPtr, SecondPtr));
  PtrsSelectedForAA.push_back(FirstPtr);
  PtrsSelectedForAA.push_back(SecondPtr);

  // Trying to collect pointers that are swapped 
  collectSwappedBasePtrs(FirstPtrUses, SecondPtrUses, AggInlAAPairs,
                         PtrsSelectedForAA, FirstPtr, SecondPtr);

  if (AggInlAATrace) {
    errs() << " AggInlAA: Pointer pairs found to apply Agg Inl AA \n";
    for(auto I = AggInlAAPairs.begin(), E = AggInlAAPairs.end(); I != E; ++I) {
      errs() << "     " << *(I->first);
      errs() << " ... " << *(I->second)  << "\n";
    }
  }

  // Create new scopes for pointers in 'AggInlAAPairs'
  createNewScopesForSelectedPtrs(F, AggInlAAPairs, PointersScopes);

  if (AggInlAATrace)
    errs() << " AggInlAA: Mem Insts before and after Agg Inl AA \n";

  generateMDForMemInsts(F, PtrsSelectedForAA, PointersScopes, AggInlAAPairs);

  return true;
}

PreservedAnalyses AggInlAAPass::run(Function &F, FunctionAnalysisManager &M) {
  rumImpl(F);
  return PreservedAnalyses::all();
}

namespace {
struct AggInlAALegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  AggInlAALegacyPass() : FunctionPass(ID) {
    initializeAggInlAALegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function& F) override {
    // Aggressive analysis is enabled for "main" only. 
    if (F.getName() != "main")
      return false;

    auto Agg = getAnalysisIfAvailable<InlineAggressiveWrapperPass>(); 
    InlineAggressiveInfo *AggI = Agg ? &Agg->getResult() : nullptr;
    if (!AggI || !AggI->isAggInlineOccured())
      return false;

    if (rumImpl(F))
      return true;

    return false;
  }

  void getAnalysisUsage(AnalysisUsage& AU) const override {
    AU.setPreservesAll();
    AU.addUsedIfAvailable<InlineAggressiveWrapperPass>();
  }
};
}

char AggInlAALegacyPass::ID = 0;
INITIALIZE_PASS(AggInlAALegacyPass, "agginlaa", "Aggressive Inline AA",
                false, false)

FunctionPass *llvm::createAggInlAALegacyPass() {
  return new AggInlAALegacyPass(); }

