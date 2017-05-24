//===-- Intel_StdContainerOpt.cpp - Std Container Optimization implementation
//-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the pass which generates the std.container.ptr
// and std.container.ptr.iter metatdata based on the input std.container
// intrinsic. It also cleans up the intrinsic after that.
//
// The pass begins by visiting all intrinsic instructions to find calls to the
// intel.std.container.ptr and intel.std.container.ptr.iter intrinsics.  For
// each of these intrinsics, we record the load instruction whose value is
// the argument to the intrinsic and then erase the intrinsic.
//
// Once all of the StdContainer load instructions have been recorded, we
// build an NxN bit matrix (where N is the number of load instructions for
// the intrinsic type -- ptr or iter) such that bit(i, j) is set if Insns[i]
// and Insns[j] either may alias or must alias.
//
// This bit matrix is then reduced to a set of "cliques" where each clique
// is a unique set of load instructions which all may alias with one another.
// The clique sets are used to generate a set of metadata nodes for the load
// instructions where metadata attribute decribing the cliques to which it
// belongs.
//
// Finally, we walk the use chain for each load instruction, propogating the
// clique sets to load, store, GEP, PHINode and IntToPtr instructions and
// their users. 
//
// Here is one example.
//
// entry:
//   %0 = call i32** @llvm.intel.std.container.ptr.iter.p0p0i32(i32** nonnull getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector2, i64 0, i32 0, i32 0, i32 0)) #2
//  %1 = load i32*, i32** %0, align 8, !tbaa !10
//  br label %for.cond
//
// for.cond:                                         ; preds = %for.inc19, %entry
//  %ita.sroa.0.0 = phi i32* [ %1, %entry ], [ %incdec.ptr.i, %for.inc19 ]
//  %2 = call i32** @llvm.intel.std.container.ptr.iter.p0p0i32(i32** nonnull getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector2, i64 0, i32 0, i32 0, i32 1)) #2
//  %3 = load i32*, i32** %2, align 8, !tbaa !10
//  %cmp.i33 = icmp eq i32* %ita.sroa.0.0, %3
//  br i1 %cmp.i33, label %for.end21, label %for.body
//
//for.body:                                         ; preds = %for.cond
//  %4 = call i32** @llvm.intel.std.container.ptr.iter.p0p0i32(i32** nonnull getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector, i64 0, i32 0, i32 0, i32 0)) #2
//  %5 = load i32*, i32** %4, align 8, !tbaa !10
//  br label %for.cond8
//
//for.cond8:                                        ; preds = %for.body13, %for.body
//  %it.sroa.0.0 = phi i32* [ %5, %for.body ], [ %incdec.ptr.i26, %for.body13 ]
//  %6 = call i32** @llvm.intel.std.container.ptr.iter.p0p0i32(i32** nonnull getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector, i64 0, i32 0, i32 0, i32 1)) #2
//  %7 = load i32*, i32** %6, align 8, !tbaa !10
//  %cmp.i = icmp eq i32* %it.sroa.0.0, %7
//  br i1 %cmp.i, label %for.inc19, label %for.body13
//
//for.body13:                                       ; preds = %for.cond8
//  %8 = load i32, i32* %it.sroa.0.0, align 4, !tbaa !11
//  %9 = load i32, i32* %ita.sroa.0.0, align 4, !tbaa !11
//  %mul = mul nsw i32 %9, %8
//  %add = add nsw i32 %8, %mul
//  store i32 %add, i32* %it.sroa.0.0, align 4, !tbaa !11
//  %incdec.ptr.i26 = getelementptr inbounds i32, i32* %it.sroa.0.0, i64 1
//  br label %for.cond8
//
//for.inc19:                                        ; preds = %for.cond8
//  %incdec.ptr.i = getelementptr inbounds i32, i32* %ita.sroa.0.0, i64 1
//  br label %for.cond
//
//for.end21:                                        ; preds = %for.cond
//  ret void
//
//  *** IR Dump After StdContainerOpt ***
//
//entry:
//  %0 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector2, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !7, !std.container.ptr.iter !8
//  br label %for.cond
//
//for.cond:                                         ; preds = %for.inc19, %entry
//  %ita.sroa.0.0 = phi i32* [ %0, %entry ], [ %incdec.ptr.i, %for.inc19 ]
//  %1 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector2, i64 0, i32 0, i32 0, i32 1), align 8, !tbaa !7, !std.container.ptr.iter !8
//  %cmp.i33 = icmp eq i32* %ita.sroa.0.0, %1
//  br i1 %cmp.i33, label %for.end21, label %for.body
//
//for.body:                                         ; preds = %for.cond
//  %2 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !7, !std.container.ptr.iter !9
//  br label %for.cond8
//
//for.cond8:                                        ; preds = %for.body13, %for.body
//  %it.sroa.0.0 = phi i32* [ %2, %for.body ], [ %incdec.ptr.i26, %for.body13 ]
//  %3 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector, i64 0, i32 0, i32 0, i32 1), align 8, !tbaa !7, !std.container.ptr.iter !9
//  %cmp.i = icmp eq i32* %it.sroa.0.0, %3
//  br i1 %cmp.i, label %for.inc19, label %for.body13
//
//for.body13:                                       ; preds = %for.cond8
//  %4 = load i32, i32* %it.sroa.0.0, align 4, !tbaa !10, !std.container.ptr.iter !9
//  %5 = load i32, i32* %ita.sroa.0.0, align 4, !tbaa !10, !std.container.ptr.iter !8
//  %mul = mul nsw i32 %5, %4
//  %add = add nsw i32 %4, %mul
//  store i32 %add, i32* %it.sroa.0.0, align 4, !tbaa !10, !std.container.ptr.iter !9
//  %incdec.ptr.i26 = getelementptr inbounds i32, i32* %it.sroa.0.0, i64 1
//  br label %for.cond8
//
//for.inc19:                                        ; preds = %for.cond8
//  %incdec.ptr.i = getelementptr inbounds i32, i32* %ita.sroa.0.0, i64 1
//  br label %for.cond
//
//for.end21:                                        ; preds = %for.cond
//  ret void
//
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_StdContainerOpt.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "std-container-opt"

