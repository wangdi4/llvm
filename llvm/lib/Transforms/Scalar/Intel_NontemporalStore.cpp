#if INTEL_FEATURE_SW_ADVANCED
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
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

#include <algorithm>

using namespace llvm;

#define DEBUG_TYPE "unaligned-nontemporal"

static cl::opt<bool> DisableUnalignedNontemporal(
    "disable-" DEBUG_TYPE, cl::init(false), cl::Hidden,
    cl::desc("Disable handling of unaligned nontemporal stores"));

// XXX: Currently chosen empirically for LBM, but could be tuned further. Making
// this value larger reduces the compute overhead of this transform but
// increases the cache overhead; making it smaller has the opposite effect.
static cl::opt<uint64_t>
    BufferSize(DEBUG_TYPE "-buffer-size", cl::Hidden, cl::init(4096),
               cl::desc("Unaligned nontemporal buffer size (in bytes)"));

namespace {

/// A block of stores which may be able to be converted together using a common
/// temp buffer. To be converted together as a single block, stores within a
/// loop must:
///
/// * Have a common type
/// * Have a common static stride
/// * Have static relative offsets smaller than the stride
/// * Not overlap
//
// TODO: Generalize to support stores of different types within structs.
struct StoreBlock {

  /// The loop this block is contiguous in.
  Loop *ContiguousLoop;

  /// The overall type of this block of stores.
  ///
  /// This should be an array type, with the element type matching the stores'
  /// value type and the element count matching the total number of stores
  /// expected based on the stride.
  ArrayType *BlockType;

  /// The instruction which the buffer flush should be inserted before.
  ///
  /// In the normal store case, this is the "last" store of this block in
  /// control flow order, which should be dominated by all of the other stores
  /// in the StoreBlock. In the compress store case, this should be the
  /// instruction after the compress store.
  Instruction *DrainPoint;

  /// Whether to drop the nontemporal metadata on these stores if they can't be
  /// converted.
  bool DropNontemporalIfNotConverted;

  /// The stores that make up this block, ordered by store address.
  ///
  /// This should be an array with the same element count as \ref BlockType. The
  /// first element is required to be non-null, but the other elements may be
  /// nullptr as the stores are collected and will be filled in as more matching
  /// stores are found. None of the entries should still be nullptr when the
  /// store block is converted, which can be verified with \ref isComplete.
  SmallVector<StoreInst *, 1> Stores;

  /// If this is set, transform this compress store intrinsic instead of a
  /// normal block of stores.
  IntrinsicInst *CompressStore = nullptr;

  /// The start location for the output buffer if this is a compress store,
  /// since we can't find it via SCEV.
  Value *CompressStoreStart = nullptr;

  /// Determines whether all of the stores in this StoreBlock have been found,
  /// which is required for converting this store block.
  bool isComplete() const;
};

class NontemporalStore {
  Function &F;
  AAResults &AA;
  DominatorTree &DT;
  LoopInfo &LI;
  ScalarEvolution &SE;
  const DataLayout &DL;
  bool HasLibFunc;
  TypeSize LargestVector = TypeSize::Fixed(0);

public:
  NontemporalStore(Function &F, AAResults &AA, DominatorTree &DT, LoopInfo &LI,
                   ScalarEvolution &SE, TargetTransformInfo &TTI)
      : F(F), AA(AA), DT(DT), LI(LI), SE(SE),
        DL(F.getParent()->getDataLayout()) {
    // The library function we use requires AVX-512 or AVX-2 to work correctly.
    // If we're not optimizing for either, then don't try to use it.
    HasLibFunc =
        TTI.isLibIRCAllowed() &&
        (TTI.isAdvancedOptEnabled(
             TargetTransformInfo::AO_TargetHasIntelAVX512) ||
         TTI.isAdvancedOptEnabled(TargetTransformInfo::AO_TargetHasIntelAVX2));

    // CMPLRLLVM-21684: For some reason, the library function does not work
    // correctly on x86-32. Until this can be understood, disable the library
    // function for now.
    if (DL.getPointerSizeInBits(0) != 64)
      HasLibFunc = false;

    // Determine the largest vector size available on this platform, which
    // should be a decent proxy for the size of the stores used in the library
    // function.
    LargestVector = TypeSize::Fixed(
        TTI.getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector) / 8);
  }
  bool run();

  /// Determines whether \p SI is independent and has a statically known stride
  /// within its containing loop. If so, a pair containing the loop and the
  /// static stride (in bytes) is returned.
  std::optional<std::pair<Loop *, int64_t>>
  getStaticStrideInLoop(StoreInst &SI);

  /// Determines whether a compress store intrinsic \p II is contiguous in its
  /// immediate parent loop and we can determine the start location for it.
  ///
  /// If so, the start location is returned. Otherwise, this returns nullptr.
  Value *isContiguousCompressStoreWithKnownStart(IntrinsicInst &II) const;

  /// Determines whether any memory accesses in \p L may conflict with \p SI.
  /// \p StoreStride should be the stride of \p SI in bytes.
  bool hasConflictingLoads(Instruction &SI, int64_t StoreStride,
                           const Loop *L) const;

