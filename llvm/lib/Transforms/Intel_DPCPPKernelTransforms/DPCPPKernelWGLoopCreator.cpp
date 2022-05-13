//==DPCPPKernelWGLoopCreator.cpp - Create WG loops in DPCPP kernels -*- C++-==//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelWGLoopCreator.h"
#include "LoopPeeling.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/WGBoundDecoder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include <sstream>

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-wgloop-creator"

using MapFunctionToReturnInst = DenseMap<Function *, ReturnInst *>;

namespace {
class WGLoopCreatorImpl {
public:
  WGLoopCreatorImpl(Module &M, MapFunctionToReturnInst &FuncReturn)
      : M(M), Ctx(M.getContext()), Builder(Ctx), FuncReturn(FuncReturn) {}

  /// Run on the module.
  bool run();

private:
  /// Struct that contains dimesion 0 loop attributes.
  struct LoopBoundaries {
    Value *PeelLoopSize;   // num peel loop iterations.
    Value *VectorLoopSize; // num vector loop iterations.
    Value *ScalarLoopSize; // num scalar loop iterations.
    Value *MaxPeel;        // max peeling global id.
    Value *MaxVector;      // max vector global id.
  };

  Module &M;

  /// LLVM context of the current module.
  LLVMContext &Ctx;

  IRBuilder<> Builder;

  /// Prefix for name of instructions used for loop of the dimension.
  std::string DimStr;

  // Remainder scalar or masked vector kernel return.
  ReturnInst *RemainderRet = nullptr;

  /// vector kernel return.
  ReturnInst *VectorRet = nullptr;

  // size_t type.
  Type *IndTy = nullptr;

  /// size_t one constant.
  Constant *ConstZero = nullptr;

  /// size_t one constant.
  Constant *ConstOne = nullptr;

  /// size_t packet constant
  Constant *ConstVF = nullptr;

  /// Function being processed.
  Function *F = nullptr;

  /// Vectorized inner loop func.
  Function *VectorF = nullptr;

  /// Masked function being processed.
  Function *MaskedF = nullptr;

  /// Scalar or masked vector kernel entry.
  BasicBlock *RemainderEntry = nullptr;

  /// Vector kernel entry.
  BasicBlock *VectorEntry = nullptr;

  /// New entry block.
  BasicBlock *NewEntry = nullptr;

  /// global_id lower bounds per dimension.
  ValueVec InitGIDs;

  /// global_id upper bounds per dimension.
  ValueVec MaxGIDs;

  /// base_global_id per dimension.
  ValueVec BaseGIDs;

  /// LoopSize per dimension.
  ValueVec LoopSizes;

  /// Index i contains vector with scalar or masked vector kernel
  /// get_global_id(i) calls.
  InstVecVec GidCalls;

  /// Index i contains vector with scalar or masked vector kernel
  /// get_local_id(i) calls.
  InstVecVec LidCalls;

  /// Index i contains vector with vector kernel get_global_id(i) calls.
  InstVecVec GidCallsVec;

  /// Index i contains vector with vector kernel get_local_id(i) calls.
  InstVecVec LidCallsVec;

  /// Early exit call.
  CallInst *EECall = nullptr;

  /// Number of WG dimensions.
  unsigned NumDim = MAX_WORK_DIM;

  /// The dimension by which we vectorize (usually 0).
  unsigned VectorizedDim = 0;

  /// Vectorization factor.
  unsigned VF = 0;

  /// Whether current function has subgroup path.
  bool HasSubGroupPath = false;

  /// Implicit GIDs in scalar/masked kernel.
  SmallVector<AllocaInst *, 3> ImplicitGIDs;

  /// Map from function to its return instruction.
  MapFunctionToReturnInst &FuncReturn;

  /// LoopRegion of scalar or masked remainder.
  LoopRegion RemainderRegion;

  /// Collect the get_global_id(), get_local_id(), and return of F.
  /// F - kernel to collect information for.
  /// Gids - array of get_global_id call to fill.
  /// Lids - array of get_local_id call to fill.
  /// Returns kernel single return instruction.
  ReturnInst *getFunctionData(Function *F, InstVecVec &Gids, InstVecVec &Lids);

  /// Public interface that allows running on pair of scalar - vector
  /// kernels not through pass manager.
  void processFunction(Function *F, Function *VectorF, unsigned VF);

  /// Create the early exit call.
  void createEECall(Function *Func);

  /// Create a condition jump over the entire WG loops according to uniform
  /// early exit value.
  /// \param RetBB Return block to jump to in case of uniform early exit.
  void handleUniformEE(BasicBlock *RetBB);

  /// Get or create the base global id for dimension Dim.
  /// \param Dim dimension to get base global id for.
  /// \returns base global id value.
  Value *getOrCreateBaseGID(unsigned Dim);

  /// Obtain initial global id, and loop size per dimension.
  void getLoopsBoundaries();

  /// Add work group loops on the kernel. converts get_***_id according
  /// to the generated loops. Moves Alloca instruction in kernel entry
  /// block to the new entry block on the way.
  /// KernelEntry - entry block of the kernel.
  /// IsVector - true iff working on vector kernel
  /// Ret - singel return instruction of the kernel.
  /// GIDs - array with get_global_id calls.
  /// LIDs - array with get_local_id calls.
  /// InitGIDs - initial global id per dimension.
  /// MaxGIDs - max (or upper bound) global id per dimension.
  /// Returns struct with preheader and exit block of the outmost loop.
  LoopRegion addWGLoops(BasicBlock *KernelEntry, bool IsVector, ReturnInst *Ret,
                        InstVecVec &GIDs, InstVecVec &LIDs, ValueVec &InitGIDs,
                        ValueVec &MaxGIDs);

  /// Replace the get***tid calls with incremented phi in loop head.
  /// TIDs - array of get***id to replace.
  /// InitVal - inital value (for the first iteration).
  /// IncBy - amount by which to increase the tid in each iteration.
  /// Head - head block of the loop.
  /// PreHead - pre header of the loop.
  /// Latch - latch block of the loop.
  void replaceTIDsWithPHI(InstVec &TIDs, Value *InitVal, Value *IncBy,
                          BasicBlock *Head, BasicBlock *PreHead,
                          BasicBlock *Latch);

  /// Create WG loops over scalar kernel.
  /// Returns a struct with entry and exit block of the WG loop region.
  LoopRegion createScalarLoops();

