// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// This pass implements an idiom recognizer that transforms simple loops into a
// non-loop form. In cases that this kicks in, it can be a significant
// performance win.
//
//===----------------------------------------------------------------------===//
//
// The following is 2 typical patterns in OCL which include a simple loop,
// Pattern 0:
//
// Entry:
//   %dim_0_ind_var.i = phi i64 [ 0, %wrapper ], [ %dim_0_inc_ind_var.i, %Entry ]
//   %dim_0_tid.i = phi i64 [ %11, %wrapper ], [ %dim_0_inc_tid.i, %Entry ]
//   ...
//   %28 = getelementptr inbounds double, double addrspace(1)* %27, i64 %dim_0_tid.i
//   ...
//   %31 = getelementptr inbounds double, double addrspace(1)* %26, i64 %dim_0_tid.i
//   ...
//   store i64 %28, i64 addrspace(1)* %31, align 8, !noalias !31
//   %dim_0_inc_ind_var.i = add nuw nsw i64 %dim_0_ind_var.i, 1
//   %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %LocalSize_0.i.i
//   %dim_0_inc_tid.i = add nuw nsw i64 %dim_0_tid.i, 1
//   br i1 %dim_0_cmp.to.max.i, label %Exit, label %Entry
// Exit:
//   ret void
//
// Pattern 1:
//
// Entry:
//   %dim_0_ind_var.i = phi i64 [ 0, %wrapper ], [ %dim_0_inc_ind_var.i, %Entry ]
//   %dim_0_tid.i = phi i64 [ 0, %wrapper ], [ %dim_0_inc_tid.i, %Entry ]
//   ...
//   %23 = add %20, %dim_0_tid.i
//   %24 =
//   %24 = getelementptr inbounds double, double addrspace(1)* %22, i64 %23
//   %27 = getelementptr inbounds double, double addrspace(1)* %21, i64 %23
//   ...
//   store i64 %24, i64 addrspace(1)* %27, align 8, !noalias !31
//   %dim_0_inc_ind_var.i = add nuw nsw i64 %dim_0_ind_var.i, 1
//   %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %LocalSize_0.i.i
//   %dim_0_inc_tid.i = add nuw nsw i64 %dim_0_tid.i, 1
//   br i1 %dim_0_cmp.to.max.i, label %Exit, label %Entry
// Exit:
//   ret void
//
//===----------------------------------------------------------------------===//


#include "OCLLoopIdiomRecognize.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"
#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Support/Debug.h"



#define DEBUG_TYPE "ocl-jit-loopidiom"