  /// Creates a new StoreBlock within \p ContiguousLoop with an initial store
  /// \p InitialStore. \p StoreStride should be the stride of \p InitialStore in
  /// bytes.
  StoreBlock createStoreBlock(Loop *ContiguousLoop, StoreInst *InitialStore,
                              int64_t StoreStride,
                              bool DropNontemporalIfNotConverted);

  /// Creates a new StoreBlock for a compress store intrinsic \p CompressStore
  /// storing into an output array starting at \p Start.
  StoreBlock createStoreBlock(IntrinsicInst *CompressStore, Value *Start);

  /// Attempts to add \p NewStore within \p ContiguousLoop to one of the
  /// existing store blocks in \p Blocks. If it is part of one of them, it is
  /// added and this will return true. Otherwise, this returns false.
  /// \p StoreStride should be the stride of \p NewStore in bytes.
  bool collectStore(SmallVectorImpl<StoreBlock> &Blocks, Loop *ContiguousLoop,
                    StoreInst *NewStore, int64_t StoreStride);
};

bool NontemporalStore::run() {
  SmallVector<StoreBlock, 2> Worklist;
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
        Align DesiredAlignment(DL.getABITypeAlign(StoreType));
        Align ActualAlignment = DL.getValueOrABITypeAlignment(SI->getAlign(),
            StoreType);
        const bool IsUnaligned = (ActualAlignment < DesiredAlignment);

        // This transform is beneficial for unaligned vectors and also for
        // type-aligned scalar/vector types as long as they're smaller than the
        // ones used in the library routine. The library routine stores should
        // match the target's largest vector register size.
        const TypeSize StoreSize = DL.getTypeStoreSize(StoreType);
        if (!(IsUnaligned || StoreSize < LargestVector))
          continue;

        // Determine whether to drop the nontemporal metadata if the store can't
        // be converted. It should still be helpful for scalar stores and
        // aligned vector stores, so only drop it for unaligned vector stores.
        const bool IsScalar =
            (DL.getTypeStoreSizeInBits(StoreType).getFixedValue() < 128);
        const bool DropNontemporalIfNotConverted = IsUnaligned && !IsScalar;

        LLVM_DEBUG(dbgs() << "Found nontemporal store: " << *SI << "\n");
        const auto LoopAndStride = getStaticStrideInLoop(*SI);
        if (HasLibFunc && LoopAndStride) {
          if (!collectStore(Worklist, LoopAndStride->first, SI,
                            LoopAndStride->second))
            Worklist.push_back(createStoreBlock(LoopAndStride->first, SI,
                                                LoopAndStride->second,
                                                DropNontemporalIfNotConverted));
        } else if (DropNontemporalIfNotConverted) {
          LLVM_DEBUG(
              dbgs()
              << "Dropping nontemporal annotation on unaligned vector store\n");
          SI->setMetadata(LLVMContext::MD_nontemporal, nullptr);
        }
      }

      // Handle compress store intrinsics marked with !nontemporal in addition
      // to normal nontemporal stores.
      else if (auto *const II = dyn_cast<IntrinsicInst>(&I)) {
        if (II->getIntrinsicID() != Intrinsic::masked_compressstore)
          continue;
        if (!II->getMetadata(LLVMContext::MD_nontemporal))
          continue;

        LLVM_DEBUG(dbgs() << "Found nontemporal compress store:" << *II
                          << "\n");
        Value *Start;
        if (HasLibFunc &&
            (Start = isContiguousCompressStoreWithKnownStart(*II))) {
          Worklist.push_back(createStoreBlock(II, Start));
        } else {
          LLVM_DEBUG(dbgs() << "  Dropping nontemporal annotation\n");
          II->setMetadata(LLVMContext::MD_nontemporal, nullptr);
        }
      }
    }
  }

  // Drop any store blocks in the worklist that are incomplete.
  Worklist.erase(
      remove_if(Worklist,
                [](const StoreBlock &Block) {
                  if (Block.isComplete())
                    return false;

                  LLVM_DEBUG(dbgs()
                             << "Removing incomplete block of stores and "
                                "dropping nontemporal annotations:\n");
                  for (StoreInst *const Store : Block.Stores) {
                    if (Store) {
                      LLVM_DEBUG(dbgs() << *Store << "\n");
                      if (Block.DropNontemporalIfNotConverted)
                        Store->setMetadata(LLVMContext::MD_nontemporal,
                                           nullptr);
                    } else {
                      LLVM_DEBUG(dbgs() << "  <Not found>\n");
                    }
                  }
                  return true;
                }),
      Worklist.end());

  if (Worklist.empty())
    return false;

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
    StructType::getTypeByName(F.getContext(), "__nontemporal_buffer_data");
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

  for (const StoreBlock &Block : Worklist) {
    Loop *L = Block.ContiguousLoop;
    assert(Block.isComplete());
    assert(!Block.CompressStore || Block.CompressStore->getIntrinsicID() ==
                                       Intrinsic::masked_compressstore);
    StoreInst *const FirstStore =
        Block.CompressStore ? nullptr : Block.Stores.front();

    LLVM_DEBUG({
      if (Block.CompressStore) {
        dbgs() << "Converting contiguous compress store:\n";
        dbgs() << *Block.CompressStore << "\n";
      } else {
        dbgs() << "Converting block of contiguous stores:\n";
        for (const StoreInst *const SI : Block.Stores)
          dbgs() << *SI << "\n";
      }
    });
    BasicBlock *ExitBB = L->getExitBlock();
    BasicBlock *PreheaderBB = L->getLoopPredecessor();
    if (!ExitBB || !PreheaderBB || !DT.dominates(PreheaderBB, ExitBB)) {
      LLVM_DEBUG(dbgs() << "Unable to convert, as the exit and preheader blocks"
          " are not in the right configuration\n");
      if (Block.DropNontemporalIfNotConverted)
        for (StoreInst *const SI : Block.Stores)
          SI->setMetadata(LLVMContext::MD_nontemporal, nullptr);
      continue;
    }

    // Determine the type and length of the buffer array. The number of elements
    // is chosen so that the total size matches BufferSize as closely as
    // possible, rounding down if the target size is exactly halfway between two
    // possible buffer sizes. There should always be at least one buffer
    // element.
    StringRef Name;
    VectorType *CompressStoreVecType = nullptr;
    uint64_t StoreSize;
    uint64_t NumBufferElements;
    Type *StoreArrayTy;
    if (Block.CompressStore) {
      Name = Block.CompressStore->getArgOperand(1)->getName();
      CompressStoreVecType =
          cast<VectorType>(Block.BlockType->getArrayElementType());
      StoreSize = DL.getTypeStoreSize(CompressStoreVecType->getElementType())
                      .getFixedValue();
      NumBufferElements = std::max<uint64_t>(
          (BufferSize % StoreSize <= StoreSize / 2)
              ? BufferSize / StoreSize
              : BufferSize / StoreSize + 1,
          CompressStoreVecType->getElementCount().getFixedValue());
      StoreArrayTy = PointerType::getUnqual(CompressStoreVecType);
      Builder.SetInstDebugLocation(Block.CompressStore);
    } else {
      Name = FirstStore->getPointerOperand()->getName();
      StoreSize = DL.getTypeStoreSize(Block.BlockType).getFixedValue();
      NumBufferElements =
          std::max<uint64_t>((BufferSize % StoreSize <= StoreSize / 2)
                                 ? BufferSize / StoreSize
                                 : BufferSize / StoreSize + 1,
                             1);
      StoreArrayTy = PointerType::getUnqual(Block.BlockType);
      Builder.SetInstDebugLocation(FirstStore);
    }
    uint64_t BufferCount = StoreSize * NumBufferElements;

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
        Builder.CreateStructGEP(StoreBufferType, StoreStruct, SrcFieldOffset),
        StoreArrayTy, Name + ".nt_store_buffer");

    // Save off alloca location for next alloca location. Once we're done with
    // the other locations, this will return to the list of allocas.
    IRBuilderBase::InsertPointGuard Guard(Builder);

    // Insert the code to set up the struct in the preheader of the loop. Start
    // by getting the first location stored to (requires expanding SCEV
    // expressions, or looking up the value found for compress stores).
    Builder.SetInsertPoint(PreheaderBB->getTerminator());
    Value *BasePtr;
    if (Block.CompressStore) {
      assert(Block.CompressStoreStart);
      BasePtr = Builder.CreatePtrToInt(Block.CompressStoreStart, IntptrTy);
    } else {
      SEExpander.setInsertPoint(PreheaderBB->getTerminator());
      SEExpander.SetCurrentDebugLocation(FirstStore->getDebugLoc());
      auto *PointerSCEV =
          cast<SCEVAddRecExpr>(SE.getSCEV(FirstStore->getPointerOperand()));
      BasePtr = SEExpander.expandCodeFor(PointerSCEV->getStart(), IntptrTy);
    }
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

    // Replace the stores with stores into the buffer.
    if (Block.CompressStore) {
      Builder.SetInsertPoint(Block.CompressStore);
      Block.CompressStore->getArgOperandUse(1).set(Builder.CreateGEP(
          CompressStoreVecType->getElementType(), StoreBufferPtr, IndexPHI));
      Block.CompressStore->setMetadata(LLVMContext::MD_nontemporal, nullptr);
    } else {
      for (const auto &SI : enumerate(Block.Stores)) {
        Builder.SetInsertPoint(SI.value());
        Builder.CreateAlignedStore(
            SI.value()->getValueOperand(),
            Builder.CreateGEP(Block.BlockType, StoreBufferPtr,
                              {IndexPHI, Builder.getInt64(SI.index())}),
            MaybeAlign(DL.getABITypeAlign(IntptrTy)));
      }
    }

    // Insert a branch, if we are drained.
    Builder.SetInsertPoint(Block.DrainPoint);
    Value *IncPHI;
    Value *ShouldBr;
    if (Block.CompressStore) {
      const uint64_t VecWidth =
          CompressStoreVecType->getElementCount().getFixedValue();
      StringRef MaskName = Block.CompressStore->getArgOperand(2)->getName();
      Value *const MaskInt = Builder.CreateBitCast(
          Block.CompressStore->getArgOperand(2), Builder.getIntNTy(VecWidth),
          MaskName + ".i" + Twine(VecWidth));
      Value *const PopCnt = Builder.CreateUnaryIntrinsic(
          Intrinsic::ctpop, MaskInt, nullptr, MaskName + ".popcnt");
      Value *const Zext = Builder.CreateZExt(PopCnt, IndexPHI->getType(),
                                             MaskName + ".popcnt.zext");
      IncPHI =
          Builder.CreateAdd(IndexPHI, Zext, Name + ".nt_buf_idx", true, true);
      ShouldBr = Builder.CreateICmpUGT(
          IncPHI, Builder.getInt64(NumBufferElements - VecWidth));
    } else {
      IncPHI = Builder.CreateAdd(IndexPHI, Builder.getInt64(1),
                                 Name + ".nt_buf_idx", true, true);
      ShouldBr =
          Builder.CreateICmpEQ(Builder.getInt64(NumBufferElements), IncPHI);
    }
    BasicBlock *OrigBB = Block.DrainPoint->getParent();
    BasicBlock *TailBB = SplitBlock(OrigBB, Block.DrainPoint, &DT, &LI);
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
    if (Block.CompressStore) {
      Builder.CreateCall(
          DrainFunc,
          {StoreStruct, Builder.CreateMul(IncPHI, Builder.getInt64(StoreSize)),
           Builder.getInt32(0)});
    } else {
      Builder.CreateCall(DrainFunc, {StoreStruct, Builder.getInt64(BufferCount),
                                     Builder.getInt32(0)});
    }
    Builder.CreateBr(TailBB);

    // The final PHI after the if statement.
    Builder.SetInsertPoint(TailBB, TailBB->getFirstInsertionPt());
    PHINode *TailPHI = Builder.CreatePHI(Builder.getInt64Ty(), 2,
        Name + ".nt_buf_post_phi");
    TailPHI->addIncoming(IncPHI, OrigBB);
    TailPHI->addIncoming(Builder.getInt64(0), DrainBB);
    IndexPHI->addIncoming(TailPHI, L->getLoopLatch());

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

    // Remove the original store instructions.
    for (StoreInst *const SI : Block.Stores)
      SI->eraseFromParent();
  }

  return true;
}