  /// Create WG loops over vector kernel and remainder loop over scalar kernel.
  /// Returns a struct with entry and exit block of the WG loop region.
  LoopRegion createVectorAndRemainderLoops();

  /// Create WG loops over vector kernel and remainder loop over masked vector
  /// kernel.
  LoopRegion createVectorAndMaskedRemainderLoops();

  /// Create WG loops over peeling loop over scalar or masked vector kernel,
  LoopRegion
  createPeelAndVectorAndRemainderLoops(LoopBoundaries &Dim0Boundaries);

  /// Inline the vector kernel into the scalar kernel.
  /// BB - location to move the vector blocks.
  /// Returns the vector kernel entry block.
  BasicBlock *inlineVectorFunction(BasicBlock *BB);

  /// computes the sizes of scalar and vector loops of the zero
  ///       dimension in case vector kernel exists.
  /// InitVal - Initial global id of zero dimension.
  /// DimSize - Number of iteration of zero dimensions.
  /// Returns struct with the sizes of the vector and scalar loop + the initial
  ///         scalar loop global id.
  LoopBoundaries getVectorLoopBoundaries(Value *InitVal, Value *DimSize);

  /// Returns the "true" dimension taking into account that
  /// the vectorized dimension might not be 0. If VectorizedDim
  /// is 0, then the returned value is always the same as Dim.
  /// but suppose m_vectorizedDim is 1, then resolveDimension(1) = 0,
  /// and resolveDimension(0) = 1. Since now Dim 1 is the innermost loop.
  unsigned resolveDimension(unsigned Dim) const;

  /// Find implicit GID alloca instructions and store initial GIDs to them.
  void initializeImplicitGID(Function *F);

  /// Compute a string indicating current loop level to later assign it to a
  /// label.
  void computeDimStr(unsigned Dim, bool IsVector);
};
}

bool WGLoopCreatorImpl::run() {
  IndTy = DPCPPKernelLoopUtils::getIndTy(&M);
  ConstZero = ConstantInt::get(IndTy, 0);
  ConstOne = ConstantInt::get(IndTy, 1);

  auto Kernels = getKernels(M);
  bool Changed = false;
  for (auto *F : Kernels) {
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
    // No need to check if NoBarrierPath Value exists, it is guaranteed that
    // KernelAnalysisPass ran before WGLoopCreator pass.
    if (!KIMD.NoBarrierPath.get()) {
      // Kernel that should be handled in barrier path, skip it.
      continue;
    }

    unsigned VectWidth = 0;
    // Get the vectorized function
    Function *VectKernel = KIMD.VectorizedKernel.get();
    // Need to check if vectorized kernel exists, it is not guaranteed that
    // Vectorized is running in all scenarios.
    if (VectKernel) {
      // Get the vectorized width
      DPCPPKernelMetadataAPI::KernelInternalMetadataAPI VKIMD(VectKernel);
      VectWidth = VKIMD.VectorizedWidth.get();

      // save the relevant information from the vectorized kernel
      // prior to erasing this information.
      KIMD.VectorizedKernel.set(nullptr);
      KIMD.VectorizedWidth.set(VectWidth);

      // Save the relevant information from the vectorized kernel in KIMD prior
      // to erasing these information.
      KIMD.VectorizationDimension.set(VKIMD.VectorizationDimension.get());
      KIMD.CanUniteWorkgroups.set(VKIMD.CanUniteWorkgroups.get());
    }

    LLVM_DEBUG(dbgs() << "VF for " << F->getName() << ": " << VectWidth
                      << "\n";);

    processFunction(F, VectKernel, VectWidth);

    Changed = true;
  }
  return Changed;
}

static void disableLoopUnrollRecursively(Loop *L) {
  LLVM_DEBUG(dbgs() << "Disable loop unroll for loop (header: "
                    << L->getHeader()->getName() << ")\n");
  L->setLoopAlreadyUnrolled();
  for (Loop *L : L->getSubLoops())
    disableLoopUnrollRecursively(L);
}

/// Disable loop unroll in scalar or masked remainder, since the loop trip
/// count is very small (less than VF).
static void disableRemainderLoopUnroll(Function &F, BasicBlock *Header) {
  DominatorTree DT(F);
  LoopInfo LI(DT);
  if (Loop *L = LI.getLoopFor(Header))
    disableLoopUnrollRecursively(L);
}

