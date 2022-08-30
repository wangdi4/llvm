//==DPCPPKernelWGLoopCreator.cpp - Create WG loops in DPCPP kernels -*- C++-==//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelWGLoopCreator.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopPeeling.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/WGBoundDecoder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include <sstream>

using namespace llvm;
using namespace CompilationUtils;
using namespace DPCPPKernelMetadataAPI;

#define DEBUG_TYPE "dpcpp-kernel-wgloop-creator"

using MapFunctionToReturnInst = DenseMap<Function *, ReturnInst *>;
extern bool EnableTLSGlobals;
static constexpr StringRef PatchLocalIDsName = "local.ids";

namespace {
class WGLoopCreatorImpl {
public:
  WGLoopCreatorImpl(Module &M, bool UseTLSGlobals,
                    MapFunctionToReturnInst &FuncReturn)
      : M(M), Ctx(M.getContext()), Builder(Ctx),
        UseTLSGlobals(UseTLSGlobals || EnableTLSGlobals),
        FuncReturn(FuncReturn) {}

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

  bool UseTLSGlobals;

  /// Prefix for name of instructions used for loop of the dimension.
  std::string DimStr;

  // Remainder scalar or masked vector kernel return.
  ReturnInst *RemainderRet = nullptr;

  /// vector kernel return.
  ReturnInst *VectorRet = nullptr;

  // size_t type.
  Type *IndTy = nullptr;

  ArrayType *LIDArrayTy = nullptr;

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

  /// TLS global local IDs.
  GlobalVariable *LocalIds = nullptr;

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

private:
  /// Create TLS local ids global variable for native debugging.
  /// \returns true if the TLS global is created.
  bool createTLSLocalIds();

  /// Process TID calls in  not-inlined functions.
  bool processTIDInNotInlinedFuncs();

  /// Fix TID calls in patched functions.
  /// get_local_id is replaced with load instruction from either TLS local id or
  /// patched function argument.
  /// get_global_id is replaced with sum of local id and get_base_global_id.
  void fixTIDCallInNotInlinedFuncs(InstVec &TIDCalls);

  /// Find TID calls in not-inlined functions, which either are used by kernels
  /// without barrier path or are not used by any kernel.
  /// \returns a vector of TID calls.
  InstVec findTIDCallsInNotInlinedFuncs(FuncSet &KernelAndSyncFuncs,
                                        FuncSet &Kernels);

  /// In non-TLS mode, patch TID calls in not-inlined functions and
  /// the functions themself prior to creating WG loops.
  void patchNotInlinedFuncs(FuncSet &KernelAndSyncFuncs,
                            InstVec &TIDCallsToFix);

  /// Collect the get_global_id(), get_local_id(), and return of F.
  /// F - kernel to collect information for.
  /// Gids - array of get_global_id call to fill.
  /// Lids - array of get_local_id call to fill.
  /// Returns kernel single return instruction.
  ReturnInst *getFunctionData(Function *F, InstVecVec &Gids, InstVecVec &Lids);

  /// Process a kernel function.
  void processFunction(Function *F, Function *VectorF, unsigned VF);

  /// Create the early exit call.
  void createEECall(Function *Func);

  /// Create a condition jump over the entire WG loops according to uniform
  /// early exit value.
  /// \param RetBB Return block to jump to in case of uniform early exit.
  void handleUniformEE(BasicBlock *RetBB);

  /// Extract initial GID of the dimension from the early exit call.
  Value *getEEInitGid(unsigned Dim);

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
  /// Returns a pair of LoopRegion and IndVar on vectorization dim.
  std::pair<LoopRegion, Value *> addWGLoops(BasicBlock *KernelEntry,
                                            bool IsVector, ReturnInst *Ret,
                                            InstVecVec &GIDs, InstVecVec &LIDs,
                                            ValueVec &InitGIDs,
                                            ValueVec &MaxGIDs);

  /// Create local id phi in loop head.
  /// InitVal - inital value (for the first iteration).
  /// IncBy - amount by which to increase the tid in each iteration.
  /// Head - head block of the loop.
  /// PreHead - pre header of the loop.
  /// Latch - latch block of the loop.
  PHINode *createLIDPHI(Value *InitVal, Value *IncBy, BasicBlock *Head,
                        BasicBlock *PreHead, BasicBlock *Latch);

  /// Create WG loops over scalar kernel.
  /// \returns a struct with entry and exit block of the WG loop region.
  LoopRegion createScalarLoops();