std::optional<std::pair<Loop *, int64_t>>
NontemporalStore::getStaticStrideInLoop(StoreInst &SI) {
  Loop *ContainingLoop = LI.getLoopFor(SI.getParent());
  assert(ContainingLoop && "Shouldn't be considering stores not in loops");

  const SCEV *PointerSCEV = SE.getSCEV(SI.getPointerOperand());
  LLVM_DEBUG(dbgs() << "Corresponding to SCEV " << *PointerSCEV << "\n");
  auto *AddRec = dyn_cast<SCEVAddRecExpr>(PointerSCEV);
  if (!AddRec || AddRec->getLoop() != ContainingLoop)
    return {};
  Type *StoreType = SI.getValueOperand()->getType();
  uint64_t StoreSize = DL.getTypeStoreSize(StoreType).getFixedValue();
  const SCEV *StepSCEV = AddRec->getStepRecurrence(SE);
  const auto *const ConstStepSCEV = dyn_cast<SCEVConstant>(StepSCEV);
  if (!ConstStepSCEV) {
    LLVM_DEBUG(dbgs() << "Store stride is not statically known\n");
    return {};
  }
  const int64_t StoreStride = ConstStepSCEV->getValue()->getSExtValue();
  if (StoreStride <= 0) {
    LLVM_DEBUG(dbgs() << "Store stride " << StoreStride
                      << " is not positive\n");
    return {};
  }
  if (StoreStride % StoreSize != 0) {
    LLVM_DEBUG(dbgs() << "Store stride " << StoreStride
                      << " is not a multiple of store size " << StoreSize
                      << "\n");
    return {};
  }

  // Is the instruction executed every loop iteration? To simplify this check,
  // avoid loops with multiple exits/latches.
  BasicBlock *StoreBlock = SI.getParent();
  BasicBlock *ExitingBlock = ContainingLoop->getExitingBlock();
  BasicBlock *LatchBlock = ContainingLoop->getLoopLatch();
  if (!ExitingBlock || !LatchBlock) {
    LLVM_DEBUG(dbgs() << "Multiple exiting/latch blocks, not handling\n");
    return {};
  }

  // For the store to execute every loop iteration, it needs to be executed on
  // every path from the header to the latch block. This is true if the store
  // dominates the latch: because the header also dominates the latch by
  // definition, if there was a path from the header to the latch that
  // didn't include the store, the store would have to strictly dominate the
  // header which is impossible because the store is also dominated by the
  // header by definition. If the store doesn't dominate the latch, the path to
  // the latch that doesn't include the store must still include the header
  // since the header does dominate it, so there's a path from the header to the
  // latch that doesn't include the store. Therefore, the store dominating the
  // latch is equivalent to the store executing every loop iteration.
  if (!DT.dominates(StoreBlock, LatchBlock)) {
    LLVM_DEBUG(dbgs() << "Store does not dominate the latch, not contiguous\n");
    return {};
  }

  // We also avoid loops where the exiting block does not dominate the latch,
  // since the behavior on the last iteration of those is more difficult to
  // handle. Note that the latch cannot strictly dominate the exiting block
  // because of the requirement that the loop only has one latch.
  if (!DT.dominates(ExitingBlock, LatchBlock)) {
    LLVM_DEBUG(dbgs() << "Exiting block is in an if statement, not handling\n");
    return {};
  }

  if (hasConflictingLoads(SI, StoreStride, ContainingLoop)) {
    LLVM_DEBUG(dbgs() << "Store cannot be delayed to end of loop.\n");
    return {};
  }

  LLVM_DEBUG(dbgs() << "Store has statically known stride " << StoreStride
                    << " and may be part of a contiguous block of "
                    << StoreStride / StoreSize << " stores\n");

  // TODO: try to guard in an outer loop.
  return {{ContainingLoop, StoreStride}};
}