void WGLoopCreatorImpl::processFunction(Function *F, Function *VectorF,
                                        unsigned VF) {
  LLVM_DEBUG(dbgs() << "Creating loop for " << F->getName() << "\n";);
  LLVM_DEBUG(dbgs().indent(2));
  LLVM_DEBUG(if (VectorF) {
    dbgs() << "Vector function name: " << VectorF->getName() << "\n";
  });

  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);

  // Update member fields with the current kernel.
  this->F = F;
  this->VectorF = VectorF;
  this->VF = VF;
  MaskedF = nullptr;
  EECall = nullptr;
  HasSubGroupPath = KIMD.KernelHasSubgroups.hasValue()
                        ? KIMD.KernelHasSubgroups.get()
                        : false;
  VectorizedDim = KIMD.VectorizationDimension.hasValue()
                      ? KIMD.VectorizationDimension.get()
                      : 0;

  // generate constants.
  ConstVF = ConstantInt::get(IndTy, VF);

  // Get the number of the for which we need to create work group loops.
  NumDim = KIMD.MaxWGDimensions.hasValue() ? KIMD.MaxWGDimensions.get()
                                           : MAX_WORK_DIM;

  // Create WG loops.
  // If no workgroup loop is created (no calls to get_*_id), avoid inlining the
  // vector function into the scalar one since there is only one workitem to be
  // executed.
  if (NumDim && KIMD.VectorizedMaskedKernel.hasValue()) {
    MaskedF = KIMD.VectorizedMaskedKernel.get();
    // Set vetorized masked kernel to F. This metadata can be used as an
    // indicator of vectorized masked kernel being used.
    KIMD.VectorizedMaskedKernel.set(F);
  }

  // Function in remainder loop.
  Function *Func = MaskedF ? MaskedF : F;

  // Collect get_*_id and return instructions from the scalar/masked kernels.
  RemainderRet = getFunctionData(Func, GidCalls, LidCalls);
  const DILocation *RetDILoc = RemainderRet->getDebugLoc().get();

  // Get scalar or marked kernel entry and create new entry block for boundaries
  // calculation.
  RemainderEntry = &Func->getEntryBlock();
  RemainderEntry->setName(MaskedF ? "masked_kernel_entry"
                                  : "scalar_kernel_entry");
  NewEntry = BasicBlock::Create(Ctx, "", Func, RemainderEntry);

  // Create early exit call to obtain boundaries from.
  createEECall(Func);

  // Obtain loops boundaries from early exit call.
  getLoopsBoundaries();

  initializeImplicitGID(Func);

  // Create loops.
  LoopRegion WGLoopRegion = MaskedF ? createVectorAndMaskedRemainderLoops()
                            : (VectorF && NumDim)
                                ? createVectorAndRemainderLoops()
                                : createScalarLoops();
  assert(WGLoopRegion.PreHeader && WGLoopRegion.Exit &&
         "Workgroup loop entry or exit not initialized");

  // Connect the new entry block with the WG loops.
  BranchInst::Create(WGLoopRegion.PreHeader, NewEntry);

  // Create return block and connect it to WG loops exit.
  // We must create separate block for the return since the it might be
  // that there are no WG loops (NumDim=0) and WGLoopRegion.Exit
  // is not empty.
  auto *NewRetBB = BasicBlock::Create(Ctx, "exit", Func);
  BranchInst::Create(NewRetBB, WGLoopRegion.Exit);
  auto *RI = ReturnInst::Create(Ctx, NewRetBB);
  if (RetDILoc)
    RI->setDebugLoc(RetDILoc);

  // Create conditional jump over the WG loops in case of uniform early exit.
  handleUniformEE(NewRetBB);

  if (NumDim > 0 && RemainderRegion.Header)
    disableRemainderLoopUnroll(*Func, RemainderRegion.Header);

  // Now the masked kernel is a full workgroup which is organized as several
  // loops of vector kernel following by one masked kernel.
  // Replace the scalar kernel body with the masked kernel body.
  if (MaskedF) {
    auto replaceScalarKernelWithMasked = [](Function *F, Function *MaskedF) {
      // Erase all BasicBlocks from scalar kernel.
      for (auto &BB : *F)
        BB.dropAllReferences();
      while (!F->empty())
        F->begin()->eraseFromParent();

      F->getBasicBlockList().splice(F->begin(), MaskedF->getBasicBlockList());
      for (auto I = F->arg_begin(), E = F->arg_end(),
                MaskedI = MaskedF->arg_begin();
           I != E; ++I, ++MaskedI)
        MaskedI->replaceAllUsesWith(I);
      F->setSubprogram(MaskedF->getSubprogram());

      MaskedF->eraseFromParent();
    };
    replaceScalarKernelWithMasked(F, MaskedF);
  }

  // Finally, remove noinline attr
  if (!F->hasFnAttribute(llvm::Attribute::OptimizeNone))
    F->removeFnAttr(llvm::Attribute::NoInline);

  LLVM_DEBUG(dbgs() << "Created loop for " << F->getName() << "\n";);
  assert(!verifyFunction(*F, &dbgs()));
}

void WGLoopCreatorImpl::initializeImplicitGID(Function *F) {
  ImplicitGIDs.clear();

  if (!F->getSubprogram())
    return;

  // Find implicit GID alloca instructions. Implicit GIDs are in order, e.g.
  // __ocl_dbg_gid0 is before __ocl_dbg_gid1, due to the way they are inserted.
  for (Instruction &I : instructions(*F)) {
    if (auto *AI = dyn_cast<AllocaInst>(&I)) {
      if (isImplicitGID(AI))
        ImplicitGIDs.push_back(AI);
    }
    if (ImplicitGIDs.size() == MAX_WORK_DIM)
      break;
  }

  if (ImplicitGIDs.empty())
    return;
  assert((unsigned)ImplicitGIDs.size() == MAX_WORK_DIM &&
         "Invalid number of implicit GIDs");

  // Get initial GID and store to implicit GIDs.
  // Insert to new entry block.
  Builder.SetInsertPoint(NewEntry);
  Builder.SetCurrentDebugLocation(DebugLoc());
  for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
    Value *InitGID;
    if (Dim < NumDim) {
      InitGID = InitGIDs[Dim];
    } else {
      if (EECall) {
        unsigned LowerInd = WGBoundDecoder::getIndexOfInitGidAtDim(Dim);
        InitGID = ExtractValueInst::Create(EECall, LowerInd, "", NewEntry);
      } else {
        InitGID = DPCPPKernelLoopUtils::getWICall(
            F->getParent(), nameGetBaseGID(), IndTy, Dim, NewEntry);
      }
    }
    Builder.CreateStore(InitGID, ImplicitGIDs[Dim], /* isVolatile */ true);
  }
}

// Create the following scalar loops:
//
// scalar_preheader:
//   scalar loops
// return
//
LoopRegion WGLoopCreatorImpl::createScalarLoops() {
  LLVM_DEBUG(dbgs() << "Create scalar loops\n");
  if (VectorF)
    VectorF->eraseFromParent();
  return addWGLoops(RemainderEntry, false, RemainderRet, GidCalls, LidCalls,
                    InitGIDs, MaxGIDs);
}

