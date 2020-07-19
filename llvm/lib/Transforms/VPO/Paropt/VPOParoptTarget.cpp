#if INTEL_COLLAB
//===--- VPOParoptTarget.cpp - Transformation of WRegion for offloading ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTarget.cpp implements the omp target feature.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/InferAddressSpacesUtils.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target"

// TODO: The following 2 flags are temporary to support MKL to transiton from
// old implementation of target variant dispatch to new implementation.
// vpo-paropt-use-interop is used to create interop object for synchronous case
// vpo-paropt-use-raw-dev-ptr is used to send raw device pointer instead of
// cl_buffer After the new implementation is fully tested, we will purge the
// old one and remove these flags.
#if INTEL_CUSTOMIZATION
// 20200520: Enabled vpo-paropt-use-interop by default as requested by A21.
#endif // INTEL_CUSTOMIZATION
static cl::opt<bool>
    UseInterop("vpo-paropt-use-interop", cl::Hidden,
               cl::init(true),
               cl::desc("Use the interop_obj for target variant dispatch."));
static cl::opt<bool>
    UseRawDevicePtr("vpo-paropt-use-raw-dev-ptr", cl::Hidden,
               cl::init(false),
               cl::desc("Pass raw device ptr to variant dispatch."));

static cl::opt<uint32_t> FixedSIMDWidth(
    "vpo-paropt-fixed-simd-width", cl::Hidden, cl::init(0),
    cl::desc("Fixed SIMD width for all target regions in the module."));

static cl::opt<bool> SimulateGetNumThreadsInTarget(
    "vpo-paropt-simulate-get-num-threads-in-target", cl::Hidden, cl::init(true),
    cl::desc("Simulate support for omp_get_num_threads in OpenMP target "
             "region. (This may have performance impact)."));

// Reset the value in the Map clause to be empty.
//
// Do not reset base pointers (including the item's getOrig() pointer),
// because we want to have explicit references of the mapped pointers
// inside the region (note that the region entry directive is considered
// to be inside the region). Outlining of the enclosed regions
// (e.g. "omp parallel for") may be different for the host and target,
// thus, explicit references to the mapped pointers may be seen inside
// the region during the target compilation but not during the host
// compilation. This may cause interface mismatch between the outlined
// functions created for the host and a target. Explicit references
// of the mapped pointers make sure that the code extraction for the mapped
// pointers is the same.
// We do want to reset the section pointers and the sizes, because
// they are not used inside the target region.
void VPOParoptTransform::resetValueInMapClause(WRegionNode *W) {
  if (!W->canHaveMap())
    return;

  MapClause const &MpClause = W->getMap();
  if (MpClause.empty())
    return;
  IRBuilder<> Builder(W->getEntryBBlock()->getFirstNonPHI());

  for (auto *Item : MpClause.items()) {
    if (!Item->getIsMapChain())
      continue;
    Value *Orig = Item->getOrig();
    MapChainTy const &MapChain = Item->getMapChain();
    for (int I = MapChain.size() - 1; I >= 0; --I) {
      MapAggrTy *Aggr = MapChain[I];
      Value *SectionPtr = Aggr->getSectionPtr();
      Value *BasePtr = Aggr->getBasePtr();
      // Do not reset section pointers in cases like this:
      //   %12 = call i8* @llvm.launder.invariant.group.p0i8(
      //       i8* bitcast (double** @f_global to i8*))
      //   %f_global = bitcast i8* %12 to double**
      //   %13 = call token @llvm.directive.region.entry() [
      //       "DIR.OMP.TARGET"(),
      //       "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(double** %f_global,
      //                                      double** %f_global, i64 8),
      //       "QUAL.OMP.MAP.TOFROM:AGGR"(double** %f_global,
      //                                  double* %6, i64 %10)
      //
      // Resetting section pointer of AGGRHEAD will cause removal
      // of all references to %f_global.
      //
      // Due to the outlining differences between host and SPIR-V target,
      // a reference to @f_global may be observable during compilation
      // for SPIR-V target, but for host it may not be observable
      // (e.g. the inner "parallel" region referencing @f_global
      // is outlined). This difference may cause a mismatch between
      // outlined functions generated for the host and the device.
      if (SectionPtr != BasePtr)
        resetValueInOmpClauseGeneric(W, SectionPtr);
      // If BasePtr of the Aggr is not same as Orig, then we don't want it
      // inside the outlined function. e.g. %y in the following:
      //   "DIR.OMP.TARGET" [ "QUAL.OMP.MAP"(%x, ...) "MAP:CHAIN"(%y, ...) ]
      if (BasePtr != Orig)
        resetValueInOmpClauseGeneric(W, BasePtr);
      Value *Size = Aggr->getSize();
      if (!dyn_cast<ConstantInt>(Size))
        resetValueInOmpClauseGeneric(W, Size);
    }
  }
}

// Replace printf() calls in F with _Z18__spirv_ocl_printfPU3AS2ci()
void VPOParoptTransform::replacePrintfWithOCLBuiltin(Function *PrintfDecl,
                                                     Function *OCLPrintfDecl,
                                                     Function *F) {
  if (!PrintfDecl)
    return;

  SmallVector<Instruction *, 4> InstsToDelete;
  assert(OCLPrintfDecl != nullptr && "OCLPrintfDecl not initialized");

  // find all printf's in this function and replace them with the OCL version
  for (User *U : PrintfDecl->users())
    if (CallInst *OldCall = dyn_cast<CallInst>(U)) {

      if (F && OldCall->getParent()->getParent() != F)
        // ignore printfs that are not in this function
        continue;

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": old printf(): " << *OldCall
                        << "\n");
      SmallVector<Value *, 4> FnArgs(OldCall->arg_operands());

      // First argument of the original printf() is of
      // ADDRESS_SPACE_GENERIC (=4) due to its addrspacecast:
      //
      //   call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)*
      //       getelementptr inbounds ([25 x i8], [25 x i8] addrspace(4)*
      //       addrspacecast ([25 x i8] addrspace(1)* @.str to [25 x i8]
      //       addrspace(4)*), i64 0, i64 0), ...)
      //
      // The OCL printf expects addrspace(2). So, we must generate constant
      // string with addrspace(2) and relink to OCL printf:
      //
      //   @.str.as2 = private target_declare addrspace(2) constant [25 x i8]
      //       c"..."
      //
      //   call i32 (i8 addrspace(2)*, ...) @_Z18__spirv_ocl_printfPU3AS2ci(
      //       i8 addrspace(2)* getelementptr inbounds
      //       ([25 x i8], [25 x i8] addrspace(2)* @.str.as2, i64 0, i64 0)
      //       , ...)
      Type* FirstParmTy = FnArgs[0]->getType();
      assert(isa<PointerType>(FirstParmTy) &&
             "First argument to printf should be a pointer.");

      if (FirstParmTy->getPointerAddressSpace() !=
          vpo::ADDRESS_SPACE_CONSTANT) {
        IRBuilder<> Builder(OldCall);

        LLVM_DEBUG(dbgs() << "Original argument 0 of printf: " << *FnArgs[0]
                          << "\n");
        auto V = FnArgs[0];
        assert(isa<ConstantExpr>(V) && "Only constant format string in argument"
                                       " 0 is supported!\n");
        SmallVector<Value *, 2> Indices;
        // Skip the constant expressions
        while (ConstantExpr *CE = dyn_cast<ConstantExpr>(V)) {
          if (CE->getOpcode() == Instruction::GetElementPtr) {
            assert(CE->getNumOperands() == 3 && "Number of operands in "
                                                "GetElementPtr must be 3!\n");
            Indices.push_back(CE->getOperand(1));
            Indices.push_back(CE->getOperand(2));
          }
          V = CE->getOperand(0);
        }
        assert(Indices.size() == 2 && "Expected only 1 GetElementPtr in "
                                      "constant expression!\n");

        auto OldArg0 = dyn_cast<GlobalVariable>(V);
        assert(OldArg0 != nullptr &&
               "The format string in printf is not global variable.\n");
        // Generate format string with addrspace(2)
        GlobalVariable *NewArg0 = new GlobalVariable(
            *(OldArg0->getParent()), OldArg0->getValueType(),
            OldArg0->isConstant(), OldArg0->getLinkage(),
            OldArg0->hasInitializer() ? OldArg0->getInitializer() : nullptr,
            OldArg0->getName() + Twine(".as2"), OldArg0,
            OldArg0->getThreadLocalMode(), vpo::ADDRESS_SPACE_CONSTANT);
        NewArg0->setTargetDeclare(true);

        // Relink the newly generated string to printf
        FnArgs[0] = Builder.CreateInBoundsGEP(NewArg0, Indices,
                                              NewArg0->getName() + ".gep");
        LLVM_DEBUG(dbgs() << "New argument 0 of printf: " << *FnArgs[0]
                          << "\n");
      }

      // Create the new call based on OCLPrintfDecl and
      // insert it before the old call
      CallInst *NewCall =
          CallInst::Create(OCLPrintfDecl->getFunctionType(), OCLPrintfDecl,
                           FnArgs, "oclPrint", OldCall);

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": new OCL printf(): " << *NewCall
                        << "\n");

      // Relace all uses of the old call's return value with
      // that from the new call
      for (User *OldU : OldCall->users())
        if (Instruction *I = dyn_cast<Instruction>(OldU))
          I->replaceUsesOfWith(OldCall, NewCall);

      // Mark old call for deletion
      InstsToDelete.push_back(OldCall);
    }

  for (Instruction *I: InstsToDelete)
    I->eraseFromParent();
}

Function *VPOParoptTransform::finalizeKernelFunction(WRegionNode *W,
                                                     Function *Fn,
                                                     CallInst *&Call) {

  assert(isTargetSPIRV() &&
         "finalizeKernelFunction called for non-SPIRV target.");

  FunctionType *FnTy = Fn->getFunctionType();
  SmallVector<Type *, 8> ParamsTy;

  unsigned AddrSpaceGlobal = vpo::ADDRESS_SPACE_GLOBAL;
  for (auto ArgTyI = FnTy->param_begin(), ArgTyE = FnTy->param_end();
       ArgTyI != ArgTyE; ++ArgTyI) {
    assert(isa<PointerType>(*ArgTyI) &&
           "finalizeKernelFunction: Expect pointer type.");
    PointerType *PtType = cast<PointerType>(*ArgTyI);
    ParamsTy.push_back(PtType->getElementType()->getPointerTo(AddrSpaceGlobal));
  }

  Type *RetTy = FnTy->getReturnType();
  FunctionType *NFnTy = FunctionType::get(RetTy, ParamsTy, false);
  Function *NFn = Function::Create(NFnTy, GlobalValue::WeakAnyLinkage);
  NFn->copyAttributesFrom(Fn);
  NFn->setCallingConv(CallingConv::SPIR_KERNEL);
  NFn->addFnAttr("target.declare", "true");

  Fn->getParent()->getFunctionList().insert(Fn->getIterator(), NFn);
  NFn->takeName(Fn);
  NFn->getBasicBlockList().splice(NFn->begin(), Fn->getBasicBlockList());

  // Everything including the routine name has been moved to the new routine.
  // Do the same with the debug information.
  NFn->setSubprogram(Fn->getSubprogram());
  Fn->setSubprogram(nullptr);

  IRBuilder<> Builder(NFn->getEntryBlock().getFirstNonPHI());
  Function::arg_iterator NewArgI = NFn->arg_begin();
  for (Function::arg_iterator I = Fn->arg_begin(), E = Fn->arg_end(); I != E;
       ++I) {
    auto ArgV = &*NewArgI;
    unsigned NewAddressSpace =
        cast<PointerType>(ArgV->getType())->getAddressSpace();
    unsigned OldAddressSpace =
        cast<PointerType>(I->getType())->getAddressSpace();

    Value *NewArgV = ArgV;
    if (NewAddressSpace != OldAddressSpace) {
      // Assert the correct addrspacecast here instead of failing
      // during SPIRV emission.
      assert(OldAddressSpace == vpo::ADDRESS_SPACE_GENERIC &&
             "finalizeKernelFunction: OpenCL global addrspaces can only be "
             "casted to generic.");
      NewArgV = Builder.CreatePointerBitCastOrAddrSpaceCast(ArgV, I->getType());
    }
    I->replaceAllUsesWith(NewArgV);
    NewArgI->takeName(&*I);
    ++NewArgI;
  }

  if (W->getSPIRVSIMDWidth() > 0) {
    Metadata *AttrMDArgs[] = {
        ConstantAsMetadata::get(Builder.getInt32(W->getSPIRVSIMDWidth())) };
    NFn->setMetadata("intel_reqd_sub_group_size",
                     MDNode::get(NFn->getContext(), AttrMDArgs));
  }

  InferAddrSpaces(*TTI, vpo::ADDRESS_SPACE_GENERIC, *NFn);

  return NFn;
}

/// This function checks if the instruction is an intrinsic instruction,
/// and depending on the boolean flag, whether it is target, parallel
/// parallel loop or simd loop.
static bool isParOrTargetDirective(Instruction *Inst,
                                   bool isTarget = false, bool isSIMD = false) {
  auto IntrinInst = dyn_cast<IntrinsicInst>(Inst);

  if (!IntrinInst || !IntrinInst->hasOperandBundles()) {
    return false;
  }

  int ID = VPOAnalysisUtils::getDirectiveID(Inst);

  if (isSIMD)
    return ID==DIR_OMP_SIMD;

  if (isTarget)
    return ID==DIR_OMP_TARGET;

  switch (ID) {
    case DIR_OMP_PARALLEL:
    case DIR_OMP_PARALLEL_LOOP:
    case DIR_OMP_PARALLEL_SECTIONS:
    case DIR_OMP_DISTRIBUTE_PARLOOP:
    case DIR_OMP_SIMD:
      // DIR_OMP_TEAMS used to be here, but it prevented guarding
      // instructions with side effects inside OpenMP teams regions.
      // We may want to get it back here, so that we insert barriers
      // around OpenMP teams region. This is currently not required,
      // since there may not be any code between "target" and "teams".
      return true;
  }

  return false;
}

/// Get the exit region directive intrinsic instruction, corresponding
/// to the begin region directive. The user instruction of the
/// begin region is the exit region directive. If \p DirectiveBegin is same
/// as \p KernelEntryDir, then returns \p KernelExitDir.
static Instruction *getExitInstruction(Instruction *DirectiveBegin,
                                       Instruction *KernelEntryDir,
                                       Instruction *KernelExitDir) {
  assert(DirectiveBegin && isa<IntrinsicInst>(DirectiveBegin) &&
         "Unexpected begin directive.");

  if (DirectiveBegin == KernelEntryDir)
    return KernelExitDir;

  // Assumption: Every intrinsic instruction that begins a directive
  // has a single exit directive corresponding to it.
  // So, if we iterate through its users, then the
  // intrinsic instruction that uses it must be the exit directive.
  // For KernelEntryDir, it's possible to not have any use, as the use is
  // replaced with 'token none' before invoking CodeExtractor, but it has
  // already been handled above.
  for (auto U : DirectiveBegin->users()) {
    if (auto DEnd = dyn_cast<IntrinsicInst>(U)) {
      LLVM_DEBUG(dbgs() << "\n Directive End::" << *DEnd);
      return DEnd;
    }
  }
  return nullptr;
}

/// If the given \p CallI is a __kmpc_critical call,
/// then this function will return its pairing __kmpc_end_critical call,
/// otherwise, it will return nullptr.
/// The function assumes that there are no early exits from the critical
/// region, which is a valid assumption for SPIR targets (where we can
/// only support well-formed critical regions).
static CallInst *getCriticalEndCall(CallInst *CallI) {
  auto CalledF = CallI->getCalledOperand()->stripPointerCasts();

  if (!CalledF || !CalledF->hasName() ||
      CalledF->getName() != "__kmpc_critical")
    return nullptr;

  SmallVector<BasicBlock *, 32> WorkStack{CallI->getParent()};
  SmallPtrSet<BasicBlock *, 32> Visited;
  while (!WorkStack.empty()) {
    BasicBlock *BB = WorkStack.pop_back_val();

    for (auto &I : *BB)
      if (auto *EndCallI = dyn_cast<CallInst>(&I)) {
        auto CalledF = EndCallI->getCalledOperand()->stripPointerCasts();

        if (CalledF && CalledF->hasName() &&
            CalledF->getName() == "__kmpc_end_critical")
          return EndCallI;
      }

    Visited.insert(BB);
    assert(BB->getTerminator()->getNumSuccessors() > 0 &&
           "Block inside critical section has zero successors.");
    // Place the successors into the work stack.
    for (auto *SBB : successors(BB))
      if (Visited.count(SBB) == 0)
        WorkStack.push_back(SBB);
  }

  llvm_unreachable("OpenMP critical region is malformed.");
  return nullptr;
}