/// Determines if \p V is a bitcast of \p Mask.
static bool isBitcast(const Value *V, const Value *Mask) {
  if (const auto *const Bitcast = dyn_cast<BitCastInst>(V))
    if (Bitcast->getOperand(0) == Mask)
      return true;

  LLVM_DEBUG(dbgs() << "  Value isn't a bitcast of Mask:" << *V << "\n");
  return false;
}

/// Determines if \p V is the popcnt of \p Mask.
static bool isPopcnt(const Value *V, const Value *Mask) {

  // If V is a popcnt, the input should be a bitcast of Mask.
  if (const auto *const II = dyn_cast<IntrinsicInst>(V)) {
    if (II->getIntrinsicID() != Intrinsic::ctpop) {
      LLVM_DEBUG(dbgs() << "  Intrinsic isn't ctpop:" << *II << "\n");
      return false;
    }
    return isBitcast(II->getArgOperand(0), Mask);
  }

  LLVM_DEBUG(dbgs() << "  Couldn't find popcnt for" << *V << "\n");
  return false;
}

/// Determines if \p V is \p Prev offset by the popcnt of \p Mask times
/// \p ElemSize.
static bool isPopcntOffset(const Value *V, const Value *Prev, const Value *Mask,
                           unsigned ElemSize) {

  // If V is a GEP, the base pointer should be Prev, the offset should be the
  // popcnt of Mask, and the GEP's stride should be ElemSize.
  if (const auto *const GEP = dyn_cast<GetElementPtrInst>(V)) {
    if (GEP->getNumIndices() > 1) {
      LLVM_DEBUG(dbgs() << "  GEP has too many indices:" << *GEP << "\n");
      return false;
    }
    if (GEP->getPointerOperand() != Prev) {
      LLVM_DEBUG(dbgs() << "  Base pointer of GEP is not Prev:"
                        << *GEP->getPointerOperand() << "\n");
      return false;
    }
    if (GEP->getSourceElementType()->getPrimitiveSizeInBits() != ElemSize) {
      LLVM_DEBUG(dbgs() << "  Mismatched GEP stride ("
                        << GEP->getSourceElementType()->getPrimitiveSizeInBits()
                        << " != " << ElemSize << "):" << *GEP << "\n");
      return false;
    }
    return isPopcnt(GEP->getOperand(1), Mask);
  }

  LLVM_DEBUG(dbgs() << "  Couldn't find popcnt offset for" << *V << "\n");
  return false;
}

