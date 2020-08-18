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
      I->replaceAllUsesWith(llvm::ConstantTokenNone::get(C));
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
  auto NewCallInst =
      Builder.CreateMaskedGather(VecPtr, Align(Alignment), Mask, PassThru);
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
  auto NewCallInst = Builder.CreateMaskedLoad(VecPtr, assumeAligned(Alignment),
                                              Mask, PassThru);
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
// the source. The size of the memcpy is the size of destination.
// The compiler will insert the typecast if the type of source or destination
// does not match with the type i8.
// One example of the output is as follows.
//   call void @llvm.memcpy.p0i8.p0i8.i32(i8* bitcast (i32* @a to i8*), i8* %2, i32 4, i32 4, i1 false)
CallInst *VPOUtils::genMemcpy(Value *D, Value *S, const DataLayout &DL,
                              unsigned Align, BasicBlock *BB) {
  return genMemcpy(D, S, DL, Align, BB->getTerminator());
}

// Generates a memcpy call before the given instruction InsertPt.
// The value D represents the destination while the value S represents
// the source. The size of the memcpy is the size of destination.
// The compiler will insert the typecast if the type of source or destination
// does not match with the type i8.
CallInst *VPOUtils::genMemcpy(Value *D, Value *S, const DataLayout &DL,
                              unsigned Align, Instruction *InsertPt) {
  IRBuilder<> MemcpyBuilder(InsertPt);

  Value *Dest, *Src, *Size;

  // The first two arguments of the memcpy expects the i8* operands.
  // The instruction bitcast is introduced if the incoming src or dest
  // operand in not in i8 type.
  if (D->getType() !=
      Type::getInt8PtrTy(InsertPt->getContext())) {
    Dest = MemcpyBuilder.CreatePointerCast(D, MemcpyBuilder.getInt8PtrTy());
    Src = MemcpyBuilder.CreatePointerCast(S, MemcpyBuilder.getInt8PtrTy());
  }
  else {
    Dest = D;
    Src = S;
  }
  // For 32/64 bit architecture, the size and alignment should be
  // set accordingly.
  if (DL.getIntPtrType(MemcpyBuilder.getInt8PtrTy())->getIntegerBitWidth() ==
      64)
    Size = MemcpyBuilder.getInt64(
        DL.getTypeAllocSize(D->getType()->getPointerElementType()));
  else
    Size = MemcpyBuilder.getInt32(
        DL.getTypeAllocSize(D->getType()->getPointerElementType()));

  AllocaInst *AI = dyn_cast<AllocaInst>(D);
  if (AI && AI->isArrayAllocation())
    Size = MemcpyBuilder.CreateMul(Size, AI->getArraySize());

  return MemcpyBuilder.CreateMemCpy(Dest, MaybeAlign(Align), Src,
                                    MaybeAlign(Align), Size);
}

