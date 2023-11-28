#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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
//==-- IntrinsicUtils.cpp - Utilities for VPO related intrinsics -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of utilities for VPO-based intrinsic function
/// calls. E.g., directives that mark the beginning and end of SIMD and
/// parallel regions.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO/Intel_InlineReport.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<unsigned>
    RefsThreshold("refs-threshold", cl::Hidden, cl::init(500),
                    cl::desc("The number of references threshold"));

// TODO: Handle VLA allocas
static cl::opt<bool> AddTypedPrivates(
    "vpo-utils-add-typed-privates", cl::Hidden, cl::init(true),
    cl::desc("Use TYPED OMP clause when adding privates in CodeExtractor."));

// Return the <ElementType, NumElements> pair for the alloca AI.
ElementTypeAndNumElements
VPOUtils::getTypedClauseInfoForAlloca(AllocaInst *AI) {
  Value *NumElements = AI->getArraySize();
  Type *AllocatedTy = AI->getAllocatedType();
  Type *I64Ty = Type::getInt64Ty(AllocatedTy->getContext());

  if (!isa<ArrayType>(AllocatedTy))
    return {AllocatedTy,
            NumElements ? NumElements : ConstantInt::get(I64Ty, 1)};

  if (NumElements && !isa<ConstantInt>(NumElements)) {
    // For %1 = alloca [10 x i16], i32 %n, we will not decay AllocatedTy.
    // If we do, we may need to insert instructions to calculate the size.
    // i.e. AllocatedTy: [10 x i16] -> i16 and Size: 10 * %n(in IR)
    return {AllocatedTy, NumElements};
  }

  uint64_t DecayedNE =
      NumElements ? cast<ConstantInt>(NumElements)->getZExtValue() : 1;
  Type *DecayedTy = AllocatedTy;

  while (ArrayType *ArrTy = dyn_cast<ArrayType>(DecayedTy)) {
    DecayedTy = ArrTy->getElementType();
    DecayedNE *= ArrTy->getNumElements();
  }

  return {DecayedTy, ConstantInt::get(I64Ty, DecayedNE)};
}

bool VPOUtils::unsetMayHaveOpenmpDirectiveAttribute(Function &F) {
  StringRef AttributeName = "may-have-openmp-directive";
  if (!F.hasFnAttribute(AttributeName))
    return false;

  if (F.getFnAttribute(AttributeName).getValueAsString() == "false")
    return false;

  F.removeFnAttr(AttributeName);
  F.addFnAttr(AttributeName, "false");
  return true;
}

bool VPOUtils::stripDirectives(WRegionNode *W) {
  bool Changed = VPOUtils::stripDirectives(*(W->getEntryBBlock()));
  Changed |= VPOUtils::stripDirectives(*(W->getExitBBlock()));

  return Changed;
}

bool VPOUtils::stripDirectives(BasicBlock &BB, ArrayRef<int> IDs) {
  SmallVector<Instruction *, 4> IntrinsicsToRemove;
  LLVMContext &C = BB.getContext();

  for (Instruction &I : BB)
    if (VPOAnalysisUtils::isOpenMPDirective(&I)) {
      int DirID = VPOAnalysisUtils::getDirectiveID(&I);
      if (IDs.empty() ||
          std::any_of(IDs.begin(), IDs.end(),
                      [DirID](int ID) {
                        return ID == DirID;
                      }))
        IntrinsicsToRemove.push_back(&I);
    }

  if (IntrinsicsToRemove.empty())
    return false;

  for (Instruction *I : IntrinsicsToRemove) {
    if (I->getType()->isTokenTy())
      // "llvm::ConstantTokenNone::get(C)" isn't supported by -debugify
      I->replaceAllUsesWith(llvm::UndefValue::get(Type::getTokenTy(C)));
    I->eraseFromParent();
  }

  return true;
}

bool VPOUtils::stripDirectives(Function &F, ArrayRef<int> IDs) {
  bool changed = false;

  for (BasicBlock &BB: F) {
    changed |= stripDirectives(BB, IDs);
  }

  return changed;
}