/// Determines if \p V strides by popcnt of \p Mask times \p ElemSize in each
/// iteration of \p L and if the start location can be determined.
///
/// If so, the start location is returned. Otherwise, this returns nullptr.
static Value *isPopcntRecurrenceWithKnownStart(const Loop *L, const Value *V,
                                               const Value *Mask,
                                               unsigned ElemSize) {

  // If V is a phi node, it should be in L's header and the backedge value
  // should be V offset by Mask's popcnt.
  if (const auto *const PHI = dyn_cast<PHINode>(V)) {
    if (PHI->getParent() != L->getHeader()) {
      LLVM_DEBUG(dbgs() << "  PHI node not in loop header:" << *PHI << "\n");
      return nullptr;
    }
    const BasicBlock *const Latch = L->getLoopLatch();
    if (!Latch) {
      LLVM_DEBUG(dbgs() << "  Loop has multiple backedges\n");
      return nullptr;
    }
    const Value *const Backedge = PHI->getIncomingValueForBlock(Latch);
    if (isPopcntOffset(Backedge, PHI, Mask, ElemSize)) {
      const BasicBlock *const Preheader = L->getLoopPreheader();
      if (!Preheader) {
        LLVM_DEBUG(dbgs() << "  Loop does not have preheader\n");
        return nullptr;
      }
      return PHI->getIncomingValueForBlock(Preheader);
    }
  }

  LLVM_DEBUG(dbgs() << "  Couldn't find popcnt recurrence for" << *V << "\n");
  return nullptr;
}