namespace intel {

char intel::OCLLoopIdiomRecognize::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(OCLLoopIdiomRecognize, "ocl-loop-idiom",
                          "Recognize ocl loop idioms", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(LoopPass)
OCL_INITIALIZE_PASS_END(OCLLoopIdiomRecognize, "ocl-loop-idiom",
                        "Recognize ocl loop idioms", false, false)

OCLLoopIdiomRecognize::OCLLoopIdiomRecognize() : LoopPass(ID) {
  initializeOCLLoopIdiomRecognizePass(*PassRegistry::getPassRegistry());
}

class OLIRecognize {
  Loop *CurLoop;
  AliasAnalysis *AA;
  DominatorTree *DT;
  LoopInfo *LI;
  ScalarEvolution *SE;
  const DataLayout *DL;

  StoreInst *SI;
  BitCastInst *SBCI;
  GetElementPtrInst *SGEPI;
  LoadInst *LDI;
  BitCastInst *LDBCI;
  GetElementPtrInst *LDGEPI;
  Instruction *SADDI;

  Value *FinalIV;
  Value *FinalGEPIndex;
  int64_t ElemSize;
  SmallVector<Instruction *, 2> LatchAdds;
  SmallVector<PHINode *, 2> PHIs;

public:
  OLIRecognize(Loop *L, AliasAnalysis *AA, DominatorTree *DT,
                        LoopInfo *LI, ScalarEvolution *SE, const DataLayout *DL)
      : CurLoop(L), AA(AA), DT(DT), LI(LI), SE(SE), DL(DL) {
    SI    = nullptr;
    SBCI  = nullptr;
    SGEPI = nullptr;
    LDI   = nullptr;
    LDBCI = nullptr;
    LDGEPI = nullptr;
    FinalGEPIndex = nullptr;
    FinalIV = nullptr;
    SADDI = nullptr;
    ElemSize = 0;
    LatchAdds.clear();
    PHIs.clear();
  }

  bool runOnLoop();

private:
  bool checkFuncName();
  bool checkStride();
  bool getFinalIVValue();
  ICmpInst *getLatchCmpInst();
  Value *findFinalIVValue(const PHINode &IndVar, const Instruction &StepInst);
  bool collectLoadStore(BasicBlock *BB);
  bool checkStoreDependInfo(BasicBlock *BB);
  bool checkLoadDependInfo(BasicBlock *BB);
  bool checkLoadStoreStride();
  bool runOnLoopBlock(BasicBlock *BB, BasicBlock * EB);
  bool processLoop(BasicBlock *BB, BasicBlock *EB);
};

void OCLLoopIdiomRecognize::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.setPreservesAll();
}

bool OCLLoopIdiomRecognize::runOnLoop(Loop *L, LPPassManager &LPM) {
  if (skipLoop(L)) {
    LLVM_DEBUG(dbgs() << "Skip Loop\n");
    return false;
  }
  AliasAnalysis *AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  const DataLayout *DL = &L->getHeader()->getModule()->getDataLayout();
  OLIRecognize OLIR(L, AA, DT, LI, SE, DL);
  return OLIR.runOnLoop();
}

//===----------------------------------------------------------------------===//
//
//          Implementation of OLIRecognize
//
//===----------------------------------------------------------------------===//

bool OLIRecognize::runOnLoop() {
  // If the loop could not be converted to canonical form, it must have an
  // indirectbr in it, just give up.
  if (!CurLoop || !CurLoop->getLoopPreheader() || !CurLoop->getHeader())
    return false;
  // Check function name firstly
  if (!checkFuncName()) {
    return false;
  }

  // Check EB, simplify the case, only one exit block
  SmallVector<BasicBlock *, 8> ExitBlocks;
  CurLoop->getUniqueExitBlocks(ExitBlocks);
  if (ExitBlocks.size() != 1) {
    LLVM_DEBUG(dbgs() << "Could not support multiple exit basic blocks \""
                      << ExitBlocks.size() << "\"\n");
    return false;
  }
  BasicBlock *EB = CurLoop->getUniqueExitBlock();
  if (!EB) {
    LLVM_DEBUG(dbgs() << "Could not find the exit basic block\n");
    return false;
  }

  Instruction *TERM = EB->getTerminator();
  if (!TERM || TERM->getOpcode() != Instruction::Ret) {
    LLVM_DEBUG(dbgs() << "Only support exit basic block which is terminated by Ret\n");
    return false;
  }

  bool MadeChange = false;
  // Scan all the blocks in the loop that are not in subloops.
  for (auto *BB : CurLoop->getBlocks()) {
    // Ignore blocks in subloops.
    if (LI->getLoopFor(BB) != CurLoop)
      continue;
    MadeChange |= runOnLoopBlock(BB, EB);
  }
  return MadeChange;
}

bool OLIRecognize::collectLoadStore(BasicBlock *BB) {
  int InstNum = 0;
  for (Instruction &PI : *BB) {
     // Simpify the case, no extra function call
     if (PI.getOpcode() == Instruction::Call)
       return false;
    StoreInst *TSI = dyn_cast<StoreInst>(&PI);
    if (!TSI)
      continue;
    SI = TSI;
    InstNum++;
  }
  // Simplify the case, only 1 store and value operand must is a scalar
  if (InstNum != 1 || SI->isVolatile() ||
      SI->getValueOperand()->getType()->isVectorTy())
    return false;
  // Only handle simple values that are a power of two bytes in size
  ElemSize = DL->getTypeSizeInBits(SI->getValueOperand()->getType());
  if (ElemSize == 0 || (ElemSize & 7) || (ElemSize & (ElemSize - 1)))
    return false;
  // Convert to size in Bytes
  ElemSize = (ElemSize >> 3);
  // Don't support larger integers
  if (ElemSize < 1 || ElemSize > 8)
    return false;
  LDI = dyn_cast<LoadInst>(SI->getOperand(0));
  if (!LDI || LDI->isVolatile() ||
      LDI->getOperand(0)->getType()->isVectorTy()) {
    LLVM_DEBUG(dbgs() << "Couldn't find load\n");
    return false;
  }
  return true;
}

bool OLIRecognize::checkStoreDependInfo(BasicBlock *BB) {
  int InstNum = 0;
  // check BitCast Instruction
  for (Use &OpU : SI->operands()) {
    Value *V = OpU.get();
    Instruction *I = dyn_cast<Instruction>(V);
    if (I->getOpcode() != Instruction::BitCast || !CurLoop->contains(I))
      continue;
    SBCI = dyn_cast<BitCastInst>(I);
    InstNum++;
  }
  // simplify the case, must have 1 BitCast
  if (!SBCI || InstNum != 1)
    return false;
  // check GetElementPtr Instruction
  if (!(SGEPI = dyn_cast<GetElementPtrInst>(SBCI->getOperand(0))))
    return false;
  if (SGEPI->getResultElementType()->isVectorTy())
    return false;
  Value *SGEPIV1 = SGEPI->getOperand(1);
  if (!SGEPIV1)
    return false;
  Instruction* IA = dyn_cast<Instruction>(SGEPIV1);
  bool ExtraAdd = (IA->getOpcode() == Instruction::Add ? true : false);
  if (ExtraAdd)
    SGEPIV1 = IA->getOperand(1);

  for (size_t i = 0; i < PHIs.size(); i++) {
    if (PHIs[i] != SGEPIV1) {
      continue;
    }
    if (!ExtraAdd) {
      // Entry:
      //   %dim_0_ind_var.i = phi i64 [ 0, %wrapper ], [ %dim_0_inc_ind_var.i, %Entry ]
      //   %dim_0_tid.i = phi i64 [ %11, %wrapper ], [ %dim_0_inc_tid.i, %Entry ]
      //   ...
      //   %28 = getelementptr inbounds double, double addrspace(1)* %27, i64 %dim_0_tid.i
      //   ...
      //   %31 = getelementptr inbounds double, double addrspace(1)* %26, i64 %dim_0_tid.i
      //   ...
      //   store i64 %28, i64 addrspace(1)* %31, align 8, !noalias !31
      //   %dim_0_inc_ind_var.i = add nuw nsw i64 %dim_0_ind_var.i, 1
      //   %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %LocalSize_0.i.i
      //   %dim_0_inc_tid.i = add nuw nsw i64 %dim_0_tid.i, 1
      //   br i1 %dim_0_cmp.to.max.i, label %Exit, label %Entry
      // Exit:
      //   ret void
      InductionDescriptor IndDesc;
      if (!InductionDescriptor::isInductionPHI(PHIs[i], CurLoop, SE, IndDesc))
        continue;
      FinalGEPIndex = PHIs[i]->getIncomingValueForBlock(CurLoop->getLoopPreheader());
      if (FinalGEPIndex)
        break;
    } else {
      // Entry:
      //   %dim_0_ind_var.i = phi i64 [ 0, %wrapper ], [ %dim_0_inc_ind_var.i, %Entry ]
      //   %dim_0_tid.i = phi i64 [ 0, %wrapper ], [ %dim_0_inc_tid.i, %Entry ]
      //   ...
      //   %23 = add %20, %dim_0_tid.i
      //   %24 = getelementptr inbounds double, double addrspace(1)* %22, i64 %23
      //   %27 = getelementptr inbounds double, double addrspace(1)* %21, i64 %23
      //   ...
      //   store i64 %24, i64 addrspace(1)* %27, align 8, !noalias !31
      //   %dim_0_inc_ind_var.i = add nuw nsw i64 %dim_0_ind_var.i, 1
      //   %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %LocalSize_0.i.i
      //   %dim_0_inc_tid.i = add nuw nsw i64 %dim_0_tid.i, 1
      //   br i1 %dim_0_cmp.to.max.i, label %Exit, label %Entry
      // Exit:
      //   ret void
      ConstantInt *CI = dyn_cast<ConstantInt>(PHIs[i]->getOperand(0));
      if (CI && CI->isZero()) {
        SADDI = IA;
        FinalGEPIndex = SADDI->getOperand(0);
        break;
      }
    }
  }
  if (!FinalGEPIndex && !SADDI)
    return false;
  return true;
}

bool OLIRecognize::checkLoadDependInfo(BasicBlock *BB) {
  int InstNum = 0;
  // check BisCast Instruction
  for (Use &OpU : LDI->operands()) {
    Value *V = OpU.get();
    Instruction *I = dyn_cast<Instruction>(V);
    if (I->getOpcode() != Instruction::BitCast || !CurLoop->contains(I))
      continue;
    LDBCI = dyn_cast<BitCastInst>(I);
    InstNum++;
  }
  if (!LDBCI || InstNum != 1)
    return false;
  // check GetElementPtr Instruction
  if (!(LDGEPI = dyn_cast<GetElementPtrInst>(LDBCI->getOperand(0))))
    return false;
  if (LDGEPI->getResultElementType()->isVectorTy())
    return false;
  return true;
}

bool OLIRecognize::runOnLoopBlock(BasicBlock *BB,
                                  BasicBlock *EB) {

  // Analyze header phi nodes for stride values.
  if (!checkStride()) {
    LLVM_DEBUG(dbgs() << "Couldn't find stride dependency\n");
    return false;
   }

  if (!getFinalIVValue()) {
    LLVM_DEBUG(dbgs() << "Couldn't get a right stride\n");
    return false;
  }

  // Look for store instructions, which may be optimized to memcpy.
  if (!collectLoadStore(BB)) {
    LLVM_DEBUG(dbgs() << "Couldn't find load/store\n");
    return false;
  }

  if (!checkStoreDependInfo(BB)) {
    LLVM_DEBUG(dbgs() << "Couldn't find store dependencies\n");
    return false;
  }

  if (!checkLoadDependInfo(BB)) {
    LLVM_DEBUG(dbgs() << "Couldn't find load dependencies\n");
    return false;
  }

  if (!checkLoadStoreStride()) {
    LLVM_DEBUG(dbgs() << "Couldn't get a same stride\n");
    return false;
  }

  // Optimize the load/store to a memcpy
  return processLoop(BB, EB);
}

/// It may be transformable into a memcpy, if
/// the stored value is a strided load in the same loop with the same stride,
/// This kicks in for stuff like "for (i) A[i] = B[i];"
bool OLIRecognize::checkLoadStoreStride() {
  Value* SGEPIIndex = SGEPI->getOperand(1);
  Value* LGEPIIndex = LDGEPI->getOperand(1);
  if (!SGEPIIndex || !LGEPIIndex)
    return false;
  if (SGEPIIndex != LGEPIIndex)
    return false;
  if (SADDI) {
     LLVM_DEBUG(dbgs() << "Find extra info for stride \n");
     return true;
  }
  // simplify the case, there are 2 adds in the latch,
  // one is for load, another is for store
  for (size_t i = 0; i < LatchAdds.size(); i++) {
    Instruction *Inc = LatchAdds[i];
    if (!Inc)
      return false;
    Value *Op0 = Inc->getOperand(0);
    Value *Op1 = Inc->getOperand(1);
    if (Op0 == SGEPIIndex || Op0 ==  LGEPIIndex ||
        Op1 == SGEPIIndex || Op1 ==  LGEPIIndex)
      return true;
  }
  return false;
}

bool OLIRecognize::processLoop(BasicBlock *BB,
                               BasicBlock *EB) {
  BasicBlock *Header = CurLoop->getHeader();
  BasicBlock *PreHeader = CurLoop->getLoopPreheader();

  Instruction *PreCondBr = PreHeader->getTerminator();
  if (!PreCondBr || PreCondBr->getOpcode() != Instruction::Br)
    return false;
  IRBuilder<> Builder(PreHeader->getTerminator());
  Value *SrcGEP = nullptr;
  Value *DstGEP = nullptr;
  Value *SrcBC  = nullptr;
  Value *DstBC  = nullptr;

  for (auto II = Header->begin(), E = Header->end(); II != E;) {
    Instruction *I = &*II++;
    if (SI == dyn_cast<StoreInst>(I)) {
      assert(DstGEP && SrcGEP && SrcBC && DstBC && "Wrong pattern");
      Value *IV = Builder.CreateMul(FinalIV,
                                    ConstantInt::get(FinalIV->getType(), ElemSize));
      auto *NewCall = Builder.CreateMemCpy(DstBC, SI->getAlignment(), SrcBC,
                                           LDI->getAlignment(), IV);
      NewCall->setDebugLoc(SI->getDebugLoc());
      Function *F = CurLoop->getHeader()->getParent();
      F->setMetadata("is_ocl_loop_idiom", MDNode::get(I->getContext(), {}));
      break;
    } else if (I->getOpcode() == Instruction::PHI) {
      continue;
    } else if (LDGEPI == dyn_cast<GetElementPtrInst>(I)) {
      Value *LGEPVal = LDGEPI->getOperand(0);
      SrcGEP = Builder.CreateGEP(LGEPVal, FinalGEPIndex);
      assert(SrcGEP && "Wrong src instruction");
      continue;
    } else if (SGEPI == dyn_cast<GetElementPtrInst>(I)) {
      Value *SGEPVal = SGEPI->getOperand(0);
      DstGEP = Builder.CreateGEP(SGEPVal, FinalGEPIndex);
      assert(DstGEP && "Wrong dst instruction");
      continue;
    } else if (LDBCI == dyn_cast<BitCastInst>(I)) {
      assert(SrcGEP && "Wrong src instruction");
      SrcBC = Builder.CreateBitCast(SrcGEP, LDBCI->getDestTy());
      continue;
    } else if (SBCI == dyn_cast<BitCastInst>(I)) {
      assert(DstGEP && "Wrong dst instruction");
      DstBC = Builder.CreateBitCast(DstGEP, SBCI->getDestTy());
      continue;
    } else if (LDI == dyn_cast<LoadInst>(I)) {
      continue;
    } else if (SADDI == I) {
      SADDI->setOperand(1, ConstantInt::get(SADDI->getOperand(1)->getType(), 0));
      SADDI->moveBefore(PreCondBr);
      continue;
    }
    // move other instruction to the pre header
    I->moveBefore(PreCondBr);
  }
  LLVMContext &Cxt = Header->getContext();
  Value *Cond = ConstantInt::get(Type::getInt1Ty(Cxt), 0);
  // It is diffcult to maintain all the inferences of Loop, e.g. LoopInfo,
  // so just disable it. i.e. transform below IR,
  //  br label %loop
  //  loop:
  //    <loop body>
  //    br i1 %exitcond, label %exit, label %loop
  //  exit:
  //
  // to like this,
  //  br i1 0, label %loop, label %exit
  //  loop:
  //    <loop body>
  //    br i1 %exitcond, label %exit, label %loop
  //  exit:
  // then using a loop-delete pass to delete disabled loop
  //
  Builder.CreateCondBr(Cond, BB, EB);
  PreCondBr->replaceAllUsesWith(UndefValue::get(PreCondBr->getType()));
  PreCondBr->eraseFromParent();
  LLVM_DEBUG(dbgs() << "Success to transform a simple ocl loop\n");
  return true;
}

/// Get the latch condition instruction.
ICmpInst *OLIRecognize::getLatchCmpInst() {
  if (BasicBlock *Latch = CurLoop->getLoopLatch())
    if (BranchInst *BI = dyn_cast_or_null<BranchInst>(Latch->getTerminator()))
      if (BI->isConditional())
        return dyn_cast<ICmpInst>(BI->getCondition());
  return nullptr;
}

/// Return the final value of the loop induction variable if found.
Value *OLIRecognize::findFinalIVValue(const PHINode &IndVar,
                                      const Instruction &StepInst) {
  ICmpInst *LatchCmpInst = getLatchCmpInst();
  if (!LatchCmpInst)
    return nullptr;
  Value *Op0 = LatchCmpInst->getOperand(0);
  Value *Op1 = LatchCmpInst->getOperand(1);
  if (Op0 == &IndVar || Op0 == &StepInst)
    return Op1;
  if (Op1 == &IndVar || Op1 == &StepInst)
    return Op0;
  return nullptr;
}

bool OLIRecognize::getFinalIVValue() {
  FinalIV = nullptr;
  BasicBlock *Header = CurLoop->getHeader();
  // Loop over all of the PHI nodes, looking for a indvar.
  for (BasicBlock::iterator I = Header->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    InductionDescriptor IndDesc;
    if (!InductionDescriptor::isInductionPHI(PN, CurLoop, SE, IndDesc))
      return false;
    Value *InitialIVValue = IndDesc.getStartValue();
    Instruction *StepInst = IndDesc.getInductionBinOp();
    if (!InitialIVValue || !StepInst)
      return false;
    Value *V = findFinalIVValue(*PN, *StepInst);
    if (V) {
      FinalIV = V;
    }
  }
  if (FinalIV)
    return true;
  return false;
}

bool OLIRecognize::checkFuncName() {
  Function *F = CurLoop->getHeader()->getParent();
  if (!F)
    return false;
  StringRef Name = F->getName();
  // Disable loop idiom recognition if the function is generated automatially.
  if (Name.startswith("__Vectorized_") ||
      Name.endswith("_separated_args") || Name.startswith("WG.boundaries.")) {
    LLVM_DEBUG(dbgs() << "\" " << Name << "\" is not a right function candidate\n");
    return false;
  }
  return true;
}

// Simplify the case, the stride must is 1
bool OLIRecognize::checkStride() {
  BasicBlock *Header = CurLoop->getHeader();
  BasicBlock *Latch = CurLoop->getLoopLatch();
  assert(Header && Latch && "loop should is a simple form");
  // Loop over all of the PHI nodes, looking for a canonical indvar.
  for (BasicBlock::iterator I = Header->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    // Currently support only scalar phi in the header block.
    if (PN->getType()->isVectorTy())
      continue;
    PHIs.push_back(PN);
    // The latch entry is an addition.
    Value *LatchVal = PN->getIncomingValueForBlock(Latch);
    Instruction *Inc = cast<Instruction>(LatchVal);
    if (!Inc || Inc->getOpcode() != Instruction::Add)
      continue;
    // Now check that all the added value is loop invariant.
    Value *Op0 = Inc->getOperand(0);
    Value *Op1 = Inc->getOperand(1);
    Value *Stride = nullptr;
    // isLoopInvariant means that the operand is either not an instruction
    // or it is an instruction outside of the loop.
    if (Op0 == PN && CurLoop->isLoopInvariant(Op1)) {
      Stride = Op1;
    } else if (Op1 == PN && CurLoop->isLoopInvariant(Op0)) {
      Stride = Op0;
    }
    if (!Stride)
      return false;
    ConstantInt *ConstStride = dyn_cast<ConstantInt>(Stride);
    if (!ConstStride || !ConstStride->isOne())
      return false;
    LatchAdds.push_back(Inc);
  }
  if (PHIs.size() < 2 || LatchAdds.size() < 1)
    return false;
  return true;
}

} // namespace intel

extern "C" {
  llvm::Pass *createOCLLoopIdiomRecognizePass() {
    return new intel::OCLLoopIdiomRecognize();
  }
}