// if the vector kernel exists then we create the following code:
//
// max_vector =
// if(vectorLoopSize != 0)
//   vector loops
// if (scalarLoopSize != 0)
//   scalar loops
// return
LoopRegion WGLoopCreatorImpl::createVectorAndRemainderLoops() {
  LLVM_DEBUG(dbgs() << "Create vector and scalar remainder loops\n");
  // Collect get_*_id and return instructions in the vector kernel.
  VectorRet = getFunctionData(VectorF, GidCallsVec, LidCallsVec);

  // Inline the vector kernel into the scalar kernel.
  VectorEntry = inlineVectorFunction(RemainderEntry);

  // Obtain boundaries for the vector loops and scalar remainder loops.
  LoopBoundaries Dim0Boundaries = getVectorLoopBoundaries(
      InitGIDs[VectorizedDim], LoopSizes[VectorizedDim]);

  if (Dim0Boundaries.PeelLoopSize)
    return createPeelAndVectorAndRemainderLoops(Dim0Boundaries);

  ValueVec InitGIDsCopy = InitGIDs; // hard copy.
  Value *MaxGIDTemp = MaxGIDs[VectorizedDim];

  // Create vector loops.
  MaxGIDs[VectorizedDim] = Dim0Boundaries.MaxVector;
  LoopRegion VectorBlocks =
      addWGLoops(VectorEntry, true, VectorRet, GidCallsVec, LidCallsVec,
                 InitGIDsCopy, MaxGIDs);

  // Create scalar loops.
  InitGIDsCopy[VectorizedDim] = Dim0Boundaries.MaxVector;
  MaxGIDs[VectorizedDim] = MaxGIDTemp;
  LoopRegion ScalarBlocks =
      addWGLoops(RemainderEntry, false, RemainderRet, GidCalls, LidCalls,
                 InitGIDsCopy, MaxGIDs);

  // Create blocks to jump over the loops.
  BasicBlock *LoopsEntry =
      BasicBlock::Create(Ctx, "vect_if", F, VectorBlocks.PreHeader);
  BasicBlock *ScalarIf =
      BasicBlock::Create(Ctx, "scalar_if", F, ScalarBlocks.PreHeader);

  BasicBlock *RetBB = BasicBlock::Create(Ctx, "ret", F);

  // Execute the vector loops if(vectorLoopSize != 0).
  Instruction *Vectcmp =
      new ICmpInst(*LoopsEntry, CmpInst::ICMP_NE, Dim0Boundaries.VectorLoopSize,
                   ConstZero, "");
  BranchInst::Create(VectorBlocks.PreHeader, ScalarIf, Vectcmp, LoopsEntry);
  BranchInst::Create(ScalarIf, VectorBlocks.Exit);

  // execute the scalar loops if(scalarLoopSize != 0)
  Instruction *ScalarCmp =
      new ICmpInst(*ScalarIf, CmpInst::ICMP_NE, Dim0Boundaries.ScalarLoopSize,
                   ConstZero, "");
  BranchInst::Create(ScalarBlocks.PreHeader, RetBB, ScalarCmp, ScalarIf);
  BranchInst::Create(RetBB, ScalarBlocks.Exit);

  RemainderRegion = ScalarBlocks;
  return LoopRegion{LoopsEntry, nullptr, RetBB};
}

LoopRegion WGLoopCreatorImpl::createVectorAndMaskedRemainderLoops() {
  LLVM_DEBUG(dbgs() << "Create vector and masked remainder loops\n");
  // Do not create scalar remainder for subgroup kernels as the semantics does
  // not allow execution of workitems in a serial manner.
  // Intentionally forget about scalar kernel. Let DCE delete it.

  // Collect get_*_id and return instructions in the vector kernel.
  VectorRet = getFunctionData(VectorF, GidCallsVec, LidCallsVec);

  // Inline the vector kernel into the masked kernel.
  VectorEntry = inlineVectorFunction(RemainderEntry);

  // Obtain boundaries for the vector loop and scalar loop.
  LoopBoundaries Dim0Boundaries = getVectorLoopBoundaries(
      InitGIDs[VectorizedDim], LoopSizes[VectorizedDim]);

  // If peeling size exists, create masked peeling loop as well.
  if (Dim0Boundaries.PeelLoopSize)
    return createPeelAndVectorAndRemainderLoops(Dim0Boundaries);

  ValueVec InitGIDsCopy = InitGIDs;   // hard copy.
  Value *MaxGIDTemp = MaxGIDs[VectorizedDim];

  // Create vector loops.
  MaxGIDs[VectorizedDim] = Dim0Boundaries.MaxVector;
  LoopRegion VectorBlocks =
      addWGLoops(VectorEntry, true, VectorRet, GidCallsVec, LidCallsVec,
                 InitGIDsCopy, MaxGIDs);

  // Create masked vector loop.
  InitGIDsCopy[VectorizedDim] = Dim0Boundaries.MaxVector;
  MaxGIDs[VectorizedDim] = MaxGIDTemp;
  LoopRegion MaskedBlocks =
      addWGLoops(RemainderEntry, true, RemainderRet, GidCalls, LidCalls,
                 InitGIDsCopy, MaxGIDs);

  // Create blocks to jump over the loops.
  auto *LoopsEntry =
      BasicBlock::Create(Ctx, "vect_if", MaskedF, VectorBlocks.PreHeader);
  auto *MaskGeneration =
      BasicBlock::Create(Ctx, "mask_generate", MaskedF, MaskedBlocks.PreHeader);
  auto *MaskedLoopEntry =
      BasicBlock::Create(Ctx, "masked_vect_if", MaskedF, MaskGeneration);

  auto *RetBB = BasicBlock::Create(Ctx, "ret", MaskedF);

  // Execute the vector loop if (VectorLoopSize != 0)/
  auto *VectCmp = new ICmpInst(*LoopsEntry, CmpInst::ICMP_NE,
                               Dim0Boundaries.VectorLoopSize, ConstZero);
  BranchInst::Create(VectorBlocks.PreHeader, MaskedLoopEntry, VectCmp,
                     LoopsEntry);
  BranchInst::Create(MaskedLoopEntry, VectorBlocks.Exit);

  // Execute the masked kernel loop if (ScalarLoopSize != 0).
  auto *MaskedLoopCmp = new ICmpInst(*MaskedLoopEntry, CmpInst::ICMP_NE,
                                     Dim0Boundaries.ScalarLoopSize, ConstZero);
  BranchInst::Create(MaskGeneration, RetBB, MaskedLoopCmp, MaskedLoopEntry);

  // Generate mask.
  auto *Mask = DPCPPKernelLoopUtils::generateRemainderMask(
      VF, Dim0Boundaries.ScalarLoopSize, MaskGeneration);
  BranchInst::Create(MaskedBlocks.PreHeader, MaskGeneration);
  auto MaskArg = MaskedF->arg_end() - 1;
  MaskArg->replaceAllUsesWith(Mask);

  BranchInst::Create(RetBB, MaskedBlocks.Exit);

  RemainderRegion = MaskedBlocks;
  return LoopRegion{LoopsEntry, nullptr, RetBB};
}