/// This function ignores special instructions with side effect.
/// Returns true if the call instruction is a special call.
/// Returns true if the store address is an alloca instruction,
/// that is allocated locally in the thread.
static bool ignoreSpecialOperands(const Instruction *I) {

  //   Ignore calls to the following OpenCL functions
  const std::set<std::string> IgnoreCalls = {
      "_Z13get_global_idj",  "_Z12get_local_idj",   "_Z14get_local_sizej",
      "_Z14get_num_groupsj", "_Z12get_group_idj",   "_Z18work_group_barrierj",
      "_Z9mem_fencej",       "_Z14read_mem_fencej", "_Z15write_mem_fencej",
#if INTEL_CUSTOMIZATION
      "_f90_dope_vector_init", "_f90_firstprivate_copy",
      "_f90_dope_vector_size", "_f90_lastprivate_copy",
#endif // INTEL_CUSTOMIZATION
      "omp_get_thread_num"};

  if (auto CallI = dyn_cast<CallInst>(I)) {
    // Unprototyped function calls may result in a call of a bitcasted
    // Function.
    auto CalledF = CallI->getCalledOperand()->stripPointerCasts();
    assert(CalledF != nullptr && "Called Function not found ");
    if (CalledF->hasName() &&
        IgnoreCalls.find(std::string(CalledF->getName())) != IgnoreCalls.end())
      return true;
  } else if (auto StoreI = dyn_cast<StoreInst>(I)) {
    const Value *StorePointer = StoreI->getPointerOperand();
    const Value *RootPointer = StorePointer->stripPointerCasts();
    LLVM_DEBUG(dbgs() << "Store op:: " << *StorePointer);
    // We must not guard stores through private pointers. The store must
    // happen in each work item, so that the variable is initialized
    // in each work item. For values privatized as local allocas RootPointer
    // will be the AllocaInst with private address space, so there is no need
    // for special handling of the regions' private values.
    if (cast<PointerType>(StorePointer->getType())->getAddressSpace() ==
            vpo::ADDRESS_SPACE_PRIVATE ||
        cast<PointerType>(RootPointer->getType())->getAddressSpace() ==
            vpo::ADDRESS_SPACE_PRIVATE)
      return true;
  }
  return false;
}

/// Guard instructions that have side effects, so that only master thread
/// (thread_id == 0) in each team executes it.
void VPOParoptTransform::guardSideEffectStatements(
    WRegionNode *W, Function *KernelF) {

  assert(isTargetSPIRV() &&
         "guardSideEffectStatements() called for non-SPIRV target.");

  Instruction *ParDirectiveBegin    = nullptr;
  Instruction *ParDirectiveExit     = nullptr;
  Instruction *TargetDirectiveBegin = nullptr;
  Instruction *TargetDirectiveExit  = nullptr;
  CallInst *KernelEntryDir          = cast<CallInst>(W->getEntryDirective());
  CallInst *KernelExitDir           = cast<CallInst>(W->getExitDirective());

  SmallVector<Instruction *, 6> SideEffectInstructions;
  SmallVector<Instruction *, 6> InsertBarrierAt;
  SmallVector<BasicBlock *, 10> ParBBVector, TargetBBSet;
  SmallVector<Instruction *, 10> EntryDirectivesToDelete;
  SmallVector<Instruction *, 10> ExitDirectivesToDelete;
  SmallVector<std::pair<CallInst *, CallInst *>, 10> OmpCriticalCalls;

  auto InsertWorkGroupBarrier = [](Instruction *InsertPt) {
    LLVMContext &C = InsertPt->getContext();

    LLVM_DEBUG(dbgs() << "\nInsert Barrier before:" << *InsertPt);

    // TODO: we only need global fences for side effect instructions
    //       inside "omp target" and outside of the enclosed regions.
    //       Moreover, it probably makes sense to guard such instructions
    //       with (get_group_id() == 0) vs (get_local_id() == 0).
    CallInst *CI =
        VPOParoptUtils::genOCLGenericCall(
            "_Z18work_group_barrierj", Type::getVoidTy(C),
            // CLK_LOCAL_MEM_FENCE  == 1
            // CLK_GLOBAL_MEM_FENCE == 2
            // CLK_IMAGE_MEM_FENCE  == 4
            { ConstantInt::get(Type::getInt32Ty(C), 1 | 2) },
            InsertPt);
    // work_group_barrier() is a convergent call.
    CI->getCalledFunction()->setConvergent();
  };

  // Find the parallel region begin and end directives,
  // and add barriers at the entry and exit of parallel region.
  for (inst_iterator I = inst_begin(KernelF), E = inst_end(KernelF);
       I != E; ++I) {

    // Collect OpenMP directive calls so that we can delete them
    // later. We may want to keep SIMD directives at some point,
    // but right now they are not supported for SPIR-V targets.
    // Moreover, llvm-spirv is not able to translate llvm.directive.region
    // intrinsic calls.
    if (VPOAnalysisUtils::isOpenMPDirective(&*I) &&
        // Target directives require special processing (see below).
        &*I != KernelEntryDir && &*I != KernelExitDir) {
      // Distinguish between entry and other directives, since
      // we want to delete the entry directives after their
      // exit companions.
      if (VPOAnalysisUtils::isBeginDirective(&*I))
        EntryDirectivesToDelete.push_back(&*I);
      else
        ExitDirectivesToDelete.push_back(&*I);
    }

    if (isParOrTargetDirective(&*I) &&
        // Barriers must not be inserted around SIMD loops.
        !isParOrTargetDirective(&*I, false, true)) {

      ParDirectiveBegin = &*I;
      ParDirectiveExit =
          getExitInstruction(ParDirectiveBegin, KernelEntryDir, KernelExitDir);
      assert(ParDirectiveExit && "Par region exit directive not found.");

      SmallVector<BasicBlock *, 10> TempParBBVec;
      GeneralUtils::collectBBSet(ParDirectiveBegin->getParent(),
                                 ParDirectiveExit->getParent(), TempParBBVec);
      ParBBVector.append(TempParBBVec.begin(), TempParBBVec.end());

      // Insert a barrier after the region exit. This barrier synchronizes
      // all side effects that might have happened inside the region
      // with all the consumers of this side effects that are reachable
      // from the region's exit. Note that we do not need a barrier
      // before the region, since we insert barriers after all side effect
      // instructions that may reach the region's entry.
      InsertBarrierAt.push_back(ParDirectiveExit);

      ParDirectiveBegin = nullptr;
      ParDirectiveExit = nullptr;
    } else if (TargetDirectiveBegin == nullptr &&
               isParOrTargetDirective(&*I, true)) {
      TargetDirectiveBegin = &*I;
      TargetDirectiveExit = getExitInstruction(TargetDirectiveBegin,
                                               KernelEntryDir, KernelExitDir);
      assert(TargetDirectiveExit && "Target region exit directive not found.");

      GeneralUtils::collectBBSet(TargetDirectiveBegin->getParent(),
          TargetDirectiveExit->getParent(), TargetBBSet);
    } else if (auto *CallI = dyn_cast<CallInst>(&*I))
      if (auto *EndCallI = getCriticalEndCall(CallI))
        // Collect critical regions. If a side-effect instruction
        // inside a critical region must be guarded (e.g. the critical
        // region is inside a teams region), then there must be no
        // barrier at the merge point.
        //
        // FIXME: this is not really needed, because we guard the
        //        lock acqure/release calls with the master thread checks
        //        as well. For some reason specACCEL/552 hangs with
        //        the barrier calls.
        OmpCriticalCalls.push_back({CallI, EndCallI});
  }

  SmallPtrSet<BasicBlock *, 10> ParBBSet(ParBBVector.begin(),
                                         ParBBVector.end());
  SmallPtrSet<BasicBlock *, 10> CriticalBBSet;
  SmallPtrSet<Instruction *, 10> SideEffectsInCritical;

  for (auto &CriticalPair : OmpCriticalCalls) {
    SmallVector<BasicBlock *, 32> BBSet;
    GeneralUtils::collectBBSet(CriticalPair.first->getParent(),
                               CriticalPair.second->getParent(),
                               BBSet);
    CriticalBBSet.insert(BBSet.begin(), BBSet.end());
  }

  // Iterate over all instructions and add the side effect instructions
  // to the set "SideEffectInstructions".

  TargetDirectiveBegin = nullptr;
  TargetDirectiveExit = nullptr;

  for (auto BB : TargetBBSet) {
    if (ParBBSet.find(BB) != ParBBSet.end())
      continue;

    for (auto &I : *BB) {
      if (TargetDirectiveBegin == nullptr &&
          isParOrTargetDirective(&I, true)) {
        TargetDirectiveBegin = &I;
        TargetDirectiveExit = getExitInstruction(TargetDirectiveBegin,
                                                 KernelEntryDir, KernelExitDir);
      }

      if (TargetDirectiveBegin == nullptr)
        continue;
      if (TargetDirectiveExit == &I)
        break;
      // they donot have side effect Ignore intrinsic calls
      if (isa<IntrinsicInst>(&I))
        continue;
      if (I.mayHaveSideEffects()) {
        if (ignoreSpecialOperands(&I))
          continue;

        LLVM_DEBUG(dbgs() << "\nInstruction has side effect::" << I
                          << "\nBasicBlock: " << I.getParent());
        SideEffectInstructions.push_back(&I);
        if (CriticalBBSet.find(BB) != CriticalBBSet.end())
          SideEffectsInCritical.insert(&I);
      }
    }
  }

  auto I = SideEffectInstructions.begin();
  auto IE = SideEffectInstructions.end();
  Value *MasterCheckPredicate = nullptr;

  if (I != IE) {
    // Prepare the master check predicate, which looks like
    //   (get_local_id(0) == 0 && get_local_id(1) == 0 &&
    //    get_local_id(2) == 0)
    //
    // TODO: we may optimize this later, based on the number of dimensions
    //       used for the target region.
    IRBuilder<> Builder(KernelEntryDir);
    auto *ZeroConst = Constant::getNullValue(GeneralUtils::getSizeTTy(F));

    for (unsigned Dim = 0; Dim < 3; ++Dim) {
      auto *Arg = ConstantInt::get(Builder.getInt32Ty(), Dim);
      Value *LocalId = VPOParoptUtils::genOCLGenericCall(
          "_Z12get_local_idj", GeneralUtils::getSizeTTy(F),
          { Arg }, KernelEntryDir);
      Value *Predicate = Builder.CreateICmpEQ(LocalId, ZeroConst);

      if (!MasterCheckPredicate) {
        MasterCheckPredicate = Predicate;
        continue;
      }

      MasterCheckPredicate = Builder.CreateAnd(MasterCheckPredicate, Predicate);
    }

    MasterCheckPredicate->setName("is.master.thread");
  }

  while (I != IE) {

    Instruction *StartI = *I;
    bool StartIHasUses = StartI->hasNUsesOrMore(1);

    // Collect a consecutive set of Instructions with side effects
    // from the same block. We want to guard them all at once.
    // Note that we cannot easily guard any intermediate instructions.
    // For example, we cannot guard a store to addrspace(0), because
    // we want it to be executed in each WI. We cannot guard
    // loads from addrspace(1|3) as well, since they may be loading
    // from the memory we are about to guard, and the memory may be outdated
    // in non-master WIs.
    // Instructions with uses are guarded separately.
    if (!StartIHasUses)
      while (std::next(I) != IE &&
             *std::next(I) == (*I)->getNextNonDebugInstruction() &&
             !((*std::next(I))->hasNUsesOrMore(1))) {
        I = std::next(I);
      }

    // I is pointing to the last instruction in the sequence.
    Instruction *StopI = *I;

    // Split the basic block after StopI, into 2 blocks (1st and 2nd Block).
    // Insert a check, before StartI if the thread id is not equal to zero,
    // then jump to the 2nd block, else jump to StartI. StopI will fall
    // through to the 2nd block:
    //   if(MasterCheckPredicate) {
    //   master.thread.code:
    //     <start instruction>
    //     ...
    //     <stop instruction>
    //   }
    //   master.thread.fallthru:

    LLVM_DEBUG(dbgs() << "\nGuarding instructions from:\n" <<
               *StartI << "\nto:\n" << *StopI);

    // For the following code:
    //
    //   #pragma omp target map(tofrom:a)
    //     int val = bar(a);
    //
    // Generate code like this:
    //
    //  @c.broadcast.ptr.__local = internal addrspace(3) global i32 0 //  (1)
    //
    //  if (is_master) {                                              //  (2)
    //    %c = call spir_func i32 @_Z3barPi(i32 addrspace(4)* %8)     // StartI
    //    store i32 %c, i32 addrspace(3)* @c.broadcast.ptr.__local    //  (3)
    //  }
    //
    //  call spir_func void @_Z18work_group_barrierj(i32 3)           //  (4)
    //  %c.new = load i32, i32 addrspace(3)* @c.broadcast.ptr.__local //  (5)
    //  store i32 %c.new, i32* %val.priv, align 4  // Replaced %c with %c.new
    //
    Value *TeamLocalVal = nullptr;
    auto &DL = StartI->getModule()->getDataLayout();
    MaybeAlign Alignment = llvm::None;
    if(StartI->getType()->isPointerTy())
      Alignment = StartI->getPointerAlignment(DL);
    if (StartIHasUses)
      TeamLocalVal = VPOParoptUtils::genPrivatizationAlloca( //           (1)
          StartI->getType(), nullptr, Alignment, TargetDirectiveBegin,
          isTargetSPIRV(), StartI->getName() + ".broadcast.ptr",
          vpo::ADDRESS_SPACE_LOCAL);

    Instruction *ThenTerm = SplitBlockAndInsertIfThen(
        MasterCheckPredicate, StartI, false, nullptr, DT, LI); //         (2)
    BasicBlock *ThenBB = ThenTerm->getParent();
    ThenBB->setName("master.thread.code");
    Instruction *ElseInst = StopI->getNextNonDebugInstruction();
    BasicBlock *ElseBB = ElseInst->getParent();
    ElseBB->setName("master.thread.fallthru");
    ThenBB->getInstList().splice(
        ThenTerm->getIterator(), StartI->getParent()->getInstList(),
        StartI->getIterator(), ElseInst->getIterator());

    Instruction *BarrierInsertPt = ElseBB->getFirstNonPHI();

    if (StartIHasUses) { //                                               (3)
      Type *StartITy = StartI->getType();
      Align StartIAlign = DL.getABITypeAlign(StartITy);
      StoreInst *StoreGuardedInstValue =
          new StoreInst(StartI, TeamLocalVal, false /*volatile*/, StartIAlign);
      StoreGuardedInstValue->insertAfter(StartI);

      LoadInst *LoadSavedValue = //                                       (5)
          new LoadInst(StartITy, TeamLocalVal, StartI->getName() + ".new",
                       false /*volatile*/, StartIAlign);
      LoadSavedValue->insertBefore(BarrierInsertPt);
      BarrierInsertPt = LoadSavedValue;

      StartI->replaceAllUsesWith(LoadSavedValue);
      StoreGuardedInstValue->replaceUsesOfWith(LoadSavedValue, StartI);
    }

    // Insert group barrier at the merge point.
    if (SideEffectsInCritical.count(StartI) == 0)
      InsertWorkGroupBarrier(BarrierInsertPt); //                         (4)
    I = std::next(I);
  }

  if (!SideEffectInstructions.empty() ||
      // FIXME: if there are multiple parallel regions,
      //        then we need to synchronize between them, otherwise
      //        some data written in a parallel region
      //        may be out of date for reading in the succeeding
      //        parallel region. The check here is not enough,
      //        because we may read data written in a parallel region
      //        in "omp target" code succeeding the parallel region.
      //        For now to avoid performance regressions, we insert
      //        barriers only when there are multiple parallel regions
      //        inside "omp target". The complete fix would be
      //        to check if any load operation after a parallel region
      //        may read data that was potentially updated inside
      //        the parallel region. Can we use alias information for that?
      W->getChildren().size() > 1) {
    for (auto *InsertPt : InsertBarrierAt) {
      LLVM_DEBUG(dbgs() << "\nInsert Barrier at :" << *InsertPt);
      InsertWorkGroupBarrier(InsertPt);
    }
  }

  // Delete the directives.
  // The target directives will be removed by paroptTransforms().
  for (auto *I : ExitDirectivesToDelete)
    VPOUtils::stripDirectives(*I->getParent());
  for (auto *I : EntryDirectivesToDelete)
    VPOUtils::stripDirectives(*I->getParent());

  // Remove all clauses from the "omp target" entry directive.
  // The extra references in the clauses may prevent address space
  // inferring. We cannot remove the directive call yet, because
  // the removal in paroptTransforms() will complain.
  OperandBundleDef B(
      std::string(VPOAnalysisUtils::getDirectiveString(KernelEntryDir)), None);
  // The following call clones the original directive call
  // with just the directive name in the operand bundles.
  auto *NewEntryDir = CallInst::Create(KernelEntryDir, {B}, KernelEntryDir);
  KernelEntryDir->replaceAllUsesWith(NewEntryDir);
  KernelEntryDir->eraseFromParent();
  W->setEntryDirective(NewEntryDir);
}

bool VPOParoptTransform::callPopPushNumThreadsAtRegionBoundary(
    WRegionNode *W, bool InsideRegion) {

  if (!SimulateGetNumThreadsInTarget || !moduleHasOmpGetNumThreadsFunction())
    return false;

  assert(W && "WRegionNode is null.");

  Instruction *EntryDir = W->getEntryDirective();
  assert(EntryDir && "Null Entry directive.");

  CallInst *PushCall, *PopCall;
  std::tie(PushCall, PopCall) =
      VPOParoptUtils::genKmpcSpmdPushPopNumThreadsCalls(EntryDir->getModule());

  VPOParoptUtils::insertCallsAtRegionBoundary(W, PopCall, PushCall,
                                              InsideRegion);
  return true;
}

