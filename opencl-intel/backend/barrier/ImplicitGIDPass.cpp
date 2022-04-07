// Copyright 2012-2021 Intel Corporation.
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

#include "ImplicitGIDPass.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char ImplicitGlobalIdPass::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(
    ImplicitGlobalIdPass, "B-ImplicitGlobalIdPass",
    "Implicit Global Id Pass - Add parameters for native (gdb) debugging",
    false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DataPerBarrierWrapper)
OCL_INITIALIZE_PASS_END(
    ImplicitGlobalIdPass, "B-ImplicitGlobalIdPass",
    "Implicit Global Id Pass - Add parameters for native (gdb) debugging",
    false, false)

ImplicitGlobalIdPass::ImplicitGlobalIdPass(bool HandleBarrier)
    : ModulePass(ID), m_pDataPerBarrier(nullptr), m_pModule(nullptr),
      m_pContext(nullptr), m_handleBarrier(HandleBarrier),
      m_skipInsertDbgDeclare(false) {
  initializeImplicitGlobalIdPassPass(*llvm::PassRegistry::getPassRegistry());
}

bool ImplicitGlobalIdPass::runOnModule(Module &M) {
  m_pModule = &M;
  m_pDIB = std::unique_ptr<DIBuilder>(new DIBuilder(M));

  m_pContext = &M.getContext();

  m_pDataPerBarrier = &getAnalysis<DataPerBarrierWrapper>().getDPB();

  m_util.init(&M);
  m_SGHelper.initialize(M);
  m_SGSyncFuncSet = m_SGHelper.getAllSyncFunctions();

  // Prime a DebugInfoFinder that can be queried about various bits of
  // debug information in the module.
  m_DbgInfoFinder = DebugInfoFinder();
  m_DbgInfoFinder.processModule(M);

  // Create the Ind DebugInfo type for GID variables.
  m_IndDIType = getOrCreateIndDIType();

  // Collect kernels.
  SmallSet<Function *, 8> Kernels;
  for (Function *F : DPCPPKernelMetadataAPI::KernelList(&M))
    Kernels.insert(F);

  bool ModuleChanged = false;

  for (auto &F : M) {
    auto kimd = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
    if (m_handleBarrier) {
      if (!(kimd.NoBarrierPath.hasValue() && kimd.NoBarrierPath.get()))
        ModuleChanged |= runOnFunction(F);
      continue;
    }

    if (!(Kernels.count(&F) && kimd.NoBarrierPath.hasValue() &&
          kimd.NoBarrierPath.get()))
      continue;

    m_skipInsertDbgDeclare = false;
    ModuleChanged |= runOnFunction(F);

    m_skipInsertDbgDeclare = true;
    if (kimd.VectorizedKernel.hasValue()) {
      Function *VectorF = kimd.VectorizedKernel.get();
      if (VectorF)
        ModuleChanged |= runOnFunction(*VectorF);
    }
    if (kimd.VectorizedMaskedKernel.hasValue()) {
      Function *MaskedF = kimd.VectorizedMaskedKernel.get();
      if (MaskedF)
        ModuleChanged |= runOnFunction(*MaskedF);
    }
  }

  return ModuleChanged;
}

bool ImplicitGlobalIdPass::runOnFunction(Function &F) {
  // Skip all functions without debug info (including built-ins, externals, etc)
  // We can't do anything for them
  if (!F.getSubprogram())
    return false;

  if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(&F))
    return false;

  const bool HasSyncInst = m_pDataPerBarrier->hasSyncInstruction(&F);
  const bool HasSGSyncInst = m_SGSyncFuncSet.count(&F);

  // Insert alloca and llvm.dbg.declare at function entry,
  // right after the leading dummy sync instruction, if any.
  insertGIDAlloca(F, HasSyncInst, HasSGSyncInst);

  // Insert store instructions after sync instructions.
  insertGIDStore(F, HasSyncInst, HasSGSyncInst);

  return true;
}