// If peeling is enabled and vector kernel exists, we create 3 loops:
//   1. peel loop which is a wrapper of the remainder loop.
//   2. vector loop.
//   3. remainder loop (scalar or masked vector).
//
// We create the following code:
//
// max_peel =
// max_vector =
// PeelEntry:
//   if (PeelLoopSize != 0)
//     br RemainderLoop
// VectorEntry:
//   if (VectorLoopSize != 0)
//     vector loop
// RemainderEntry:
//   if (ScalarLoopSize == 0)
//     br ret
// RemainderLoop:
//   IsPeelLoop = phi [true, PeelEntry], [false, RemainderEntry]
//   LoopSize = phi [PeelLoopSize, PeelEntry], [ScalarLoopSize,
//   RemainderEntry] if (Masked)
//     compute Mask from LoopSize
//     LoopSize = 1
//   scalar or masked vector loop
// RemainderLoopEnd:
//   if (IsPeelLoop)
//     br VectorEntry
// ret
LoopRegion WGLoopCreatorImpl::createPeelAndVectorAndRemainderLoops(
    LoopBoundaries &Dim0Boundaries) {
  LLVM_DEBUG(
      dbgs() << (MaskedF
                     ? "Create masked peel, vector and masked remainder loops"
                     : "Create scalar peel, vector and scalar remainder loops")
             << "\n");

  bool IsMasked = MaskedF != nullptr;
  Function *Func = IsMasked ? MaskedF : F;

  ValueVec InitGIDsCopy = InitGIDs;   // hard copy.

  // Create peeling loop region.
  LoopRegion PeelBlocks;
  PeelBlocks.Exit = BasicBlock::Create(Ctx, "peel_exit", Func, VectorEntry);
  PeelBlocks.PreHeader =
      BasicBlock::Create(Ctx, "peel_pre_head", Func, PeelBlocks.Exit);

  // Skip the remainder loop if (ScalarLoopSize == 0).
  auto *RemainderPreEntry =
      BasicBlock::Create(Ctx, "remainder_pre_entry", Func, RemainderEntry);
  auto *RemainderIf =
      BasicBlock::Create(Ctx, "remainder_if", Func, RemainderPreEntry);
  auto *RemainderCmp = new ICmpInst(*RemainderIf, CmpInst::ICMP_NE,
                                    Dim0Boundaries.ScalarLoopSize, ConstZero);
  auto *RetBB = BasicBlock::Create(Ctx, "ret", Func);
  BranchInst::Create(RemainderPreEntry, RetBB, RemainderCmp, RemainderIf);

  // Create vector loop.
  InitGIDsCopy[VectorizedDim] = Dim0Boundaries.MaxPeel;
  Value *MaxGIDTemp = MaxGIDs[VectorizedDim];
  MaxGIDs[VectorizedDim] = Dim0Boundaries.MaxVector;
  LoopRegion VectorBlocks =
      addWGLoops(VectorEntry, true, VectorRet, GidCallsVec, LidCallsVec,
                 InitGIDsCopy, MaxGIDs);

  // Create peel/remainder loops.
  auto *IsPeelLoop = PHINode::Create(Type::getInt1Ty(Ctx), 2, "is.peel.loop",
                                     RemainderPreEntry);
  IsPeelLoop->addIncoming(ConstantInt::getFalse(Ctx), RemainderIf);
  IsPeelLoop->addIncoming(ConstantInt::getTrue(Ctx), PeelBlocks.PreHeader);

  Value *PeelInitGID = InitGIDs[VectorizedDim];
  auto *RemainderInitGID =
      PHINode::Create(IndTy, 2, "peel.remainder.init.gid", RemainderPreEntry);
  RemainderInitGID->addIncoming(PeelInitGID, PeelBlocks.PreHeader);
  RemainderInitGID->addIncoming(Dim0Boundaries.MaxVector, RemainderIf);
  InitGIDsCopy[VectorizedDim] = RemainderInitGID;

  auto *RemainerMaxGID =
      PHINode::Create(IndTy, 2, "peel.remainder.max.gid", RemainderPreEntry);
  RemainerMaxGID->addIncoming(Dim0Boundaries.MaxPeel, PeelBlocks.PreHeader);
  RemainerMaxGID->addIncoming(MaxGIDTemp, RemainderIf);
  MaxGIDs[VectorizedDim] = RemainerMaxGID;

  if (IsMasked) {
    auto *ScalarLoopSize =
        PHINode::Create(Dim0Boundaries.ScalarLoopSize->getType(), 2,
                        "peel.remainder.loop.size", RemainderPreEntry);
    ScalarLoopSize->addIncoming(Dim0Boundaries.PeelLoopSize,
                                PeelBlocks.PreHeader);
    ScalarLoopSize->addIncoming(Dim0Boundaries.ScalarLoopSize, RemainderIf);

    // Generate mask.
    auto *Mask = DPCPPKernelLoopUtils::generateRemainderMask(VF, ScalarLoopSize,
                                                             RemainderPreEntry);
    auto MaskArg = Func->arg_end() - 1;
    MaskArg->replaceAllUsesWith(Mask);
  }

  LoopRegion RemainderBlocks =
      addWGLoops(RemainderEntry, IsMasked, RemainderRet, GidCalls, LidCalls,
                 InitGIDsCopy, MaxGIDs);

  // Create blocks to jump over the loops.
  auto *PeelIf = BasicBlock::Create(Ctx, "peel_if", Func, PeelBlocks.PreHeader);
  auto *VectIf =
      BasicBlock::Create(Ctx, "vect_if", Func, VectorBlocks.PreHeader);

  // Execute the peel loop if (PeelLoopSize != 0).
  auto *PeelCmp = new ICmpInst(*PeelIf, CmpInst::ICMP_NE,
                               Dim0Boundaries.PeelLoopSize, ConstZero);
  BranchInst::Create(PeelBlocks.PreHeader, VectIf, PeelCmp, PeelIf);
  BranchInst::Create(RemainderPreEntry, PeelBlocks.PreHeader);
  BranchInst::Create(VectIf, PeelBlocks.Exit);

  // Execute the vector loop if (VectorLoopSize != 0).
  auto *VectCmp = new ICmpInst(*VectIf, CmpInst::ICMP_NE,
                               Dim0Boundaries.VectorLoopSize, ConstZero);
  BranchInst::Create(VectorBlocks.PreHeader, RemainderIf, VectCmp, VectIf);
  BranchInst::Create(RemainderIf, VectorBlocks.Exit);

  // Execute the remainder loop if (ScalarLoopSize != 0).
  BranchInst::Create(RemainderBlocks.PreHeader, RemainderPreEntry);

  // Jump to peel exit if current remainder loop is peel loop.
  BranchInst::Create(PeelBlocks.Exit, RetBB, IsPeelLoop, RemainderBlocks.Exit);

  RemainderRegion = RemainderBlocks;
  return LoopRegion{PeelIf, nullptr, RetBB};
}

