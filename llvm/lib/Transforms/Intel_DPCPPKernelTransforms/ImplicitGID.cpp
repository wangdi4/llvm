//==-- ImplicitGID.cpp - Add implicit gid for OpenCL debugging -------------==//
//
// Copyright (C) 2022 Intel Corporation
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
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ImplicitGID.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-implicit-gid"

namespace {
class ImplicitGIDImpl {
public:
  ImplicitGIDImpl(Module &M, bool HandleBarrier, DataPerBarrier *DPB)
      : M(M), HandleBarrier(HandleBarrier), DPB(DPB), DIB(new DIBuilder(M)) {}

  bool run();

private:
  /// @brief execute pass on given function.
  /// @param F Function to modify.
  /// @returns True if function was modified.
  bool runOnFunction(Function &F);

  /// @brief Insert alloca and llvm.dbg.declare instructions of implicit GID
  /// variables.
  /// @param F Function to modify.
  /// @param HasSyncInst Function has dummy_barrier or barrier calls.
  /// @param HasSyncInst Function has dummy_sg_barrier or sg_barrier calls.
  void insertGIDAlloca(Function &F, bool HasSyncInst, bool HasSGSyncInst);

  /// @brief Insert store instructions of implicit GID variables for given
  /// function.
  /// @param F Function to modify.
  /// @param HasSyncInst Function has dummy_barrier or barrier calls.
  /// @param HasSyncInst Function has dummy_sg_barrier or sg_barrier calls.
  void insertGIDStore(Function &F, bool HasSyncInst, bool HasSGSyncInst);

  /// @brief Insert store instructions of implicit GID variables at given
  /// insert point.
  /// @param B IRBuilder reference.
  /// @param InsertPoint Instruction insert point.
  void insertGIDStore(IRBuilder<> &B, Instruction *InsertPoint);

  /// @brief Gets or create Ind debug info tyoe.
  /// returns Ind DIType.
  DIType *getOrCreateIndDIType() const;

private:
  /// This holds the processed module.
  Module &M;

  /// Handle barrier if true, kernels without barrier otherwise.
  bool HandleBarrier;

  /// This holds the data per barrier analysis pass.
  DataPerBarrier *DPB;

  /// This holds the debug info builder.
  std::unique_ptr<DIBuilder> DIB;

  /// This holds debug information for a module which can be queried.
  DebugInfoFinder DIFinder;

  /// This is barrier utility class.
  BarrierUtils BUtils;

  /// This is subgroup emulation helper class.
  SGHelper SGH;

  /// This holds the set of functions containing subgroup barrier.
  FuncSet SGSyncFuncSet;

  /// This holds the insert point at entry block for the running function.
  Instruction *InsertPoint;

  /// This holds the GID allocas.
  Instruction *GIDAllocas[3];

  /// This holds the Ind DIType for GID variables.
  DIType *IndDIType;

  /// Skip insertDeclare if true.
  bool SkipInsertDbgDeclare = false;
};
} // namespace

bool ImplicitGIDImpl::run() {
  BUtils.init(&M);
  SGH.initialize(M);
  SGSyncFuncSet = SGH.getAllSyncFunctions();

  // Prime a DebugInfoFinder that can be queried about various bits of
  // debug information in the module.
  DIFinder.processModule(M);

  // Create the Ind DebugInfo type for GID variables.
  IndDIType = getOrCreateIndDIType();

  // Collect kernels.
  SmallSet<Function *, 8> Kernels;
  for (Function *F : DPCPPKernelMetadataAPI::KernelList(&M))
    Kernels.insert(F);

  bool Changed = false;

  for (auto &F : M) {
    auto KIMD = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
    if (HandleBarrier) {
      if (!(KIMD.NoBarrierPath.hasValue() && KIMD.NoBarrierPath.get()))
        Changed |= runOnFunction(F);
      continue;
    }

    if (!(Kernels.count(&F) && KIMD.NoBarrierPath.hasValue() &&
          KIMD.NoBarrierPath.get()))
      continue;

    SkipInsertDbgDeclare = false;
    Changed |= runOnFunction(F);

    SkipInsertDbgDeclare = true;
    if (KIMD.VectorizedKernel.hasValue()) {
      Function *VectorF = KIMD.VectorizedKernel.get();
      if (VectorF)
        Changed |= runOnFunction(*VectorF);
    }
    if (KIMD.VectorizedMaskedKernel.hasValue()) {
      Function *MaskedF = KIMD.VectorizedMaskedKernel.get();
      if (MaskedF)
        Changed |= runOnFunction(*MaskedF);
    }
  }

  return Changed;
}