  /// Create WG loops over vector kernel and remainder loop over scalar kernel.
  /// \returns a struct with entry and exit block of the WG loop region.
  LoopRegion createVectorAndRemainderLoops();

  /// Create WG loops over vector kernel and remainder loop over masked vector
  /// kernel.
  LoopRegion createVectorAndMaskedRemainderLoops();

  /// Create WG loops over peeling loop over scalar or masked vector kernel,
  LoopRegion
  createPeelAndVectorAndRemainderLoops(LoopBoundaries &Dim0Boundaries);

  /// Create WG loops over masked kernel.
  /// \returns a struct with entry and exit block of the WG loop region.
  LoopRegion createMaskedLoop();

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
  IndTy = LoopUtils::getIndTy(&M);
  ConstZero = ConstantInt::get(IndTy, 0);
  ConstOne = ConstantInt::get(IndTy, 1);

  bool Changed = createTLSLocalIds();

  Changed |= processTIDInNotInlinedFuncs();

  auto Kernels = getKernels(M);
  for (auto *F : Kernels) {
    KernelInternalMetadataAPI KIMD(F);
    // No need to check if NoBarrierPath Value exists, it is guaranteed that
    // KernelAnalysisPass ran before WGLoopCreator pass.
    if (!KIMD.NoBarrierPath.get()) {
      // Kernel that should be handled in barrier path, skip it.
      LLVM_DEBUG(dbgs() << "Skip " << F->getName()
                        << " since no_barrier_path is false\n");
      continue;
    }

    unsigned VectWidth = 0;
    // Get the vectorized function
    Function *VectKernel = KIMD.VectorizedKernel.get();
    Function *MaskedKernel = KIMD.VectorizedMaskedKernel.get();
    // Need to check if vectorized kernel exists, it is not guaranteed that
    // vectorizer is running in all scenarios.
    Function *VectOrMaskedKernel = VectKernel     ? VectKernel
                                   : MaskedKernel ? MaskedKernel
                                                  : nullptr;
    if (VectOrMaskedKernel) {
      // Get the vectorized width
      KernelInternalMetadataAPI VKIMD(VectOrMaskedKernel);
      VectWidth = VKIMD.VectorizedWidth.get();

      // Save the relevant information from the vectorized kernel in KIMD prior
      // to erasing these information.
      KIMD.VectorizedKernel.set(nullptr);
      KIMD.VectorizedWidth.set(VectWidth);
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

bool WGLoopCreatorImpl::createTLSLocalIds() {
  LIDArrayTy = ArrayType::get(IndTy, MAX_WORK_DIM);
  if (UseTLSGlobals) {
    // Add TLS for local IDs.
    LocalIds = new GlobalVariable(
        M, LIDArrayTy, /*isConstant*/ false, GlobalValue::LinkOnceODRLinkage,
        UndefValue::get(LIDArrayTy), getTLSLocalIdsName(),
        /*InsertBefore*/ nullptr, GlobalValue::GeneralDynamicTLSModel);
    LocalIds->setAlignment(M.getDataLayout().getPreferredAlign(LocalIds));
    return true;
  }
  return false;
}

bool WGLoopCreatorImpl::processTIDInNotInlinedFuncs() {
  // Get all synchronize built-ins declared in module and their user functions.
  FuncSet SyncFunctions = getAllSyncBuiltinsDecls(M);
  FuncSet KernelAndSyncFuncs;
  LoopUtils::fillFuncUsersSet(SyncFunctions, KernelAndSyncFuncs);

  FuncSet Kernels = getAllKernels(M);
  KernelAndSyncFuncs.insert(Kernels.begin(), Kernels.end());

  InstVec TIDCallsToFix =
      findTIDCallsInNotInlinedFuncs(KernelAndSyncFuncs, Kernels);

  if (TIDCallsToFix.empty())
    return false;

  if (!UseTLSGlobals)
    patchNotInlinedFuncs(KernelAndSyncFuncs, TIDCallsToFix);

  fixTIDCallInNotInlinedFuncs(TIDCallsToFix);

  return true;
}

InstVec
WGLoopCreatorImpl::findTIDCallsInNotInlinedFuncs(FuncSet &KernelAndSyncFuncs,
                                                 FuncSet &Kernels) {
  InstVec TIDCalls;

  auto FindTIDsToPatch = [&](const InstVec &TIDs) {
    for (auto *I : TIDs) {
      auto *CI = cast<CallInst>(I);
      if (!KernelAndSyncFuncs.contains(CI->getFunction()))
        TIDCalls.push_back(CI);
    }
  };
  FindTIDsToPatch(getCallInstUsersOfFunc(M, mangledGetLID()));
  FindTIDsToPatch(getCallInstUsersOfFunc(M, mangledGetGID()));

  if (TIDCalls.empty())
    return TIDCalls;

  FuncSet NoBarrierPathKernels;
  FuncSet BarrierPathKernels;
  for (auto *Kernel : Kernels) {
    KernelInternalMetadataAPI KIMD(Kernel);
    if (KIMD.NoBarrierPath.get())
      NoBarrierPathKernels.insert(Kernel);
    else
      BarrierPathKernels.insert(Kernel);
  }

  FuncSet TIDCallers;
  for (auto *CI : TIDCalls)
    TIDCallers.insert(CI->getFunction());

  // Find TID callers that are used kernels without barrier path.
  FuncSet TIDCallersToFix;
  // Compute call graph on demand.
  CallGraph CG(M);
  for (auto *Kernel : NoBarrierPathKernels) {
    CallGraphNode *N = CG[Kernel];
    for (auto It = ++df_begin(N); It != df_end(N); ++It)
      if (TIDCallers.contains(It->getFunction()))
        TIDCallersToFix.insert(It->getFunction());
  }

  InstVec TIDCallsToFix;
  InstVec TIDCallsMayInBarrierPath;
  FuncSet TIDCallersMayInBarrierPath;
  for (auto *I : TIDCalls) {
    if (TIDCallersToFix.contains(I->getFunction())) {
      TIDCallsToFix.push_back(I);
    } else {
      TIDCallsMayInBarrierPath.push_back(I);
      TIDCallersMayInBarrierPath.insert(I->getFunction());
    }
  }

  // Find TID callers that aren't used by any kernel.
  FuncSet TIDCallersInBarrierPath;
  for (auto *Kernel : BarrierPathKernels) {
    CallGraphNode *N = CG[Kernel];
    for (auto It = ++df_begin(N); It != df_end(N); ++It)
      if (TIDCallersMayInBarrierPath.contains(It->getFunction()))
        TIDCallersInBarrierPath.insert(It->getFunction());
  }
  for (auto *I : TIDCallsMayInBarrierPath)
    if (!TIDCallersInBarrierPath.count(I->getFunction()))
      TIDCallsToFix.push_back(I);

  return TIDCallsToFix;
}

void WGLoopCreatorImpl::patchNotInlinedFuncs(FuncSet &KernelAndSyncFuncs,
                                             InstVec &TIDCallsToFix) {
  auto *LocalIdAllocTy = PointerType::get(LIDArrayTy, 0);
  DenseMap<Function *, Value *> PatchedFToLocalIdArg;
  DenseMap<Function *, std::pair<SmallVector<Value *, MAX_WORK_DIM>, Value *>>
      FuncToCreatedLIDArg;
  auto CreateLIDArg = [&](CallInst *CI) -> Value * {
    // Caller is either kernel function or function with barrier path.
    Function *Caller = CI->getFunction();
    SmallVector<Value *, MAX_WORK_DIM> LIDs;
    Value *LIDArg = nullptr;
    auto It = FuncToCreatedLIDArg.find(Caller);
    if (It == FuncToCreatedLIDArg.end()) {
      Builder.SetInsertPoint(&*Caller->getEntryBlock().begin());
      auto *AI = Builder.CreateAlloca(LIDArrayTy, nullptr, PatchLocalIDsName);
      for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
        auto *LID = Builder.CreateInBoundsGEP(
            LIDArrayTy, AI, {ConstZero, Builder.getInt32(Dim)},
            AppendWithDimension("local.id", Dim));
        LIDs.push_back(LID);
      }
      LIDArg = AI;
    } else {
      LIDs = It->second.first;
      LIDArg = It->second.second;
    }

    // TODO only do the stores once.
    Builder.SetInsertPoint(CI);
    for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
      auto *LIDCall = LoopUtils::getWICall(&M, mangledGetLID(), IndTy, Dim, CI,
                                           Twine("lid") + Twine(Dim));
      Builder.CreateStore(LIDCall, LIDs[Dim]);
    }

    if (It == FuncToCreatedLIDArg.end())
      FuncToCreatedLIDArg.insert({Caller, {LIDs, LIDArg}});

    return LIDArg;
  };

  patchNotInlinedTIDUserFunc(M, Builder, KernelAndSyncFuncs, TIDCallsToFix,
                             PatchedFToLocalIdArg, LocalIdAllocTy,
                             CreateLIDArg);
}

void WGLoopCreatorImpl::fixTIDCallInNotInlinedFuncs(InstVec &TIDCalls) {
  using KeyTy = std::pair<Function *, uint64_t>;
  DenseMap<KeyTy, std::pair<Value *, AllocaInst *>> FuncsToGID;
  DenseMap<KeyTy, std::pair<Value *, AllocaInst *>> FuncsToLID;
  DenseMap<Function *, Instruction *> FuncsToInsertPoint;

  // Create LID values for both LID and GID calls.
  for (auto *I : TIDCalls) {
    LLVM_DEBUG(dbgs() << "Replace " << *I << " in not-inlined function "
                      << I->getFunction()->getName() << "\n");
    auto *CI = cast<CallInst>(I);
    auto *F = CI->getFunction();
    ConstantInt *DimVal = cast<ConstantInt>(CI->getArgOperand(0));
    uint64_t Dim = DimVal->getZExtValue();

    Value *LID;
    AllocaInst *LIDAddr;
    KeyTy Key{F, Dim};
    auto It = FuncsToLID.find(Key);
    if (It != FuncsToLID.end()) {
      LID = It->second.first;
      LIDAddr = It->second.second;
    } else {
      // Create LID.
      auto PtRes = FuncsToInsertPoint.insert(
          {F, &*F->getEntryBlock().getFirstInsertionPt()});
      Builder.SetInsertPoint(PtRes.first->second);
      LIDAddr = Builder.CreateAlloca(
          IndTy, nullptr, Twine("lid") + Twine(Dim) + Twine(".addr"));
      if (UseTLSGlobals) {
        Value *Ptr = createGetPtrToLocalId(LocalIds, DimVal, Builder);
        LID = Builder.CreateLoad(IndTy, Ptr, AppendWithDimension("lid", Dim));
      } else {
        auto *LIDArg = &*(F->arg_end() - 1);
        Value *GEP = Builder.CreateInBoundsGEP(
            LIDArrayTy, LIDArg, {ConstZero, Builder.getInt32(Dim)},
            AppendWithDimension("ptr.lid", Dim));
        LID = Builder.CreateLoad(IndTy, GEP, AppendWithDimension("lid", Dim));
      }
      Builder.CreateStore(LID, LIDAddr);
      FuncsToLID.insert({Key, {LID, LIDAddr}});
    }

    // Fix LID calls.
    if (isGetLocalId(CI->getCalledFunction()->getName())) {
      Builder.SetInsertPoint(CI);
      auto *LI = Builder.CreateLoad(IndTy, LIDAddr,
                                    Twine("lid") + Twine(Dim) + Twine(".ld"));
      CI->replaceAllUsesWith(LI);
    }
  }

  // Fix GID calls.
  for (auto *I : TIDCalls) {
    auto *CI = cast<CallInst>(I);
    if (!isGetGlobalId(CI->getCalledFunction()->getName()))
      continue;

    auto *F = CI->getFunction();
    ConstantInt *DimVal = cast<ConstantInt>(CI->getArgOperand(0));
    uint64_t Dim = DimVal->getZExtValue();

    Value *GID;
    AllocaInst *GIDAddr;
    KeyTy Key{F, Dim};
    auto It = FuncsToGID.find(Key);
    if (It != FuncsToGID.end()) {
      GID = It->second.first;
      GIDAddr = It->second.second;
    } else {
      // Create GID.
      Value *LID = FuncsToLID[Key].first;
      Builder.SetInsertPoint(FuncsToInsertPoint[F]);
      GIDAddr = Builder.CreateAlloca(
          IndTy, nullptr, Twine("gid") + Twine(Dim) + Twine(".addr"));
      CallInst *BaseGID =
          LoopUtils::getWICall(&M, nameGetBaseGID(), IndTy, DimVal, Builder,
                               Twine("base.gid") + Twine(Dim));
      GID = Builder.CreateAdd(LID, BaseGID, AppendWithDimension("gid", Dim));
      Builder.CreateStore(GID, GIDAddr);
      FuncsToGID.insert({Key, {GID, GIDAddr}});
    }

    Builder.SetInsertPoint(CI);
    auto *LI = Builder.CreateLoad(IndTy, GIDAddr,
                                  Twine("gid") + Twine(Dim) + Twine(".ld"));
    CI->replaceAllUsesWith(LI);
  }

  for (auto *I : TIDCalls)
    I->eraseFromParent();

  // Move alloca to beginning of entry basic block.
  for (auto &Pair : FuncsToInsertPoint) {
    auto &BB = Pair.first->getEntryBlock();
    moveAlloca(&BB, &BB);
  }
}

static void disableLoopUnrollRecursively(Loop *L) {
  LLVM_DEBUG(dbgs().indent(2) << "Disable loop unroll for loop (header: "
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
  LLVM_DEBUG(dbgs() << "Creating WG loop for " << F->getName() << ", VF " << VF
                    << "\n";);
  LLVM_DEBUG(dbgs().indent(2));
  LLVM_DEBUG(if (VectorF) {
    dbgs() << "Vector function name: " << VectorF->getName() << "\n";
  });

  KernelInternalMetadataAPI KIMD(F);

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
  Builder.SetCurrentDebugLocation(DebugLoc());

  // generate constants.
  ConstVF = ConstantInt::get(IndTy, VF);

  // Get the number of the for which we need to create work group loops.
  NumDim = KIMD.MaxWGDimensions.hasValue() ? KIMD.MaxWGDimensions.get()
                                           : MAX_WORK_DIM;
  assert((NumDim == 0 || VectorizedDim < NumDim) &&
         "vectorized dimension is out of range");

  // Create WG loops.
  // If no workgroup loop is created (no calls to get_*_id), avoid inlining
  // the vector function into the scalar one since there is only one workitem
  // to be executed.
  if (KIMD.VectorizedMaskedKernel.hasValue()) {
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

  // Get scalar or marked kernel entry and create new entry block for
  // boundaries calculation.
  RemainderEntry = &Func->getEntryBlock();
  RemainderEntry->setName(MaskedF ? "masked_kernel_entry"
                                  : "scalar_kernel_entry");
  NewEntry = BasicBlock::Create(Ctx, "entry", Func, RemainderEntry);

  // Create early exit call to obtain boundaries from.
  createEECall(Func);

  // Obtain loops boundaries from early exit call.
  getLoopsBoundaries();

  initializeImplicitGID(Func);

  // Create loops.
  LoopRegion WGLoopRegion = (VectorF && MaskedF)
                                ? createVectorAndMaskedRemainderLoops()
                            : VectorF ? createVectorAndRemainderLoops()
                            : MaskedF ? createMaskedLoop()
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

  // Now the masked kernel is a full workgroup which is organized as one of
  //   * several loops of vector kernel following by one masked kernel
  //   * loops of masked kernel.
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

  LLVM_DEBUG(dbgs().indent(2) << "Created loop for " << F->getName() << "\n";);
  assert(!verifyFunction(*F, &dbgs()));
}

Value *WGLoopCreatorImpl::getEEInitGid(unsigned Dim) {
  unsigned LowerInd = WGBoundDecoder::getIndexOfInitGidAtDim(Dim);
  return ExtractValueInst::Create(EECall, LowerInd,
                                  Twine("init.gid.dim") + Twine(Dim), NewEntry);
}

void WGLoopCreatorImpl::initializeImplicitGID(Function *F) {
  ImplicitGIDs.clear();

  if (!F->getSubprogram())
    return;

  // Find implicit GID alloca instructions. Implicit GIDs are in order, e.g.
  // __ocl_dbg_gid0 is before __ocl_dbg_gid1, due to the way they are
  // inserted.
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

  // If NumDim is 0, workgroup loop won't be created, and we don't need to
  // store initial GID to implicit GID, because ImplicitGIDPass already did
  // it.
  if (NumDim == 0)
    return;

  // Get initial GID and store to implicit GIDs.
  // Insert to new entry block.
  Builder.SetInsertPoint(NewEntry);
  Builder.SetCurrentDebugLocation(DebugLoc());
  for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
    Builder.CreateStore(InitGIDs[Dim], ImplicitGIDs[Dim],
                        /* isVolatile */ true);
}

// Create the following scalar loops:
//
// scalar_preheader:
//   scalar loops
// return
//
LoopRegion WGLoopCreatorImpl::createScalarLoops() {
  LLVM_DEBUG(dbgs().indent(2) << "Create scalar loops\n");
  if (VectorF)
    VectorF->eraseFromParent();
  LoopRegion Region;
  std::tie(Region, std::ignore) =
      addWGLoops(RemainderEntry, false, RemainderRet, GidCalls, LidCalls,
                 InitGIDs, MaxGIDs);
  return Region;
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
  LLVM_DEBUG(dbgs().indent(2) << "Create vector and scalar remainder loops\n");
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
  LoopRegion VectorBlocks;
  std::tie(VectorBlocks, std::ignore) =
      addWGLoops(VectorEntry, true, VectorRet, GidCallsVec, LidCallsVec,
                 InitGIDsCopy, MaxGIDs);

  // Create scalar loops.
  InitGIDsCopy[VectorizedDim] = Dim0Boundaries.MaxVector;
  MaxGIDs[VectorizedDim] = MaxGIDTemp;
  LoopRegion ScalarBlocks;
  std::tie(ScalarBlocks, std::ignore) =
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
  LLVM_DEBUG(dbgs().indent(2) << "Create vector and masked remainder loops\n");
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

  ValueVec InitGIDsCopy = InitGIDs; // hard copy.
  Value *MaxGIDTemp = MaxGIDs[VectorizedDim];

  // Create vector loops.
  MaxGIDs[VectorizedDim] = Dim0Boundaries.MaxVector;
  LoopRegion VectorBlocks;
  std::tie(VectorBlocks, std::ignore) =
      addWGLoops(VectorEntry, true, VectorRet, GidCallsVec, LidCallsVec,
                 InitGIDsCopy, MaxGIDs);

  // Create masked vector loop.
  InitGIDsCopy[VectorizedDim] = Dim0Boundaries.MaxVector;
  MaxGIDs[VectorizedDim] = MaxGIDTemp;
  LoopRegion MaskedBlocks;
  std::tie(MaskedBlocks, std::ignore) =
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
  auto *Mask = LoopUtils::generateRemainderMask(
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
  LLVM_DEBUG(dbgs().indent(2)
             << (MaskedF
                     ? "Create masked peel, vector and masked remainder loops"
                     : "Create scalar peel, vector and scalar remainder loops")
             << "\n");

  bool IsMasked = MaskedF != nullptr;
  Function *Func = IsMasked ? MaskedF : F;

  ValueVec InitGIDsCopy = InitGIDs; // hard copy.

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
  LoopRegion VectorBlocks;
  std::tie(VectorBlocks, std::ignore) =
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
    auto *Mask =
        LoopUtils::generateRemainderMask(VF, ScalarLoopSize, RemainderPreEntry);
    auto MaskArg = Func->arg_end() - 1;
    MaskArg->replaceAllUsesWith(Mask);
  }

  LoopRegion RemainderBlocks;
  std::tie(RemainderBlocks, std::ignore) =
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

LoopRegion WGLoopCreatorImpl::createMaskedLoop() {
  LLVM_DEBUG(dbgs().indent(2) << "Create masked loops\n");
  LoopRegion MaskedBlocks;
  Value *VecDimIndVar;
  std::tie(MaskedBlocks, VecDimIndVar) =
      addWGLoops(RemainderEntry, true, RemainderRet, GidCalls, LidCalls,
                 InitGIDs, MaxGIDs);
  assert(VecDimIndVar && "invalid IndVar on vectorization dim");

  // Generate mask.
  Builder.SetInsertPoint(&*RemainderEntry->getFirstInsertionPt());
  Value *Splat = Builder.CreateVectorSplat(VF, VecDimIndVar, "ind.var");
  Value *StepVec = Builder.CreateStepVector(FixedVectorType::get(IndTy, VF));
  Value *IndVars = Builder.CreateNUWAdd(Splat, StepVec, "ind.var.vec");
  Value *MaxGIDSplat =
      Builder.CreateVectorSplat(VF, MaxGIDs[VectorizedDim], "max.gid");
  Value *Mask = Builder.CreateICmpULT(IndVars, MaxGIDSplat, "ind.var.mask.i1");

  Value *MaskArg = &*(MaskedF->arg_end() - 1);
  if (Mask->getType() != MaskArg->getType()) {
    assert(cast<FixedVectorType>(Mask->getType())->getNumElements() == VF &&
           "number of elements doesn't match");
    Mask = Builder.CreateZExt(Mask, MaskArg->getType(), "ind.var.mask");
  }
  MaskArg->replaceAllUsesWith(Mask);

  return MaskedBlocks;
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
      PeelLoopSize = PeelSize.value();
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

  for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
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
      LLVM_DEBUG(dbgs().indent(2)
                 << "Erase vector implicit GID: " << *I << "\n");
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
      ExtractValueInst::Create(EECall, UniInd, "uniform.early.exit", InsertPt);
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
  LoopUtils::collectTIDCallInst(GID, Gids, F);
  LoopUtils::collectTIDCallInst(LID, Lids, F);

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
  CallInst *BaseGID =
      LoopUtils::getWICall(F->getParent(), nameGetBaseGID(), IndTy, Dim,
                           NewEntry, Twine("base.gid.dim") + Twine(Dim));
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
      InitGID = getEEInitGid(Dim);
      unsigned LoopSizeInd = WGBoundDecoder::getIndexOfSizeAtDim(Dim);
      LoopSize = ExtractValueInst::Create(
          EECall, LoopSizeInd, Twine("loop.size.dim") + Twine(Dim), NewEntry);
    } else {
      InitGID = getOrCreateBaseGID(Dim);
      LoopSize = LoopUtils::getWICall(F->getParent(), mangledGetLocalSize(),
                                      IndTy, Dim, NewEntry,
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

  // Create initial TID with dimension on which no loop will be created. They
  // are typically created by ImplicitGID pass.
  for (unsigned Dim = NumDim; Dim < MAX_WORK_DIM; ++Dim) {
    if (GidCalls[Dim].empty())
      break;
    Value *InitGID =
        EECall ? getEEInitGid(Dim)
               : LoopUtils::getWICall(F->getParent(), nameGetBaseGID(), IndTy,
                                      Dim, NewEntry);
    InitGIDs.push_back(InitGID);
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

std::pair<LoopRegion, Value *> WGLoopCreatorImpl::addWGLoops(
    BasicBlock *KernelEntry, bool IsVector, ReturnInst *Ret, InstVecVec &GIDs,
    InstVecVec &LIDs, ValueVec &InitGIDs, ValueVec &MaxGIDs) {
  assert(KernelEntry && Ret && "uninitialized parameters");

  // Move alloca, llvm.dbg.declare and local id GEP in the entry kernel entry
  // block to the new entry block.
  moveInstructionIf(KernelEntry, NewEntry, [](Instruction &I) {
    if (isa<AllocaInst>(&I))
      return true;
    if (auto *DI = dyn_cast<DbgDeclareInst>(&I)) {
      Metadata *Meta = cast<MetadataAsValue>(DI->getOperand(0))->getMetadata();
      if (!isa<ValueAsMetadata>(Meta))
        return true;
      auto *AI = dyn_cast<AllocaInst>(cast<ValueAsMetadata>(Meta)->getValue());
      // Don't move implicit gid llvm.dbg.declare.
      return !AI || !isImplicitGID(AI);
    }
    auto *GEP = dyn_cast<GetElementPtrInst>(&I);
    return GEP && GEP->getPointerOperand()->getName() == PatchLocalIDsName;
  });

  // Initial head and latch are the kernel entry and return block
  // respectively.
  BasicBlock *Head = KernelEntry;
  BasicBlock *Latch = Ret->getParent();
  LLVM_DEBUG(dbgs().indent(2) << "Add " << (IsVector ? "vector" : "scalar")
                              << " WG Loops. Head: " << Head->getName()
                              << ". Latch: " << Latch->getName() << "\n");

  // Header of outmost loop.
  BasicBlock *HeaderOutmost = nullptr;

  // Erase original return instruction.
  Ret->eraseFromParent();

  // In case of vector kernel the tid generators are incremented by the packet
  // width. In case of scalar loop increment by 1.
  Value *Dim0IncBy = IsVector ? ConstVF : ConstOne;

  Value *VecDimIndVar = nullptr;

  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    unsigned ResolvedDim = resolveDimension(Dim);
    computeDimStr(ResolvedDim, IsVector);
    Value *IncBy = (ResolvedDim == VectorizedDim) ? Dim0IncBy : ConstOne;
    LLVM_DEBUG(dbgs().indent(4) << "Add WG Loop for Dim " << Dim
                                << " with step " << *IncBy << "\n");
    // Create the loop
    Value *InitGID = InitGIDs[ResolvedDim];
    LoopRegion Blocks;
    PHINode *IndVar;
    std::tie(Blocks, IndVar) = LoopUtils::createLoop(
        Head, Latch, InitGID, IncBy, MaxGIDs[ResolvedDim],
        MaskedF ? CmpInst::ICMP_SGE : CmpInst::ICMP_EQ, DimStr, Ctx);

    if (Dim == VectorizedDim)
      VecDimIndVar = IndVar;

    // Modify get***id accordingly.
    if (!GIDs[ResolvedDim].empty()) {
      for (auto *TID : GIDs[ResolvedDim]) {
        LLVM_DEBUG(dbgs().indent(6)
                   << "Replace " << *TID << " with " << *IndVar << "\n");
        TID->replaceAllUsesWith(IndVar);
        TID->eraseFromParent();
      }
    }

    PHINode *LIDPhi = nullptr;
    if (!LIDs[ResolvedDim].empty() || UseTLSGlobals) {
      BasicBlock *InsertAtEnd = NewEntry;
      if (auto *PN = dyn_cast<PHINode>(InitGID))
        InsertAtEnd = PN->getParent();
      auto *InitLID = BinaryOperator::Create(Instruction::Sub, InitGID,
                                             getOrCreateBaseGID(ResolvedDim),
                                             DimStr + "sub_lid", InsertAtEnd);
      InitLID->setHasNoSignedWrap();
      InitLID->setHasNoUnsignedWrap();
      LIDPhi = createLIDPHI(InitLID, IncBy, Head, Blocks.PreHeader, Latch);
    }

    // Replace the get_local_tid calls with incremented phi in loop head.
    if (!LIDs[ResolvedDim].empty()) {
      LIDPhi->setDebugLoc(LIDs[ResolvedDim][0]->getDebugLoc());
      for (auto *TID : LIDs[ResolvedDim]) {
        LLVM_DEBUG(dbgs().indent(6)
                   << "Replace " << *TID << " with " << *LIDPhi << "\n");
        TID->replaceAllUsesWith(LIDPhi);
        TID->eraseFromParent();
      }
    }

    if (UseTLSGlobals) {
      Builder.SetInsertPoint(Head->getFirstNonPHI());
      Value *Ptr = createGetPtrToLocalId(
          LocalIds, ConstantInt::get(Type::getInt32Ty(Ctx), Dim), Builder);
      Builder.CreateStore(LIDPhi, Ptr);
    }

    // head, latch for the next loop are the pre-header and exit block
    // respectively.
    Head = Blocks.PreHeader;
    Latch = Blocks.Exit;
    HeaderOutmost = Blocks.Header;
  }

  // Resolve TID with dimension on which no loop is created. They are
  // typically created by ImplicitGID pass.
  for (unsigned Dim = NumDim; Dim < MAX_WORK_DIM; ++Dim) {
    for (auto *GID : GIDs[Dim]) {
      GID->replaceAllUsesWith(InitGIDs[Dim]);
      GID->eraseFromParent();
    }

    assert(LIDs[Dim].empty() && "Unexpected Local Id on the dimension");
  }

  return {LoopRegion{Head, HeaderOutmost, Latch}, VecDimIndVar};
}

PHINode *WGLoopCreatorImpl::createLIDPHI(Value *InitVal, Value *IncBy,
                                         BasicBlock *Head, BasicBlock *PreHead,
                                         BasicBlock *Latch) {
  PHINode *DimTID =
      PHINode::Create(IndTy, 2, DimStr + "tid", Head->getFirstNonPHI());
  BinaryOperator *IncTID =
      BinaryOperator::Create(Instruction::Add, DimTID, IncBy,
                             DimStr + "inc_tid", Latch->getTerminator());
  IncTID->setHasNoSignedWrap();
  IncTID->setHasNoUnsignedWrap();
  DimTID->addIncoming(InitVal, PreHead);
  DimTID->addIncoming(IncTID, Latch);
  return DimTID;
}

namespace {

class DPCPPKernelWGLoopCreatorLegacy : public ModulePass {
public:
  static char ID;

  DPCPPKernelWGLoopCreatorLegacy(bool UseTLSGlobals = false)
      : ModulePass(ID), UseTLSGlobals(UseTLSGlobals) {
    initializeDPCPPKernelWGLoopCreatorLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "WGLoopCreatorLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<UnifyFunctionExitNodesLegacyPass>();
  }

private:
  bool UseTLSGlobals;
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
  WGLoopCreatorImpl Impl(M, UseTLSGlobals, FuncReturn);
  return Impl.run();
}

llvm::ModulePass *
llvm::createDPCPPKernelWGLoopCreatorLegacyPass(bool UseTLSGlobals) {
  return new DPCPPKernelWGLoopCreatorLegacy(UseTLSGlobals);
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
  WGLoopCreatorImpl Impl(M, UseTLSGlobals, FuncReturn);
  return Impl.run() ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