bool VPOParoptTransform::callPushPopNumThreadsAtRegionBoundary(
    WRegionNode *W, bool InsideRegion) {
  assert(W && "WRegionNode is null.");

  if (!SimulateGetNumThreadsInTarget || !moduleHasOmpGetNumThreadsFunction())
    return false;

  Instruction *EntryDir = W->getEntryDirective();
  assert(EntryDir && "Null Entry directive.");

  CallInst *PushCall, *PopCall;
  std::tie(PushCall, PopCall) =
      VPOParoptUtils::genKmpcSpmdPushPopNumThreadsCalls(EntryDir->getModule());

  VPOParoptUtils::insertCallsAtRegionBoundary(W, PushCall, PopCall,
                                              InsideRegion);
  return true;
}

// Generate the code for the directive omp target
bool VPOParoptTransform::genTargetOffloadingCode(WRegionNode *W) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTargetOffloadingCode\n");

  W->populateBBSet();

  resetValueInOmpClauseGeneric(W, W->getIf());
  resetValueInOmpClauseGeneric(W, W->getDevice());
  resetValueInIsDevicePtrClause(W);
  resetValueInPrivateClause(W);
  resetValueInMapClause(W);

  // Set up Fn Attr for the new function
  Function *NewF = VPOParoptUtils::genOutlineFunction(*W, DT, AC);

  LLVM_DEBUG(
      dbgs()
      << "\nEnter VPOParoptTransform::genTargetOffloadingCode: Dump Func::\n"
      << *NewF);
  if (!VPOAnalysisUtils::isTargetSPIRV(F->getParent()))
    NewF->addFnAttr("target.declare", "true");

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (isTargetCSA()) {
    // Add "target.entry" attribute to the outlined function.
    NewF->addFnAttr("omp.target.entry");
    NewF->setLinkage(GlobalValue::WeakAnyLinkage);
  }
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  CallInst *NewCall = cast<CallInst>(NewF->user_back());

  Constant *RegionId = nullptr;
  if (isa<WRNTargetNode>(W)) {
    assert(MT && "target region with no module transform");
    RegionId = MT->registerTargetRegion(W, NewF);

    // Please note that the name of NewF is updated in the
    // function registerTargetRegion.
    if (isTargetSPIRV()) {
      LLVM_DEBUG(dbgs() << "\nBefore guardSideEffectStatemets the function ::"
                 << *NewF);
      guardSideEffectStatements(W, NewF);
      LLVM_DEBUG(dbgs() << "\nAfter guardSideEffectStatemets the function ::"
                 << *NewF);

      // Make sure to run kernel finalization after guardSideEffectStatemets(),
      // which is responsible for cleaning up all directive calls that
      // were left until this point for guardSideEffectStatemets() to work.
      // The extra directive call may prevent address space inferring.
      NewF = finalizeKernelFunction(W, NewF, NewCall);
      LLVM_DEBUG(dbgs() << "\nAfter finalizeKernel Dump the function ::"
                 << *NewF);
    }
  }

  if (hasOffloadCompilation())
    // Everything below only makes sense on the host.
    return true;

  // allocas should stay close to the call, in case the target region is
  // enclosed in another region which is outlined later.
  IRBuilder<> Builder(NewCall->getParent()->getFirstNonPHI());
  AllocaInst *OffloadError = Builder.CreateAlloca(
      Type::getInt32Ty(F->getContext()), nullptr, ".run_host_version");

  Value *VIf = W->getIf();
  CallInst *Call;
  Instruction *InsertPt = NewCall;

  if (VIf) {
    // If the target construct has if clause, the compiler will generate a
    // if-then-else statement.
    //
    // Example:
    //   #pragma omp target enter data map(to: arg) if(arg)
    //
    // *** IR Dump After VPO Paropt Pass ***
    // entry:
    //   ...
    //   %arg.addr = alloca i32, align 4
    //   store i32 %arg, i32* %arg.addr, align 4, !tbaa !2
    //   %tobool = icmp ne i32 %arg, 0
    //   %.run_host_version = alloca i32
    //   %0 = icmp ne i1 %tobool, false
    //   br label %codeRepl
    //
    // codeRepl:
    //   br i1 %0, label %if.then, label %if.else
    //
    //  if.then:
    //    ...
    //    call void @__tgt_target_data_begin(i64 -1, i32 1, i8** %5,
    //      i8** %6, i64* getelementptr inbounds ([1 x i64],
    //      [1 x i64]* @.offload_sizes, i32 0, i32 0),
    //      i64* getelementptr inbounds ([1 x i64],
    //      [1 x i64]* @.offload_maptypes, i32 0, i32 0))
    //    br label %if.end
    //
    // if.else:
    //   store i32 -1, i32* %.run_host_version
    //   br label %if.end
    //
    // if.end:
    //   ...
    //
    Builder.SetInsertPoint(NewCall);
    Value *Cmp = Builder.CreateICmpNE(VIf, ConstantInt::get(VIf->getType(), 0));
    Instruction *ThenTerm, *ElseTerm;
    VPOParoptUtils::buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, InsertPt, DT);
    InsertPt = ThenTerm;
    Call = genTargetInitCode(W, NewCall, RegionId, InsertPt);
    Builder.SetInsertPoint(ElseTerm);
    Builder.CreateStore(
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), -1),
        OffloadError);
    if (isa<WRNTargetDataNode>(W)) {
      // For target data directive, if the "if" clause is evaluated to false,
      // device is host, and the outlined function is called without mapping
      // data.
      SmallVector<Value *, 4> FnArgs(NewCall->arg_operands());
      Builder.CreateCall(NewF, FnArgs, "");
    }
  } else
    Call = genTargetInitCode(W, NewCall, RegionId, InsertPt);

  if (!hasOffloadCompilation()) {
    if (isa<WRNTargetNode>(W)) {
      Builder.SetInsertPoint(InsertPt);
      Builder.CreateStore(Call, OffloadError);

      Builder.SetInsertPoint(NewCall);
      LoadInst *LastLoad = Builder.CreateLoad(OffloadError);
      ConstantInt *ValueZero =
          ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 0);
      Value *ErrorCompare = Builder.CreateICmpNE(LastLoad, ValueZero);
      Instruction *Term = SplitBlockAndInsertIfThen(ErrorCompare, NewCall,
                                                    false, nullptr, DT, LI);
      Term->getParent()->setName("omp_offload.failed");
      LastLoad->getParent()->getTerminator()->getSuccessor(1)->setName(
          "omp_offload.cont");
      NewCall->removeFromParent();
      NewCall->insertBefore(Term->getParent()->getTerminator());
    } else if (isa<WRNTargetDataNode>(W)) {
      NewCall->removeFromParent();
      NewCall->insertAfter(Call);
      useUpdatedUseDevicePtrsInTgtDataRegion(W, NewCall);
      if (!NewF->hasFnAttribute(Attribute::OptimizeNone)) {
        NewF->removeFnAttr(Attribute::NoInline);
        NewF->addFnAttr(Attribute::AlwaysInline);
      }
    } else if (isa<WRNTargetEnterDataNode>(W) ||
               isa<WRNTargetExitDataNode>(W) ||
               isa<WRNTargetUpdateNode>(W)) {
      // We cannot delete the outlined functions, because they
      // may contain some meaningful code (e.g. InstCombine may put
      // something into the initially empty blocks).
      if (!NewF->hasFnAttribute(Attribute::OptimizeNone)) {
        NewF->removeFnAttr(Attribute::NoInline);
        NewF->addFnAttr(Attribute::AlwaysInline);
      }
    }
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genTargetOffloadingCode\n");

  W->resetBBSet(); // Invalidate BBSet after transformations
  return true;
}

// Set the value in num_teams, thread_limit and num_threads clauses to be empty.
void VPOParoptTransform::resetValueInNumTeamsAndThreadsClause(WRegionNode *W) {
  if (W->getIsTeams()) {
    if (auto *NumTeamsPtr = W->getNumTeams())
      resetValueInOmpClauseGeneric(W, NumTeamsPtr);

    if (auto *ThreadLimitPtr = W->getThreadLimit())
      resetValueInOmpClauseGeneric(W, ThreadLimitPtr);

    return;
  }

  if (W->getIsPar())
    if (auto *NumThreadsPtr = W->getNumThreads())
      resetValueInOmpClauseGeneric(W, NumThreadsPtr);
}

// Reset the expression value in IsDevicePtr clause to be empty.
void VPOParoptTransform::resetValueInIsDevicePtrClause(WRegionNode *W) {
  if (!W->canHaveIsDevicePtr())
    return;

  IsDevicePtrClause &IDevicePtrClause = W->getIsDevicePtr();
  if (IDevicePtrClause.empty())
    return;

  for (auto *I : IDevicePtrClause.items()) {
    resetValueInOmpClauseGeneric(W, I->getOrig());
  }
}

// Returns the corresponding flag for a given map clause modifier.
uint64_t VPOParoptTransform::getMapTypeFlag(MapItem *MapI,
                                            bool AddrIsTargetParamFlag,
                                            bool IsFirstComponentFlag) {

  if (MapI->getInUseDevicePtr())
    return (TGT_MAP_TARGET_PARAM | TGT_MAP_RETURN_PARAM);

  uint64_t Res = 0u;
  if (!AddrIsTargetParamFlag && IsFirstComponentFlag)
    return TGT_MAP_TARGET_PARAM;

  assert(!MapI->getIsMapNone() &&
         "Cannot compute map type flag. No type info in clause.");

  if (MapI->getIsMapTofrom())
    Res = TGT_MAP_TO | TGT_MAP_FROM;
  else if (MapI->getIsMapTo() || MapI->getInFirstprivate() ||
           MapI->getIsMapUpdateTo())
    Res = TGT_MAP_TO;
  else if (MapI->getIsMapFrom() || MapI->getIsMapUpdateFrom())
    Res = TGT_MAP_FROM;
  else if (MapI->getIsMapDelete())
    Res = TGT_MAP_DELETE;

  // WRNMapAlloc and WRNMapRelease are the default behavior in the runtime.

  if (MapI->getIsMapAlways())
    Res |= TGT_MAP_ALWAYS;

  if (MapI->getIsMapClose())
    Res |= TGT_MAP_CLOSE;

  // Memberof is given by the 16 MSB of the flag, so rotate by 48 bits.
  // It is workaroud. Need more work.
  auto getMemberOfFlag = [&]() {
    return (uint64_t)1 << 48;
  };

  // The flag AddrIsTargetParamFlag indicates that the map clause is
  // not in a chain. If it is head of the chain, according to the logic at
  // the entry of function getMapTypeFlag, it returns TGT_MAP_TARGET_PARAM.
  if (AddrIsTargetParamFlag)
    Res |= TGT_MAP_TARGET_PARAM;
  else
    Res |= TGT_MAP_PTR_AND_OBJ | getMemberOfFlag();

  return Res;
}

// Generate the sizes and map type flags for the given map type, map
// modifier and the expression V.
void VPOParoptTransform::genTgtInformationForPtrs(
    WRegionNode *W, Value *V, SmallVectorImpl<Constant *> &ConstSizes,
    SmallVectorImpl<uint64_t> &MapTypes,
    bool &hasRuntimeEvaluationCaptureSize) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTgtInformationForPtrs:"
                    << " ConstSizes.size()=" << ConstSizes.size()
                    << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");

  const DataLayout DL = F->getParent()->getDataLayout();
  LLVMContext &C = F->getContext();

  bool ForceMapping =
      isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W) ||
      isa<WRNTargetVariantNode>(W);

  MapClause const &MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    if (!ForceMapping &&
        (MapI->getOrig() != V || !MapI->getOrig()))
      continue;
    Type *T = MapI->getOrig()->getType()->getPointerElementType();
    if (MapI->getIsMapChain()) {
      uint64_t MapTypeIndexForBaseOfChain = MapTypes.size() + 1;
      MapChainTy const &MapChain = MapI->getMapChain();
      for (unsigned I = 0; I < MapChain.size(); ++I) {
        MapAggrTy *Aggr = MapChain[I];
        auto ConstValue = dyn_cast<ConstantInt>(Aggr->getSize());
        if (!ConstValue) {
          hasRuntimeEvaluationCaptureSize = true;
          ConstSizes.push_back(ConstantInt::get(
              Type::getInt64Ty(C), DL.getTypeAllocSize(T)));
        } else {
          // Sign extend the constant to signed 64-bit integer.
          // This is the format of arg_sizes passed to __tgt_target.
          ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                                ConstValue->getSExtValue()));
        }

        uint64_t MapType = Aggr->getMapType();
        if (MapType) {
          // MemberOf flag is in the 16 MSBs of the 64 bit MapType.
          uint64_t MemberOfFlag = MapType >> 48;
          uint64_t NewMapType = MapType;

          if (MemberOfFlag && MemberOfFlag != MapTypeIndexForBaseOfChain) {
            // We need to set MemberOf to the index of the base of the chain,
            // instead of what the frontend sent in.
            assert(MapTypeIndexForBaseOfChain < (1 << 16) &&
                   "Too many maps. MemberOf flag exceeding 16 bits.");
            uint64_t Mask = (~(0ull)) >> 16;
            NewMapType = NewMapType & Mask;
            NewMapType = NewMapType | (MapTypeIndexForBaseOfChain << 48);
            LLVM_DEBUG(dbgs()
                       << __FUNCTION__ << ": Updated MemberOf Flag from '"
                       << MemberOfFlag << "' to '" << MapTypeIndexForBaseOfChain
                       << "'.\n");
          }

          // For operands in a map chain, as well as use_device_ptr clause, we
          // need to add TGT_MAP_RETURN_PARAM to the map-type of the base of
          // the chain (if not already present).
          if (I == 0 && MapI->getInUseDevicePtr()) {
            NewMapType = NewMapType | TGT_MAP_RETURN_PARAM;
            LLVM_DEBUG(dbgs() << __FUNCTION__
                              << ": Added TGT_MAP_RETURN_PARAM flag.\n");
          }

          if (MapType != NewMapType) {
            LLVM_DEBUG(dbgs() << __FUNCTION__ << ": MapType changed from '"
                              << MapType << "' to '" << NewMapType << "'.\n");
            MapType = NewMapType;
          }
          MapTypes.push_back(MapType);
        } else
          MapTypes.push_back(
              getMapTypeFlag(MapI, MapChain.size() <= 1, I == 0));
      }
    } else {
      assert(!MapI->getIsArraySection() &&
             "Map with an array section must have a map chain.");
      ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                            DL.getTypeAllocSize(T)));
      MapTypes.push_back(getMapTypeFlag(MapI, true, true));
    }
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      if (FprivI->getOrig() != V)
        continue;
      if (FprivI->getInMap())
        continue;
      Type *T = V->getType()->getPointerElementType();
      if (FprivI->getIsPointer()) {
        // firstprivate() pointers are mapped with zero size
        // and map type NONE.
        ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C), 0));
        MapTypes.push_back(TGT_MAP_TARGET_PARAM);
      }
      else {
        ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                              DL.getTypeAllocSize(T)));
        MapTypes.push_back(TGT_MAP_TARGET_PARAM | TGT_MAP_TO);
      }
    }
  }

  if (W->canHaveIsDevicePtr()) {
    IsDevicePtrClause &IDevicePtrClause = W->getIsDevicePtr();
    for (IsDevicePtrItem *IsDevicePtrI : IDevicePtrClause.items()) {
      if (IsDevicePtrI->getOrig() != V)
        continue;
      Type *T = V->getType();
      ConstSizes.push_back(
          ConstantInt::get(Type::getInt64Ty(C), DL.getTypeAllocSize(T)));
      // Example:
      //   int *p;
      //   #pragma omp target is_device_ptr(p)
      //
      // The IR will look like:
      //   %p = load i32* i32** @p
      //   ...IS_DEVICE_PTR(i32* %p) PRIVATE(i32** @p)
      //   store i32* %p, i32** @p
      //
      // We have to map 'p' as MAP_TYPE_LITERAL so that we pass the value
      // of the pointer as is to the target construct without mapping/
      // allocating memory.
      MapTypes.push_back(TGT_MAP_TARGET_PARAM | TGT_MAP_LITERAL);
    }
  }
  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca() == V) {
    Type *T = W->getParLoopNdInfoAlloca()->getType()->getPointerElementType();
    ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                          DL.getTypeAllocSize(T)));
    MapTypes.push_back(TGT_MAP_ND_DESC);
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genTgtInformationForPtrs:"
                    << " ConstSizes.size()=" << ConstSizes.size()
                    << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");
}