bool ImplicitGIDImpl::runOnFunction(Function &F) {
  // Skip all functions without debug info (including built-ins, externals, etc)
  // We can't do anything for them
  if (!F.getSubprogram())
    return false;

  if (DPCPPKernelCompilationUtils::isGlobalCtorDtorOrCPPFunc(&F))
    return false;

  const bool HasSyncInst = DPB->hasSyncInstruction(&F);
  const bool HasSGSyncInst = SGSyncFuncSet.count(&F);

  // Insert alloca and llvm.dbg.declare at function entry,
  // right after the leading dummy sync instruction, if any.
  insertGIDAlloca(F, HasSyncInst, HasSGSyncInst);

  // Insert store instructions after sync instructions.
  insertGIDStore(F, HasSyncInst, HasSGSyncInst);

  return true;
}

void ImplicitGIDImpl::insertGIDAlloca(Function &F, bool HasSyncInst,
                                      bool HasSGSyncInst) {
  // Take first instruction as insert point
  Instruction *IP = &(F.getEntryBlock().front());
  assert(IP && "Function has no instruction at all");

  // If there's a leading dummy_barrier or dummy_sg_barrier,
  // then we should insert right AFTER it instead.
  if (HasSyncInst || HasSGSyncInst) {
    IP = IP->getNextNode();
    assert(IP && "Entry block has only one instruction");
  }

  // Save the insert point for insertGIDStore() to use.
  // Any insert points come before IP would be invalid for GID stores,
  // since lifetime of GID variables start from here.
  InsertPoint = IP;

  // Just use the subprogram as the scope of implicit GID variables
  DISubprogram *SP = F.getSubprogram();
  assert(SP && "Function has no debug info");
  DebugLoc DIL = DILocation::get(F.getContext(), SP->getLine(), 0, SP);

  for (unsigned I = 0; I < MAX_WORK_DIM; ++I) {
    AllocaInst *GIDAlloca = new AllocaInst(DPCPPKernelLoopUtils::getIndTy(&M),
                                           0, "__ocl_dbg_gid" + Twine(I), IP);

    if (!SkipInsertDbgDeclare) {
      // Create debug info
      DILocalVariable *DIV = DIB->createAutoVariable(
          SP, GIDAlloca->getName(), nullptr, 1, IndDIType,
          /*AlwaysPreserve*/ true, DINode::FlagArtificial);
      DIB->insertDeclare(GIDAlloca, DIV, DIB->createExpression(), DIL, IP);
    }
    GIDAllocas[I] = GIDAlloca;
  }
}

