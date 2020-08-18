//===- Intel_NontemporalStore.cpp - Unaligned nontemporal store opts ------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Convert unaligned nontemporal stores inside of loops to use a library
// function that dynamically aligns them.
//
// This would convert loops like this:
// for (int i = 0; i < N; i++)
//   __builtin_nontemporal_store(data, &addr[i]);
//
// into something that looks like this:
// struct __nontemporal_buffer_data *buffer = alloca(
//     sizeof(__nontemporal_buffer_data) + BUF_SIZE * sizeof(double));
// buffer->dest = addr; buffer->misalign = (uintptr_t)addr & 63;
// buffer->buf_size = 0; buffer->buffer = _mm512_setzero_pd();
// int buf_index = 0;
// for (int i = 0; i < N; i++) {
//   ((double*)buffer->src)[buf_index++] = data;
//   if (buf_index == BUF_SIZE) {
//     __libirc_nontemporal_store(buffer, BUF_SIZE * sizeof(double), 0);
//     buf_index = 0;
//   }
// }
// __libirc_nontemporal_store(buffer, buf_index * sizeof(double), 1);
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_NontemporalStore.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

using namespace llvm;

#define DEBUG_TYPE "unaligned-nontemporal"

namespace {

class NontemporalStore {
  Function &F;
  AAResults &AA;
  DominatorTree &DT;
  LoopInfo &LI;
  ScalarEvolution &SE;
  const DataLayout &DL;
  bool HasLibFunc;

public:
  NontemporalStore(Function &F, AAResults &AA, DominatorTree &DT, LoopInfo &LI,
      ScalarEvolution &SE, TargetTransformInfo &TTI)
    : F(F), AA(AA), DT(DT), LI(LI), SE(SE),
      DL(F.getParent()->getDataLayout()) {
    // The library function we use requires AVX-512 to work correctly. If we're
    // not optimizing for AVX-512, then don't try to use it.
    HasLibFunc =
      TTI.isAdvancedOptEnabled(TargetTransformInfo::AO_TargetHasAVX512);
  }
  void run();
  Optional<Loop *> getContiguousInLoop(StoreInst &SI);
  bool hasConflictingLoads(StoreInst &SI, const Loop *L);
};

void NontemporalStore::run() {
  // If we don't support the library function, do not bother trying to do
  // anything.
  if (!HasLibFunc)
    return;

  SmallVector<std::pair<StoreInst *, Loop *>, 2> Worklist;
  for (auto &BB : F) {
    // Only consider instructions in loops.
    if (LI.getLoopFor(&BB) == nullptr)
      continue;

    for (auto &I : BB) {
      if (auto SI = dyn_cast<StoreInst>(&I)) {
        // Is it nontemporal?
        if (!SI->getMetadata(LLVMContext::MD_nontemporal))
          continue;

        // Is it misaligned?
        Type *StoreType = SI->getValueOperand()->getType();
        Align DesiredAlignment(DL.getABITypeAlignment(StoreType));
        Align ActualAlignment = DL.getValueOrABITypeAlignment(SI->getAlign(),
            StoreType);
        if (ActualAlignment >= DesiredAlignment)
          continue;

        LLVM_DEBUG(dbgs() << "Found unaligned nontemporal store: " <<
            *SI << "\n");
        Loop *L = getContiguousInLoop(*SI).getValueOr(nullptr);
        if (!L)
          continue;

        Worklist.push_back(std::make_pair(SI, L));
      }
    }
  }

  if (Worklist.empty())
    return;

  // Find the first non alloca in the entry block. This is where we'll insert
  // our allocas.
  BasicBlock::iterator FirstNonAlloca;
  BasicBlock &Entry = F.getEntryBlock();
  for (FirstNonAlloca = Entry.begin(); FirstNonAlloca != Entry.end();
      ++FirstNonAlloca) {
    if (!isa<AllocaInst>(*FirstNonAlloca))
      break;
  }
  IRBuilder<> Builder(&Entry, FirstNonAlloca);
  SCEVExpander SEExpander(SE, DL, "nt_buffer");

  Type *IntptrTy = DL.getIntPtrType(F.getContext());
  Type *VectorTy = VectorType::get(Builder.getInt64Ty(), 8, false);

  // Create a struct to store the type. Keep this in sync with the definition
  // in the library code (libirc/rt_nontemporal_store.c).
  Type *Tys[] = {
      VectorTy, // Recirculation buffer
      IntptrTy, // Destination (as uintptr_t)
      IntptrTy, // Misalignment
      IntptrTy, // Buffer size
      ArrayType::get(Builder.getInt8Ty(), 0) // Source data pointer
  };
  StructType *StoreBufferType =
    F.getParent()->getTypeByName("__nontemporal_buffer_data");
  if (!StoreBufferType || StoreBufferType->elements() != ArrayRef<Type*>(Tys)) {
    StoreBufferType = StructType::create(Tys, "__nontemporal_buffer_data");
  }
  const unsigned SrcFieldOffset = 4;
  const StructLayout *Layout = DL.getStructLayout(StoreBufferType);
  uint64_t HeaderSize = Layout->getElementOffset(SrcFieldOffset);

  auto DrainFunc = F.getParent()->getOrInsertFunction(
    "__libirc_nontemporal_store",
    FunctionType::get(Builder.getVoidTy(), {
      PointerType::getUnqual(StoreBufferType),
      Builder.getInt64Ty(),
      Builder.getInt32Ty(),
    }, false));

  for (auto &Pair : Worklist) {
    StoreInst *SI = Pair.first;
    Loop *L = Pair.second;

    LLVM_DEBUG(dbgs() << "Converting " << *SI << "...\n");
    BasicBlock *ExitBB = L->getExitBlock();
    BasicBlock *PreheaderBB = L->getLoopPredecessor();
    if (!ExitBB || !PreheaderBB || !DT.dominates(PreheaderBB, ExitBB)) {
      LLVM_DEBUG(dbgs() << "Unable to convert, as the exit and preheader blocks"
          " are not in the right configuration\n");
      continue;
    }

    StringRef Name = SI->getPointerOperand()->getName();
    Type *StoreType = SI->getValueOperand()->getType();
    uint64_t StoreSize = DL.getTypeStoreSize(StoreType).getFixedSize();
    uint64_t NumBufferElements = 128; // XXX: choose number better
    uint64_t BufferCount = StoreSize * NumBufferElements;
    Type *StoreArrayTy = PointerType::getUnqual(StoreType);
    Builder.SetInstDebugLocation(SI);
    Align DesiredAlign(DL.getABITypeAlignment(StoreType));

    // Create the alloca for the data. We're allocating as a char array, so
    // that the buffer is allocated in the padding of the store struct.
    AllocaInst *FullAlloca = Builder.CreateAlloca(Builder.getInt8Ty(),
        Builder.getInt64(BufferCount + HeaderSize),
        Name + ".nt_store_alloca");
    FullAlloca->setAlignment(Layout->getAlignment());
    Value *StoreStruct = Builder.CreateBitCast(FullAlloca,
        PointerType::getUnqual(StoreBufferType),
        Name + ".nt_store_struct");
    Value *StoreBufferPtr = Builder.CreateBitCast(
        Builder.CreateStructGEP(StoreStruct, SrcFieldOffset),
        StoreArrayTy,
        Name + ".nt_store_buffer");

    // Save off alloca location for next alloca location. Once we're done with
    // the other locations, this will return to the list of allocas.
    IRBuilderBase::InsertPointGuard Guard(Builder);

    // Insert the code to set up the struct in the preheader of the loop. Start
    // by getting the first location stored to (requires expanding SCEV
    // expressions).
    Builder.SetInsertPoint(PreheaderBB->getTerminator());
    SEExpander.setInsertPoint(PreheaderBB->getTerminator());
    SEExpander.SetCurrentDebugLocation(SI->getDebugLoc());
    auto *PointerSCEV =
      cast<SCEVAddRecExpr>(SE.getSCEV(SI->getPointerOperand()));
    Value *BasePtr = SEExpander.expandCodeFor(PointerSCEV->getStart(),
      IntptrTy);
    Builder.CreateStore(BasePtr,
        Builder.CreateConstGEP2_32(StoreBufferType, StoreStruct, 0, 1));

    // Compute the misalignment of this pointer.
    Value *AlignedPtr = Builder.CreateAnd(BasePtr,
        ConstantInt::get(IntptrTy, 63));
    Builder.CreateStore(AlignedPtr,
        Builder.CreateConstGEP2_32(StoreBufferType, StoreStruct, 0, 2));
    // Store 0 to the number of elements in buf.
    Builder.CreateStore(ConstantInt::get(IntptrTy, 0),
        Builder.CreateConstGEP2_32(StoreBufferType, StoreStruct, 0, 3));

    // Insert a PHI for the buffer store index.
    BasicBlock *Header = L->getHeader();
    assert(Header->hasNPredecessors(2) &&
        "The loop should only have a preheader and single latch block!");
    Builder.SetInsertPoint(Header, Header->getFirstInsertionPt());
    PHINode *IndexPHI = Builder.CreatePHI(Builder.getInt64Ty(), 2,
        Name + ".nt_buf_idx");
    IndexPHI->addIncoming(Builder.getInt64(0), PreheaderBB);

    // Replace the store with a store into the buffer.
    Builder.SetInsertPoint(SI);
    Builder.CreateAlignedStore(SI->getValueOperand(),
        Builder.CreateGEP(StoreBufferPtr, IndexPHI),
        MaybeAlign(DL.getABITypeAlignment(IntptrTy)));
    Value *IncPHI = Builder.CreateAdd(IndexPHI, Builder.getInt64(1),
        Name + ".nt_buf_idx", true, true);
    Value *ShouldBr = Builder.CreateICmpEQ(Builder.getInt64(NumBufferElements),
        IncPHI);

    // Insert a branch, if we are drained.
    BasicBlock *OrigBB = SI->getParent();
    BasicBlock *TailBB = SplitBlock(OrigBB, SI, &DT, &LI);
    BasicBlock *DrainBB = BasicBlock::Create(F.getContext(),
        Name + ".nt_buf_drain", &F, TailBB);
    Builder.SetInsertPoint(OrigBB->getTerminator());
    Builder.CreateCondBr(ShouldBr, DrainBB, TailBB);
    OrigBB->getTerminator()->eraseFromParent();

    // Add drain to dominator tree/loop info (tail is handled by SplitBlock)
    DT.addNewBlock(DrainBB, OrigBB);
    L->addBasicBlockToLoop(DrainBB, LI);

    // Add the drain buffer call.
    Builder.SetInsertPoint(DrainBB);
    Builder.CreateCall(DrainFunc,
        { StoreStruct, Builder.getInt64(BufferCount), Builder.getInt32(0) });
    Builder.CreateBr(TailBB);

    // The final PHI after the if statement.
    Builder.SetInsertPoint(TailBB, TailBB->getFirstInsertionPt());
    PHINode *TailPHI = Builder.CreatePHI(Builder.getInt64Ty(), 2,
        Name + ".nt_buf_post_phi");
    TailPHI->addIncoming(IncPHI, OrigBB);
    TailPHI->addIncoming(Builder.getInt64(0), DrainBB);
    IndexPHI->addIncoming(TailPHI, L->getLoopLatch());
    (void)StoreStruct;

    // In the exit block of the loop, drain the remainder of the store buffer.
    Builder.SetInsertPoint(ExitBB, ExitBB->getFirstInsertionPt());
    bool AfterExitingBB = !DT.dominates(OrigBB, L->getExitingBlock());
    Value *ExitCount = AfterExitingBB ? IndexPHI : TailPHI;
    if (!ExitBB->hasNPredecessors(1)) {
      PHINode *ExitPHI = Builder.CreatePHI(Builder.getInt64Ty(), 2,
          Name + ".nt_buf_exit_phi");
      for (auto BB : predecessors(ExitBB)) {
        ExitPHI->addIncoming(L->contains(BB) ? ExitCount : Builder.getInt64(0),
            BB);
      }
      ExitCount = ExitPHI;
    }
    Builder.CreateCall(DrainFunc,
        { StoreStruct,
        Builder.CreateMul(ExitCount, Builder.getInt64(StoreSize)),
        Builder.getInt32(1) });

    // Remove the original store instruction.
    SI->eraseFromParent();
  }
}

Optional<Loop *> NontemporalStore::getContiguousInLoop(StoreInst &SI) {
  Loop *ContainingLoop = LI.getLoopFor(SI.getParent());
  assert(ContainingLoop && "Shouldn't be considering stores not in loops");

  const SCEV *PointerSCEV = SE.getSCEV(SI.getPointerOperand());
  LLVM_DEBUG(dbgs() << "Corresponding to SCEV " << *PointerSCEV << "\n");
  auto *AddRec = dyn_cast<SCEVAddRecExpr>(PointerSCEV);
  if (!AddRec || AddRec->getLoop() != ContainingLoop)
    return {};
  Type *StoreType = SI.getValueOperand()->getType();
  uint64_t StoreSize = DL.getTypeStoreSize(StoreType).getFixedSize();
  const SCEV *StepSCEV = AddRec->getStepRecurrence(SE);
  const SCEV *SizeSCEV = SE.getConstant(AddRec->getType(), StoreSize);
  if (!SE.getURemExpr(StepSCEV, SizeSCEV)->isZero() ||
      !SE.getUDivExpr(StepSCEV, SizeSCEV)->isOne()) {
    LLVM_DEBUG(dbgs() << "Store is not contiguous\n");
    return {};
  }

  // Is the instruction executed every loop iteration? The header dominates all
  // nodes in the loop by definition, and a single exiting block postdominates
  // the loop by definition. Dominating this postdominator means we postdominate
  // the loop header, and that puts us into the same control-dependence region.
  // The extra check for the exiting block dominating the latch block guarantees
  // that the exit of a loop is not hidden inside an if statement.
  BasicBlock *StoreBlock = SI.getParent();
  BasicBlock *ExitingBlock = ContainingLoop->getExitingBlock();
  BasicBlock *LatchBlock = ContainingLoop->getLoopLatch();
  if (!ExitingBlock || !LatchBlock) {
    LLVM_DEBUG(dbgs() << "Multiple exiting/latch blocks, not handling\n");
    return {};
  }

  if (!DT.dominates(StoreBlock, ExitingBlock) &&
      !DT.dominates(ExitingBlock, StoreBlock)) {
    LLVM_DEBUG(dbgs() << "Store is in an if statement, not contiguous\n");
    return {};
  }

  if (!DT.dominates(ExitingBlock, LatchBlock) &&
      !DT.dominates(LatchBlock, ExitingBlock)) {
    LLVM_DEBUG(dbgs() << "Exiting block is in an if statement, not handling\n");
    return {};
  }

  if (hasConflictingLoads(SI, ContainingLoop)) {
    LLVM_DEBUG(dbgs() << "Store cannot be delayed to end of loop.\n");
    return {};
  }

  LLVM_DEBUG(dbgs() << "Store is contiguous\n");

  // TODO: try to guard in an outer loop.
  return ContainingLoop;
}

bool NontemporalStore::hasConflictingLoads(StoreInst &SI, const Loop *L) {
  // Volatiles generally can't be reordered.
  if (SI.isVolatile())
    return true;

  // Since we're deferring stores until (potentially) the end of the loop, it's
  // theoretically safe for us to reorder even most atomics, so long as our
  // library code could uphold the atomic guarantee. However, atomics also come
  // with a requirement that they be properly aligned, so we shouldn't even
  // fall into this case, as we're looking for underaligned stores.
  assert(SI.getOrdering() == AtomicOrdering::NotAtomic &&
      "Atomics cannot be underaligned.");

  // SE.getSmallConstantTripCount would seem to be what we want here, but it
  // returns 0 for unknown reasons. Use getBackedgeTakenCount instead.
  auto TripCount = dyn_cast<SCEVConstant>(SE.getBackedgeTakenCount(L));

  // Get the location that we store. Adjust it for the range of the array that
  // we could be storing, as well as the size of the type we are storing.
  LocationSize Size = LocationSize::unknown();
  if (TripCount) {
    Size = LocationSize::precise(
        DL.getTypeStoreSize(SI.getValueOperand()->getType()) *
        TripCount->getAPInt().getZExtValue());
    LLVM_DEBUG(dbgs() << "  (uses up to " << Size << ")\n");
  }
  MemoryLocation Loc = MemoryLocation::get(&SI).getWithNewSize(Size);

  // Check if any other instruction in the loop can modify or refer to the
  // locations that we store.
  for (const BasicBlock *BB : L->blocks()) {
    for (auto &Inst : *BB) {
      // Ignore prefetch intrinsics.
      if (auto II = dyn_cast<IntrinsicInst>(&Inst)) {
        if (II->getIntrinsicID() == Intrinsic::prefetch)
          continue;
      }
      if (&Inst == &SI)
        continue;

      // Get the effective store size of the instruction we are checking for the
      // alias. If it's an easy affine expression, check for an alias of the
      // range between the first and last store in the loop. Otherwise, just
      // check for aliasing in the entire object.
      auto ILoc = MemoryLocation::getOrNone(&Inst);
      LocationSize QuerySize = LocationSize::unknown();
      if (ILoc && TripCount) {
        const SCEV *QuerySCEV = SE.getSCEV(const_cast<Value*>(ILoc->Ptr));
        if (auto AddRec = dyn_cast<SCEVAddRecExpr>(QuerySCEV)) {
          if (auto Inc = dyn_cast<SCEVConstant>(AddRec->getStepRecurrence(SE))) {
            QuerySize = LocationSize::upperBound(
              (Inc->getAPInt() * TripCount->getAPInt()).getZExtValue());
          }
        }
      }

      // Actually query alias analysis for the information.
      if (isModOrRefSet(AA.getSizedModRefInfo(&Inst, QuerySize, Loc))) {
        LLVM_DEBUG(dbgs() << "Conflicts with " << Inst << "\n");
        return true;
      }
    }
  }

  return false;
}

class NontemporalStoreWrapperPass : public FunctionPass {
public:
  static char ID;