WGLoopCreatorImpl::LoopBoundaries
WGLoopCreatorImpl::getVectorLoopBoundaries(Value *InitVal, Value *DimSize) {
  // computes constant log VF
  assert(VF && ((VF & (VF - 1)) == 0) && "VF is not power of 2");
  unsigned LogVF = Log2_32(VF);
  Constant *LogVFConst = ConstantInt::get(IndTy, LogVF);

  // If subgroup call exists, don't create peeling loop because spec requires:
  // "All sub-groups must be the same size, while the last subgroup in any
  //  work-group (i.e. the subgroup with the maximum index) could be the same
  //  or smaller size".
  Value *PeelLoopSize = nullptr;
  if (!HasSubGroupPath) {
    if (auto PeelSize = LoopDynamicPeeling::computePeelCount(
            *NewEntry, *VectorEntry, InitGIDs, DimSize))
      PeelLoopSize = PeelSize.getValue();
  }
  Value *VectorScalarSize;
  Value *MaxPeel = nullptr;
  if (PeelLoopSize) {
    MaxPeel = BinaryOperator::Create(Instruction::Add, PeelLoopSize, InitVal,
                                     "max.peel.gid", NewEntry);
    VectorScalarSize =
        BinaryOperator::Create(Instruction::Sub, DimSize, PeelLoopSize,
                               "vector.scalar.size", NewEntry);
  } else {
    VectorScalarSize = DimSize;
  }

  // vector loops size can be derived by shifting size with log VF bits.
  Value *VectorLoopSize = BinaryOperator::Create(
      Instruction::AShr, VectorScalarSize, LogVFConst, "vector.size", NewEntry);
  Value *NumVectorWI = BinaryOperator::Create(
      Instruction::Shl, VectorLoopSize, LogVFConst, "num.vector.wi", NewEntry);
  Value *MaxVector = BinaryOperator::Create(Instruction::Add, NumVectorWI,
                                            PeelLoopSize ? MaxPeel : InitVal,
                                            "max.vector.gid", NewEntry);
  Value *ScalarLoopSize = BinaryOperator::Create(
      Instruction::Sub, VectorScalarSize, NumVectorWI, "scalar.size", NewEntry);

  return LoopBoundaries{PeelLoopSize, VectorLoopSize, ScalarLoopSize, MaxPeel,
                        MaxVector};
}

BasicBlock *WGLoopCreatorImpl::inlineVectorFunction(BasicBlock *BB) {
  // Create denseMap of function arguments
  ValueToValueMapTy ValueMap;
  assert(F->getFunctionType() == VectorF->getFunctionType() &&
         "vector and scalar functtion type mismatch");
  Function *Fn = BB->getParent();
  Function::const_arg_iterator VArgIt = VectorF->arg_begin();
  Function::const_arg_iterator VArgE = VectorF->arg_end();
  Function::arg_iterator ArgIt = Fn->arg_begin();
  for (; VArgIt != VArgE; ++ArgIt, ++VArgIt) {
    ValueMap[&*VArgIt] = &*ArgIt;
  }

  // create a list for return values
  SmallVector<ReturnInst *, 2> Returns;

  // Map debug information metadata entries in the cloned code from the vector
  // DISubprogram to the scalar DISubprogram.
  DISubprogram *SSP = Fn->getSubprogram();
  DISubprogram *VSP = VectorF->getSubprogram();
  if (SSP && VSP) {
    auto &MD = ValueMap.MD();
    MD[VSP].reset(SSP);
  }

  // To prevent Metadata merge, drop old Metadata. They were cloned anyway
  // before vectorization.
  SmallVector<std::pair<unsigned, MDNode *>, 32> MDs;
  VectorF->getAllMetadata(MDs);
  SmallDenseMap<unsigned, MDNode *, 32> VectorMDsMap;
  VectorMDsMap.insert(MDs.begin(), MDs.end());
  MDs.clear();
  Fn->getAllMetadata(MDs);
  for (const auto &MD : MDs)
    if (MD.first != LLVMContext::MD_dbg && VectorMDsMap.count(MD.first))
      Fn->setMetadata(MD.first, nullptr);

  // Do actual cloning work
  // TODO: replace manual inlining by llvm::InlineFunction()
  CloneFunctionInto(Fn, VectorF, ValueMap,
                    CloneFunctionChangeType::LocalChangesOnly, Returns,
                    "vector_func");

  // The CloneFunctionInto() above will move all function metadata from the
  // vector function to the scalar function. Because the scalar function
  // already has debug info metadata, it ends up with two DISubprograms
  // assigned. The easiest way to assign the correct one is to erase both of
  // them by resetting the original scalar subprogram.
  Fn->setSubprogram(SSP);

  for (auto &VBB : *VectorF) {
    BasicBlock *ClonedBB = dyn_cast<BasicBlock>(ValueMap[&VBB]);
    ClonedBB->moveBefore(BB);
  }

  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    for (unsigned I = 0, E = GidCallsVec[Dim].size(); I < E; ++I)
      GidCallsVec[Dim][I] = cast<Instruction>(ValueMap[GidCallsVec[Dim][I]]);
    for (unsigned I = 0, E = LidCallsVec[Dim].size(); I < E; ++I)
      LidCallsVec[Dim][I] = cast<Instruction>(ValueMap[LidCallsVec[Dim][I]]);
  }

  // Find implicit GID alloca instructions in vector kernel and replace them
  // with corresponding implicit GID in scalar/masked kernel.
  // Note implicit GIDs are in order, e.g. __ocl_dbg_gid0 is before
  // __ocl_dbg_gid1, due to the way they are inserted.
  if (VSP) {
    size_t NumImplicitGIDs = ImplicitGIDs.size();
    SmallVector<AllocaInst *, 3> VectorImplicitGIDs;
    for (auto &I : instructions(*VectorF)) {
      auto *AI = dyn_cast<AllocaInst>(&I);
      if (AI && isImplicitGID(AI))
        VectorImplicitGIDs.push_back(AI);
      if (VectorImplicitGIDs.size() == NumImplicitGIDs)
        break;
    }
    assert(VectorImplicitGIDs.size() == NumImplicitGIDs &&
           "vector and scalar kernels must have the same number of implicit "
           "GIDs");
    for (unsigned Dim = 0; Dim < NumImplicitGIDs; ++Dim) {
      auto *I = cast<Instruction>(ValueMap[VectorImplicitGIDs[Dim]]);
      I->replaceAllUsesWith(ImplicitGIDs[Dim]);
      I->eraseFromParent();
    }
  }

  VectorRet = cast<ReturnInst>(ValueMap[VectorRet]);
  // Get hold of the entry to the vector section in the vectorized function.
  BasicBlock *VectorEntryBlock =
      dyn_cast<BasicBlock>(ValueMap[&*(VectorF->begin())]);
  assert(VectorF->getNumUses() == 0 && "vector kernel should have no use");
  VectorF->eraseFromParent();
  return VectorEntryBlock;
}