Value *NontemporalStore::isContiguousCompressStoreWithKnownStart(
    IntrinsicInst &CompressStore) const {
  assert(CompressStore.getIntrinsicID() == Intrinsic::masked_compressstore);
  const Loop *const L = LI.getLoopFor(CompressStore.getParent());
  assert(L && "Should not be considering compress stores outside of loops");
  const Value *const Pointer = CompressStore.getArgOperand(1);
  const Value *const Mask = CompressStore.getArgOperand(2);
  unsigned ElemSize =
      CompressStore.getArgOperand(0)->getType()->getScalarSizeInBits();

  // Check that the compress store is executed unconditionally in the loop.
  // As in the normal store case, we only handle loops with a single latch and
  // exiting block where the exiting block dominates the latch.
  BasicBlock *CompressStoreBlock = CompressStore.getParent();
  BasicBlock *ExitingBlock = L->getExitingBlock();
  BasicBlock *LatchBlock = L->getLoopLatch();
  if (!ExitingBlock || !LatchBlock) {
    LLVM_DEBUG(dbgs() << "  Multiple exiting/latch blocks, not handling\n");
    return nullptr;
  }
  if (!DT.dominates(CompressStoreBlock, LatchBlock)) {
    LLVM_DEBUG(dbgs() << "  Does not dominate the latch, not contiguous\n");
    return nullptr;
  }
  if (!DT.dominates(ExitingBlock, LatchBlock)) {
    LLVM_DEBUG(
        dbgs() << "  Exiting block is in an if statement, not handling\n");
    return nullptr;
  }

  Value *Start;
  if (!(Start = isPopcntRecurrenceWithKnownStart(L, Pointer, Mask, ElemSize)))
    return nullptr;

  if (hasConflictingLoads(CompressStore, 0, L)) {
    LLVM_DEBUG(dbgs() << "  Found conflicting memory accesses in the loop\n");
    return nullptr;
  }

  LLVM_DEBUG(dbgs() << "  CompressStore is contiguous and can be converted\n");

  return Start;
}

/// Computes \p X mod \p Y. Unlike operator%, this value is adjusted to be
/// non-negative regardless of the sign of \p X.
static int64_t nonNegativeModulus(int64_t X, int64_t Y) {
  const int64_t BaseMod = X % Y;
  return BaseMod >= 0 ? BaseMod : BaseMod + Y;
}