namespace {
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
  bool bitTest(int rowno, int colno) { return BV.test(rowno * RecLen + colno); }
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

struct StdContainerOpt : public FunctionPass,
                         public InstVisitor<StdContainerOpt> {
public:
  static char ID;
  StdContainerOpt() : FunctionPass(ID) {
    initializeStdContainerOptPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F);
  void visitInstruction(Instruction &I) { return; }
  void visitIntrinsicInst(IntrinsicInst &II);
  StringRef getPassName() const override { return "StdContainerOpt"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
  void setStdContainPtrMD();
  void initAliasMatrix(std::vector<Instruction *> &Insns, unsigned KindID);
  void calculateClique(std::vector<Instruction *> &Insns, unsigned KindID);
  void formClique(BitVector &C, int Col, int Row, class BitMatrix &CM);
  void genMDForCliques(std::vector<BitVector> &CliqueSet,
                       std::vector<Instruction *> &Insns, unsigned KindID);
  void propMD(std::vector<Instruction *> &Insns, unsigned KindID);
  void propMDForInsn(Value *V, unsigned KindID, MDNode *MD,
                     SmallPtrSetImpl<const PHINode *> &PhiUsers);

private:
  friend class InstVisitor<StdContainerOpt>;
  std::vector<Instruction *> ContainerPtrIterInsns;
  std::vector<Instruction *> ContainerPtrInsns;
  BitMatrix BM;
  AliasAnalysis *AA;
  const DataLayout *DL;
};
}
char StdContainerOpt::ID = 0;
INITIALIZE_PASS_BEGIN(StdContainerOpt, "std-container-opt",
                      "Propagate the TbaaMD through intrinsic", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(StdContainerOpt, "std-container-opt",
                    "Propagate the TbaaMD through intrinsic", false, false)

FunctionPass *llvm::createStdContainerOptPass() {
  return new StdContainerOpt();
}

PreservedAnalyses StdContainerOptPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  auto PA = PreservedAnalyses();
  PA.preserve<GlobalsAA>();
  return PA;
}

bool StdContainerOpt::runOnFunction(Function &F) {
  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DL = &(F.getParent()->getDataLayout());
  ContainerPtrIterInsns.clear();
  ContainerPtrInsns.clear();

  for (BasicBlock &BB : F) {
    for (auto II = BB.begin(), IE = BB.end(); II != IE;) {
      // The call to visit() may erase the instruction, so we need to
      // increment the iterator here before we visit.
      Instruction &I = *II;
      ++II;
      visit(&I);
    }
  }

  setStdContainPtrMD();

  return false;
}

// The metadata std.container.ptr and std.cotnainer.ptr.iter is propagated
// along the refernce SSA chain.
void
StdContainerOpt::propMDForInsn(Value *V, unsigned KindID, MDNode *MD,
                               SmallPtrSetImpl<const PHINode *> &PhiUsers) {
  for (Use &U : V->uses()) {
    User *UR = U.getUser();
    if (Instruction *I = dyn_cast<Instruction>(UR)) {
      if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
        LI->setMetadata(KindID, MD);
        propMDForInsn(LI, KindID, MD, PhiUsers);
      } else if (StoreInst *SI = dyn_cast<StoreInst>(I))
        SI->setMetadata(KindID, MD);
      else if (isa<GetElementPtrInst>(I))
        propMDForInsn(I, KindID, MD, PhiUsers);
      else if (PHINode *PN = dyn_cast<PHINode>(I)) {
        if (PhiUsers.insert(PN).second)
          propMDForInsn(I, KindID, MD, PhiUsers);
      }
      else if (isa<PtrToIntInst>(I)) 
        propMDForInsn(I, KindID, MD, PhiUsers);
      else if (isa<IntToPtrInst>(I))
        propMDForInsn(I, KindID, MD, PhiUsers);
    }
  }
}