  NontemporalStoreWrapperPass() : FunctionPass(ID) {
    initializeNontemporalStoreWrapperPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesAll();
  };

  bool runOnFunction(Function &F) override;
};
} // namespace

PreservedAnalyses NontemporalStorePass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  NontemporalStore Impl(F,
      AM.getResult<AAManager>(F),
      AM.getResult<DominatorTreeAnalysis>(F),
      AM.getResult<LoopAnalysis>(F),
      AM.getResult<ScalarEvolutionAnalysis>(F),
      AM.getResult<TargetIRAnalysis>(F));
  Impl.run();
  return PreservedAnalyses::all();
}

char NontemporalStoreWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(NontemporalStoreWrapperPass, "unaligned-nontemporal",
                      "Unaligned Nontemporal Store Conversion", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(NontemporalStoreWrapperPass, "unaligned-nontemporal",
                    "Unaligned Nontemporal Store Conversion", false, false)

FunctionPass *llvm::createNontemporalStoreWrapperPass() {
  return new NontemporalStoreWrapperPass();
}

bool NontemporalStoreWrapperPass::runOnFunction(Function &F) {
  NontemporalStore Impl(F,
    getAnalysis<AAResultsWrapperPass>().getAAResults(),
    getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
    getAnalysis<LoopInfoWrapperPass>().getLoopInfo(),
    getAnalysis<ScalarEvolutionWrapperPass>().getSE(),
    getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F));
  Impl.run();
  return true;
}