// Initialize the loop descriptor struct with the loop level
// as well as the lb, ub, stride for each level of the loop.
//
// The data structure created for runtime is declared as:
//   typedef struct {
//     int64_t Lb;     // The lower bound of the loop in i-th dimension
//     int64_t Ub;     // The upper bound of the loop in i-th dimension
//     int64_t Stride; // The stride of the loop in i-th dimension
//   } TgtLoopDescTy;
//
//   typedef struct {
//     int32_t NumLoops;        // Number of loops/dimensions
//     int32_t DistributeDim;   // Dimensions lower than this one
//                              // must end up in one WG
//     TgtLoopDescTy Levels[3]; // Up to 3 loops
//   } TgtNDRangeDescTy;
AllocaInst *VPOParoptTransform::genTgtLoopParameter(WRegionNode *W) {
  auto &UncollapsedNDRange = W->getUncollapsedNDRange();
  uint8_t DistributeDim = W->getNDRangeDistributeDim();

  if (UncollapsedNDRange.empty())
    return nullptr;

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB = SplitBlock(EntryBB, &*(EntryBB->begin()), DT, LI);
  W->setEntryBBlock(NewEntryBB);

  unsigned NumLoops = UncollapsedNDRange.size();
  NumLoops += DistributeDim;
  assert(NumLoops <= 3 && "Max 3 dimensions for ND-range execution.");

  LLVMContext &C = F->getContext();
  IntegerType *Int64Ty = Type::getInt64Ty(C);
  Instruction *InsertPt = EntryBB->getTerminator();
  IRBuilder<> Builder(InsertPt);
  SmallVector<Type *, 4> CLLoopParameterRecTypeArgs;
  CLLoopParameterRecTypeArgs.push_back(Builder.getInt32Ty());
  CLLoopParameterRecTypeArgs.push_back(Builder.getInt32Ty());
  for (unsigned I = 0; I < NumLoops; I++) {
    CLLoopParameterRecTypeArgs.push_back(Int64Ty);
    CLLoopParameterRecTypeArgs.push_back(Int64Ty);
    CLLoopParameterRecTypeArgs.push_back(Int64Ty);
  }
  StructType *CLLoopParameterRecType =
      StructType::get(C,
                      makeArrayRef(CLLoopParameterRecTypeArgs.begin(),
                                   CLLoopParameterRecTypeArgs.end()),
                      false);
  AllocaInst *DummyCLLoopParameterRec = Builder.CreateAlloca(
      CLLoopParameterRecType, nullptr, "loop.parameter.rec");
  Value *NumLoopsGep =
      Builder.CreateInBoundsGEP(CLLoopParameterRecType, DummyCLLoopParameterRec,
                                {Builder.getInt32(0), Builder.getInt32(0)});

  Builder.CreateStore(Builder.getInt32(NumLoops), NumLoopsGep);

  Value *DistributeDimGep =
      Builder.CreateInBoundsGEP(CLLoopParameterRecType, DummyCLLoopParameterRec,
                                {Builder.getInt32(0), Builder.getInt32(1)});
  Builder.CreateStore(Builder.getInt32(DistributeDim), DistributeDimGep);

  for (unsigned I = 0; I < NumLoops; I++) {
    // We assume that the innermost OpenMP loop stepping provides
    // the best data locality. OpenCL execution assumes that the
    // fastest changing dimension in ND-range is the 1st one.
    // We need to specify the ND-range such that the 1st dimension
    // corresponds to the innermost loop (with loop index NumLoops - 1).
    unsigned Idx = NumLoops - I - 1;
    Value *LowerBndGep = Builder.CreateInBoundsGEP(
        CLLoopParameterRecType, DummyCLLoopParameterRec,
        {Builder.getInt32(0), Builder.getInt32(3 * I + 2)});
    Builder.CreateStore(Builder.getInt64(0), LowerBndGep);

    Value *UpperBndGep = Builder.CreateInBoundsGEP(
        CLLoopParameterRecType, DummyCLLoopParameterRec,
        {Builder.getInt32(0), Builder.getInt32(3 * I + 3)});
    Value *CloneUB = nullptr;
    if (I < DistributeDim)
      CloneUB = Builder.getInt64(0);
    else
      CloneUB = Builder.CreateLoad(UncollapsedNDRange[Idx]);

    assert(CloneUB && "genTgtLoopParameter: unexpected null CloneUB");
    Builder.CreateStore(Builder.CreateSExtOrTrunc(CloneUB, Int64Ty),
                        UpperBndGep);

    Value *StrideGep = Builder.CreateInBoundsGEP(
        CLLoopParameterRecType, DummyCLLoopParameterRec,
        {Builder.getInt32(0), Builder.getInt32(3 * I + 4)});
    Builder.CreateStore(Builder.getInt64(1), StrideGep);
  }

  return DummyCLLoopParameterRec;
}

void VPOParoptTransform::genMapChainsForMapArraySections(
    WRegionNode *W, Instruction *InsertPt) {
  LLVMContext &C = F->getContext();
  MapClause const &MpClause = W->getMap();
  const auto DL = F->getParent()->getDataLayout();

  for (MapItem *MapI : MpClause.items()) {
    if (!MapI->getIsArraySection())
      continue;

    computeArraySectionTypeOffsetSize(*MapI, InsertPt);
    IRBuilder<> GepBuilder(InsertPt);
    const ArraySectionInfo &ArrSecInfo = MapI->getArraySectionInfo();
    auto *BasePtr = MapI->getOrig();
    if (ArrSecInfo.getBaseIsPointer())
      BasePtr = GepBuilder.CreateLoad(BasePtr, BasePtr->getName() + ".load");

    auto *ElementTy = ArrSecInfo.getElementType();
    auto *SectionPtr =
        GepBuilder.CreateBitCast(BasePtr,
                                 PointerType::getUnqual(ElementTy),
                                 BasePtr->getName() + ".cast");
    SectionPtr = GepBuilder.CreateGEP(SectionPtr, ArrSecInfo.getOffset(),
                                      SectionPtr->getName() + ".plus.offset");

    auto *NumElements = ArrSecInfo.getSize();
    NumElements = GepBuilder.CreateSExtOrTrunc(NumElements,
                                               Type::getInt64Ty(C));

    auto *TypeSize = ConstantInt::get(Type::getInt64Ty(C),
                                      DL.getTypeAllocSize(ElementTy));
    auto *Size = GepBuilder.CreateMul(NumElements, TypeSize,
                                      BasePtr->getName() + ".map.size");

    auto *Chain = new MapAggrTy(BasePtr, SectionPtr, Size);
    MapI->setMapChainForArraySection(Chain);
  }
}

// Generate the initialization code for the directive omp target.
// Given a program as follows. The compiler creates the four arrays
// offload_baseptrs, offload_ptrs, offload_sizes and offload_maptypes.
// The compiler initializes the arrays based on the target clauses and passes
// the arrays into the library call __tgt_target.
//
// struct SC *p;
// void foo(int size) {
// #pragma omp target map(p->s.a)
//   {  p->a++; }
//
// *** IR Dump After Module Verifier ***
//
// %0 = load %struct.SC*, %struct.SC** @p, align 8
// %a = getelementptr inbounds %struct.SB, %struct.SB* %s, i32 0, i32 0
// %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
//   "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.SC* %0, i32* %a, i64 4) ]
//
// *** IR Dump After VPO Paropt Pass ***;
//
//  %2 = bitcast %struct.SC* %1 to i8*
//  %3 = getelementptr inbounds [1 x i8*],
//       [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
//  store i8* %2, i8** %3
//  %4 = getelementptr inbounds [1 x i8*],
//       [1 x i8*]* %.offload_ptrs, i32 0, i32 0
//  %5 = bitcast i32* %a to i8*
//  store i8* %5, i8** %4
//  %6 = getelementptr inbounds [1 x i8*],
//       [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
//  %7 = getelementptr inbounds [1 x i8*],
//       [1 x i8*]* %.offload_ptrs, i32 0, i32 0
//  %8 = call i32 @__tgt_target(i64 -1, i8* @.omp_offload.region_id,
//       i32 1, i8** %6, i8** %7, i64* getelementptr inbounds ([1 x i64],
//       [1 x i64]* @.offload_sizes, i32 0, i32 0),
//       i64* getelementptr inbounds ([1 x i64],
//       [1 x i64]* @.offload_maptypes, i32 0, i32 0))

//
CallInst *VPOParoptTransform::genTargetInitCode(WRegionNode *W, CallInst *Call,
                                                Value *RegionId,
                                                Instruction *InsertPt) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTargetInitCode\n");
  assert(!hasOffloadCompilation() &&
         "genTargetInitCode() called for device compilation.");

  TgDataInfo Info;

  Info.NumberOfPtrs = Call->getNumArgOperands();
  bool hasRuntimeEvaluationCaptureSize = false;
  bool ForceMapping =
      // These regions will not have any real references to the mapped
      // items, but we still have to notify the runtime about the mappings.
      isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W);

  // Transform array sections in maps to map chains to handle
  // them uniformly.
  genMapChainsForMapArraySections(W, InsertPt);

  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
    Info.NumberOfPtrs++;

  if (Info.NumberOfPtrs || ForceMapping) {

    SmallVector<Constant *, 16> ConstSizes;
    SmallVector<uint64_t, 16> MapTypes;

    if (ForceMapping)
      genTgtInformationForPtrs(W, nullptr, ConstSizes, MapTypes,
                               hasRuntimeEvaluationCaptureSize);
    else {
      for (unsigned II = 0; II < Call->getNumArgOperands(); ++II) {
        Value *BPVal = Call->getArgOperand(II);
        genTgtInformationForPtrs(W, BPVal, ConstSizes, MapTypes,
                                 hasRuntimeEvaluationCaptureSize);
      }
      if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
        genTgtInformationForPtrs(W, W->getParLoopNdInfoAlloca(), ConstSizes,
                                 MapTypes, hasRuntimeEvaluationCaptureSize);
    }

    Info.NumberOfPtrs = MapTypes.size();

    genOffloadArraysInit(W, &Info, Call, InsertPt, ConstSizes,
                         MapTypes, hasRuntimeEvaluationCaptureSize);
  }

  genOffloadArraysArgument(&Info, InsertPt);

  CallInst *TgtCall = nullptr;
  if (isa<WRNTargetNode>(W)) {
    auto *IT = W->wrn_child_begin();
    if (IT != W->wrn_child_end() && isa<WRNTeamsNode>(*IT)) {
      WRNTeamsNode *TW = cast<WRNTeamsNode>(*IT);
      TgtCall = VPOParoptUtils::genTgtTargetTeams(
          TW, RegionId, Info.NumberOfPtrs, Info.ResBaseDataPtrs,
          Info.ResDataPtrs, Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
    } else
      TgtCall = VPOParoptUtils::genTgtTarget(
          W, RegionId, Info.NumberOfPtrs, Info.ResBaseDataPtrs,
          Info.ResDataPtrs, Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
  } else if (isa<WRNTargetDataNode>(W)) {
    TgtCall = VPOParoptUtils::genTgtTargetDataBegin(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
    genOffloadArraysArgument(&Info, InsertPt);
    VPOParoptUtils::genTgtTargetDataEnd(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
  } else if (isa<WRNTargetUpdateNode>(W))
    TgtCall = VPOParoptUtils::genTgtTargetDataUpdate(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
  else if (isa<WRNTargetEnterDataNode>(W))
    TgtCall = VPOParoptUtils::genTgtTargetDataBegin(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
  else if (isa<WRNTargetExitDataNode>(W))
    TgtCall = VPOParoptUtils::genTgtTargetDataEnd(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
  else
    llvm_unreachable("genTargetInitCode: Unexpected region node.");

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genTargetInitCode\n");
  return TgtCall;
}

// Generate the cast i8* for the incoming value BPVal.
Value *VPOParoptTransform::genCastforAddr(Value *BPVal, IRBuilder<> &Builder) {
  if (BPVal->getType()->isPointerTy())
    return Builder.CreateBitCast(BPVal, Builder.getInt8PtrTy());
  else
    return Builder.CreateIntToPtr(BPVal, Builder.getInt8PtrTy());
}

bool VPOParoptTransform::addMapForUseDevicePtr(WRegionNode *W,
                                         Instruction *InsertPt ) {
  assert(W && "Null WRegionNode.");

  if (!W->canHaveUseDevicePtr() || !(isa<WRNTargetDataNode>(W) ||
                                  isa<WRNTargetVariantNode>(W)))
    return false;

  UseDevicePtrClause &UDPC = W->getUseDevicePtr();
  if (UDPC.empty())
    return false;

  // Create an empty BB before the entry BB, to insert loads for the new map
  // clauses if InsertPt is null.
  if (!InsertPt) {
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *NewEntryBB =
        SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
    W->setEntryBBlock(NewEntryBB);
    W->populateBBSet(true); // rebuild BBSet unconditionlly as EntryBB changed
    InsertPt = EntryBB->getTerminator();
  }
  IRBuilder<> LoadBuilder(InsertPt);
  Value *MapSize = LoadBuilder.getInt64(0);
  uint64_t MapType = TGT_MAP_TARGET_PARAM | TGT_MAP_RETURN_PARAM;
  MapClause &MapC = W->getMap();
  for (UseDevicePtrItem *UDPI : UDPC.items()) {
    // There's already a map clause present for the use_device_ptr clause item.
    if (UDPI->getInMap())
      continue;

    Value *UDP = UDPI->getOrig();
    Value *MappedVal =
        UDPI->getIsPointerToPointer()
            ? LoadBuilder.CreateLoad(
                  cast<PointerType>(UDP->getType())->getPointerElementType(),
                  UDP, UDP->getName() + ".load")
            : UDP;
    MapAggrTy *MapAggr = new MapAggrTy(MappedVal, MappedVal, MapSize, MapType);
    MapItem *MapI = new MapItem(MapAggr);
    MapI->setOrig(MappedVal);
    MapC.add(MapI);

    MapI->setInUseDevicePtr(UDPI);
    UDPI->setInMap(MapI);

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Added map clause on '";
               MappedVal->printAsOperand(dbgs());
               dbgs() << "', for use_device_ptr '"; UDP->printAsOperand(dbgs());
               dbgs() << "'.\n");
  }
  return true;
}

// For the following code:
//   int *udp;
//   #pragma omp target use_device_ptr(udp)
//
// the IR before/after this util will look like:
//
// Case 1: Type of %udp is i32**, and it's marked as PTR_TO_PTR:
// Before:
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     call @outlined.funtion(...i32** %udp)  ; TgtDataOutlinedFunctionCall
//   call void @__tgt_target_data_end()
//
// After:
//   $udp.new = alloca i32*                                          ; (2)
//   ...
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     %udp.gep.cast = cast i8* %udp.gep to i32**                    ; (3)
//     %udp.updated.val = load i32*, i32** %udp.gep.cast             ; (4)
//     store i32* %udp.updated.val, i32** %udp.new                   ; (5)
//     call @outlined.funtion(...i32** %udp.new)                     ; (6)
//   call void @__tgt_target_data_end()
//
// Case 2: Type of %udp is i32*, and it's NOT marked as PTR_TO_PTR:
// Before:
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     call @outlined.funtion(...i32* %udp)  ; TgtDataOutlinedFunctionCall
//   call void @__tgt_target_data_end()
//
// After:
//   ...
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     %udp.gep.cast = cast i8* %udp.gep to i32**                    ; (3)
//     %udp.updated.val = load i32*, i32** %udp.gep.cast             ; (4)
//     call @outlined.funtion(...i32* %udp.updated.val)              ; (6)
//   call void @__tgt_target_data_end()
//
void VPOParoptTransform::useUpdatedUseDevicePtrsInTgtDataRegion(
    WRegionNode *W, Instruction *TgtDataOutlinedFunctionCall) {
  assert(W && "Null WRegionNode.");
  assert(TgtDataOutlinedFunctionCall && "Null outlined function call.");

  if (!W->canHaveUseDevicePtr() || !isa<WRNTargetDataNode>(W))
    return;

  UseDevicePtrClause &UDPC = W->getUseDevicePtr();
  if (UDPC.empty())
    return;

  IRBuilder<> Builder(TgtDataOutlinedFunctionCall);
  Function *F = TgtDataOutlinedFunctionCall->getFunction();
  Instruction *AllocaInsertPt = VPOParoptUtils::getInsertionPtForAllocas(W, F);

  for (UseDevicePtrItem *UDPI : UDPC.items()) {
    MapItem *MapI = UDPI->getInMap();
    assert(MapI && "No map found for use-device-ptr item.");

    Instruction *BasePtrGEP = MapI->getBasePtrGEPForOrig(); //              (1)
    assert(BasePtrGEP && "No GEP found for base-ptr of Map item's orig.");

    Value *OrigV = UDPI->getOrig();
    Type *OrigElemTy = UDPI->getOrigElemType();

    Value *GepCast = Builder.CreateBitOrPointerCast(
        BasePtrGEP, MapI->getOrig()->getType()->getPointerTo(),
        BasePtrGEP->getName() + ".cast"); //                                (3)

    LoadInst *UpdatedUDPVal =
        Builder.CreateLoad(GepCast, OrigV->getName() + ".updated.val"); //  (4)
    Value *NewV = nullptr;
    if (UDPI->getIsPointerToPointer()) {
      NewV = genPrivatizationAlloca(OrigV, OrigElemTy, AllocaInsertPt,
                                    ".new");                        //      (2)
      Builder.CreateStore(UpdatedUDPVal, NewV);                     //      (5)
    } else
      NewV = UpdatedUDPVal;

    TgtDataOutlinedFunctionCall->replaceUsesOfWith(OrigV, NewV); //         (6)

    LLVM_DEBUG(
        dbgs() << __FUNCTION__ << ": Replaced references to use_device_ptr '";
        OrigV->printAsOperand(dbgs()); dbgs() << "', with '";
        NewV->printAsOperand(dbgs()); dbgs() << "' in tgt_data region.\n");
  }
}

// Utilities to construct the assignment to the base pointers, section
// pointers and size pointers if the flag hasRuntimeEvaluationCaptureSize is
// true.
void VPOParoptTransform::genOffloadArraysInitUtil(
    IRBuilder<> &Builder, Value *BasePtr, Value *SectionPtr, Value *Size,
    TgDataInfo *Info, SmallVectorImpl<Constant *> &ConstSizes, unsigned &Cnt,
    bool hasRuntimeEvaluationCaptureSize, Instruction **BasePtrGEPOut) {

  assert(BasePtr && "Unexpected: BasePtr is null");
  assert(SectionPtr && "Unexpected: SectionPtr is null");

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genOffloadArraysInitUtil:"
                    << " BasePtr=(" << *BasePtr << ") SectionPtr=("
                    << *SectionPtr << ") Cnt=" << Cnt << " ConstSizes.size()="
                    << ConstSizes.size() << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");

  Value *NewBPVal, *BP, *P, *S, *SizeValue;

  NewBPVal = genCastforAddr(BasePtr, Builder);
  BP = Builder.CreateConstInBoundsGEP2_32(
      ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
      Info->BaseDataPtrs, 0, Cnt);
  Builder.CreateStore(NewBPVal, BP);
  if (BasePtrGEPOut)
    *BasePtrGEPOut = cast<Instruction>(BP);

  P = Builder.CreateConstInBoundsGEP2_32(
      ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
      Info->DataPtrs, 0, Cnt);
  NewBPVal = genCastforAddr(SectionPtr, Builder);
  Builder.CreateStore(NewBPVal, P);

  if (hasRuntimeEvaluationCaptureSize) {
    LLVMContext &C = F->getContext();
    S = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Type::getInt64Ty(C), Info->NumberOfPtrs),
        Info->DataSizes, 0, Cnt);

    LLVM_DEBUG(dbgs() << "genOffloadArraysInitUtil: Size#" << Cnt << " is ");

    if (Size && !dyn_cast<ConstantInt>(Size)) {
      LLVM_DEBUG(dbgs() << "Nonconstant: ");
      SizeValue = Size;
    } else {
      LLVM_DEBUG(dbgs() << "Constant: ");
      SizeValue = ConstSizes[Cnt];
    }
    LLVM_DEBUG(dbgs() << *SizeValue << "\n");
    Builder.CreateStore(
        Builder.CreateSExt(SizeValue, Type::getInt64Ty(C)), S);
  }
  Cnt++;

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genOffloadArraysInitUtil:"
                    << " Cnt=" << Cnt
                    << " ConstSizes.size()=" << ConstSizes.size() << "\n");
}

// Generate the target intialization code for the pointers based
// on the order of the map clause.
void VPOParoptTransform::genOffloadArraysInitForClause(
    WRegionNode *W, TgDataInfo *Info, CallInst *Call, Instruction *InsertPt,
    SmallVectorImpl<Constant *> &ConstSizes,
    bool hasRuntimeEvaluationCaptureSize, Value *BPVal, bool &Match,
    IRBuilder<> &Builder, unsigned &Cnt) {

  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genOffloadArraysInitForClause:"
             << " ConstSizes.size()=" << ConstSizes.size() << " Match="
             << Match << " Cnt=" << Cnt << " hasRuntimeEvaluationCaptureSize="
             << hasRuntimeEvaluationCaptureSize << " BPVal=(");
  if (BPVal)
    LLVM_DEBUG(dbgs() << *BPVal << ")\n");
  else
    LLVM_DEBUG(dbgs() << "nullptr)\n");

  bool ForceMapping =
      isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W) ||
      isa<WRNTargetVariantNode>(W);

  MapClause const &MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    if (!ForceMapping &&
        (MapI->getOrig() != BPVal || !MapI->getOrig()))
      continue;
    if (ForceMapping)
      BPVal = MapI->getOrig();
    Match = true;
    Instruction *BasePtrGEP = nullptr;
    if (MapI->getIsMapChain()) {
      MapChainTy const &MapChain = MapI->getMapChain();
      for (unsigned I = 0; I < MapChain.size(); ++I) {
        MapAggrTy *Aggr = MapChain[I];
        genOffloadArraysInitUtil(
            Builder, Aggr->getBasePtr(), Aggr->getSectionPtr(), Aggr->getSize(),
            Info, ConstSizes, Cnt, hasRuntimeEvaluationCaptureSize,
            I == 0 ? &BasePtrGEP : nullptr);
      }
    } else {
      assert(!MapI->getIsArraySection() &&
             "Map with an array section must have a map chain.");
      genOffloadArraysInitUtil(Builder, BPVal, BPVal, nullptr, Info, ConstSizes,
                               Cnt, hasRuntimeEvaluationCaptureSize,
                               &BasePtrGEP);
    }
    MapI->setBasePtrGEPForOrig(BasePtrGEP);
  }

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genOffloadArraysInitForClause:"
             << " ConstSizes.size()=" << ConstSizes.size()
             << " Match=" << Match << " Cnt=" << Cnt << "\n");
}