// Propagate the std.container metadata for the instructions in the 
// given set.
void StdContainerOpt::propMD(std::vector<Instruction *> &Insns,
                             unsigned KindID) {
  for (unsigned I = 0; I < Insns.size(); ++I) {
    SmallPtrSet<const PHINode *, 16> PhiUsers;
    MDNode *MD = Insns[I]->getMetadata(KindID);
    propMDForInsn(Insns[I], KindID, MD, PhiUsers);
  }
}

// It generates a unique metadata ID for every clique.
void StdContainerOpt::genMDForCliques(std::vector<BitVector> &CliqueSet,
                                      std::vector<Instruction *> &Insns,
                                      unsigned KindID) {
  if (Insns.size() == 0)
    return;
  LLVMContext &C = Insns[0]->getContext();

  unsigned Idx = 0;

  DenseMap<unsigned, std::vector<Metadata*>> InstrToCliqueSetMap(Insns.size());

  for (const BitVector &Bv : CliqueSet) {
    Metadata *CliqueIdMD = 
      ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C), Idx));
    for (unsigned I = 0; I < Bv.size(); ++I) {
      if (Bv.test(I))
        InstrToCliqueSetMap[I].push_back(CliqueIdMD);
    }
    ++Idx;
  }
  for (unsigned I = 0; I < Insns.size(); ++I) {
    const std::vector<Metadata*> &InstrCliques = InstrToCliqueSetMap[I];
    if (InstrCliques.empty()) {
      Metadata *MDs[] = {
          ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C), Idx))
      };
      MDNode *M = MDNode::get(C, MDs);
      Insns[I]->setMetadata(KindID, M);
      ++Idx;
    } else {
      MDNode *M = MDNode::get(C, InstrCliques);
      Insns[I]->setMetadata(KindID, M);
    }
  }
}

