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

#include "llvm/Transforms/SYCLTransforms/ImplicitGID.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/SYCLTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-implicit-gid"

/// For test purpose only.
static cl::opt<bool> HandleBarrierOpt("implicit-gid-handle-barrier",
                                      cl::init(true), cl::Hidden,
                                      cl::desc("Only handle barrier path"));

namespace {
class ImplicitGIDImpl {
public:
  ImplicitGIDImpl(Module &M, bool HandleBarrier, DataPerBarrier *DPB)
      : M(M), HandleBarrier(HandleBarrier), DPB(DPB), DIB(new DIBuilder(M)) {
    if (HandleBarrierOpt.getNumOccurrences())
      this->HandleBarrier = HandleBarrierOpt;
  }

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
  CompilationUtils::FuncSet SGSyncFuncSet;

  /// This holds the insert point at entry block for the running function.
  Instruction *InsertPoint = nullptr;

  /// This holds the GID allocas.
  Instruction *GIDAllocas[3] = {nullptr};

  /// This holds the Ind DIType for GID variables.
  DIType *IndDIType = nullptr;

  /// Skip insertDeclare if true.
  bool SkipInsertDbgDeclare = false;
};
} // namespace

/// Return true if the function is not reachable from a kernel whose
/// NoBarrierPath is false.
static bool noBarrierPath(SmallPtrSetImpl<Function *> &Kernels,
                          DenseMap<Function *, bool> &KernelsNoBarrierPath,
                          Function &F) {
  CompilationUtils::FuncSet FS;
  FS.insert(&F);
  CompilationUtils::FuncSet FuncUsers;
  LoopUtils::fillFuncUsersSet(FS, FuncUsers);
  return all_of(FuncUsers, [&](Function *FuncUser) {
    return Kernels.count(FuncUser) ? KernelsNoBarrierPath[FuncUser] : true;
  });
}

/// Return true if the unit of the function has FullDebug emission kind.
static bool hasFullDebugEmissionKind(Function &F) {
  DISubprogram *SP = F.getSubprogram();
  if (!SP)
    return false;
  DICompileUnit *CU = SP->getUnit();
  return CU && CU->getEmissionKind() == DICompileUnit::FullDebug;
}

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
  auto KL = SYCLKernelMetadataAPI::KernelList(&M);
  SmallPtrSet<Function *, 8> Kernels(KL.begin(), KL.end());
  DenseMap<Function *, bool> KernelsNoBarrierPath;

  bool Changed = false;

  // Use worklist since this pass may insert TID function declarations.
  CompilationUtils::FuncSet AllKernels = CompilationUtils::getAllKernels(M);
  SmallVector<Function *, 16> NonKernelFuncs;
  for (auto &F : M)
    if (!F.isDeclaration() && !AllKernels.contains(&F) &&
        hasFullDebugEmissionKind(F))
      NonKernelFuncs.push_back(&F);

  // Process kernel functions.
  for (auto *F : KL) {
    if (!hasFullDebugEmissionKind(*F))
      continue;

    auto KIMD = SYCLKernelMetadataAPI::KernelInternalMetadataAPI(F);
    if (HandleBarrier) {
      if (!KIMD.NoBarrierPath.get())
        Changed |= runOnFunction(*F);
      continue;
    }

    KernelsNoBarrierPath[F] = KIMD.NoBarrierPath.get();
    if (!KIMD.NoBarrierPath.get())
      continue;

    SkipInsertDbgDeclare = false;
    Changed |= runOnFunction(*F);

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

  // Process non-kernel functions.
  for (auto *F : NonKernelFuncs) {
    if (HandleBarrier) {
      Changed |= runOnFunction(*F);
      continue;
    }

    if (!noBarrierPath(Kernels, KernelsNoBarrierPath, *F))
      continue;

    SkipInsertDbgDeclare = false;
    Changed |= runOnFunction(*F);
  }

  return Changed;
}

/// Return true if the function already has implicit GID, i.e. inserted by the
/// first run of this pass before WGLoopCreator.
bool hasImplicitGID(Function &F) {
  for (auto &I : instructions(&F)) {
    if (auto *AI = dyn_cast<AllocaInst>(&I))
      if (CompilationUtils::isImplicitGID(AI))
        return true;
  }
  return false;
}

bool ImplicitGIDImpl::runOnFunction(Function &F) {
  // Skip all functions without debug info (including built-ins, externals, etc)
  // We can't do anything for them
  if (!F.getSubprogram())
    return false;

  if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(&F))
    return false;

  if (F.empty())
    return false;

  // Skip the function if implicit GIDs are already added.
  if (HandleBarrier && hasImplicitGID(F))
    return false;

  LLVM_DEBUG(dbgs() << "ImplicitGID: process function " << F.getName() << "\n");

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
    AllocaInst *GIDAlloca = new AllocaInst(
        LoopUtils::getIndTy(&M), 0, Twine("__ocl_dbg_gid") + Twine(I), IP);
    LLVM_DEBUG(dbgs().indent(2) << "Insert " << *GIDAlloca << " to function "
                                << F.getName() << "\n");

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
    insertGIDStore(B, InsertPoint);
    return;
  }

  CompilationUtils::InstSet GenericSyncInsts;
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
    auto *SI = B.CreateStore(GID, GIDAllocas[I], /*isVolatile*/ true);
    (void)SI;
    LLVM_DEBUG(dbgs().indent(2) << "Insert " << *SI << " to function "
                                << SI->getFunction()->getName() << "\n");
  }
}

DIType *ImplicitGIDImpl::getOrCreateIndDIType() const {
  for (auto const &T : DIFinder.types())
    if (T->getName() == "ind type")
      return T;

  // If the type wasn't found, create it now.
  Type *IndTy = LoopUtils::getIndTy(&M);
  uint64_t IndTySize =
      M.getDataLayout().getTypeSizeInBits(IndTy).getFixedValue();
  return DIB->createBasicType("ind type", IndTySize, dwarf::DW_ATE_unsigned);
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