// Create array of base pointer,  array of section pointer
// array of MAP types and array of Target pointer.
// Pass the data to the array of base pointer as well as  array of
// section pointers. If the flag hasRuntimeEvaluationCaptureSize is true,
// the compiler needs to generate the init code for the size array.
void VPOParoptTransform::genOffloadArraysInit(
    WRegionNode *W, TgDataInfo *Info, CallInst *Call, Instruction *InsertPt,
    SmallVectorImpl<Constant *> &ConstSizes,
    SmallVectorImpl<uint64_t> &MapTypes,
    bool hasRuntimeEvaluationCaptureSize) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genOffloadArraysInit:"
                    << " ConstSizes.size()=" << ConstSizes.size()
                    << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");

  Value *BPVal;
  IRBuilder<> Builder(InsertPt);
  unsigned Cnt = 0;
  bool Match = false;
  Value *SizesArray;
  LLVMContext &C = F->getContext();

  // Build the alloca defs of the target parms.
  // The allocas must be kept in the same region as their uses,
  // in case more outlining transformations are made.
  if (hasRuntimeEvaluationCaptureSize)
    SizesArray = Builder.CreateAlloca(
          ArrayType::get(Type::getInt64Ty(C), Info->NumberOfPtrs),
          nullptr, ".offload_sizes");
  else {
    auto *SizesArrayInit = ConstantArray::get(
        ArrayType::get(Type::getInt64Ty(C), ConstSizes.size()),
        ConstSizes);

    GlobalVariable *SizesArrayGbl =
        new GlobalVariable(*(F->getParent()), SizesArrayInit->getType(), true,
                           GlobalValue::PrivateLinkage, SizesArrayInit,
                           ".offload_sizes", nullptr);
    SizesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    SizesArray = SizesArrayGbl;
  }

  AllocaInst *TgBasePointersArray = Builder.CreateAlloca(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs), nullptr,
        ".offload_baseptrs");

  AllocaInst *TgPointersArray = Builder.CreateAlloca(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs), nullptr,
        ".offload_ptrs");

  Constant *MapTypesArrayInit =
        ConstantDataArray::get(Builder.getContext(), MapTypes);
  auto *MapTypesArrayGbl =
      new GlobalVariable(*(F->getParent()), MapTypesArrayInit->getType(),
                         true, GlobalValue::PrivateLinkage, MapTypesArrayInit,
                         ".offload_maptypes", nullptr);
  MapTypesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  Info->BaseDataPtrs = TgBasePointersArray;
  Info->DataPtrs = TgPointersArray;
  Info->DataSizes = SizesArray;
  Info->DataMapTypes = MapTypesArrayGbl;

  if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W) ||
      isa<WRNTargetVariantNode>(W)) {
    genOffloadArraysInitForClause(W, Info, Call, InsertPt, ConstSizes,
                                  hasRuntimeEvaluationCaptureSize, nullptr,
                                  Match, Builder, Cnt);
    LLVM_DEBUG(dbgs() << "\nExit1 VPOParoptTransform::genOffloadArraysInit:"
                      << " ConstSizes.size()=" << ConstSizes.size() << "\n");
    return;
  }

  // FIXME: this code partly duplicates genTargetInitCode().
  //        Code in genOffloadArraysInitForClause() partly duplicates
  //        genTgtInformationForPtr(). We walk through the same list
  //        of arguments and create offload data structures in one place,
  //        while generating dynamic initialization in another place.
  //        This is a potential source of issues, since the data
  //        structures and the initializations must be properly ordered.
  //        We'd better have all the information in some structure,
  //        e.g. TgDataInfo, and just process it here.
  for (unsigned II = 0; II < Call->getNumArgOperands(); ++II) {
    BPVal = Call->getArgOperand(II);

    Match = false;
    genOffloadArraysInitForClause(W, Info, Call, InsertPt, ConstSizes,
                                  hasRuntimeEvaluationCaptureSize, BPVal, Match,
                                  Builder, Cnt);

    if (!Match)
      genOffloadArraysInitUtil(Builder, BPVal, BPVal, nullptr, Info, ConstSizes,
                               Cnt, hasRuntimeEvaluationCaptureSize);
  }
  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
    genOffloadArraysInitUtil(Builder, W->getParLoopNdInfoAlloca(),
                             W->getParLoopNdInfoAlloca(), nullptr, Info,
                             ConstSizes, Cnt, hasRuntimeEvaluationCaptureSize);

  LLVM_DEBUG(dbgs() << "\nExit2 VPOParoptTransform::genOffloadArraysInit:"
                    << " ConstSizes.size()=" << ConstSizes.size() << "\n");
}

// Generate the pointers pointing to the array of base pointer, the
// array of section pointers, the array of sizes, the array of map types.
void VPOParoptTransform::genOffloadArraysArgument(
    TgDataInfo *Info, Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);

  LLVMContext &C = F->getContext();

  LLVM_DEBUG(dbgs() << "\nVPOParoptTransform::genOffloadArraysArgument:"
                    << " Info->NumberOfPtrs=" << Info->NumberOfPtrs << "\n");

  if (Info->NumberOfPtrs) {
    Info->ResBaseDataPtrs = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->BaseDataPtrs, 0, 0);
    Info->ResDataPtrs = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->DataPtrs, 0, 0);
    Info->ResDataSizes = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Type::getInt64Ty(C), Info->NumberOfPtrs),
        Info->DataSizes, 0, 0);
    Info->ResDataMapTypes = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Type::getInt64Ty(C), Info->NumberOfPtrs),
        Info->DataMapTypes, 0, 0);
  } else {
    Info->ResBaseDataPtrs = ConstantPointerNull::get(
        PointerType::getUnqual(Builder.getInt8PtrTy()));
    Info->ResDataPtrs = ConstantPointerNull::get(
        PointerType::getUnqual(Builder.getInt8PtrTy()));
    Info->ResDataSizes =
        ConstantPointerNull::get(PointerType::getUnqual(Type::getInt64Ty(C)));
    Info->ResDataMapTypes =
        ConstantPointerNull::get(PointerType::getUnqual(Type::getInt64Ty(C)));
  }
}


/// Remove the launder intrinsics inserted while renaming globals by
/// genGlobalPrivatizationLaunderIntrin(). The capturing happens in
/// vpo-paropt-prepare pass, and the generated intrinsics are later removed
/// in the vpo-paropt transform pass.
///
/// Before:
/// \code
/// %1 = bitcast
///   %0 = bitcast i32* @V to i8*
///   %1 = call i8* @llvm.launder.invariant.group.p0i8(i8* %0)
///   %V1 = bitcast i8* %1 to i32*
///   %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
///        "QUAL.OMP.FIRSTPRIVATE"(i32* %V1) ]
/// \endcode
///
/// After:
/// \code
///   %0 = bitcast i32* @V to i8*
///   %V1 = bitcast i8* %0 to i32*
///   %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
///        "QUAL.OMP.FIRSTPRIVATE"(i32* %V1) ]
/// \endcode
bool VPOParoptTransform::clearLaunderIntrinBeforeRegion(WRegionNode *W) {

  DenseMap<Value *, Value *> RenameMap;
  bool Changed = false;

  // Check if Orig is a launder intrinsic, or a bitcast whose operand is a
  // launder intrinsic, and if so, remove the launder intrinsic.
  // Return the Value which can be used to replace uses of Orig.
  auto removeLaunderIntrinsic = [&](Value *Orig, bool CheckAlreadyHandled) {
    if (CheckAlreadyHandled) {
      auto VOrigAndNew = RenameMap.find(Orig);
      if (VOrigAndNew != RenameMap.end())
        return VOrigAndNew->second;
    }

    BitCastInst *BI = dyn_cast_or_null<BitCastInst>(Orig);
    // For i8* operands, there is no BitCast, so the clause operand may itself
    // be a launder intrinsic.
    Value *V = (BI == nullptr) ? Orig : BI->getOperand(0);

    CallInst *CI = dyn_cast<CallInst>(V);
    if (CI && isFenceCall(CI)) {
      LLVM_DEBUG(dbgs() << "clearLaunderIntrinBeforeRegion: Replacing "
                           "launder intrinsic '";
                 CI->printAsOperand(dbgs()); dbgs() << "' with its operand.\n");

      Value *NewV = CI->getOperand(0);
      CI->replaceAllUsesWith(NewV);
      CI->eraseFromParent();
      RenameMap.insert({V, NewV});
      Changed = true;
      if (V == Orig) // If V is Orig, we want to replace uses of Orig with NewV,
        return NewV; // but not when V is a bitcast on Orig.
    }
    RenameMap.insert({Orig, Orig});
    return Orig;
  };

  Value *NewV = nullptr;

  if (W->canHavePrivate()) {
    PrivateClause const &PrivClause = W->getPriv();
    for (PrivateItem *PrivI : PrivClause.items()) {
      NewV = removeLaunderIntrinsic(PrivI->getOrig(), false);
      PrivI->setOrig(NewV);
    }
  }

  if (W->canHaveReduction()) {
    ReductionClause const &RedClause = W->getRed();
    for (ReductionItem *RedI : RedClause.items()) {
      NewV = removeLaunderIntrinsic(RedI->getOrig(), false);
      RedI->setOrig(NewV);
    }
  }

  if (W->canHaveLinear()) {
    LinearClause const &LrClause = W->getLinear();
    for (LinearItem *LrI : LrClause.items()) {
      NewV = removeLaunderIntrinsic(LrI->getOrig(), false);
      LrI->setOrig(NewV);
    }
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      NewV = removeLaunderIntrinsic(FprivI->getOrig(), false);
      FprivI->setOrig(NewV);
    }
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause const &LprivClause = W->getLpriv();
    for (LastprivateItem *LprivI : LprivClause.items()) {
      NewV = removeLaunderIntrinsic(LprivI->getOrig(), true);
      LprivI->setOrig(NewV);
    }
  }

  if (W->canHaveMap()) {
    MapClause const &MpClause = W->getMap();
    for (MapItem *MapI : MpClause.items()) {
      if (MapI->getIsMapChain()) {
        MapChainTy &MapChain = MapI->getMapChain();
        for (int I = MapChain.size() - 1; I >= 0; --I) {
          MapAggrTy *Aggr = MapChain[I];
          NewV = removeLaunderIntrinsic(Aggr->getSectionPtr(), true);
          Aggr->setSectionPtr(NewV);
          NewV = removeLaunderIntrinsic(Aggr->getBasePtr(), true);
          Aggr->setBasePtr(NewV);
        }
      }
      NewV = removeLaunderIntrinsic(MapI->getOrig(), true);
      MapI->setOrig(NewV);
    }
  }

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

#if 0
// Return original global variable if the value Orig is the return value
// of a fence call.
Value *VPOParoptTransform::getRootValueFromFenceCall(Value *Orig) {
  BitCastInst *BI = dyn_cast<BitCastInst>(Orig);
  if (!BI)
    return Orig;
  Value *V = BI->getOperand(0);
  CallInst *CI = dyn_cast<CallInst>(V);
  if (CI && isFenceCall(CI)) {
    Value *CallOperand = CI->getOperand(0);
    ConstantExpr *Expr = dyn_cast_or_null<ConstantExpr>(CallOperand);
    assert(Expr && "getRootValue: expect non empty constant expression");
    if (Expr->isCast())
      return Expr->getOperand(0);
    return CallOperand;
  }
  return Orig;
}
#endif