void ImplicitGlobalIdPass::insertGIDAlloca(Function &F, bool HasSyncInst,
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
  m_pInsertPoint = IP;

  DIBuilder *DIB = m_pDIB.get();
  // Just use the subprogram as the scope of implicit GID variables
  DISubprogram *SP = F.getSubprogram();
  assert(SP && "Function has no debug info");
  DebugLoc DIL = DILocation::get(*m_pContext, SP->getLine(), 0, SP);

  for (unsigned i = 0; i < 3; ++i) {
    AllocaInst *GIDAlloca = new AllocaInst(LoopUtils::getIndTy(m_pModule), 0,
                                           "__ocl_dbg_gid" + Twine(i), IP);

    if (!m_skipInsertDbgDeclare) {
      // Create debug info
      DILocalVariable *DIV = DIB->createAutoVariable(
          SP, GIDAlloca->getName(), nullptr, 1, m_IndDIType,
          /*AlwaysPreserve*/ true, DINode::FlagArtificial);
      DIB->insertDeclare(GIDAlloca, DIV, DIB->createExpression(), DIL, IP);
    }
    m_pGIDAllocas[i] = GIDAlloca;
  }
}

void ImplicitGlobalIdPass::insertGIDStore(Function &F, bool HasSyncInst,
                                          bool HasSGSyncInst) {
  assert(m_pInsertPoint->getParent() == &F.getEntryBlock() &&
         "Insert point must lie in current function's entry block");

  // Create get_global_id() and store instructions for each insert point
  IRBuilder<> B(m_pInsertPoint);

  if (!m_handleBarrier) {
    auto kimd = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
    if (!(kimd.NoBarrierPath.hasValue() && kimd.NoBarrierPath.get()))
      return;
    insertGIDStore(B, m_pInsertPoint);
    return;
  }

  InstSet GenericSyncInsts;
  if (HasSGSyncInst) {
    // We don't have to consider HasSyncInst in this situation,
    // since subgroup emulation loops are inside barrier loop,
    // just tracking GIDs in the inner loop would be enough.

    // Should insert stores after each dummy_sg_barrier and sg_barrier
    GenericSyncInsts = m_SGHelper.getSyncInstsForFunction(&F);
  } else if (HasSyncInst) {
    // Should insert stores after each dummy_barrier and barrier
    GenericSyncInsts = m_pDataPerBarrier->getSyncInstructions(&F);
  }
  // If the first instruction is a sync inst, it should be excluded from the
  // set, since its corresponding insert point has already been recorded as
  // m_pInsertPoint.
  Instruction *FirstInst = &(F.getEntryBlock().front());
  GenericSyncInsts.remove(FirstInst);

  // If we encounter a dummybarrier -> dummybarrier/dummy_sg_barrier region at
  // the beginning of the function, we should not insert store instructions, as
  // such region is outside the workgroup/subgroup loop.
  const bool HasDummyBarrierRegion = !m_util.findDummyRegion(F).empty();
  const bool HasDummyBarrierToDummySGBarrierRegion =
      HasSyncInst && HasSGSyncInst;
  if (!(HasDummyBarrierRegion || HasDummyBarrierToDummySGBarrierRegion))
    insertGIDStore(B, m_pInsertPoint);

  for (Instruction *I : GenericSyncInsts) {
    Instruction *IP = I->getNextNode();
    assert(IP && "Sync instruction should not be terminator");
    insertGIDStore(B, IP);
  }
}

void ImplicitGlobalIdPass::insertGIDStore(IRBuilder<> &B,
                                          Instruction *InsertPoint) {
  B.SetInsertPoint(InsertPoint);

  // DO NOT ADD DEBUG INFO TO THE CODE WHICH GENERATES GET_GLOBAL_ID AND THE
  // STORE, OR THE DEBUGGER WILL NOT BREAK AT A GIVEN GLOBAL_ID
  B.SetCurrentDebugLocation(DebugLoc());

  for (unsigned i = 0; i < 3; ++i) {
    Value *GID = m_util.createGetGlobalId(i, B);
    B.CreateStore(GID, m_pGIDAllocas[i], /*isVolatile*/ true);
  }
}

DIType *ImplicitGlobalIdPass::getOrCreateIndDIType() const {
  for (auto const &t : m_DbgInfoFinder.types()) {
    if (t->getName() == "ind type") {
      return t;
    }
  }
  // If the type wasn't found, create it now
  DIBuilder *diBuilder = m_pDIB.get();
  Type *indTy = LoopUtils::getIndTy(m_pModule);
  uint64_t indTySize =
      m_pModule->getDataLayout().getTypeSizeInBits(indTy).getFixedSize();
  return diBuilder->createBasicType("ind type", indTySize,
                                    dwarf::DW_ATE_unsigned);
}

} // namespace intel

extern "C" {
void *createImplicitGIDPass(bool HandleBarrier) {
  return new intel::ImplicitGlobalIdPass(HandleBarrier);
}
}