// In order to reduce the number of metdata, the compiler calculates
// the number of cliques in the alias matrix.
void StdContainerOpt::calculateClique(std::vector<Instruction *> &Insns,
                                      unsigned KindID) {
  unsigned N = Insns.size();
  std::vector<BitVector> CliqueSet;
  BitMatrix CliquedMatrix;
  CliquedMatrix.resize(N);
  BitVector Clique(N);

  for (int I = N - 1; I >= 0; --I) {
    for (int J = I - 1; J >= 0; --J) {
      if (BM.bitTest(I, J) && !CliquedMatrix.bitTest(I, J)) {
        // Clique is effectively local to this scope, but declaring it
        // in the outer scope avoids having to reallocate the buffer,
        // which is the same size every time through the loop.
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
  genMDForCliques(CliqueSet, Insns, KindID);
  propMD(Insns, KindID);
}

// Form a clique if the references are proved to be aliased with
// each other.
void StdContainerOpt::formClique(BitVector &C, int Col, int Row,
                                 class BitMatrix &CM) {
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
}

void StdContainerOpt::setStdContainPtrMD() {
  initAliasMatrix(ContainerPtrIterInsns,
                  LLVMContext::MD_std_container_ptr_iter);
  calculateClique(ContainerPtrIterInsns,
                  LLVMContext::MD_std_container_ptr_iter);
  initAliasMatrix(ContainerPtrInsns, LLVMContext::MD_std_container_ptr);
  calculateClique(ContainerPtrInsns, LLVMContext::MD_std_container_ptr);
}

// Build an alias matrix based on the baiscAA for the load/store's 
// pointers if the load/stores are relevant to the std.container intrinsic.
void StdContainerOpt::initAliasMatrix(std::vector<Instruction *> &Insns,
                                      unsigned KindID) {
  int N = Insns.size();
  BM.clear();
  BM.resize(N);
  for (int I = 0; I < N; I++) {
    LoadInst *LIA = dyn_cast<LoadInst>(Insns[I]);
    assert(LIA && "Expected Load Instruction");
    for (int J = I + 1; J < N; J++) {
      LoadInst *LIB = dyn_cast<LoadInst>(Insns[J]);
      assert(LIB && "Expected Load Instruction");
      Value *V1, *V2;
      V1 = LIA->getPointerOperand();
      V2 = LIB->getPointerOperand();
      if (KindID == LLVMContext::MD_std_container_ptr_iter) {
        if (!AA->isNoAlias(V1, V2))
          BM.bitSet(J, I);
      } else {
        uint64_t I1Size = MemoryLocation::UnknownSize;
        Type *I1ElTy = cast<PointerType>(V1->getType())->getElementType();
        if (I1ElTy->isSized())
          I1Size = DL->getTypeStoreSize(I1ElTy);
        uint64_t I2Size = MemoryLocation::UnknownSize;
        Type *I2ElTy = cast<PointerType>(V2->getType())->getElementType();
        if (I2ElTy->isSized())
          I2Size = DL->getTypeStoreSize(I2ElTy);
        if (!AA->isNoAlias(V1, I1Size, V2, I2Size))
          BM.bitSet(J, I);
      }
    }
  }
  DEBUG(BM.dump(dbgs()));
}

// Process every std.container instruction and clean them up later.
void StdContainerOpt::visitIntrinsicInst(IntrinsicInst &II) {
  unsigned IntrinID = II.getIntrinsicID();
  if ((IntrinID != Intrinsic::intel_std_container_ptr_iter) &&
      (IntrinID != Intrinsic::intel_std_container_ptr))
    return;

  Value *V = II.getArgOperand(0);
  assert(V && "Expected non empty argument in std.container intrinsic");
  // FIXME: Should we use cast<> to assert that this will be a LoadInst?
  if (LoadInst *LI = dyn_cast<LoadInst>(V)) {
    if (II.getIntrinsicID() == Intrinsic::intel_std_container_ptr_iter) 
      ContainerPtrIterInsns.push_back(LI);
    else
      ContainerPtrInsns.push_back(LI);
  }
  II.replaceAllUsesWith(V);
  II.eraseFromParent();
}