void WGLoopCreatorImpl::createEECall(Function *Func) {
  // Obtain early exit function, which should have the same arguments
  // as the kernel.
  std::string EEFuncName = WGBoundDecoder::encodeWGBound(F->getName());
  Function *EEFunc = F->getParent()->getFunction(EEFuncName);

  // If WGLoopBoundaries pass is not run, early exit function doesn't exit.
  if (!EEFunc)
    return;

  // TODO do following in WGLoopBoundaries pass?
  // Set the linkage type of the loop boundary function as private, so that it
  // will be removed after inlining and DCE.
  assert(!EEFunc->isDeclaration() &&
         "WG boundary function should not be declaration");
  EEFunc->setLinkage(GlobalValue::PrivateLinkage);

  SmallVector<Value *, 8> Args;
  unsigned Idx = 0;
  for (auto It = EEFunc->arg_begin(), E = EEFunc->arg_end(); It != E;
       ++It, ++Idx) {
    Value *Arg = Func->getArg(Idx);
    assert(Arg->getType() == It->getType() &&
           "mismatch argument type between function and early exit");
    Args.push_back(Arg);
  }

  // Return a call in the new entry block.
  Builder.SetInsertPoint(NewEntry);
  EECall = Builder.CreateCall(EEFunc, Args, "early_exit_call");
  // Make -debugify happy.
  if (EEFunc->getSubprogram())
    if (DISubprogram *SP = Func->getSubprogram())
      EECall->setDebugLoc(DILocation::get(Ctx, 0, 0, SP));
}

void WGLoopCreatorImpl::handleUniformEE(BasicBlock *RetBB) {
  if (!EECall)
    return;

  // Obtain uniform early exit function.
  // If it is equal to 0, jump to return block.
  Instruction *InsertPt = EECall->getNextNonDebugInstruction();
  assert(InsertPt && "expect insert point after early exit call");
  unsigned UniInd = WGBoundDecoder::getUniformIndex();
  Instruction *UniEECond =
      ExtractValueInst::Create(EECall, UniInd, "", InsertPt);
  auto *TruncCond =
      new TruncInst(UniEECond, Type::getInt1Ty(Ctx), "", InsertPt);
  // Split the basic block after obtaining the uniform early exit condition,
  // and conditionally jump to the WG loops.
  BasicBlock *WGLoopsEntry =
      NewEntry->splitBasicBlock(InsertPt, "WGLoopsEntry");
  NewEntry->getTerminator()->eraseFromParent();
  BranchInst::Create(WGLoopsEntry, RetBB, TruncCond, NewEntry);
}

ReturnInst *WGLoopCreatorImpl::getFunctionData(Function *F, InstVecVec &Gids,
                                               InstVecVec &Lids) {
  std::string GID = mangledGetGID();
  std::string LID = mangledGetLID();
  DPCPPKernelLoopUtils::collectTIDCallInst(GID, Gids, F);
  DPCPPKernelLoopUtils::collectTIDCallInst(LID, Lids, F);

  auto It = FuncReturn.find(F);
  if (It != FuncReturn.end())
    return It->second;

  // Create dummy return block for e.g. infinite loop.
  BasicBlock *DummyRetBB = BasicBlock::Create(Ctx, "dummy_ret", F);
  return ReturnInst::Create(Ctx, DummyRetBB);
}

Value *WGLoopCreatorImpl::getOrCreateBaseGID(unsigned Dim) {
  // If it is cached, return the cached value.
  assert(BaseGIDs.size() > Dim && "Dim is out of range");
  if (BaseGIDs[Dim])
    return BaseGIDs[Dim];

  // Create the value.
  CallInst *BaseGID = DPCPPKernelLoopUtils::getWICall(
      F->getParent(), nameGetBaseGID(), IndTy, Dim, NewEntry,
      Twine("base.gid.dim") + Twine(Dim));
  BaseGIDs[Dim] = BaseGID;
  return BaseGID;
}

void WGLoopCreatorImpl::getLoopsBoundaries() {
  BaseGIDs.assign(MAX_WORK_DIM, nullptr);
  InitGIDs.clear();
  LoopSizes.clear();
  MaxGIDs.clear();
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    Value *InitGID;
    Value *LoopSize;
    if (EECall) {
      unsigned LowerInd = WGBoundDecoder::getIndexOfInitGidAtDim(Dim);
      InitGID = ExtractValueInst::Create(
          EECall, LowerInd, Twine("init.gid.dim") + Twine(Dim), NewEntry);
      unsigned LoopSizeInd = WGBoundDecoder::getIndexOfSizeAtDim(Dim);
      LoopSize = ExtractValueInst::Create(
          EECall, LoopSizeInd, Twine("loop.size.dim") + Twine(Dim), NewEntry);
    } else {
      InitGID = getOrCreateBaseGID(Dim);
      LoopSize = DPCPPKernelLoopUtils::getWICall(
          F->getParent(), mangledGetLocalSize(), IndTy, Dim, NewEntry,
          Twine("local.size.dim") + Twine(Dim));
    }
    InitGIDs.push_back(InitGID);
    LoopSizes.push_back(LoopSize);

    // Create MaxGID for each dimension. Unsed MaxGID will be eliminated by
    // later optimizations.
    auto *MaxGID =
        BinaryOperator::Create(Instruction::Add, InitGID, LoopSize,
                               Twine("max.gid.dim") + Twine(Dim), NewEntry);
    MaxGIDs.push_back(MaxGID);
  }
}

unsigned WGLoopCreatorImpl::resolveDimension(unsigned Dim) const {
  if (Dim == 0)
    return VectorizedDim;
  else if (Dim > VectorizedDim)
    return Dim;
  else
    return Dim - 1;
}

void WGLoopCreatorImpl::computeDimStr(unsigned Dim, bool IsVector) {
  DimStr = ("dim_" + Twine(Dim) + "_" + (IsVector ? "vector_" : "")).str();
}

