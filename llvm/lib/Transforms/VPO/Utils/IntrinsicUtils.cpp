#if INTEL_COLLAB
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

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<unsigned>
    RefsThreshold("refs-threshold", cl::Hidden, cl::init(500),
                    cl::desc("The number of references threshold"));

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

CallInst *VPOUtils::createMaskedGatherCall(Value *VecPtr,
                                           IRBuilder<> &Builder,
                                           unsigned Alignment,
                                           Value *Mask,
                                           Value *PassThru) {
  auto *VecPtrTy = cast<VectorType>(VecPtr->getType());
  auto *PtrTy = cast<PointerType>(VecPtrTy->getElementType());
  ElementCount NumElts = VecPtrTy->getElementCount();
  auto *Ty = PtrTy->getElementType();
  auto *VecTy = VectorType::get(Ty, NumElts);

  auto NewCallInst = Builder.CreateMaskedGather(VecTy, VecPtr, Align(Alignment),
                                                Mask, PassThru);
  return NewCallInst;
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

CallInst *VPOUtils::createMaskedLoadCall(Value *VecPtr,
                                         IRBuilder<> &Builder,
                                         unsigned Alignment,
                                         Value *Mask,
                                         Value *PassThru) {
  auto *VecTy = VecPtr->getType()->getPointerElementType();
  auto NewCallInst = Builder.CreateMaskedLoad(
      VecTy, VecPtr, assumeAligned(Alignment), Mask, PassThru);
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

// Generates a memcpy call at the end of the given basic block BB.
// The value D represents the destination while the value S represents
// the source. The size of the memcpy is specified as Size.
// The compiler will insert the typecast if the type of source or destination
// does not match with the type i8.
// One example of the output is as follows.
//   call void @llvm.memcpy.p0i8.p0i8.i32(i8* bitcast (i32* @a to i8*), i8* %2, i32 4, i32 4, i1 false)
CallInst *VPOUtils::genMemcpy(Value *D, Value *S, uint64_t Size,
                              unsigned Align, BasicBlock *BB) {
  IRBuilder<> MemcpyBuilder(BB->getTerminator());
  return genMemcpy(D, S, Size, Align, MemcpyBuilder);
}

// Generates a memcpy call using MemcpyBuilder.
// The value D represents the destination while the value S represents
// the source. The size of the memcpy is specified as Size.
// The compiler will insert the typecast if the type of source or destination
// does not match with the type i8.
CallInst *VPOUtils::genMemcpy(Value *D, Value *S, uint64_t Size,
                              unsigned Align, IRBuilder<> &MemcpyBuilder) {
  Value *Dest = D;
  Value *Src = S;
#if !ENABLE_OPAQUEPOINTER
  // The first two arguments of the memcpy expects the i8* operands.
  // The instruction bitcast is introduced if the incoming src or dest
  // operand in not in i8 type.
  Type *NewDestTy = MemcpyBuilder.getInt8PtrTy(
      cast<PointerType>(D->getType())->getAddressSpace());
  if (D->getType() != NewDestTy)
    Dest = MemcpyBuilder.CreatePointerCast(D, NewDestTy);

  Type *NewSrcTy = MemcpyBuilder.getInt8PtrTy(
      cast<PointerType>(S->getType())->getAddressSpace());
  if (S->getType() != NewSrcTy)
    Src = MemcpyBuilder.CreatePointerCast(S, NewSrcTy);
#endif // ENABLE_OPAQUEPOINTER

  // For 32/64 bit architecture, the size and alignment should be
  // set accordingly.
  Function *F = MemcpyBuilder.GetInsertBlock()->getParent();
  Type *SizeTTy = GeneralUtils::getSizeTTy(F);
  unsigned SizeTBitWidth = SizeTTy->getIntegerBitWidth();
  Value *SizeVal = MemcpyBuilder.getIntN(SizeTBitWidth, Size);

  AllocaInst *AI = dyn_cast<AllocaInst>(D);
  if (AI && AI->isArrayAllocation())
    SizeVal = MemcpyBuilder.CreateMul(
        SizeVal, MemcpyBuilder.CreateZExtOrTrunc(AI->getArraySize(),
                                                 SizeVal->getType()));

  return MemcpyBuilder.CreateMemCpy(Dest, MaybeAlign(Align), Src,
                                    MaybeAlign(Align), SizeVal);
}

// Generates a memset call using MemsetBuilder.
// The value P represents the pointer to the block of memory to fill while the
// value V represents the value to be set. The size of the memset is specified
// as Size. The compiler will insert the typecast if the
// type of pointer and value does not match with the type i8.
CallInst *VPOUtils::genMemset(Value *P, Value *V, uint64_t Size,
                              unsigned Align, IRBuilder<> &MemsetBuilder) {
  Value *Ptr = P;

#if !ENABLE_OPAQUEPOINTER
  // The first argument of the memset expect the i8* operand.
  // The instruction bitcast is introduced if the incoming pointer operand in
  // not in i8 type.
  if (P->getType() != Type::getInt8PtrTy(MemsetBuilder.getContext()))
    Ptr = MemsetBuilder.CreatePointerCast(P, MemsetBuilder.getInt8PtrTy());

  assert((V->getType() == Type::getInt8Ty(MemsetBuilder.getContext())) &&
         "Unsupported type for value in genMemset");
#endif // !ENABLE_OPAQUEPOINTER

  // For 32/64 bit architecture, the size and alignment should be
  // set accordingly.
  Function *F = MemsetBuilder.GetInsertBlock()->getParent();
  Type *SizeTTy = GeneralUtils::getSizeTTy(F);
  unsigned SizeTBitWidth = SizeTTy->getIntegerBitWidth();
  Value *SizeVal = MemsetBuilder.getIntN(SizeTBitWidth, Size);

  AllocaInst *AI = dyn_cast<AllocaInst>(P);
  if (AI && AI->isArrayAllocation())
    SizeVal = MemsetBuilder.CreateMul(SizeVal, AI->getArraySize());

  return MemsetBuilder.CreateMemSet(Ptr, V, SizeVal, MaybeAlign(Align));
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
                                           DominatorTree &DT, bool SimdOnly) {
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
        auto *Repl = VPOUtils::addOperandBundlesInCall(
            cast<CallInst>(Begin), {{"QUAL.OMP.PRIVATE", {I}}});
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
    unsigned RecLen;

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

  // Concatenate is expensive and we try not to call these two functions
  // on the same instruction twice.

  // Set alias_scope MetaData for 'I' using new scope. Preserve existing
  // scope MD.
  auto generateScopeMD = [&](Instruction *I, Metadata *NewScope) {
    MDNode* SM = I->getMetadata(LLVMContext::MD_alias_scope);
    MDNode *SM1 = MDNode::concatenate(SM, cast<MDNode>(NewScope));
    I->setMetadata(LLVMContext::MD_alias_scope, SM1);
  };

  // Set noAlias MetaData for 'I' using new scope. Preserve existing
  // noalias MD.
  auto generateNoAliasMD = [](Instruction *I, Metadata *NewScope) {
    MDNode* AM = I->getMetadata(LLVMContext::MD_noalias);
    MDNode *AM1 = MDNode::concatenate(AM, cast<MDNode>(NewScope));
    I->setMetadata(LLVMContext::MD_noalias, AM1);
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
      MDNode *M = MDNode::get(C, CliquesI.getArrayRef());
      generateScopeMD(Ins, M);
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
        MDNode *M = MDNode::get(C, NoAliasMDI.getArrayRef());
        generateNoAliasMD(Insns[I], M);
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
#endif // INTEL_COLLAB