/// If the incoming data is global variable, Create the stack variable and
/// replace the the global variable with the stack variable.
///
/// For a global variable reference in an OpenMP target construct, the
/// corresponding target outline function needs to pass the address of the
/// global variable as one of its arguments. The utility CodeExtractor which
/// is used by paropt, does not generate that argument for global variables.
/// In order to help the CodeExtractor to achieve this, we rename the uses of
/// this global variable within the WRegion (including the directive).
/// The outline function will have an entry for the renamed variable. We do the
/// renaming for ConstantExpr operands as well.
///
/// Before:
/// \code
///  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), ... ,
///       "QUAL.OMP.MAP.TOFROM:AGGRHEAD"([10 x i16]* @a,
///       i16* getelementptr inbounds (@a, i64 0, i64 1),
///       i64 2) ]
///  .... ; @a is used inside the region.
/// \endcode
///
/// After:
/// \code
///  %0 = call i8* @llvm.launder.invariant.group.p0i8(
///       i8* bitcast (i16* getelementptr inbounds (@a, i64 0, i64 1) to i8*))
///  %1 = bitcast i8* %0 to i16*
///  %2 = call i8* @llvm.launder.invariant.group.p0i8(
///       i8* bitcast (@a to i8*))
///  %a = bitcast i8* %2 to [10 x i16]*
///
///  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), ... ,
///       "QUAL.OMP.MAP.TOFROM:AGGRHEAD"([10 x i16]* %a,
///       i16* %1,
///       i64 2) ]
/// ... ; %a is used inside the region instead of @a
/// \endcode
///
/// With these changes, CodeExtractor will pass in %a and %1 as parameters of
/// the outlined function. After that, the renaming will be undone using
/// clearLaunderIntrinBeforeRegion().
bool VPOParoptTransform::genGlobalPrivatizationLaunderIntrin(WRegionNode *W) {
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB =
      SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
  W->setEntryBBlock(NewEntryBB);
  bool Changed = false;
  W->populateBBSet(true); // rebuild BBSet unconditionlly as EntryBB changed

  assert(W->canHaveMap() &&
         "Function called for a WRegion that cannot have a Map clause.");

  // For the example in the header comment, this function will create the
  // renamed %1 for the gep on @a, and %a for @a.
  auto createRenamedValueForV = [&](Value *V) {
    IRBuilder<> Builder(EntryBB->getTerminator());
    Value *NewV = Builder.CreateLaunderInvariantGroup(V);
    NewV->setName(V->getName());
    LLVM_DEBUG(dbgs() << "createRenamedValueForV : Created renamed value via "
                         "launder intrinsic: '";
               V->printAsOperand(dbgs()); dbgs() << "'.\n");
    return NewV;
  };

  // Map between Original Value V and the renamed value NewV. If no renaming
  // happens, The map will have {V, V}. Use MapVector so that the replacement
  // happens in the same order as the generation of renamed values.
  MapVector<Value *, Value *> RenameMap;

  // Create a renamed value for V if it's a Global or ConstantExpr.
  auto createRenamedValueForGlobalsAndConstExprs = [&](Value *V) {
    auto VOrigAndNew = RenameMap.find(V);
    if (VOrigAndNew != RenameMap.end())
      return VOrigAndNew->second;

    if (!GeneralUtils::isOMPItemGlobalVAR(V) && !isa<ConstantExpr>(V)) {
      RenameMap.insert({V, V});
      return V;
    }

    Value *VNew = createRenamedValueForV(V);
    RenameMap.insert({V, VNew});
    Changed = true;
    return VNew;
  };

  // For all renamed values created, replace the original value with the renamed
  // value in the region. This will update the region directive in the header
  // example, from %0 to %3, and replace all uses of @a with %a in the region.
  auto replaceRenamedGlobalsAndConstExprs = [&](bool Globals) {
    for (auto &VNewV: RenameMap) {
      Value *V = VNewV.first;
      Value *NewV = VNewV.second;
      if (V == NewV)
        continue;

      if (Globals != GeneralUtils::isOMPItemGlobalVAR(V))
        continue;

      genPrivatizationReplacement(W, V, NewV);
    }
  };

  MapClause &MpClause = W->getMap();
  Value *VNew = nullptr;
  // The capturing also needs to happen for Constant EXPRs in SectionPtrs.
  for (MapItem *MapI : MpClause.items()) {
    if (MapI->getIsMapChain()) {
      MapChainTy &MapChain = MapI->getMapChain();
      // Iterate through a map chain in reverse order. For example,
      // for (p1, p2) (p2, p3), handle (p2, p3) before (p1, p2).
      // We can also have things like (p1, p1) (p1, p2) for cases like:
      //   int (*p1)[10];
      //   ...
      //   ... target map(tofrom:p1[0][1]) ...
      for (int I = MapChain.size() - 1; I >= 0; --I) {
        MapAggrTy *Aggr = MapChain[I];
        VNew = createRenamedValueForGlobalsAndConstExprs(Aggr->getSectionPtr());
        Aggr->setSectionPtr(VNew);
        VNew = createRenamedValueForGlobalsAndConstExprs(Aggr->getBasePtr());
        Aggr->setBasePtr(VNew);
      }
    }

    VNew = createRenamedValueForGlobalsAndConstExprs(MapI->getOrig());
    MapI->setOrig(VNew);
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      VNew = createRenamedValueForGlobalsAndConstExprs(FprivI->getOrig());
      FprivI->setOrig(VNew);
    }
  }

#if INTEL_CUSTOMIZATION
  // We need this for private F90 DVs as well, as we pass in the original
  // DV into the target region as a parameter of the outlined function.
  if (W->canHavePrivate()) {
    PrivateClause &PrivClause = W->getPriv();
    for (PrivateItem *PrivI : PrivClause.items()) {
      if (!PrivI->getIsF90DopeVector())
        continue;
      VNew = createRenamedValueForGlobalsAndConstExprs(PrivI->getOrig());
      PrivI->setOrig(VNew);
    }
  }
#endif // INTEL_CUSTOMIZATION
  if (W->canHaveIsDevicePtr()) {
    IsDevicePtrClause &IsDevPtrClause = W->getIsDevicePtr();
    for (IsDevicePtrItem *Item : IsDevPtrClause.items()) {
      VNew = createRenamedValueForGlobalsAndConstExprs(Item->getOrig());
      Item->setOrig(VNew);
    }
  }

  // Replace all ConstExpressions first, as replacing a global would cause
  // break-expressions to be called, and re-evalutation of existing
  // ConstantExpressions on the global.
  replaceRenamedGlobalsAndConstExprs(false); // Replace non-globals
  replaceRenamedGlobalsAndConstExprs(true);  // Replace globals

  W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
  return Changed;
}

// Return true if the device triple contains spir64 or spir.
bool VPOParoptTransform::deviceTriplesHasSPIRV() {
  for (const auto &T : MT->getDeviceTriples()) {
    if (T.getArch() == Triple::ArchType::spir ||
        T.getArch() == Triple::ArchType::spir64)
      return true;
  }
  return false;
}

bool VPOParoptTransform::moduleHasOmpGetNumThreadsFunction() {
  return F->getParent()->getFunction("omp_get_num_threads");
}

bool VPOParoptTransform::isFunctionOpenMPTargetDeclare() {
  return (F->getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                          "openmp-target-declare"));
}

// Return true if one of the region W's ancestor is OMP target
// construct or the function where W lies in has target declare attribute.
bool VPOParoptTransform::hasParentTarget(WRegionNode *W) {
  if (F->getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                      "target.declare"))
    return true;

  WRegionNode *PW = W->getParent();
  while (PW) {
    if (PW->getIsTarget())
      return true;

    PW = PW->getParent();
  }

  return false;
}

// This function inserts artificial uses for arguments of some clauses
// of the given region.
//
// With O2 clang inserts llvm.lifetime markers, which trigger implicit
// privatization in CodeExtractor.  For example,
//   %a = alloca i8
//   call void @llvm.lifetime.start.p0i8(i64 1, i8* %a)
//   %0 = call token @llvm.directive.region.entry() [
//            "DIR.OMP.TARGET"(), "QUAL.OMP.FIRSTPRIVATE"(i8* %a) ]
//   call void @llvm.directive.region.exit(token %0)
//   call void @llvm.lifetime.end.p0i8(i64 1, i8* %a)
//
// Paropt will copy original value of %a to its private version
// at the beginning of the target region.  The CodeExtractor
// will shrinkwrap %a into the target region, i.e. it will
// move the alloca inside the target region, and %a will not
// be represented as an argument for the outline function.
//
// If llvm.lifetime markers are not used (e.g. at O0), CodeExtractor
// will not shrinkwrap %a into the target region, and it will be
// represented as an argument for the outline function.
//
// If the host and target compilations use different options
// (e.g. "-fopenmp-targets=x86_64=-O0 -O2"), then this will result
// in interface mismatch between the outline functions created
// during the host and target compilation.
//
// We try to block CodeExtractor's implicit privatization by
// inserting artificial uses of %a before the target region.
//
// Note that this problem is specific to "omp target", but
// it exists for any host/target combination.
//
// This problem is only related to clauses that result in
// auto-generation (by Paropt) of new Value references only inside
// the "omp target" region.  For example, array section size
// for map clause will be used outside the target region, so
// CodeExtractor will not be able to shrinkwrap it.
// To summarize, the problem seems to affect only firstprivate clause.
//
// TODO: we probably have to explicitly instruct CodeExtractor
//       not to auto-privatize some variables.  We may collect
//       a set of firstprivate values and pass it to the CodeExtractor.
//
// The "artitifical use" sequence we emit in this function is easily
// optimizable by SROA and CSE, so we do not care about explicitly
// removing these new instructions, when we do not need them any more.
// At O0 the sequence will appear in the generated code.  If this
// ever becomes a problem, we need to find a way to delete these
// artificial uses, generated for "omp parallel for", after
// we outline "omp target".
bool VPOParoptTransform::promoteClauseArgumentUses(WRegionNode *W) {
  assert(isa<WRNTargetNode>(W) &&
         "promoteClauseArgumentUses called for non-target region.");
  assert(W->canHaveFirstprivate() &&
         "promoteClauseArgumentUses: target region "
         "does not support firstprivate clause?");

  bool Changed = false;
  AllocaInst *ArtificialAlloca = nullptr;
  IRBuilder<> Builder(F->getContext());

  auto InsertArtificialUseForValue = [&](Value *V) {
    // Emit a sequence, which is easily optimizable by
    // SROA and CSE:
    //     %promote.uses = alloca i8
    //     store i8 ptrtoint (type* @G to i8), i8* %promote.uses
    //
    // FIXME: to aid further optimization, we have to insert
    //        the alloca either to the entry block of the parent region
    //        that will be later outlined, or to the entry block
    //        of the current Function.  Not all optimizations are able
    //        to handle allocas appearing in the middle of the Function.
    if (!ArtificialAlloca)
      ArtificialAlloca =
        Builder.CreateAlloca(Builder.getInt8Ty(), nullptr,
                             "promoted.clause.args");

    auto *Cast = Builder.CreateBitOrPointerCast(V, Builder.getInt8Ty());
    Builder.CreateStore(Cast, ArtificialAlloca);
    Changed = true;
  };

  auto InsertArtificialUseForItem = [&InsertArtificialUseForValue](Item *I) {
    InsertArtificialUseForValue(I->getOrig());
  };

  // firstprivate() is the only clause handled for "omp target".
  if (W->getFpriv().size() != 0) {
    auto *EntryBB = W->getEntryBBlock();
    auto *NewEntryBB = SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
    W->setEntryBBlock(NewEntryBB);
    Builder.SetInsertPoint(EntryBB->getTerminator());

    std::for_each(W->getFpriv().items().begin(),
                  W->getFpriv().items().end(),
                  InsertArtificialUseForItem);

    W->resetBBSet();
  }

  return Changed;
}

// Find and return the variant function name from the Declare Variant
// information embedded in the "openmp-variant" string attribute of BaseCall.
// The context to match is given by MatchConstruct and MatchArch.
StringRef getVariantName(CallInst *BaseCall, StringRef &MatchConstruct,
                         StringRef &MatchArch) {
  assert(BaseCall && "BaseCall is null");
  Function *BaseFunc = BaseCall->getCalledFunction();

  StringRef VariantAttributeString =
      BaseFunc->getFnAttribute("openmp-variant").getValueAsString();

  if (VariantAttributeString.empty()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Base function "
                      << BaseFunc->getName()
                      << " does not have an openmp variant\n");
    return VariantAttributeString; // null string
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Base function " << BaseFunc->getName()
                    << " has openmp-variant attribute: "
                    << VariantAttributeString << "\n");

  // VariantAttributeString is of the form
  //   <variant>[;;<variant>...]
  // where <variant> is a string of the form
  //   <field>:<value>[;<field>:<value>...]
  // Currently <field> can be "name", "construct", or "arch"
  //
  // An example of VariantAttributeString with only one <variant>:
  //   "name:foo_gpu;construct:target_variant_dispatch;arch:gen"
  //
  // An example of VariantAttributeString with two <variant>s:
  //   "name:foo_gpu;construct:target_variant_dispatch;arch:gen;;
  //    name:foo_xxx;construct:parallel;arch:xxx"
  //
  // We want to find the <variant> whose "construt" field's <value> is
  // MatchConstruct and "arch" field's <value> is MatchArch.
  // If such a <variant> is found, return the string from its "name" field.

  SmallVector<StringRef,1> Variants;   // holds <variant> substrings
  SmallVector<StringRef,3> Fields;     // holds <field>:<value> substrings
  SmallVector<StringRef,2> FV;         // FV[0]= <field>; FV[1]= <value>
  StringRef VariantName;               // string to return
  bool FoundConstruct = false;
  bool FoundArch = false;
  bool FoundName = false;

  // Split VariantAttributeString so that each <variant> substring is
  // separately stored in the Variants vector
  VariantAttributeString.split(Variants, ";;");

  // Inspect each <variant> to find the "construct" and "arch" of interest.
  for (StringRef &Variant : Variants) {

    // LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Variant: " << Variant << "\n");

    // Split Variant so that each <field>:<value> substring is separately
    // stored in the Fields vector
    Fields.clear();
    Variant.split(Fields, ";");

    FoundConstruct = false;
    FoundArch = false;
    FoundName = false;
    for (StringRef &Field : Fields) {

      // LLVM_DEBUG(dbgs() << __FUNCTION__ << ":   Field: " << Field << "\n");

      // Split Field so that FV[0] has <field> and FV[1] has <value>
      FV.clear();
      Field.split(FV, ":");
      assert(FV.size() == 2 &&
             "Malformed <field>:<value> in openmp-variant attribute");
      if (FV[0] == "construct" && FV[1] == MatchConstruct)
        FoundConstruct = true;
      else if (FV[0] == "arch" && FV[1] == MatchArch)
        FoundArch = true;
      else if (FV[0] == "name") {
        VariantName = FV[1];
        FoundName = true;
      }
    } // for (StringRef &Field : Fields)

    if (FoundConstruct && FoundArch) {
      if (FoundName)
        break;
      // found <variant> with matching construct and arch, but without a
      // "name" field. It must be corrupt.
      llvm_unreachable("No variant function name in openmp-variant attribute");
    }
  } // for (StringRef &Variant : Variants)

  if (FoundConstruct && FoundArch && FoundName) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Found variant function: "
                      << VariantName << "\n");
  } else {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Variant function not found\n");
  }
  return VariantName;
}