bool NontemporalStore::hasConflictingLoads(Instruction &SI, int64_t StoreStride,
                                           const Loop *L) const {
  // Volatiles generally can't be reordered.
  if (SI.isVolatile())
    return true;

  if (const auto *const Store = dyn_cast<StoreInst>(&SI)) {
    // Since we're deferring stores until (potentially) the end of the loop,
    // it's theoretically safe for us to reorder even most atomics, so long as
    // our library code could uphold the atomic guarantee. However, atomics also
    // come with a requirement that they be properly aligned, so we shouldn't
    // even fall into this case, as we're looking for underaligned stores.
    assert(Store->getOrdering() == AtomicOrdering::NotAtomic &&
           "Atomics cannot be underaligned.");
  }

  // Try to determine if loop-carried dependences are possible. In
  // general we need to rule out loop-independent dependences too, but this
  // becomes easier if we can reason only about a single IV value.
  bool NoLoopCarriedDeps =
      findStringMetadataForLoop(L, "llvm.loop.vectorize.ivdep_loop").has_value();
  LLVM_DEBUG(if (NoLoopCarriedDeps) dbgs() << "  (has ivdep_loop) ");

  // SE.getSmallConstantTripCount would seem to be what we want here, but it
  // returns 0 for unknown reasons. Use getBackedgeTakenCount instead.
  auto TripCount = dyn_cast<SCEVConstant>(SE.getBackedgeTakenCount(L));

  if (NoLoopCarriedDeps) {
    if (auto *const Store = dyn_cast<StoreInst>(&SI)) {
      // If we have no loop-carried deps then we can query AA with a precise
      // size; the dependence would be within a particular iteration. To do
      // this, override/set the trip count used for our MemoryLocation sizes
      // to 1.
      auto *AddRec =
          dyn_cast<SCEVAddRecExpr>(SE.getSCEV(Store->getPointerOperand()));
      assert(AddRec && "hasConflictingLoads assumes an AddRec Ptr");
      Type *TCType = AddRec->getStepRecurrence(SE)->getType();
      TripCount = dyn_cast<SCEVConstant>(SE.getOne(TCType));
    } else {
      TripCount =
          dyn_cast<SCEVConstant>(SE.getOne(Type::getInt64Ty(SI.getContext())));
    }
  }

  // Get the location that we store. Adjust it for the range of the array that
  // we could be storing, as well as the size of the type we are storing.
  LocationSize Size = LocationSize::beforeOrAfterPointer();
  if (TripCount) {
    if (const auto *const Store = dyn_cast<StoreInst>(&SI)) {
      Size = LocationSize::precise(
          (TripCount->getAPInt().getZExtValue() - 1) * StoreStride +
          DL.getTypeStoreSize(Store->getValueOperand()->getType()));
    } else if (const auto *const CompressStore = dyn_cast<IntrinsicInst>(&SI)) {
      if (CompressStore->getIntrinsicID() == Intrinsic::masked_compressstore) {
        const TypeSize FullVecSize =
            DL.getTypeStoreSize(CompressStore->getArgOperand(0)->getType());
        Size = LocationSize::precise(TripCount->getAPInt().getZExtValue() *
                                     FullVecSize);
      }
    }
    LLVM_DEBUG(dbgs() << "  (uses up to " << Size << ")\n");
  }
  MemoryLocation Loc;
  if (const auto *const Store = dyn_cast<StoreInst>(&SI)) {
    Loc = MemoryLocation::get(&SI).getWithNewSize(Size);
  } else if (const auto *const II = dyn_cast<IntrinsicInst>(&SI)) {
    if (II->getIntrinsicID() != Intrinsic::masked_compressstore)
      return true;
    Loc = MemoryLocation::getForArgument(II, 1, nullptr);
  } else {
    return true;
  }

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
      LocationSize QuerySize = LocationSize::beforeOrAfterPointer();
      if (ILoc) {
        const SCEV *QuerySCEV = SE.getSCEV(const_cast<Value*>(ILoc->Ptr));
        if (TripCount)
          if (auto AddRec = dyn_cast<SCEVAddRecExpr>(QuerySCEV)) {
            if (auto Inc =
                    dyn_cast<SCEVConstant>(AddRec->getStepRecurrence(SE))) {
              const APInt &IncVal = Inc->getAPInt();
              const APInt &TripCountVal = TripCount->getAPInt();
              unsigned BitWidth =
                  std::max(IncVal.getBitWidth(), TripCountVal.getBitWidth());
              QuerySize =
                  LocationSize::upperBound((IncVal.zext(BitWidth) *
                                            TripCountVal.zext(BitWidth))
                                               .getZExtValue());
            }
          }

        if (auto *const Store = dyn_cast<StoreInst>(&SI)) {
          // Try to prove simple inequality using SCEV. This is similar to what
          // SCEVAA would do if enabled, and is only helpful because SCEV can do
          // GEP/indexing simplifications that BasicAA cannot. We want to prove
          // that Diff > Loc.Size. (SI and Inst can be flipped depending on
          // which pointer happens to be greater.)
          // +                +
          // |                |
          // ----- Diff ----->|
          // |-->Loc.Size     |-------> QuerySize
          // SI               Inst
          //
          // For simplicity, we don't actually check which pointer is greater;
          // instead we check that the absolute difference is greater than
          // either location's footprint. This is slightly conservative.
          const SCEV *Diff = SE.getMinusSCEV(
              QuerySCEV, SE.getSCEV(Store->getPointerOperand()));
          assert(Diff && "Unexpected nullptr SCEV");
          if (!isa<SCEVCouldNotCompute>(Diff) && Loc.Size.hasValue() &&
              QuerySize.hasValue()) {
            const SCEV *Abs = SE.getAbsExpr(Diff, false);
            assert(Abs && "Unexpected nullptr SCEV");
            uint64_t LargestFootprint =
                std::max(Loc.Size.getValue(), QuerySize.getValue());
            const SCEV *Footprint =
                SE.getConstant(Abs->getType(), LargestFootprint);
            assert(Footprint && "Unexpected nullptr SCEV");

            if (SE.isKnownPredicate(ICmpInst::ICMP_UGT, Abs, Footprint))
              continue;
          }

          // Try to prove another simple test: if the accesses have a common
          // stride, they can't overlap if the distance between them modulo the
          // stride in both directions is at least as big as the individual
          // access sizes:
          // +            +             +            +
          // |--------- Stride -------->|--------- Stride -------->
          // |--->SISize  |-->InstSize  |--->SISize  |-->InstSize
          // SI           Inst          SI           Inst
          //
          // This can never be the case for individual contiguous stores as
          // SIToInst < StoreStride == SISize in that case, but this test is
          // needed to prove that there is no interference between multiple
          // stores within a contiguous block.
          const auto *const ConstDiff = dyn_cast<SCEVConstant>(Diff);
          if (ConstDiff && ILoc->Size.hasValue()) {
            const int64_t Distance = ConstDiff->getAPInt().getSExtValue();
            const uint64_t SIToInst = nonNegativeModulus(Distance, StoreStride);
            const uint64_t InstToSI =
                nonNegativeModulus(-Distance, StoreStride);
            const uint64_t SISize =
                DL.getTypeStoreSize(Store->getValueOperand()->getType());
            const uint64_t InstSize = ILoc->Size.getValue();
            if (SIToInst >= SISize && InstToSI >= InstSize)
              continue;
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

bool StoreBlock::isComplete() const {
  if (CompressStore)
    return true;
  assert(!Stores.empty());
  assert(Stores.front());
  return none_of(drop_begin(Stores),
                 [](const StoreInst *Store) { return Store == nullptr; });
}

StoreBlock
NontemporalStore::createStoreBlock(Loop *ContiguousLoop,
                                   StoreInst *InitialStore, int64_t StoreStride,
                                   bool DropNontemporalIfNotConverted) {

  // Determine the BlockType, which should be an array of StoreStride/StoreSize
  // elements of the store's type.
  Type *const StoreType = InitialStore->getValueOperand()->getType();
  const int64_t StoreSize = DL.getTypeStoreSize(StoreType);
  assert(StoreStride > 0);
  assert(StoreStride % StoreSize == 0);
  const int64_t StoreCount = StoreStride / StoreSize;
  auto *const BlockType = ArrayType::get(StoreType, StoreCount);

  // Construct an initial Stores array where InitialStore is the first element
  // and the rest are nullptr.
  SmallVector<StoreInst *, 1> Stores(StoreCount, nullptr);
  Stores.front() = InitialStore;

  return {ContiguousLoop, BlockType, InitialStore,
          DropNontemporalIfNotConverted, std::move(Stores)};
}

StoreBlock NontemporalStore::createStoreBlock(IntrinsicInst *CompressStore,
                                              Value *Start) {
  assert(CompressStore->getIntrinsicID() == Intrinsic::masked_compressstore);
  assert(Start);

  // The loop should be the immediate one containing CompressStore.
  Loop *const L = LI.getLoopFor(CompressStore->getParent());

  // The BlockType should be a single element with the vector type of the
  // compress store.
  Type *const VecType = CompressStore->getArgOperand(0)->getType();
  auto *const BlockType = ArrayType::get(VecType, 1);

  return {L,    BlockType, CompressStore->getNextNode(),
          true, {},        CompressStore,
          Start};
}

bool NontemporalStore::collectStore(SmallVectorImpl<StoreBlock> &Blocks,
                                    Loop *ContiguousLoop, StoreInst *NewStore,
                                    int64_t StoreStride) {
  for (StoreBlock &Block : Blocks) {

    // Skip blocks that are already complete.
    if (Block.isComplete())
      continue;

    // For this store to be compatible, the loop, type, and stride must match.
    if (Block.ContiguousLoop != ContiguousLoop)
      continue;
    Type *const StoreType = NewStore->getValueOperand()->getType();
    if (Block.BlockType->getElementType() != StoreType)
      continue;
    if (DL.getTypeStoreSize(Block.BlockType) != uint64_t(StoreStride))
      continue;

    // Make sure there is a known static offset between this store and the ones
    // in the block which is less than StoreStride.
    assert(!Block.Stores.empty());
    assert(Block.Stores.front());
    const SCEV *const NewStoreSCEV = SE.getSCEV(NewStore->getPointerOperand());
    const SCEV *const FirstStoreSCEV =
        SE.getSCEV(Block.Stores.front()->getPointerOperand());
    const SCEV *const OffsetSCEV =
        SE.getMinusSCEV(NewStoreSCEV, FirstStoreSCEV);
    const auto *const ConstOffsetSCEV = dyn_cast<SCEVConstant>(OffsetSCEV);
    if (!ConstOffsetSCEV)
      continue;
    const int64_t Offset = ConstOffsetSCEV->getAPInt().getSExtValue();
    if (std::abs(Offset) >= StoreStride)
      continue;

    // Determine where this store should go in the Stores array.
    const int64_t StoreSize = DL.getTypeStoreSize(StoreType);
    assert(Offset % StoreSize == 0);
    const int64_t Index = Offset / StoreSize;
    assert(Index != 0);

    // If it is after the first store, put it in the correct slot.
    if (Index > 0) {
      assert(!Block.Stores[Index]);
      Block.Stores[Index] = NewStore;
    }

    // Otherwise, make this the new first store by shifting all of the existing
    // stores.
    else {
      const int64_t Shift = -Index;
      const auto StoresBegin = std::begin(Block.Stores);
      const auto StoresEnd = std::end(Block.Stores);
      std::move_backward(StoresBegin, StoresEnd - Shift, StoresEnd);
      *StoresBegin = NewStore;
      std::fill(StoresBegin + 1, StoresBegin + Shift, nullptr);
    }

    // Update DrainPoint if needed.
    if (DT.dominates(Block.DrainPoint, NewStore)) {
      Block.DrainPoint = NewStore;
    } else {
      assert(DT.dominates(NewStore, Block.DrainPoint));
    }

    return true;
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
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
  };

  bool runOnFunction(Function &F) override;
};
} // namespace

PreservedAnalyses NontemporalStorePass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  if (DisableUnalignedNontemporal)
    return PreservedAnalyses::all();

  NontemporalStore Impl(F,
      AM.getResult<AAManager>(F),
      AM.getResult<DominatorTreeAnalysis>(F),
      AM.getResult<LoopAnalysis>(F),
      AM.getResult<ScalarEvolutionAnalysis>(F),
      AM.getResult<TargetIRAnalysis>(F));
  bool Changed = Impl.run();

  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<LoopAnalysis>();
  PA.preserve<ScalarEvolutionAnalysis>();
  return PA;
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
  if (skipFunction(F))
    return false;

  if (DisableUnalignedNontemporal)
    return false;

  NontemporalStore Impl(F,
    getAnalysis<AAResultsWrapperPass>().getAAResults(),
    getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
    getAnalysis<LoopInfoWrapperPass>().getLoopInfo(),
    getAnalysis<ScalarEvolutionWrapperPass>().getSE(),
    getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F));
  return Impl.run();
}

#endif // INTEL_FEATURE_SW_ADVANCED