// Generates a memset call before the given instruction InsertPt.
// The value P represents the pointer to the block of memory to fill while the
// value V represents the value to be set. The size of the memset is the size of
// pointer to the memory block. The compiler will insert the typecast if the
// type of pointer and value does not match with the type i8.
CallInst *VPOUtils::genMemset(Value *P, Value *V, const DataLayout &DL,
                              unsigned Align, IRBuilder<> &MemsetBuilder) {
  Value *Ptr, *Size;

  // The first argument of the memset expect the i8* operand.
  // The instruction bitcast is introduced if the incoming pointer operand in
  // not in i8 type.
  if (P->getType() != Type::getInt8PtrTy(MemsetBuilder.getContext()))
    Ptr = MemsetBuilder.CreatePointerCast(P, MemsetBuilder.getInt8PtrTy());
  else
    Ptr = P;

  assert((V->getType() == Type::getInt8Ty(MemsetBuilder.getContext())) &&
         "Unsupported type for value in genMemset");

  // For 32/64 bit architecture, the size and alignment should be
  // set accordingly.
  if (DL.getIntPtrType(MemsetBuilder.getInt8PtrTy())->getIntegerBitWidth() ==
      64) {
    Size = MemsetBuilder.getInt64(
        DL.getTypeAllocSize(P->getType()->getPointerElementType()));
  } else {
    Size = MemsetBuilder.getInt32(
        DL.getTypeAllocSize(P->getType()->getPointerElementType()));
  }

  AllocaInst *AI = dyn_cast<AllocaInst>(P);
  if (AI && AI->isArrayAllocation())
    Size = MemsetBuilder.CreateMul(Size, AI->getArraySize());

  return MemsetBuilder.CreateMemSet(Ptr, V, Size, MaybeAlign(Align));
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

// "Privatizes" an Instruction by adding it to a supported entry
// directive clause.
// If the Instruction is already used in a directive, nothing is done.
// BlockPos: The first dominating entry directive over this block is
// chosen.
// SimdOnly: Process SIMD directives only. This must be true inside VPO, as
// non-SIMD directives cannot be modified during VPO.
// Return false if no directive was found.
bool VPOUtils::addPrivateToEnclosingRegion(Instruction *I, BasicBlock *BlockPos,
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

  // Search upwards through the dominator tree until we find a block
  // that supports the private clause.
  auto *DomNode = DT[BlockPos];
  assert(DomNode && "Dominator tree not built for this function.");
  auto *IDom = DomNode->getIDom();
  while (IDom) {
    auto *IDomBlock = IDom->getBlock();
    if (VPOAnalysisUtils::supportsPrivateClause(IDomBlock)) {
      auto *II = cast<IntrinsicInst>(&(IDomBlock->front()));
      if (II &&
          (!SimdOnly || VPOAnalysisUtils::getDirectiveID(II) == DIR_OMP_SIMD)) {
        auto *Repl =
            VPOUtils::addOperandBundlesInCall(II, {{"QUAL.OMP.PRIVATE", {I}}});
        if (Repl) {
          LLVM_DEBUG(dbgs() << "Added private for " << I->getName()
                            << " to: " << *Repl << "\n");
        }
        return true;
      }
    }
    IDom = DT[IDomBlock]->getIDom();
  }
  return false;
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
      LoadInst *LIA = dyn_cast<LoadInst>(Insns[I]);
      StoreInst *STA = dyn_cast<StoreInst>(Insns[I]);
      assert((LIA || STA) && "Expect load/store instruction");
      for (int J = I + 1; J < N; J++) {
        LoadInst *LIB = dyn_cast<LoadInst>(Insns[J]);
        StoreInst *STB = dyn_cast<StoreInst>(Insns[J]);
        assert((LIB || STB) && "Expect load/store instruction");
        if (LIA && LIB)
          continue;
        Value *V1, *V2;
        V1 = LIA ? LIA->getPointerOperand() : STA->getPointerOperand();
        V2 = LIB ? LIB->getPointerOperand() : STB->getPointerOperand();
        // If the size is UnknownSize, the alias result is valid for
        // loop carried case. We have to make conservative assumption
        // since the information may be used by the loop optimizations.
        if (!AA->isNoAlias(V1, MemoryLocation::UnknownSize, V2,
                           MemoryLocation::UnknownSize))
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
        generateScopeMD(Ins, M);
      } else if (CliquesI.size() == 1) {
        // 1 clique, attach the MDNode directly to the inst instead of
        // creating a 1-element list.
        generateScopeMD(Ins, CliquesI[0]);
      } else {
        // Multiple cliques, create a list of clique MDNodes.
        // !3 = distinct !{!3, !99, "OMPAliasScope"}
        // !4 = distinct !{!4, !99, "OMPAliasScope"}
        // !8 = {!3, !4}
        MDNode *M = MDNode::get(C, CliquesI.getArrayRef());
        generateScopeMD(Ins, M);
      }
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
        if (NoAliasMDI.size() == 1) {
          generateNoAliasMD(Insns[I], NoAliasMDI[0]);
        } else {
          // Multiple scopes, make a list.
          MDNode *M = MDNode::get(C, NoAliasMDI.getArrayRef());
          generateNoAliasMD(Insns[I], M);
        }
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