// To support asynchronous execution for a TARGET VARIANT DISPATCH NOWAIT
// construct, an AsyncObj is created and passed to @__tgt_create_interop_obj().
// Its type is that of a struct of this form:
//
//      struct AsyncObjTy { // struct size = 8+8+4+4 = 24 on 64bit arch
//        void* UDPtrs;     // pointer to a UseDevicePtrsTy struct
//                          // unused when UseRawDevicePtr is set
//        void* task_entry; // unused
//        int   part_id;    // unused
//        int   num_ptrs;   // number of use_device_ptr pointers
//                          // unused when UseRawDevicePtr is set
//      };
//
// TODO : Remove struct UseDevicePtrsTy when UseRawDevicePtr is default
// where the UseDevicePtrsTy is a struct to hold void* pointers corresponding
// to the target buffers created for each pointer. For example, for the
// clause use_device_ptr(a,b,c,d), the structure will look like this:
//
//      struct UseDevicePtrsTy { // struct size = num_ptrs*8 on 64bit arch
//        void* a.buffer;
//        void* b.buffer;
//        void* c.buffer;
//        void* d.buffer;
//      };
//
// The AsyncObjTy is actually a __kmp_task_t struct, and
// the UseDevicePtrsTy corresponds to the "shareds" struct.
//
// The AsyncObj is allocated by calling __kmpc_omp_task_alloc, with
// the proxy flag bit set (WRNTaskFlag::Proxy is 0x10).
static Value *createAsyncObj(WRegionNode *W, Value *DeviceNum,
                             StructType *IdentTy, Instruction *InsertPt) {
  Function *F = InsertPt->getFunction();
  LLVMContext &C = F->getContext();
  const DataLayout DL = F->getParent()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  IntegerType *Int32Ty = Builder.getInt32Ty();
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  Value *Zero = Builder.getInt32(0);
  ConstantPointerNull *NullPtr = ConstantPointerNull::get(Int8PtrTy);

  // Build the struct for AsyncObjTy:
  //
  //   %__struct.AsyncObj = type { i8*, i8*, i32, i32 }

  Type *AsyncObjTyFields[] = {Int8PtrTy, // 0: Pointer to UseDevicePtrsTy struct
                              Int8PtrTy, // 1: unused: task_entry
                              Int32Ty,   // 2: unused: part_id
                              Int32Ty};  // 3: number of ptrs in UseDevicePtrsTy
  StructType *AsyncObjTy =
      StructType::create(C, AsyncObjTyFields, "__struct.AsyncObj", false);
  int AsyncObjTySize = DL.getTypeAllocSize(AsyncObjTy);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": AsyncObjTy: " << *AsyncObjTy
                    << "; Size = " << AsyncObjTySize << " bytes\n");

  // Build the struct for UseDevicePtrsTy if use_device_ptr clause is present.
  // Add one "i8*" field for each item in the clause.
  // Example: for 4 pointers, the struct is:
  //
  //   %__struct.UDPtrs = type { i8*, i8*, i8*, i8* }

  StructType *UseDevicePtrsTy = nullptr;
  int UseDevicePtrsTySize = 0;
  UseDevicePtrClause &UDPtrClause = W->getUseDevicePtr();
  int NumberOfPointers = UseRawDevicePtr ? 0 : UDPtrClause.size();
  if (NumberOfPointers > 0) {
    SmallVector<Type *, 4> StructFields; // {i8*, i8*, i8*, etc.}
    for (int I = 0; I < NumberOfPointers; I++)
      StructFields.push_back(Int8PtrTy);
    UseDevicePtrsTy =
        StructType::create(C, StructFields, "__struct.UDPtrs", false);
    UseDevicePtrsTySize = DL.getTypeAllocSize(UseDevicePtrsTy);
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": UseDevicePtrsTy: " << *UseDevicePtrsTy
                      << "; Size = " << UseDevicePtrsTySize << " bytes\n");
  }

  // Create AsyncObj by calling __kmpc_omp_task_alloc()
  // For the example with 4 use_device_ptr pointers, the call looks like this:
  //
  //   %asyncobj = call i8* @__kmpc_omp_task_alloc(
  //                        %__struct.ident_t* @.kmpc_loc.0.0,
  //                        i32 0,      // unused
  //                        i32 16,     // "proxy" flag 0x10
  //                        i64 24,     // sizeof(AsyncObjTy) = 8+8+4+4 = 24
  //                        i64 32,     // sizeof(UseDevicePtrsTy) = 4*8 = 32
  //                                    // 0 for UseRawDevicePtr
  //                        i8* null)   // unused

  CallInst *AsyncObj = VPOParoptUtils::genKmpcTaskAllocForAsyncObj(
      W, IdentTy, AsyncObjTySize, UseDevicePtrsTySize, InsertPt);
  assert(AsyncObj && "AsyncObj not created for Target Variant Dispatch Nowait");
  AsyncObj->setName("asyncobj"); // void* pointer
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": AsyncObj: " << *AsyncObj << "\n");

  // Create a base pointer to AsyncObj by casting the void* ptr to AsyncObjTy*
  // like this:
  //   %asyncobj.ptr = bitcast i8* %asyncobj to %__struct.AsyncObj*
  Value *AsyncObjPtr = Builder.CreateBitCast( // base pointer to AsyncObjTy
      AsyncObj, PointerType::getUnqual(AsyncObjTy), "asyncobj.ptr");

  // Initialize fields 1,2,3 of AsyncObj. Field 0 is already initialized
  // by the runtime to hold a pointer to UseDevicePtrsTy.

  // Field 1: task_entry pointer is unused. Just init it to null.
  //
  //   %task.entry.gep = getelementptr inbounds %__struct.AsyncObj,
  //                     %__struct.AsyncObj* %asyncobj.ptr, i32 0, i32 1
  //   store i8* null, i8** %task.entry.gep

  Value *TaskEntryGep = Builder.CreateInBoundsGEP(
      AsyncObjTy, AsyncObjPtr, {Zero, Builder.getInt32(1)}, "task.entry.gep");
  Builder.CreateStore(NullPtr, TaskEntryGep);

  // Field 2: part_id is unused. Just init it to 0.
  //
  //   %part.id.gep = getelementptr inbounds %__struct.AsyncObj,
  //                     %__struct.AsyncObj* %asyncobj.ptr, i32 0, i32 2
  //   store i32 0, i32* %part.id.gep

  Value *PartIdGep = Builder.CreateInBoundsGEP(
      AsyncObjTy, AsyncObjPtr, {Zero, Builder.getInt32(2)}, "part.id.gep");
  Builder.CreateStore(Zero, PartIdGep);

  // Field 3: save the number of use_device_ptr pointers here.
  //
  //   %num.ptrs.gep = getelementptr inbounds %__struct.AsyncObj,
  //                     %__struct.AsyncObj* %asyncobj.ptr, i32 0, i32 3
  //   store i32 <NumberOfPointers>, i32* %num.ptrs.gep

  Value *NumPointersGep =
      Builder.CreateInBoundsGEP(AsyncObjTy, AsyncObjPtr,
                                {Zero, Builder.getInt32(3)}, "num.ptrs.gep");
  Builder.CreateStore(Builder.getInt32(NumberOfPointers), NumPointersGep);

  // Save the target buffer for a use_device_ptr pointer in the
  // UseDevicePtrsTy struct.
  if (NumberOfPointers > 0) {

    // Build "UDPtrsPtr" to point to the UseDevicePtrsTy struct.
    // Field 0 of AsyncObj holds the address of the UseDevicePtrsTy struct.
    // Load it and cast it to UseDevicePtrsTy*. The IR looks like this:
    //
    //   %UDPtrs.gep = getelementptr inbounds %__struct.AsyncObj,
    //                   %__struct.AsyncObj* %asyncobj.ptr, i32 0, i32 0
    //   %11 = load i8*, i8** %UDPtrs.gep
    //   %UDPtrs.ptr = bitcast i8* %11 to %__struct.UDPtrs*

    Value *UDPtrsGep = Builder.CreateInBoundsGEP(AsyncObjTy, AsyncObjPtr,
                                                 {Zero, Zero}, "UDPtrs.gep");
    LoadInst *UDPtrsLoad = Builder.CreateLoad(UDPtrsGep);
    Value *UDPtrsPtr = Builder.CreateBitCast(
        UDPtrsLoad, PointerType::getUnqual(UseDevicePtrsTy), "UDPtrs.ptr");

    // Store each target buffer in the UseDevicePtrsTy struct.
    int Index = 0;
    for (UseDevicePtrItem *Item : UDPtrClause.items()) {
      StringRef OrigName = Item->getOrig()->getName();

      // Build GEP for the Index'th field in the UseDevicePtrsTy struct. E.g.,
      //
      //   %aaa.gep = getelementptr inbounds %__struct.UDPtrs,
      //                 %__struct.UDPtrs* %UDPtrs.ptr, i32 0, i32 <Index>

      Value *Gep = Builder.CreateInBoundsGEP(UseDevicePtrsTy, UDPtrsPtr,
                                             {Zero, Builder.getInt32(Index++)},
                                             OrigName + ".gep");
      // Store the buffer in the UseDevicePtrsTy field corresponding to the GEP
      //
      //   %aaa.buffer = load i8*, i8** %aaa.tgt.buffer.addr
      //   store i8* %aaa.buffer, i8** %aaa.gep

      Value *TgtBufferAddr = Item->getNew(); // i8**
      LoadInst *Load =
          Builder.CreateLoad(TgtBufferAddr, OrigName + ".buffer"); // i8*
      Builder.CreateStore(Load, Gep);
    }
  }

  return AsyncObj;
}

// Create an InteropObj that is used for TARGET VARIANT DISPATCH [NOWAIT],
// for both synchronouos and asynchronous modes.
//
// The struct is of this form:
//
//      struct __tgt_interop_obj {
//        int64_t device_id;              // OpenMP device id
//        bool    is_async;               // true for asynchronous
//        void   *async_obj;              // Pointer to the asynchronous object
//        void  (*async_handler)(void*);  // Callback function for asynchronous
//        void   *pipe;       // Opaque handle to device-dependent offload pipe
//      };
//
// If the NOWAIT clause is absent, then execution is synchronous,
// and the fields is_async=false, async_obj=nullptr, async_handler=nullptr
//
// If the NOWAIT clause is present, then execution is asynchronous,
// and the fields is_async=true, and all fields will be populated.
//
// The field values for device_id, is_async, and async_obj are emitted by the
// compiler and passed to the __tgt_create_interop_obj() runtime. The values
// for the async_handler and the pipe are provided by the runtime.
static Value *createInteropObj(WRegionNode *W, Value *DeviceNum,
                               StructType *IdentTy, Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  Value *AsyncObj = nullptr;
  bool IsAsync = W->getNowait(); // true means asynchronous execution

  if (IsAsync)
    AsyncObj = createAsyncObj(W, DeviceNum, IdentTy, InsertPt);

  // Call __tgt_create_interop_obj(device_num, is_async, asyncobj)
  //
  // For is_async==false (no NOWAIT clause) it looks like
  //   call i8* @__tgt_create_interop_obj(i64 0, i8 0, i8* null)
  //
  // For is_async==true it looks like
  //   call i8* @__tgt_create_interop_obj(i64 0, i8 1, i8* %asyncobj)

  CallInst *InteropObj = VPOParoptUtils::genTgtCreateInteropObj(
      DeviceNum, IsAsync, AsyncObj, InsertPt);

  assert(InteropObj && "InteropObj not created for Target Variant Dispatch");

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": InteropObj: " << *InteropObj << "\n");

  return InteropObj;
}

/// Auxiliary function called from genTargetVariantDispatchCode() to
///   (A) Emit calls to __tgt_create_buffer() to create a target buffer
///       for each host pointer
///   (B) Compute the dispatch condition; it is
///          (device.available) && (all tgt buffers successfully created)
///   (C) If (device.available) but some target buffers failed to create (ie,
///       dispatch is false), emit cleanup code with __tgt_release_buffer()
///       calls to free all target buffers that got created.
///
/// Pseudocode:
/// \code
///   bool dispatch = false;                                  (B1)
///   void *a_tgtBuff = nullptr;                              (A1)
///   void *b_tgtBuff = nullptr;                              (A1)
///   if (available) {
///
///     // "if.device.available.create.buffers"
///
///     a_tgtBuff = __tgt_create_buffer(dnum, a);             (A2)
///     if (a_tgtBuff != nullptr) {                           (A2)
///       b_tgtBuff = __tgt_create_buffer(dnum, b);           (A2)
///       if (b_tgtBuff != nullptr)                           (A2)
///         dispatch = true;                                  (B2)
///     }
///
///     // "begin.check.buffer"
///
///     if (dispatch == false) {                              (C)
///       if (a_tgtBuff != nullptr)                           (C)
///         __tgt_release_buffer(dnum, a_tgtBuff);            (C)
///       if (b_tgtBuff != nullptr)                           (C)
///         __tgt_release_buffer(dnum, b_tgtBuff);            (C)
///     }
///
///     // "end.check.buffer"
///
///   }
///   // "end.if.device.available.create.buffers"
/// \endcode
static Value *
createTargetVariantDispatchHostPtrs(WRegionNode *W, Instruction *InsertPt,
                                    Value *DeviceNum, Value *Available,
                                    DominatorTree *DT, LoopInfo *LI) {
  UseDevicePtrClause &UDPtrClause = W->getUseDevicePtr();
  assert(!UDPtrClause.empty() && "Unexpected: no use_device_ptr clause");

  IRBuilder<> Builder(InsertPt);
  IntegerType *Int1Ty = Builder.getInt1Ty();
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  ConstantInt *ValueFalse = Builder.getFalse();
  ConstantInt *ValueTrue = Builder.getTrue();
  ConstantPointerNull *NullPtr = ConstantPointerNull::get(Int8PtrTy);

  Value *DispatchTmp; // the dispatch condition

  // (A1)
  // For each host pointer in the use_device_pointer clause:
  //   Alloc a target buffer pointer (void**)
  //   Initialized it to null
  //   Save it in the Clause Item's "New" field
  for (UseDevicePtrItem *Item : UDPtrClause.items()) {
    StringRef OrigName = Item->getOrig()->getName();
    AllocaInst *TgtBufferAddr =
        Builder.CreateAlloca(Int8PtrTy, nullptr, OrigName + ".tgt.buffer.addr");
    Builder.CreateStore(NullPtr, TgtBufferAddr);
    Item->setNew(TgtBufferAddr);
  }

  // (B1)
  // Initialize dispatch flag to false. Later, set it to true
  // if device is available && all target buffers are created successfully
  AllocaInst *DispatchFlag =
      Builder.CreateAlloca(Int1Ty, nullptr, "dispatch.flag");
  Builder.CreateStore(ValueFalse, DispatchFlag);

  // Split CFG for the if(available) {...} code
  Instruction *AvailableTerm =
      SplitBlockAndInsertIfThen(Available, InsertPt, false, nullptr, DT, LI);
  AvailableTerm->getParent()->setName("if.device.available.create.buffers");

  BasicBlock *BBCheckBuffer = InsertPt->getParent();
  BasicBlock *BBEndIf = SplitBlock(BBCheckBuffer, InsertPt, DT, LI);
  BBCheckBuffer->setName("begin.check.buffer");
  BBEndIf->setName("end.if.device.available.create.buffers");

  // After the previous SplitBlockAndInsertIfThen(Available,...) call,
  // the successor of the False branch is BBCheckBuffer. We need to
  // changed it to BBEndIf because we want to skip the tgtBuffer cleanup
  // code since device is not available.
  BasicBlock *BBIfAvailable = DispatchFlag->getParent();
  Instruction *ITerm = BBIfAvailable->getTerminator();
  assert(isa<BranchInst>(ITerm) && "Expected a branch");
  BranchInst *IfAvailable = cast<BranchInst>(ITerm);
  assert(IfAvailable->isConditional() && "Expected a conditional branch");
  IfAvailable->setSuccessor(1, BBEndIf);

  // CFG so far:
  //   ...
  //   br i1 %available, label %if.device.available.create.buffers,
  //                     label %end.if.device.available.create.buffers
  //
  //   if.device.available.create.buffers:
  //      br label %begin.check.buffer ; <--- AvailableTerm
  //
  //   begin.check.buffer:
  //      br label %end.if.device.available.create.buffers
  //
  //   end.if.device.available.create.buffers:
  //      ; <--- InsertPt
  //   ...

  // (B2)
  // Instruction to set dispatch condition to true
  Builder.SetInsertPoint(AvailableTerm);
  Instruction *InsertBefore = Builder.CreateStore(ValueTrue, DispatchFlag);

  // (A2)
  // Create target buffer for each host pointer
  for (UseDevicePtrItem *Item : UDPtrClause.items()) {
    Builder.SetInsertPoint(InsertBefore);
    Value *Orig = Item->getOrig();
    StringRef OrigName = Orig->getName();
    Instruction *NextInsertBefore = nullptr;
    Value *HostPtr = nullptr;
    if (Item->getIsPointerToPointer()) {
      Type *OrigElemType = Item->getOrigElemType();
      HostPtr = Builder.CreateLoad(OrigElemType, Orig, "hostPtr");
      NextInsertBefore = cast<Instruction>(HostPtr);
    } else
      HostPtr = Orig;

    assert(HostPtr->getType()->isPointerTy() &&
           "Target Variant: Expected a pointer");
    Value *PtrCast = Builder.CreateBitCast(HostPtr, Int8PtrTy);
    if (!NextInsertBefore && isa<Instruction>(PtrCast))
      NextInsertBefore = cast<Instruction>(PtrCast);
    CallInst *BufferCall =
        VPOParoptUtils::genTgtCreateBuffer(DeviceNum, PtrCast, InsertBefore);
    BufferCall->setName(OrigName + ".buffer");
    if (!NextInsertBefore)
      NextInsertBefore = BufferCall;
    assert(BufferCall->getType() == Int8PtrTy &&
           "Expected __tgt_create_buffer() to return a void*");
    Value *TgtBufferAddr = Item->getNew();
    assert(TgtBufferAddr != nullptr && "Target Variant: missing TgtBufferAddr");
    Builder.CreateStore(BufferCall, TgtBufferAddr);

    Value *IsNull = Builder.CreateICmpEQ(BufferCall, NullPtr, "isNull");
    SplitBlockAndInsertIfThen(IsNull, InsertBefore, false, nullptr, DT, LI,
                              BBCheckBuffer);
    InsertBefore->getParent()->setName("if.ptr.not.null");
    InsertBefore = NextInsertBefore;
  }

  // (C)
  // Emit cleanup code
  Instruction *CheckTerm = BBCheckBuffer->getTerminator();
  Builder.SetInsertPoint(CheckTerm);
  DispatchTmp = Builder.CreateLoad(DispatchFlag, "dispatch");
  Value *NotDispatch = Builder.CreateNot(DispatchTmp, "notDispatch");
  Instruction *CleanupTerm =
      SplitBlockAndInsertIfThen(NotDispatch, CheckTerm, false, nullptr, DT, LI);
  for (UseDevicePtrItem *Item : UDPtrClause.items()) {
    // Note: we cannot hoist Builder.SetInsertPoint(CleanupTerm) above the
    // for loop. After SplitBlockAndInsertIfThen() below, CleanupTerm ends
    // up in a new BB, so we need to call Builder.SetInsertPoint(CleanupTerm)
    // inside the loop. If not, the Buffer=Builder.CreateLoad() below, while
    // inserted into the right BB, will have its parent BB set to the wrong
    // one (the old parent of CleanupTerm before the split).
    // At -O2 this isn't a problem (somehow cleaned up) but at -O0 it
    // dies in the verifier.
    StringRef OrigName = Item->getOrig()->getName();
    Builder.SetInsertPoint(CleanupTerm);
    CleanupTerm->getParent()->setName("check.unused.buffer");
    Value *TgtBufferAddr = Item->getNew();
    LoadInst *Buffer = Builder.CreateLoad(TgtBufferAddr, OrigName + ".buffer");
    Value *NotNull = Builder.CreateICmpNE(Buffer, NullPtr, "notNull");
    Instruction *FreeTerm =
        SplitBlockAndInsertIfThen(NotNull, CleanupTerm, false, nullptr, DT, LI);
    FreeTerm->getParent()->setName("free.unused.buffer");
    VPOParoptUtils::genTgtReleaseBuffer(DeviceNum, Buffer, FreeTerm);
  }
  CleanupTerm->getParent()->setName("end.check.unused.buffer");
  CheckTerm->getParent()->setName("end.check.buffer");
  Builder.SetInsertPoint(InsertPt);
  DispatchTmp = Builder.CreateLoad(DispatchFlag, "dispatch");
  return DispatchTmp;
}