CallInst *VPOUtils::createMaskedScatterCall(Value *VecPtr,
                                            Value *VecData,
                                            IRBuilder<> &Builder,
                                            unsigned Alignment,
                                            Value *Mask) {
  auto NewCallInst =
      Builder.CreateMaskedScatter(VecData, VecPtr, Align(Alignment), Mask);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedStoreCall(Value *VecPtr,
                                          Value *VecData,
                                          IRBuilder<> &Builder,
                                          unsigned Alignment,
                                          Value *Mask) {
  auto NewCallInst = Builder.CreateMaskedStore(VecData, VecPtr,
                                               assumeAligned(Alignment), Mask);
  return NewCallInst;
}

// Recursively strip Casts from ValWithCasts, until a non CastInst is found.
Value *VPOUtils::stripCasts(Value *ValWithCasts,
                            SmallVectorImpl<Instruction *> &SeenCastInsts) {

  assert(ValWithCasts && "Null input value.");

  while (CastInst *CI = dyn_cast<CastInst>(ValWithCasts)) {
    SeenCastInsts.push_back(CI);
    ValWithCasts = CI->getOperand(0);
  }

  return ValWithCasts;
}

// Return true if the given type can be registerized.
bool VPOUtils::canBeRegisterized(Type *AllocaTy, const DataLayout &DL) {
  Type *ScalarTy = AllocaTy->getScalarType();
  if (!AllocaTy->isSingleValueType() ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0)
    return false;
  return true;
}

// Generates a memcpy call using MemcpyBuilder.
// The value D represents the destination while the value S represents
// the source. The size of the memcpy is specified as Size multiplied
// by NumElements.
// The compiler will insert the typecast if the type of source or destination
// does not match with the type i8.
CallInst *VPOUtils::genMemcpy(Value *D, Value *S, uint64_t Size,
                              Value *NumElements, unsigned Align,
                              IRBuilder<> &MemcpyBuilder) {
  // For 32/64 bit architecture, the size and alignment should be
  // set accordingly.
  Function *F = MemcpyBuilder.GetInsertBlock()->getParent();
  Type *SizeTTy = GeneralUtils::getSizeTTy(F);
  unsigned SizeTBitWidth = SizeTTy->getIntegerBitWidth();
  Value *SizeVal = MemcpyBuilder.getIntN(SizeTBitWidth, Size);

  if (NumElements)
    SizeVal = MemcpyBuilder.CreateMul(
        SizeVal, MemcpyBuilder.CreateZExtOrTrunc(NumElements,
                                                 SizeVal->getType()));

  return MemcpyBuilder.CreateMemCpy(D, MaybeAlign(Align), S, MaybeAlign(Align),
                                    SizeVal);
}

// Generates a memset call using MemsetBuilder.
// The value P represents the pointer to the block of memory to fill while the
// value V represents the value to be set. The size of the memset is specified
// as Size. The compiler will insert the typecast if the
// type of pointer and value does not match with the type i8.
CallInst *VPOUtils::genMemset(Value *P, Value *V, uint64_t Size, unsigned Align,
                              IRBuilder<> &MemsetBuilder) {
  // For 32/64 bit architecture, the size and alignment should be
  // set accordingly.
  Function *F = MemsetBuilder.GetInsertBlock()->getParent();
  Type *SizeTTy = GeneralUtils::getSizeTTy(F);
  unsigned SizeTBitWidth = SizeTTy->getIntegerBitWidth();
  Value *SizeVal = MemsetBuilder.getIntN(SizeTBitWidth, Size);

  AllocaInst *AI = dyn_cast<AllocaInst>(P);
  if (AI && AI->isArrayAllocation())
    SizeVal = MemsetBuilder.CreateMul(SizeVal, AI->getArraySize());

  return MemsetBuilder.CreateMemSet(P, V, SizeVal, MaybeAlign(Align));
}

// Creates a clone of CI, adds OpBundlesToAdd to it, and returns it.
CallInst *VPOUtils::addOperandBundlesInCall(
    CallInst *CI,
    ArrayRef<std::pair<StringRef, ArrayRef<Value *>>> OpBundlesToAdd) {

  assert(CI && "addOperandBundlesInCall: Null CallInst");

  if (OpBundlesToAdd.empty())
    return CI;

  SmallVector<Value *, 8> Args;
  for (auto AI = CI->arg_begin(), AE = CI->arg_end(); AI != AE; AI++)
    Args.push_back(*AI);

  SmallVector<OperandBundleDef, 1> OpBundles;
  CI->getOperandBundlesAsDefs(OpBundles);

  for (auto &StrValVec : OpBundlesToAdd) {
    OperandBundleDef B(std::string(StrValVec.first), StrValVec.second);
    LLVM_DEBUG(
        dbgs() << __FUNCTION__ << ": Adding bundle '" << StrValVec.first
               << "' with operands:";
        if (StrValVec.second.empty()) { dbgs() << " N/A"; } else {
          for (auto *V : StrValVec.second) {
            dbgs() << " ";
            V->printAsOperand(dbgs());
          }
        };
        dbgs() << ".\n");
    OpBundles.push_back(B);
  }

  FunctionType *FnTy = CI->getFunctionType();
  Value *Fn = CI->getCalledOperand();
  auto NewI = CallInst::Create(FnTy, Fn, Args, OpBundles, "", CI);

  NewI->takeName(CI);
  NewI->setCallingConv(CI->getCallingConv());
  NewI->setAttributes(CI->getAttributes());
  NewI->setDebugLoc(CI->getDebugLoc());
  NewI->copyMetadata(*CI);
#if INTEL_CUSTOMIZATION
  getInlineReport()->replaceCallBaseWithCallBase(CI, NewI);
  getMDInlineReport()->replaceCallBaseWithCallBase(CI, NewI);
#endif // INTEL_CUSTOMIZATION

  CI->replaceAllUsesWith(NewI);
  CI->eraseFromParent();
  return NewI;
}

// Creates a clone of CI without the operand bundles in OpBundlesToRemove from
// it, and returns it.
CallInst *
VPOUtils::removeOperandBundlesFromCall(CallInst *CI,
                                       ArrayRef<StringRef> OpBundlesToRemove) {
  return IntrinsicUtils::removeOperandBundlesFromCall(
      CI, [&OpBundlesToRemove](const OperandBundleDef &Bundle) {
        return std::any_of(
            OpBundlesToRemove.begin(), OpBundlesToRemove.end(),
            [&Bundle](StringRef S) { return Bundle.getTag() == S; });
      });
}

// Creates a clone of CI without the operand bundles representing OpenMP
// clauses specified in ClauseIds.
CallInst *VPOUtils::removeOpenMPClausesFromCall(CallInst *CI,
                                                ArrayRef<int> ClauseIds) {
  return IntrinsicUtils::removeOperandBundlesFromCall(
      CI, [&ClauseIds](const OperandBundleDef &Bundle) {
        return std::any_of(
            ClauseIds.begin(), ClauseIds.end(),
            [&Bundle](int Id) {
              StringRef Name = Bundle.getTag();
              // We cannot construct ClauseInfo with a Name
              // not corresponding to an OpenMP clause, so
              // we have to check the Name, first.
              if (!VPOAnalysisUtils::isOpenMPClause(Name))
                return false;

              ClauseSpecifier ClauseInfo(Name);
              return ClauseInfo.getId() == Id;
            });
      });
}

// "Privatizes" an Instruction by adding it to a supported entry
// directive clause.
// If the Instruction is already used in a directive, nothing is done.
// BlockPos: The first dominating entry directive over this block is
// chosen.
// SimdOnly: Process SIMD directives only. This must be true inside VPO, as
// non-SIMD directives cannot be modified during VPO.
// Return false if no directive was found.
bool VPOUtils::addPrivateToEnclosingRegion(AllocaInst *I, BasicBlock *BlockPos,
                                           DominatorTree &DT, bool SimdOnly,
                                           bool ForceTypedClause) {
  // Check if I is already used in a directive, don't introduce a possible
  // conflict.
  for (User *UseI : I->users())
    if (auto *IntrInst = dyn_cast<IntrinsicInst>(UseI))
      if (VPOAnalysisUtils::isRegionDirective(IntrInst))
        // Ignore non-simd if we are only intending to modify simd.
        if (!SimdOnly ||
            VPOAnalysisUtils::getDirectiveID(IntrInst) == DIR_OMP_SIMD)
          return false;
  LLVM_DEBUG(dbgs() << "Looking for OMP begin for block " << BlockPos->getName()
                    << "\n");

  // Start at BlockPos and search each dominating block in turn,
  // looking for a llvm.directive that supports the private clause.
  Instruction *Begin = BlockPos->getTerminator();
  while (Begin) {
    if (isa<IntrinsicInst>(Begin) &&
        VPOAnalysisUtils::supportsPrivateClause(Begin)) {
      // If SimdOnly is true, ignore all except SIMD directives.
      if (!SimdOnly || VPOAnalysisUtils::getDirectiveID(Begin) == DIR_OMP_SIMD)

      {
        CallInst *Repl;
        if (ForceTypedClause || AddTypedPrivates) {
          ElementTypeAndNumElements EltTyAndNumElem =
              VPOUtils::getTypedClauseInfoForAlloca(I);
          Repl = VPOUtils::addOperandBundlesInCall(
              cast<CallInst>(Begin),
              {{"QUAL.OMP.PRIVATE:TYPED",
                {I, Constant::getNullValue(EltTyAndNumElem.first),
                 EltTyAndNumElem.second}}});
        } else {
          Repl = VPOUtils::addOperandBundlesInCall(cast<CallInst>(Begin),
                                                   {{"QUAL.OMP.PRIVATE", {I}}});
        }
        if (Repl) {
          LLVM_DEBUG(dbgs() << "Added private clause for: " << *I << "\nto "
                            << *Repl << "\n");
        }
        return true;
      }
    }
    Begin = enclosingBeginDirective(Begin, &DT);
  }
  return false;
}

// Returns the next enclosing OpenMP begin directive, or nullptr if none.
// The instruction must be reachable and have a dominator available.
IntrinsicInst *VPOUtils::enclosingBeginDirective(Instruction *I,
                                                 DominatorTree *DT) {
  assert(DT && "Dominator tree not available.");
  auto *DomNode = DT->getNode(I->getParent());

  // We assert here rather than returning nullptr, as we don't know what
  // the caller actually would like to do with unreachable blocks.
  assert(DomNode && "Dominator for inst not available: may be unreachable.");

  // Start at the previous inst, in case I is already a begin directive.
  // We want the enclosing scope in that case, not I.
  // If prev is null, the loop will choose a new block.
  auto *CurrI = I->getPrevNode();
  SmallVector<IntrinsicInst *, 4> EndStack;
  do {
    // Iterate backwards over instructions in the current block. Do not assume
    // that directives are in any specific position.
    while (CurrI) {
      if (auto *II = dyn_cast<IntrinsicInst>(CurrI)) {
        if (VPOAnalysisUtils::isBeginDirective(II)) {
          // See comment below.
          if (EndStack.empty())
            return II;
          else
            EndStack.pop_back_n(1);
        } else if (VPOAnalysisUtils::isEndDirective(II)) {
          // Keep track of open scopes, by looking for end directives.
          // Example:
          //  DIR.BEGIN
          //   DIR.BEGIN // skip this
          //   DIR.END
          //   ...
          //   I =
          // If we are dominated by an end, the matching begin must be skipped.
          // This assumes that begin dominates end.
          EndStack.emplace_back(II);
        }
      }
      CurrI = CurrI->getPrevNode();
    }
    // If we didn't find anything in DomNode's block, go to the immediate
    // dominator and look again.
    DomNode = DomNode->getIDom();
    if (!DomNode)
      return nullptr;
    CurrI = DomNode->getBlock()->getTerminator();
  } while (1);
  // while loop must return on one of these conditions:
  // - we found the enclosing begin directive
  // - we reach the DT root.
  llvm_unreachable("Cannot get here.");
  return nullptr;
}

// Searches for the enclosing begin directive intrinsic starting from \p BB
// using BFS search and returns the next enclosing OpenMP begin directive if it
// is available, or nullptr if none.
IntrinsicInst *VPOUtils::enclosingBeginDirective(BasicBlock *BB) {
  SmallVector<BasicBlock *, 4> BFT;
  SmallPtrSet<BasicBlock *, 4> Visited;
  bool EntryBlockFound = false;
  Visited.insert(BB);
  BFT.push_back(BB);

  while (!BFT.empty() && !EntryBlockFound) {
    BasicBlock *Cur = BFT.pop_back_val();
    if (Cur->isEntryBlock())
      EntryBlockFound = true;
    // Check if the current bblock contains the begin directive intrinsic
    // instruction.
    if (IntrinsicInst *II = VPOAnalysisUtils::getBeginDirective(Cur))
      return II;
    // visit all predecessors of the current bblock
    for (BasicBlock *Pred : predecessors(Cur)) {
      if (!Visited.contains(Pred)) {
        BFT.push_back(Pred);
        Visited.insert(Pred);
      }
    }
  }
  return nullptr;
}

// Return true if ClauseId is found in II, or false if not.
bool VPOUtils::hasOpenMPClause(IntrinsicInst *II, int ClauseId) {
  assert(II && "Begin directive intrinsic instruction is null.");
  unsigned NumOB = II->getNumOperandBundles();
  // Index i start from (NumOB -1) and runs till it becomes 1 (not
  // 0) because we want to skip the first OperandBundle, which is
  // the directive name.
  for (unsigned i = NumOB - 1; i >= 1; i--) {
    // BU is the i-th OperandBundle, which represents a clause
    OperandBundleUse BU = II->getOperandBundleAt(i);
    if (!VPOAnalysisUtils::isOpenMPClause(BU.getTagName()))
      continue;
    if (ClauseSpecifier(BU.getTagName()).getId() == ClauseId)
      return true;
  }
  return false;
}

/// Find BlockAddress references in NewFunction that point to OldFunction,
/// and replace them. This must be called after all code has moved to
/// NewFunction.
void VPOUtils::replaceBlockAddresses(Function *OldFunction,
                                     Function *NewFunction) {
  SmallVector<BlockAddress *, 4> BlockAddresses;
  // Collect all BlockAddresses that reference OldFunction.
  for (User *U : OldFunction->users())
    if (auto *BA = dyn_cast<BlockAddress>(U))
      BlockAddresses.push_back(BA);
  // If any of these BlockAddresses have a BasicBlock that points to
  // NewFunction, replace it.
  for (auto *BA : BlockAddresses) {
    if (BA->getBasicBlock()->getParent() == NewFunction)
      BA->replaceAllUsesWith(
          BlockAddress::get(NewFunction, BA->getBasicBlock()));
  }
}

#if INTEL_CUSTOMIZATION

// Alias scope is an optimization
using ScopeSetType = SmallSetVector<Metadata *, 8>;

void VPOUtils::genAliasSet(ArrayRef<BasicBlock *> BBs, AAResults *AA,
                           const DataLayout *DL) {

  SmallVector<Instruction *, 8> MemoryInsns;

  auto collectMemReferences = [&](ArrayRef<BasicBlock *> BBs,
                                  SmallVectorImpl<Instruction *> &MemoryInsns) {
    for (auto *BB : BBs) {
      for (auto &I : *BB) {
        if (isa<LoadInst>(&I) || isa<StoreInst>(&I))
          MemoryInsns.push_back(&I);
      }
    }
  };

  collectMemReferences(BBs, MemoryInsns);

  // CMPLRS-51441 [VPO][SPEC OMP 2012] spec_omp2012/367 times out at
  // compile time in VPOUtils::genAliasSet()
  // Set the memory reference thresold to force the function
  // genAliasSet to give up once the number of reference exceeds the threshold.
  if (MemoryInsns.size() > RefsThreshold)
    return;

  class BitMatrix {
  private:
    BitVector BV;
    unsigned RecLen = 0;

  public:
    BitMatrix(){};
    ~BitMatrix(){};
    void clear() {
      RecLen = 0;
      BV.clear();
    }
    void resize(unsigned N) {
      RecLen = N;
      BV.resize(N * N);
    }
    bool bitTest(int rowno, int colno) {
      return BV.test(rowno * RecLen + colno);
    }
    void bitSet(int rowno, int colno) { BV.set(rowno * RecLen + colno); }
    void bitReset(int rowno, int colno) { BV.flip(rowno * RecLen + colno); }
    void dump(raw_ostream &OS) {
      unsigned I, J;
      if (BV.size() == 0)
        return;
      OS << "alias matrix\n";
      for (I = 0; I < RecLen; I++) {
        for (J = 0; J < RecLen; J++)
          if (bitTest(I, J))
            OS << "1 ";
          else
            OS << "0 ";
        OS << "\n";
      }
      OS << "\n";
    }
  };

  // Build an alias matrix based on the AA for the incoming loads/stores.
  auto initAliasMatrix = [&](SmallVectorImpl<Instruction *> &Insns,
                             AAResults *AA, const DataLayout *DL,
                             BitMatrix &BM) {
    int N = Insns.size();
    BM.clear();
    BM.resize(N);
    for (int I = 0; I < N; I++) {
      assert((isa<LoadInst>(Insns[I]) || isa<StoreInst>(Insns[I])) &&
             "Expect load/store instruction");
      for (int J = I + 1; J < N; J++) {
        assert((isa<LoadInst>(Insns[J]) || isa<StoreInst>(Insns[J])) &&
               "Expect load/store instruction");
        if (isa<LoadInst>(Insns[I]) && isa<LoadInst>(Insns[J]))
          continue;
        auto LocA = MemoryLocation::get(Insns[I]).getWithNewSize(
            LocationSize::beforeOrAfterPointer());
        auto LocB = MemoryLocation::get(Insns[J]).getWithNewSize(
            LocationSize::beforeOrAfterPointer());
        // TODO: Even with unknown size, the alias result is not quite valid
        // for loop carried case; it is possible for the pointers to vary in
        // such a way that they never alias in any one iteration but still
        // alias across different iterations.
        if (!AA->isNoAlias(LocA, LocB))
          BM.bitSet(J, I);
      }
    }
  };

  BitMatrix BM;
  initAliasMatrix(MemoryInsns, AA, DL, BM);

  // Form a clique if the references are proved to be aliased with
  // each other.
  auto formClique = [&](BitVector &C, int Col, int Row, class BitMatrix &CM) {
    for (int I = Col; I >= 0; --I) {
      if (C.test(I)) {
        CM.bitSet(Row, I);
        for (int J = Row; J > I; --J) {
          if (C.test(J))
            CM.bitSet(J, I);
        }
        for (int K = I - 1; K >= 0; --K) {
          if (C.test(K) && !BM.bitTest(I, K))
            C.flip(K);
        }
      }
    }
    C.set(Row);
  };

  // Append a list of metadata nodes of "KindID" to I.
  // Preserve any existing MD of this type. Removes duplicate nodes.
  auto AppendMD = [&](Instruction *I, unsigned KindID,
                      ArrayRef<Metadata *> NewMD) {
    auto &C = I->getContext();
    MDNode *AM = I->getMetadata(KindID);
    if (!AM) {
      I->setMetadata(KindID, MDNode::get(C, NewMD));
    } else {
      SmallSetVector<Metadata *, 4> Operands(AM->op_begin(), AM->op_end());
      Operands.insert(NewMD.begin(), NewMD.end());
      I->setMetadata(KindID, MDNode::get(C, Operands.getArrayRef()));
    }
  };

  // It generates a unique metadata ID for every clique.
  // Then every load/stores is updated with alias_scope metadata
  // as well as noalias metadata.
  auto genMDForCliques = [&](std::vector<BitVector> &CliqueSet,
                             SmallVectorImpl<Instruction *> &Insns,
                             BitMatrix &BM) {
    if (Insns.size() == 0)
      return;
    LLVMContext &C = Insns[0]->getContext();
    MDBuilder MDB(C);
    MDNode *NewDomain = MDB.createAnonymousAliasScopeDomain("OMPDomain");

    // For each instruction in Insns[], this vector stores the alias.scope
    // MDNodes for the cliques that the instruction belongs to.
    SmallVector<ScopeSetType, 0> ScopeSetVec(Insns.size());

    StringRef Name = "OMPAliasScope";

    // Each clique is identified by one MDNode in alias.scope format:
    // !2 = distinct !{!2, !99, "OMPAliasScope"}
    // where !99 is the OMPDomain scope domain (created above).
    // Each inst may belong to 0->N cliques.
    // For each clique, create the MDNode for that clique, then insert the
    // MDNode into the CliqueSet of each instruction in that clique.
    for (const BitVector &Bv : CliqueSet) {
      MDNode *CliqueIdMD = MDB.createAnonymousAliasScope(NewDomain, Name);
      for (unsigned I = 0; I < Bv.size(); ++I) {
        if (Bv.test(I))
          ScopeSetVec[I].insert(CliqueIdMD);
      }
    }

    // Attach the clique MDNodes to the instructions.
    // Assign a new scope MDNode to instructions without a clique,
    // so that each instruction will have an alias.scope node.
    for (unsigned I = 0; I < Insns.size(); ++I) {
      ScopeSetType &CliquesI = ScopeSetVec[I];
      Instruction *Ins = Insns[I];

      if (CliquesI.empty()) {
        MDNode *M = MDB.createAnonymousAliasScope(NewDomain, Name);
        CliquesI.insert(M);
      }

      // Create a list of clique MDNodes.
      // There may be one or more of them:
      // !3 = distinct !{!3, !99, "OMPAliasScope"}
      // !4 = distinct !{!4, !99, "OMPAliasScope"}
      // !8 = {!3, !4}
      //
      AppendMD(Ins, LLVMContext::MD_alias_scope, CliquesI.getArrayRef());
    }

    // For each pair of Insns: (I,J), if I and J do not alias,
    // record that I does not alias the scopes of J, and J does not
    // alias the scopes of I.
    SmallVector<ScopeSetType, 0> NoAliasMDs(Insns.size());

    for (unsigned I = 0; I < Insns.size(); ++I) {
      LoadInst *LIA = dyn_cast<LoadInst>(Insns[I]);
      for (unsigned J = I + 1; J < Insns.size(); ++J) {
        LoadInst *LIB = dyn_cast<LoadInst>(Insns[J]);
        // 2 loads never alias, don't bother tracking that.
        if (LIA && LIB)
          continue;
        if (!BM.bitTest(J, I)) {
          ScopeSetType &CliquesI = ScopeSetVec[I];
          ScopeSetType &CliquesJ = ScopeSetVec[J];
          NoAliasMDs[J].insert(CliquesI.begin(), CliquesI.end());
          NoAliasMDs[I].insert(CliquesJ.begin(), CliquesJ.end());
        }
      }
    }

    // Now attach the noalias scope lists that we computed, to the actual
    // insts.
    for (unsigned I = 0; I < Insns.size(); ++I) {
      ScopeSetType &NoAliasMDI = NoAliasMDs[I];
      if (!NoAliasMDI.empty()) {
        // Make a list.
        AppendMD(Insns[I], LLVMContext::MD_noalias, NoAliasMDI.getArrayRef());
      }
    }
  };

  // In order to reduce the number of metadata, the compiler calculates
  // the number of cliques in the alias matrix.
  auto calculateClique = [&](SmallVectorImpl<Instruction *> &Insns,
                             BitMatrix &BM) {
    unsigned N = Insns.size();
    std::vector<BitVector> CliqueSet;
    BitMatrix CliquedMatrix;
    CliquedMatrix.resize(N);
    BitVector Clique(N);

    for (int I = N - 1; I >= 0; --I) {
      for (int J = I - 1; J >= 0; --J) {
        if (BM.bitTest(I, J) && !CliquedMatrix.bitTest(I, J)) {
          Clique.reset();
          for (int K = J; K >= 0; --K) {
            if (BM.bitTest(I, K))
              Clique.set(K);
          }
          formClique(Clique, J, I, CliquedMatrix);
          CliqueSet.push_back(Clique);
        }
      }
    }

    genMDForCliques(CliqueSet, Insns, BM);
  };

  calculateClique(MemoryInsns, BM);
}
#endif // INTEL_CUSTOMIZATION

/// Insert a call to the region entry directive for \p DirID before \p
/// InsertBefore.
static CallInst *CreateBeginDirectiveCall(OMP_DIRECTIVES DirID,
                                          Instruction *InsertBefore,
                                          const Twine &CallName = "") {
  assert(InsertBefore && "Null insertion point.");
  assert(VPOAnalysisUtils::isBeginDirective(DirID) &&
         "ID is not for an entry directive");

  Function *DirIntrin = Intrinsic::getDeclaration(
      InsertBefore->getModule(), Intrinsic::directive_region_entry);

  OperandBundleDef OpBundle(IntrinsicUtils::getDirectiveString(DirID).str(),
                            {});

  return IRBuilder<>(InsertBefore)
      .CreateCall(DirIntrin, /*Args=*/{}, {OpBundle}, CallName);
}

/// Emit a call to the region exit directive corresponding to the entry
/// directive \p BeginDirective, before \p InsertBefore.
static CallInst *CreateEndDirectiveCall(CallInst *BeginDirective,
                                        Instruction *InsertBefore,
                                        const Twine &CallName = "") {
  assert(BeginDirective && VPOAnalysisUtils::isBeginDirective(BeginDirective) &&
         "Invalid begin directive");
  assert(InsertBefore && "Null insertion point.");

  Function *DirIntrin = Intrinsic::getDeclaration(
      InsertBefore->getModule(), Intrinsic::directive_region_exit);

  int EndDirID = VPOAnalysisUtils::getMatchingEndDirective(
      VPOAnalysisUtils::getDirectiveID(BeginDirective));

  OperandBundleDef OpBundle(IntrinsicUtils::getDirectiveString(EndDirID).str(),
                            {});

  return IRBuilder<>(InsertBefore)
      .CreateCall(DirIntrin, /*Args=*/{BeginDirective}, {OpBundle}, CallName);
}

/// Obtain the first basic block of the loop body (LoopBodyBB).
/// There may be two patterns, rotated and non-rotated loops.
/// The non-rotated loop starts from the block like below
/// - \code
///
///   LoopHeaderBB:
///    %iv = load i32, ptr %.omp.iv
///    %ub = load i32, ptr %.omp.ub
///    %cmp4 = icmp sle i32 %14, %15
///    br i1 %cmp4, %LoopBodyBB, %LoopEnd
///
/// \endcode
/// The rotated loops usually don't have the compare instruciton and the
/// branch is unconditional.
//
static BasicBlock *getFirstBodyBBForLoop(Loop *L) {
  auto *LpHeader = L->getHeader();
  BranchInst *BI = dyn_cast<BranchInst>(LpHeader->getTerminator());
  assert(BI && "No Branch found for loop.");

  if (BI->isUnconditional())
    return LpHeader;

  auto *LoopBodyBB = BI->getSuccessor(0);
  auto *ExitBB = BI->getSuccessor(1);
  if (L->contains(ExitBB)) {
    std::swap(LoopBodyBB, ExitBB);
    // Check for conditional branch in the rotated loop
    if (L->contains(ExitBB))
      return LpHeader;
  }
  assert(LoopBodyBB && "Loop is not expected to be empty!");

  return LoopBodyBB;
}

/// Return the guard directive that is used to prohibit memory motion outside
/// the given loop. If guard isn't found, then insert it in the following format
/// - \code
///
///   LoopHeaderBB:
///    <Loop header PHIs>
///    %g = call @llvm.directive.region.entry(); [ DIR.VPO.GUARD.MEM.MOTION() ]
///
///   ...
///
///   LoopLatchBB:
///    ...
///    call @llvm.directive.region.exit(%g); [ DIR.VPO.END.GUARD.MEM.MOTION() ]
///    %backedge.cmp = icmp ...
///
/// \endcode
// TODO: Region being created below is not explicitly in
// single-entry-single-exit (SESE) format. We currently haven't run into any
// use-case/issue, but if the need arises in future, then consider splitting the
// loop's header and latch BBs to insert the region's directives in standlone
// BBs. This would ensure that the new region is in SESE format.
CallInst *VPOUtils::getOrCreateLoopGuardForMemMotion(Loop *L) {
  auto *LpHeader = L->getHeader();
  Instruction *FirstNonPHIInst = LpHeader->getFirstNonPHI();
  // Guard directive is expected to be the first non-PHI instruction in loop's
  // header block.
  int DirID = VPOAnalysisUtils::getRegionDirectiveID(FirstNonPHIInst);

  // Guard already exists for this loop.
  if (DirID == DIR_VPO_GUARD_MEM_MOTION)
    return cast<CallInst>(FirstNonPHIInst);

  // Create @llvm.directive.region.entry(); [ DIR.VPO.GUARD.MEM.MOTION() ]
  CallInst *GuardBegin = CreateBeginDirectiveCall(
      DIR_VPO_GUARD_MEM_MOTION, FirstNonPHIInst, "guard.start");

  // Create @llvm.directive.region.exit(); [ DIR.VPO.END.GUARD.MEM.MOTION() ]
  auto *LatchCondInst = L->getLatchCmpInst();
  assert(LatchCondInst && "Latch condition not found for loop.");
  CreateEndDirectiveCall(GuardBegin, LatchCondInst);

  return GuardBegin;
}

/// Return the starting guard directives (DIR.VPO.GUARD.MEM.MOTION) created
/// for the loop \p L to prohibit memory motion. Input and scan phase are
/// enclosed in the guard directives:
/// - \code
///
///  LoopHeaderBB:
///    %iv = load i32, ptr %.omp.iv
///    %ub = load i32, ptr %.omp.ub
///    %cmp4 = icmp sle i32 %14, %15
///    br i1 %cmp4, %LoopBody, %LoopEnd
///
///  LoopBody:
///    %g1 = call @llvm.directive.region.entry() [ DIR.VPO.GUARD.MEM.MOTION() ]
///
///    ...
///
///    call @llvm.directive.region.exit(%g1) [ DIR.VPO.END.GUARD.MEM.MOTION() ]
///    %s = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), ... ]
///    call void @llvm.directive.region.exit(token %s) [ "DIR.OMP.END.SCAN"() ]
///    %g2 = call @llvm.directive.region.entry() [ DIR.VPO.GUARD.MEM.MOTION() ]
///
///    ...
///
///    call @llvm.directive.region.exit(%g2) [ DIR.VPO.END.GUARD.MEM.MOTION() ]
///    br %LoopLatchBB
///
///  LoopLatchBB:
///    %iv = load i32, i32* %.omp.iv
///    %iv.next = add nsw i32 %iv, 1
///    store i32 %iv.next, i32* %.omp.iv
///
/// \endcode
std::pair<CallInst *, CallInst *>
VPOUtils::createInscanLoopGuardForMemMotion(Loop *L) {
  // Guard directive is expected to be the first non-PHI instruction in loop's
  // body block.
  Instruction *BeginInsertPt = getFirstBodyBBForLoop(L)->getFirstNonPHI();

  // Guard already exists for this loop.
  assert(VPOAnalysisUtils::getRegionDirectiveID(BeginInsertPt) !=
             DIR_VPO_GUARD_MEM_MOTION &&
         "Do not expect Guard regions present!");

  // Locate the separating pragma Begin and End.
  Instruction *ScanBegin = nullptr;
  Instruction *ScanEnd = nullptr;
  for (auto *BB : L->blocks()) {
    for (auto &I : *BB) {
      int DirID = VPOAnalysisUtils::getRegionDirectiveID(&I);
      if (DirID == DIR_OMP_SCAN) {
        ScanBegin = &I;
        assert(I.hasOneUser() && "Expect only END directive user for BEGIN");
        ScanEnd = cast<Instruction>(*I.user_begin());
        break;
      }
    }
  }

  // Create the first region.
  CallInst *FirstGuardBegin = CreateBeginDirectiveCall(
      DIR_VPO_GUARD_MEM_MOTION, BeginInsertPt, "pre.scan.guard.start");
  CreateEndDirectiveCall(FirstGuardBegin, ScanBegin);

  // Create the second region.
  CallInst *SecondGuardBegin = CreateBeginDirectiveCall(
      DIR_VPO_GUARD_MEM_MOTION, ScanEnd->getParent()->getTerminator(),
      "post.scan.guard.start");
  auto *EndInstPoint =
      L->getLoopLatch()->getSinglePredecessor()->getTerminator();
  CreateEndDirectiveCall(SecondGuardBegin, EndInstPoint);

  return std::make_pair(FirstGuardBegin, SecondGuardBegin);
}
#endif // INTEL_COLLAB