void ImplicitGIDImpl::insertGIDStore(Function &F, bool HasSyncInst,
                                     bool HasSGSyncInst) {
  assert(InsertPoint->getParent() == &F.getEntryBlock() &&
         "Insert point must lie in current function's entry block");

  // Create get_global_id() and store instructions for each insert point
  IRBuilder<> B(InsertPoint);

  if (!HandleBarrier) {
    auto KIMD = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
    if (!(KIMD.NoBarrierPath.hasValue() && KIMD.NoBarrierPath.get()))
      return;
    insertGIDStore(B, InsertPoint);
    return;
  }

  InstSet GenericSyncInsts;
  if (HasSGSyncInst) {
    // We don't have to consider HasSyncInst in this situation,
    // since subgroup emulation loops are inside barrier loop,
    // just tracking GIDs in the inner loop would be enough.

    // Should insert stores after each dummy_sg_barrier and sg_barrier
    GenericSyncInsts = SGH.getSyncInstsForFunction(&F);
  } else if (HasSyncInst) {
    // Should insert stores after each dummy_barrier and barrier
    GenericSyncInsts = DPB->getSyncInstructions(&F);
  }
  // If the first instruction is a sync inst, it should be excluded from the
  // set, since its corresponding insert point has already been recorded as
  // InsertPoint.
  Instruction *FirstInst = &(F.getEntryBlock().front());
  GenericSyncInsts.remove(FirstInst);

  // If we encounter a dummybarrier -> dummybarrier/dummy_sg_barrier region at
  // the beginning of the function, we should not insert store instructions, as
  // such region is outside the workgroup/subgroup loop.
  const bool HasDummyBarrierRegion = !BUtils.findDummyRegion(F).empty();
  const bool HasDummyBarrierToDummySGBarrierRegion =
      HasSyncInst && HasSGSyncInst;
  if (!(HasDummyBarrierRegion || HasDummyBarrierToDummySGBarrierRegion))
    insertGIDStore(B, InsertPoint);

  for (Instruction *I : GenericSyncInsts) {
    Instruction *IP = I->getNextNode();
    assert(IP && "Sync instruction should not be terminator");
    insertGIDStore(B, IP);
  }
}

void ImplicitGIDImpl::insertGIDStore(IRBuilder<> &B, Instruction *InsertPoint) {
  B.SetInsertPoint(InsertPoint);

  // DO NOT ADD DEBUG INFO TO THE CODE WHICH GENERATES GET_GLOBAL_ID AND THE
  // STORE, OR THE DEBUGGER WILL NOT BREAK AT A GIVEN GLOBAL_ID
  B.SetCurrentDebugLocation(DebugLoc());

  for (unsigned I = 0; I < MAX_WORK_DIM; ++I) {
    Value *GID = BUtils.createGetGlobalId(I, B);
    B.CreateStore(GID, GIDAllocas[I], /*isVolatile*/ true);
  }
}

DIType *ImplicitGIDImpl::getOrCreateIndDIType() const {
  for (auto const &T : DIFinder.types())
    if (T->getName() == "ind type")
      return T;

  // If the type wasn't found, create it now.
  Type *IndTy = DPCPPKernelLoopUtils::getIndTy(&M);
  uint64_t IndTySize =
      M.getDataLayout().getTypeSizeInBits(IndTy).getFixedSize();
  return DIB->createBasicType("ind type", IndTySize, dwarf::DW_ATE_unsigned);
}

namespace {
class ImplicitGIDLegacy : public ModulePass {
  bool HandleBarrier;

public:
  static char ID;

  ImplicitGIDLegacy(bool HandleBarrier = true)
      : ModulePass(ID), HandleBarrier(HandleBarrier) {
    initializeImplicitGIDLegacyPass(*PassRegistry::getPassRegistry());
  }

  llvm::StringRef getPassName() const override { return "ImplicitGIDLegacy"; }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DataPerBarrierWrapper>();
    AU.addPreserved<DataPerBarrierWrapper>();
  }

  bool runOnModule(Module &M) override {
    auto *DPB = &getAnalysis<DataPerBarrierWrapper>().getDPB();
    ImplicitGIDImpl Impl(M, HandleBarrier, DPB);
    return Impl.run();
  }
};
} // namespace

char ImplicitGIDLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(ImplicitGIDLegacy, DEBUG_TYPE,
                      "Add implicit gid for native OpenCL (gdb) debugging",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(DataPerBarrierWrapper)
INITIALIZE_PASS_END(ImplicitGIDLegacy, DEBUG_TYPE,
                    "Add implicit gid for native OpenCL (gdb) debugging", false,
                    false)

ModulePass *llvm::createImplicitGIDLegacyPass(bool HandleBarrier) {
  return new ImplicitGIDLegacy(HandleBarrier);
}

PreservedAnalyses ImplicitGIDPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto *DPB = &AM.getResult<DataPerBarrierAnalysis>(M);
  ImplicitGIDImpl Impl(M, HandleBarrier, DPB);
  if (!Impl.run())
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<DataPerBarrierAnalysis>();
  return PA;
}