/// Gen code for the target variant dispatch construct
///
/// Case 1. No use_device_ptr clause:
/// =================================
/// \code
///   #pragma omp target variant dispatch [device(dnum)]
///      foo(<args>);
/// \endcode
///
/// If the device clause is absent, the default dnum is -1. This tells
/// the runtime to use the default device.
///
/// The variant version of the base function "foo" is assumed to have been
/// specified in a declare variant construct which Clang has processed and
/// saved the information in the string attribute "openmp-variant" of foo.
///
/// The dispatch condition is simply a check for the device being available.
/// If true, call the variant function; else call the original base function.
///
/// Pseudocode of the codegen:
/// \code
///   bool dispatch = __tgt_is_device_available(dnum, nullptr)
///   if (dispatch == true)
///      foo_variant(<args>);
///   else
///      foo(<args>);
/// \endcode
///
/// IR:
///    %0 = load i32, i32* @dnum, align 4
///    %1 = sext i32 %0 to i64
///    %call = call i32 @__tgt_is_device_available(i64 %0, i8* null)       (1)
///    %dispatch = icmp ne i32 %call1, 0                                   (2)
///    br i1 %dispatch, label %variant.call, label %base.call              (3)
///
///  variant.call:
///    %variant = call i32 @foo_gpu(<args>)                                (4)
///    br label %if.end
///
///  base.call:
///    %call = call i32 @foo(<args>)                                       (5)
///    br label %if.end
///
///  if.end:
///    %callphi = phi i32 [%variant, %variant.call], [%call, %base.call]   (6)
///    ; replace all other uses of %call with %callphi
///
///
/// Case 2. With use_device_ptr clause:
/// ===================================
/// \code
///   #pragma omp target variant dispatch [device(dnum)] use_device_ptr(a,b)
///      foo(a, b);
/// \endcode
///
/// The list items (a,b above) in the use_device_ptr clause are host pointers.
///
/// Old Scheme uses opencl buffers.  New scheme does not create buffers
/// To use new scheme UseRawDevicePtr flag should be true.
/// At some point old Scheme will be delete.
///
/// The compiler emits extra code (on top of Case1) as follows:
///
///   (A) Emit calls to __tgt_create_buffer() to create a target buffer
///       for each host pointer
///   (B) Compute the dispatch condition; it is
///          (device.available) && (all tgt buffers successfully created)
///   (C) If (device.available) but some target buffers failed to create (ie,
///       dispatch is false), emit cleanup code with __tgt_release_buffer()
///       calls to free all target buffers that got created.
///   (D) Replace args in the foo_variant() call such that each arg that is a
///       load from a host pointer becomes a load from the target buffer ptr
///   (E) Emit calls to __tgt_release_buffer() after returning from
///       the foo_variant() call
///
/// The pseudocode looks like this:
/// \code
///   bool available = __tgt_is_device_available(dnum, ...)   (1)
///   bool dispatch = false;                                  (B)
///   void *a_tgtBuff = nullptr;                              (A)
///   void *b_tgtBuff = nullptr;                              (A)
///   if (available) {
///     a_tgtBuff = __tgt_create_buffer(dnum, a);             (A)
///     if (a_tgtBuff != nullptr) {                           (A)
///       b_tgtBuff = __tgt_create_buffer(dnum, b);           (A)
///       if (b_tgtBuff != nullptr)                           (A)
///         dispatch = true;                                  (B)
///     }
///     if (dispatch == false) {                              (C)
///       if (a_tgtBuff != nullptr)                           (C)
///         __tgt_release_buffer(dnum, a_tgtBuff);            (C)
///       if (b_tgtBuff != nullptr)                           (C)
///         __tgt_release_buffer(dnum, b_tgtBuff);            (C)
///     }
///   }
///   if (dispatch == true) {
///      foo_gpu(a_tgtBuff, b_tgtBuff);                       (D)
///      __tgt_release_buffer(dnum, a_tgtBuff);               (E)
///      __tgt_release_buffer(dnum, b_tgtBuff);               (E)
///   }
///   else
///     foo(a, b);
/// \endcode
///
/// Tasks (A,B,C) are done in createTargetVariantDispatchHostPtrs()
/// Task (D) is done in VPOParoptUtils::genVariantCall()
/// Task (E) is done here in VPOParoptTransform::genTargetVariantDispatchCode()
///
/// New Scheme when UseRawDevicePointer is true.
/// Will replace the above old scheme in the future.
///
///   bool available = __tgt_is_device_available(dnum, ...)
///
///    call void @__tgt_target_data_begin(i64 -1, i32 1, i8** %5,
///      i8** %6, i64* getelementptr inbounds ([1 x i64],
///      [1 x i64]* @.offload_sizes, i32 0, i32 0),
///      i64* getelementptr inbounds ([1 x i64],
///      [1 x i64]* @.offload_maptypes, i32 0, i32 0))
///   foo_gpu(a_tgtBuff, b_tgtBuff);
///   call void @__tgt_target_data_end()
bool VPOParoptTransform::genTargetVariantDispatchCode(WRegionNode *W) {
  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genTargetVariantDispatchCode\n");
  W->populateBBSet();

  // The first and last BasicBlocks contain the region.entry/exit calls.
  // The first call instruction found in the remaining BBs is the
  // base function call. All other instructions in the region are ignored.

  CallInst *BaseCall = nullptr;
  for (auto *BB : make_range(W->bbset_begin()+1, W->bbset_end()-1)) {
    for (Instruction &I : *BB) {
      if ((BaseCall = dyn_cast<CallInst>(&I)) != nullptr)
        break;
    }
    if (BaseCall)
      break;
  }

  assert(BaseCall && "Base call not found in Target Variant Dispatch");
  if (!BaseCall)
    return false;

  // Find the variant name from BaseCall's attributes, which is expected to
  // contain a string attribute of this form:
  // "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen"
  // where the variant name is "foo_gpu" in this example.
  StringRef MatchConstruct("target_variant_dispatch");
  StringRef MatchArch("gen");
  StringRef VariantName = getVariantName(BaseCall, MatchConstruct, MatchArch);

  if (VariantName.empty()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Variant function not found\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Found variant function name: "
                    << VariantName << "\n");

  // Initialize types and constants
  Instruction *InsertPt = BaseCall;
  IRBuilder<> Builder(InsertPt);
  IntegerType *Int32Ty = Builder.getInt32Ty();
  IntegerType *Int64Ty = Builder.getInt64Ty();
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  ConstantInt *ValueZero = ConstantInt::get(Int32Ty, 0);
  ConstantPointerNull *NullPtr = ConstantPointerNull::get(Int8PtrTy);

  // Default device num is -1
  Value *DeviceNum = W->getDevice();
  if (DeviceNum == nullptr) {
    DeviceNum = ConstantInt::get(Int64Ty, -1);
  }

  // Emit call to check for device availability:
  //
  //   %call = call i32 @__tgt_is_device_available(i64 %0, i8* null)      (1)
  //   %available  = icmp ne i32 %call, 0                                 (2)
  //
  // The second argument of __tgt_is_device_available() is a pointer
  // that is currently unused. When we support device types in the
  // future, it will point to a struct holding device-type information.
  Value *DeviceType = NullPtr;
  CallInst *IsDeviceAvailable =
      VPOParoptUtils::genTgtIsDeviceAvailable(DeviceNum, DeviceType, InsertPt);
  IsDeviceAvailable->setName("call");
  Value *Available =
      Builder.CreateICmpNE(IsDeviceAvailable, ValueZero, "available");

  Value *DispatchTmp; // the dispatch condition

  UseDevicePtrClause &UDPtrClause = W->getUseDevicePtr();
  // NOTE : Remove after switching to UseDevicePtr
  if (!UseRawDevicePtr && !UDPtrClause.empty()) {
    // A use_device_ptr clause is present. Therefore:
    //   (A) Create the target buffers
    //   (C) Free all target buffers if some failed to create
    //   (B) return the dispatch condition
    DispatchTmp = createTargetVariantDispatchHostPtrs(W, InsertPt,
                                          DeviceNum, Available, DT, LI);
  } else {
    DispatchTmp = Available;
    DispatchTmp->setName("dispatch");
  }

  // Here, Builder insertion point is "InsertPt"
  // Emit dispatch code:
  //
  //   br i1 %dispatch, label %variant.call, label %base.call               (3)
  //
  // variant.call:
  //   %variant = call i32 @foo_gpu(<args>)                               (4,D)
  //   release buffer is in old scheme only
  //   [calls to __tgt_release_buffer if use_device_ptr exists]             (E)
  //   br label %if.end                                              (ThenTerm)
  //
  // base.call:
  //   %call = call i32 @foo(<args>)                                        (5)
  //   br label %if.end                                              (ElseTerm)
  //
  // if.end:
  //   %callphi = phi i32 [%variant, %variant.call], [%call, %base.call]    (6)

  Instruction *ThenTerm, *ElseTerm;
  VPOParoptUtils::buildCFGForIfClause(DispatchTmp, ThenTerm, ElseTerm, InsertPt,
                                      DT); // (3)

  // Create the interop object for
  // (1) Asynchronous case (i.e., NOWAIT is present), or
  // (2) Synchronous case when "UseInterop" flag is true.
  //     If the flag is false, then the synchronous case will revert to
  //     the old implementation without using interop_obj.
  //     TODO: remove the old implementation when the new one if fully tested.
  Value *InteropObj = nullptr;
  if (UseInterop || W->getNowait())
    InteropObj = createInteropObj(W, DeviceNum, IdentTy, ThenTerm);

  // Create and insert Variant call before ThenTerm
  ThenTerm->getParent()->setName("variant.call");
  bool IsVoidType = (BaseCall->getType() == Builder.getVoidTy());
  CallInst *VariantCall = nullptr;
  if (UseRawDevicePtr && !UDPtrClause.empty()) {
    Builder.SetInsertPoint(ThenTerm);
    TgDataInfo Info;
    Info.NumberOfPtrs = UDPtrClause.size();
    bool hasRuntimeEvaluationCaptureSize = false;
    SmallVector<Constant *, 16> ConstSizes;
    SmallVector<uint64_t, 16> MapTypes;

    (void) addMapForUseDevicePtr(W, ThenTerm);

    genTgtInformationForPtrs(W, nullptr, ConstSizes, MapTypes,
                                 hasRuntimeEvaluationCaptureSize);

    InsertPt = ThenTerm;
    CallInst *DummyCall = nullptr;
    genOffloadArraysInit(W, &Info, DummyCall, InsertPt, ConstSizes,
                        MapTypes, hasRuntimeEvaluationCaptureSize);

    genOffloadArraysArgument(&Info, InsertPt);
    (void) VPOParoptUtils::genTgtTargetDataBegin(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
    for (UseDevicePtrItem *Item : UDPtrClause.items()) {
      MapItem *MapI = Item->getInMap();
      assert(MapI && "No map found for use-device-ptr item.");
      Instruction *BasePtrGEP = MapI->getBasePtrGEPForOrig();
      assert(BasePtrGEP && "No GEP found for base-ptr of Map item's orig.");
      Value *OrigV = Item->getOrig();
      Value *GepCast = Builder.CreateBitOrPointerCast(
        BasePtrGEP, MapI->getOrig()->getType()->getPointerTo(),
        OrigV->getName() + ".cast");
      Item->setNew(GepCast);
    }
    VariantCall =
            VPOParoptUtils::genVariantCall(BaseCall, VariantName,
                                           InteropObj, InsertPt, W);
    VPOParoptUtils::genTgtTargetDataEnd(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, InsertPt);
  }
  else
      VariantCall =
            VPOParoptUtils::genVariantCall(BaseCall, VariantName,
                                           InteropObj, ThenTerm, W);  // (4,D)
  if (!IsVoidType)
    VariantCall->setName("variant");

  if (!UseRawDevicePtr) // should be removed later
    // Release target buffers after Variant call only for synchronous cases;
    // i.e., when NOWAIT is false. For async cases they are released by the
    // async_handler callback routine.
    if (!UDPtrClause.empty() && W->getNowait() == false) {
      Builder.SetInsertPoint(ThenTerm);
      for (UseDevicePtrItem *Item : UDPtrClause.items()) {
        StringRef OrigName = Item->getOrig()->getName();
        Value *TgtBufferAddr = Item->getNew();
        LoadInst *Buffer =
            Builder.CreateLoad(TgtBufferAddr, OrigName + ".buffer");
        VPOParoptUtils::genTgtReleaseBuffer(DeviceNum, Buffer, ThenTerm); // (E)
      }
    }

  // Release the interop object for synchronous execution (no NOWAIT clause).
  // Don't do this for the asynchronous case; the async_handler will do it.
  if (InteropObj != nullptr && W->getNowait() == false)
    VPOParoptUtils::genTgtReleaseInteropObj(InteropObj, ThenTerm);

  // Move BaseCall to before ElseTerm
  ElseTerm->getParent()->setName("base.call");
  InsertPt = BaseCall->getNextNode(); // insert PHI before this point later
  assert(InsertPt && "Corrupt IR: BaseCall cannot be last instruction in BB");
  BaseCall->moveBefore(ElseTerm);                                        // (5)

  // If BaseCall has users, then insert a PHI before InsertPt
  // and replace all uses of BaseCall with PHI
  if (BaseCall->getNumUses() > 0) {
    Builder.SetInsertPoint(InsertPt);
    PHINode *Phi = Builder.CreatePHI(BaseCall->getType(), 2, "callphi"); // (6)
    Phi->addIncoming(VariantCall, ThenTerm->getParent());
    Phi->addIncoming(BaseCall, ElseTerm->getParent());
    for (User *U : BaseCall->users())
      if (Instruction *UI = dyn_cast<Instruction>(U))
        if (UI != Phi) // don't replace in Phi
          UI->replaceUsesOfWith(BaseCall, Phi);
  }

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genTargetVariantDispatchCode\n");

  W->resetBBSet(); // Invalidate BBSet after transformations
  return true;
}

// Set SIMD widening width for the target region based
// on simdlen() clauses of the enclosed SIMD loops (if any).
void VPOParoptTransform::propagateSPIRVSIMDWidth() const {
  for (auto *WT : WRegionList) {
    if (!isa<WRNTargetNode>(WT))
      continue;

    if (FixedSIMDWidth > 0) {
      // Override the SIMD width with an option.
      WT->setSPIRVSIMDWidth(FixedSIMDWidth);
      continue;
    }

    // Choose minimum SIMD width, if there are multiple SIMD loops.
    unsigned MinSIMDLen = 0;
    SmallVector<WRegionNode*, 32> WorkList{WT};
    while (!WorkList.empty()) {
      unsigned CurSIMDLen = 0;
      auto *W = WorkList.pop_back_val();

      if (W != WT && isa<WRNTargetNode>(W))
        // If this is an enclosed target region, then
        // we have already processed it and we can take its
        // SIMD width without processing the children.
        CurSIMDLen = W->getSPIRVSIMDWidth();
      else if (!isa<WRNVecLoopNode>(W)) {
        WorkList.insert(
            WorkList.end(), W->wrn_child_begin(), W->wrn_child_end());
        continue;
      } else
        CurSIMDLen = W->getSimdlen();

      if (CurSIMDLen == 0 ||
          // Ignore unsupported SIMD widths.
          (CurSIMDLen != 8 && CurSIMDLen != 16 && CurSIMDLen != 32))
        continue;
      if (MinSIMDLen == 0 ||
          MinSIMDLen > CurSIMDLen)
        MinSIMDLen = CurSIMDLen;
    }

    if (MinSIMDLen != 0)
      WT->setSPIRVSIMDWidth(MinSIMDLen);
  }
}
#endif // INTEL_COLLAB