LoopRegion WGLoopCreatorImpl::addWGLoops(BasicBlock *KernelEntry, bool IsVector,
                                         ReturnInst *Ret, InstVecVec &GIDs,
                                         InstVecVec &LIDs, ValueVec &InitGIDs,
                                         ValueVec &MaxGIDs) {
  assert(KernelEntry && Ret && "uninitialized parameters");

  // Move allocas and llvm.dbg.declare in the entry kernel entry block to the
  // new entry block.
  moveInstructionIf(KernelEntry, NewEntry, [](Instruction &I) {
    return isa<AllocaInst>(&I) || isa<DbgDeclareInst>(&I);
  });

  // Initial head and latch are the kernel entry and return block
  // respectively.
  BasicBlock *Head = KernelEntry;
  BasicBlock *Latch = Ret->getParent();
  LLVM_DEBUG(dbgs() << "  Add " << (IsVector ? "vector" : "scalar")
                    << " WG Loops. Head: " << Head->getName()
                    << ". Latch: " << Latch->getName() << "\n");

  // Header of outmost loop.
  BasicBlock *HeaderOutmost = nullptr;

  // Erase original return instruction.
  Ret->eraseFromParent();

  // In case of vector kernel the tid generators are incremented by the packet
  // width. In case of scalar loop increment by 1.
  Value *Dim0IncBy = IsVector ? ConstVF : ConstOne;
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    unsigned ResolvedDim = resolveDimension(Dim);
    computeDimStr(ResolvedDim, IsVector);
    Value *IncBy = (ResolvedDim == VectorizedDim) ? Dim0IncBy : ConstOne;
    LLVM_DEBUG(dbgs() << "    Add WG Loop for Dim " << Dim << " with step "
                      << *IncBy << "\n");
    // Create the loop
    Value *InitGID = InitGIDs[ResolvedDim];
    LoopRegion Blocks;
    PHINode *IndVar;
    std::tie(Blocks, IndVar) = DPCPPKernelLoopUtils::createLoop(
        Head, Latch, InitGID, IncBy, MaxGIDs[ResolvedDim],
        MaskedF ? CmpInst::ICMP_SGE : CmpInst::ICMP_EQ, DimStr, Ctx);
    // Modify get***id accordingly.
    if (!GIDs[ResolvedDim].empty()) {
      for (auto *TID : GIDs[ResolvedDim]) {
        TID->replaceAllUsesWith(IndVar);
        TID->eraseFromParent();
      }
    }
    if (!LIDs[ResolvedDim].empty()) {
      BasicBlock *InsertAtEnd = NewEntry;
      if (auto *PN = dyn_cast<PHINode>(InitGID))
        InsertAtEnd = PN->getParent();
      auto *InitLID = BinaryOperator::Create(Instruction::Sub, InitGID,
                                             getOrCreateBaseGID(ResolvedDim),
                                             DimStr + "sub_lid", InsertAtEnd);
      InitLID->setHasNoSignedWrap();
      InitLID->setHasNoUnsignedWrap();
      replaceTIDsWithPHI(LIDs[ResolvedDim], InitLID, IncBy, Head,
                         Blocks.PreHeader, Latch);
    }
    // head, latch for the next loop are the pre-header and exit block
    // respectively.
    Head = Blocks.PreHeader;
    Latch = Blocks.Exit;
    HeaderOutmost = Blocks.Header;
  }
  return LoopRegion{Head, HeaderOutmost, Latch};
}

void WGLoopCreatorImpl::replaceTIDsWithPHI(InstVec &TIDs, Value *InitVal,
                                           Value *IncBy, BasicBlock *Head,
                                           BasicBlock *PreHead,
                                           BasicBlock *Latch) {
  assert(TIDs.size() && "unexpected empty tid vector");
  PHINode *DimTID =
      PHINode::Create(IndTy, 2, DimStr + "tid", Head->getFirstNonPHI());
  BinaryOperator *IncTID =
      BinaryOperator::Create(Instruction::Add, DimTID, IncBy,
                             DimStr + "inc_tid", Latch->getTerminator());
  IncTID->setHasNoSignedWrap();
  IncTID->setHasNoUnsignedWrap();
  DimTID->addIncoming(InitVal, PreHead);
  DimTID->addIncoming(IncTID, Latch);
  DimTID->setDebugLoc(TIDs[0]->getDebugLoc());
  for (auto *TID : TIDs) {
    TID->replaceAllUsesWith(DimTID);
    TID->eraseFromParent();
  }
}

namespace {

class DPCPPKernelWGLoopCreatorLegacy : public ModulePass {
public:
  static char ID;

  DPCPPKernelWGLoopCreatorLegacy() : ModulePass(ID) {
    initializeDPCPPKernelWGLoopCreatorLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "WGLoopCreatorLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<UnifyFunctionExitNodesLegacyPass>();
  }
};

} // namespace

char DPCPPKernelWGLoopCreatorLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(DPCPPKernelWGLoopCreatorLegacy, DEBUG_TYPE,
                      "Create loops over dpcpp kernels", false, false)
INITIALIZE_PASS_DEPENDENCY(UnifyFunctionExitNodesLegacyPass)
INITIALIZE_PASS_END(DPCPPKernelWGLoopCreatorLegacy, DEBUG_TYPE,
                    "Create loops over dpcpp kernels", false, false)

bool DPCPPKernelWGLoopCreatorLegacy::runOnModule(Module &M) {
  FuncSet FSet = getAllKernels(M);
  MapFunctionToReturnInst FuncReturn;
  for (auto *F : FSet) {
    bool Changed = false;
    BasicBlock *SingleRetBB =
        getAnalysis<UnifyFunctionExitNodesLegacyPass>(*F, &Changed)
            .getReturnBlock();
    if (SingleRetBB)
      FuncReturn[F] = cast<ReturnInst>(SingleRetBB->getTerminator());
  }
  WGLoopCreatorImpl Impl(M, FuncReturn);
  return Impl.run();
}

llvm::ModulePass *llvm::createDPCPPKernelWGLoopCreatorLegacyPass() {
  return new DPCPPKernelWGLoopCreatorLegacy();
}

PreservedAnalyses DPCPPKernelWGLoopCreatorPass::run(Module &M,
                                                    ModuleAnalysisManager &) {
  FuncSet FSet = getAllKernels(M);
  MapFunctionToReturnInst FuncReturn;
  for (auto *F : FSet) {
    auto &BBList = F->getBasicBlockList();
    auto It = std::find_if(BBList.rbegin(), BBList.rend(), [](BasicBlock &BB) {
      return isa<ReturnInst>(BB.getTerminator());
    });
    if (It != BBList.rend())
      FuncReturn[F] = cast<ReturnInst>(It->getTerminator());
  }
  WGLoopCreatorImpl Impl(M, FuncReturn);
  return Impl.run() ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
