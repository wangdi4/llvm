#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CaptureTracking.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/PhiValues.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Analysis/VPO/VPOParoptConstants.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"

#include <algorithm>
#include <optional>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target"

// TODO: The following flag is temporary to support MKL to transiton from
// old implementation of target variant dispatch to new implementation.
// vpo-paropt-use-interop is used to create interop object for synchronous case
#if INTEL_CUSTOMIZATION
// 20200520: Enabled vpo-paropt-use-interop by default as requested by A21.
#endif // INTEL_CUSTOMIZATION
static cl::opt<bool>
    UseInterop("vpo-paropt-use-interop", cl::Hidden,
               cl::init(true),
               cl::desc("Use the interop_obj for target variant dispatch."));

// This flag controls various codegen versions for the dispatch construct.
// Keeping the old codegen is useful for debugging. This is important as
// the work is in progress and we expect future change(s) to dispatch codegen.
//   Version 0: original implementation.
//               Calls __tgt_create_interop_obj() to create interop objs;
//                 this API does not support prefer_type in append_args.
//               Calls __tgt_use_interop() for #pragma omp interop use.
//   Version 1(default):
//               Calls __tgt_get_interop_obj() to create interop objs;
//                 this API supports prefer_type in append_args.
//               Calls __tgt_interop_use_async() for #pragma omp interop use.
static cl::opt<uint32_t>
    DispatchCodegenVersion("vpo-paropt-dispatch-codegen-version", cl::Hidden,
                           cl::init(1),
                           cl::desc("Codegen version for dispatch construct."));

static cl::opt<bool> SimulateGetNumThreadsInTarget(
    "vpo-paropt-simulate-get-num-threads-in-target", cl::Hidden, cl::init(true),
    cl::desc("Simulate support for omp_get_num_threads in OpenMP target "
             "region. (This may have performance impact)."));

static cl::opt<bool> FrugalNumThreadsSimulation(
    "vpo-paropt-simulate-get-num-threads-frugally", cl::Hidden, cl::init(true),
    cl::desc("Try to limit code inserted to enable the simulated support for "
             "omp_get_num_threads in target regions, when sure that a region "
             "cannot call omp_get_num_threads."));

cl::opt<bool> llvm::vpo::UseMapperAPI(
    "vpo-paropt-use-mapper-api", cl::Hidden, cl::init(true),
    cl::desc("Emit calls to mapper specific functions in tgt RTL."));

static cl::opt<bool> ForceMemberofToPointToBase(
    "vpo-paropt-force-member-of-to-point-to-base", cl::Hidden, cl::init(false),
    cl::desc(
        "Force all member-of maps to point to the base of the map-chain."));

#if INTEL_CUSTOMIZATION
// Controls adding noalias attribute to outlined target function arguments.
static cl::opt<bool>
    EnableTargetArgsNoAlias("vpo-paropt-enable-target-args-noalias", cl::Hidden,
                            cl::init(true), cl::ZeroOrMore,
                            cl::desc("Enable adding noalias attribute to "
                                     "outlined target function arguments"));
#endif // INTEL_CUSTOMIZATION

cl::opt<uint32_t> AtomicFreeRedGlobalBufSize(
    "vpo-paropt-atomic-free-red-global-buf-size", cl::Hidden, cl::init(1024),
    cl::desc("Maximum number of elements (and teams) in the global buffer for "
             "atomic-free reduction"));
cl::opt<uint32_t> AtomicFreeRedLocalBufSize(
    "vpo-paropt-atomic-free-red-local-buf-size", cl::Hidden, cl::init(1024),
    cl::desc("Maximum number of elements (and hence workitems) in the local "
             "buffer used for tree-like local update in"
             "atomic-free reduction"));

cl::opt<bool> AtomicFreeReductionDynamicBuffer(
    "vpo-paropt-atomic-free-reduction-dyn-buffer", cl::Hidden, cl::init(true),
    cl::desc("Enable RT-managed number-of-teams adjusted reduction buffer "
             "allocation"));

extern cl::opt<bool> AtomicFreeReductionUseSLM;
extern cl::opt<bool> AtomicFreeRedUseFPTeamsCounter;

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
  SmallPtrSet<Value*, 8> MapOrigs;
  llvm::transform(MpClause.items(), std::inserter(MapOrigs, MapOrigs.end()),
                  [](Item *I) { return I->getOrig(); });

  // Replace V with null in the clause list, unless it's an orig in a map. e.g.
  //   "MAP(%v, %x, ...), "MAP"(%y, %v, ...)
  auto resetValueInClausesIfNotSeenAsMapOrig = [&](Value *V) {
    if (!MapOrigs.contains(V))
      resetValueInOmpClauseGeneric(W, V);
  };

  for (auto *Item : MpClause.items()) {
    if (!Item->getIsMapChain())
      continue;
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
      resetValueInClausesIfNotSeenAsMapOrig(SectionPtr);
      // If BasePtr of the Aggr is not same as Orig, then we don't want it
      // inside the outlined function. e.g. %y in the following:
      //   "DIR.OMP.TARGET" [ "QUAL.OMP.MAP"(%x, ...) "MAP:CHAIN"(%y, ...) ]
      resetValueInClausesIfNotSeenAsMapOrig(BasePtr);
      Value *Size = Aggr->getSize();
      if (!isa<ConstantInt>(Size))
        resetValueInClausesIfNotSeenAsMapOrig(Size);
    }
  }
}

// Replace printf() calls in F with _Z18__spirv_ocl_printfPU3AS2cz()
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
      SmallVector<Value *, 4> FnArgs(OldCall->args());

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
      //   call i32 (i8 addrspace(2)*, ...) @_Z18__spirv_ocl_printfPU3AS2cz(
      //       i8 addrspace(2)* getelementptr inbounds
      //       ([25 x i8], [25 x i8] addrspace(2)* @.str.as2, i64 0, i64 0)
      //       , ...)
      // For opaque pointers, the string operand does not have a zero-offset
      // GEP, and the printf call looks like:
      //   call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4)
      //   addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), ...)
      Type* FirstParmTy = FnArgs[0]->getType();
      assert(isa<PointerType>(FirstParmTy) &&
             "First argument to printf should be a pointer.");

      if (FirstParmTy->getPointerAddressSpace() !=
          vpo::ADDRESS_SPACE_CONSTANT) {
        IRBuilder<> Builder(OldCall);

        LLVM_DEBUG(dbgs() << "Original argument 0 of printf: " << *FnArgs[0]
                          << "\n");
        auto V = FnArgs[0];
        assert(isa<Constant>(V) && "Only constant format string in argument"
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
        assert((Indices.empty() || Indices.size() == 2) &&
               "Expected 0 or 1 GetElementPtr in constant expression!");

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
        FnArgs[0] = Indices.empty() ? NewArg0
                                    : Builder.CreateInBoundsGEP(
                                          NewArg0->getValueType(), NewArg0,
                                          Indices, NewArg0->getName() + ".gep");
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

// Add "paropt_guarded_by_thread_check" metadata to the Instruction to mark that
// it is already guarded by a check to ensure that only one thread executes it.
void VPOParoptTransform::markAsGuardedByThreadCheck(Instruction *I) {
  LLVMContext &C = I->getContext();
  Constant *One = ConstantInt::get(Type::getInt32Ty(C), 1);
  I->setMetadata(GuardedByThreadCheckMDStr,
                 MDNode::get(C, ConstantAsMetadata::get(One)));
}

bool VPOParoptTransform::callBeginEndSpmdParallelAtRegionBoundary(
    WRegionNode *W) {

  if (!SimulateGetNumThreadsInTarget || !mayCallOmpGetNumThreads(W))
    return false;

  assert(W && "WRegionNode is null.");

  Instruction *EntryDir = W->getEntryDirective();
  assert(EntryDir && "Null Entry directive.");

  CallInst *BeginCall, *EndCall;
  std::tie(BeginCall, EndCall) =
      VPOParoptUtils::genKmpcBeginEndSpmdParallelCalls(EntryDir->getModule());

  VPOParoptUtils::insertCallsAtRegionBoundary(W, BeginCall, EndCall,
                                              /*InsideRegion=*/false);
  VPOParoptUtils::addFuncletOperandBundle(BeginCall, W->getDT());
  VPOParoptUtils::addFuncletOperandBundle(EndCall, W->getDT());
  return true;
}

bool VPOParoptTransform::callBeginEndSpmdTargetAtRegionBoundary(
    WRegionNode *W) {
  assert(W && "WRegionNode is null.");

  if (!SimulateGetNumThreadsInTarget || !mayCallOmpGetNumThreads(W))
    return false;

  Instruction *EntryDir = W->getEntryDirective();
  assert(EntryDir && "Null Entry directive.");

  CallInst *BeginCall, *EndCall;
  std::tie(BeginCall, EndCall) =
      VPOParoptUtils::genKmpcBeginEndSpmdTargetCalls(EntryDir->getModule());

  VPOParoptUtils::insertCallsAtRegionBoundary(W, BeginCall, EndCall,
                                              /*InsideRegion=*/true);
  VPOParoptUtils::addFuncletOperandBundle(BeginCall, W->getDT());
  VPOParoptUtils::addFuncletOperandBundle(EndCall, W->getDT());
  return true;
}

void VPOParoptTransform::renameDuplicateBasesInMapClauses(WRegionNode *W) {
  if (!W->canHaveMap())
    return;

  SmallPtrSet<Value *, 16> Bases;
  BasicBlock *CopyBlock = nullptr;

  auto RenameBase = [this, W, &Bases, &CopyBlock](MapItem *Item, Use &Base) {
    Value *Orig = Item->getOrig();
    if (isa<Constant>(Orig))
      return;

    auto Res = Bases.insert(Orig);
    if (Res.second)
      return;

    // We have already seen this base in the map clauses, so it has to be
    // renamed. Create new empty block right before the entry block for new
    // instructions.
    if (!CopyBlock) {
      CopyBlock = W->getEntryBBlock();
      W->setEntryBBlock(SplitBlock(CopyBlock, &CopyBlock->front(), DT, LI));
      W->populateBBSet(true);
    }

    // Add noop cast that renames the value.
    auto *Copy = CastInst::CreateBitOrPointerCast(Orig, Orig->getType(),
                                                  Orig->getName() + ".copy",
                                                  CopyBlock->getTerminator());

    LLVM_DEBUG(dbgs() << "renameDuplicateBasesInMapClauses: Renamed map item's "
                         "base from '";
               Orig->printAsOperand(dbgs(), false); dbgs() << "' to '";
               Copy->printAsOperand(dbgs(), false); dbgs() << "'.");
    Item->setOrig(Copy);
    Base.set(Copy);
  };

  MapClause &Map = W->getMap();
  auto MapIt = Map.begin();

  auto *EntryDir = cast<IntrinsicInst>(W->getEntryDirective());
  Use *EntryOps = EntryDir->getOperandList();

  // Remove duplicate bases in directive's map bundles.
  for (auto &BOI : make_range(std::next(EntryDir->bundle_op_info_begin()),
                              EntryDir->bundle_op_info_end())) {
    // Get clause ID and check if this is the clause of interest.
    ClauseSpecifier CS(BOI.Tag->getKey());
    if (!VPOAnalysisUtils::isMapClause(CS.getId()))
      continue;

    // Bundle operands.
    MutableArrayRef<Use> Args(EntryOps + BOI.Begin, EntryOps + BOI.End);

    // The code below replicates the logic of creating map items from the
    // parsing routine WRegionNode::extractMapOpndList().
    if (CS.getIsArraySection()) {
      // TODO: this needs to be cleaned up, after the parsing
      //       code is removed from WRegion analysis code.
      llvm_unreachable("Paropt only supports map chains now.");
    } else if (CS.getIsMapAggrHead() || CS.getIsMapAggr() ||
               ((Args.size() == 4 || Args.size() == 6) &&
                isa<ConstantInt>(Args[3]))) {
      bool AggrStartsNewStyleMapChain =
          (!CS.getIsMapChainLink() && !CS.getIsMapAggrHead() &&
           !CS.getIsMapAggr() &&
           (Args.size() == 4 || Args.size() == 6));

      if (CS.getIsMapAggrHead() || AggrStartsNewStyleMapChain) {
        // This bundle starts a new chain.
        assert(MapIt != Map.end());
        RenameBase(*MapIt++, Args[0]);
      }
    } else
      // TODO: Remove this loop and add an assertion that non-chain maps should
      // each have their own clause string.
      // Scalar map items. Each of them has its own MapItem.
      for (unsigned I = 0; I < Args.size(); ++I) {
        assert(MapIt != Map.end());
        RenameBase(*MapIt++, Args[I]);
      }
  }
}

// Generate the code for the directive omp target
bool VPOParoptTransform::genTargetOffloadingCode(WRegionNode *W) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTargetOffloadingCode\n");

  W->populateBBSet();

  resetValueInOmpClauseGeneric(W, W->getIf());
  resetValueInOmpClauseGeneric(W, W->getDevice());
  resetValueInSubdeviceClause(W);
  resetValueInPrivateClause(W);
  resetValueInLiveinClause(W);
  resetValueInMapClause(W);
  if (isa<WRNTargetNode>(W))
    resetValueInOmpClauseGeneric(W, W->getThreadLimit());

  renameDuplicateBasesInMapClauses(W);

  // Outlining is not needed for target enter/exit data, target update.
  bool GenOutlinedFunction = W->needsOutlining();

  // Set up Fn Attr for the new function
  Function *NewF = nullptr;
  CallInst *NewCall = nullptr;
  if (GenOutlinedFunction) {
    NewF = VPOParoptUtils::genOutlineFunction(*W, DT, AC);

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
    NewCall = cast<CallInst>(NewF->user_back());
#if INTEL_CUSTOMIZATION

    if (EnableTargetArgsNoAlias) {
      // Add noalias attribute to outlined function's pointer arguments. It
      // should be safe to do it if actual value that is passed to the outlined
      // region
      // - is function local object that does not alias with any other object
      // - is not captured before the call
      // - does not alias with any other actual argument
      SmallVector<Argument *, 16u> PtrArgs;
      for (Argument &A : NewF->args())
        if (isa<PointerType>(A.getType()))
          PtrArgs.push_back(&A);
      for (Argument *Arg : PtrArgs) {
        Value *Ptr = NewCall->getArgOperand(Arg->getArgNo());

        if (isIdentifiedFunctionLocal(Ptr->stripPointerCasts()) &&
            !PointerMayBeCapturedBefore(Ptr, /*ReturnCaptures=*/true,
                                        /*StoreCaptures=*/true, NewCall, DT) &&
            none_of(PtrArgs, [this, Arg, Ptr, NewCall](const Argument *A) {
              return A != Arg &&
                     !AA->isNoAlias(Ptr, NewCall->getArgOperand(A->getArgNo()));
            }))
          Arg->addAttr(Attribute::NoAlias);
      }
    }
#endif // INTEL_CUSTOMIZATION
  }

  Constant *RegionId = nullptr;
  if (auto *WT = dyn_cast<WRNTargetNode>(W)) {
    assert(MT && "target region with no module transform");
    assert(NewF && "target region was not outlined");
    RegionId = MT->registerTargetRegion(W, NewF);

    // Use weak linkage for x86_64 device compilation, which is needed for NewF
    // to be visible to the runtime in some cases. For spir64, it is done in
    // finalizeKernelFunction.
    if (hasOffloadCompilation() &&
        Triple(NewF->getParent()->getTargetTriple()).isX86())
      NewF->setLinkage(GlobalValue::WeakODRLinkage);

    // Please note that the name of NewF is updated in the
    // function registerTargetRegion.
    if (isTargetSPIRV())
      genTargetSPIRVOffloadingCode(WT, NewF, NewCall);
  }

  if (hasOffloadCompilation())
    // Everything below only makes sense on the host.
    return true;

  // Insert the .run_host_version alloca in the entry block of the first parent
  // region what would be outlined, if any, otherwise of the parent function.
  Instruction *AllocaInsertPt =
      VPOParoptUtils::getInsertionPtForAllocas(W, F, /*OutsideRegion=*/true);
  IRBuilder<> Builder(AllocaInsertPt);
  AllocaInst *OffloadError = Builder.CreateAlloca(
      Type::getInt32Ty(F->getContext()), nullptr, ".run_host_version");

  Value *VIf = W->getIf();
  CallInst *Call;
  Instruction *APIInsertPt = nullptr;
  if (GenOutlinedFunction) {
    APIInsertPt = NewCall;
  } else {
    // If no outlining is done (target enter/exit data and target update),
    // we create an empty BB inside the region, and use that for inserting the
    // code generated, like the tgt_target_... API calls etc.
    Instruction *ExitDirective = W->getExitDirective();
    BasicBlock *OldExitBB = ExitDirective->getParent();
    BasicBlock *NewExitBB = SplitBlock(OldExitBB, ExitDirective, DT, LI);
    W->setExitBBlock(NewExitBB);
    APIInsertPt = OldExitBB->getFirstNonPHI();
  }

  if (VIf) {
    // If the target construct has if clause, the compiler will generate a
    // if-then-else statement.
    //
    // Example:
    //   #pragma omp target enter data map(to: arg) if(arg)
    //
    // *** IR Dump After VPO Paropt Pass ***
    // entry:
    //   %.run_host_version = alloca i32
    //   ...
    //   %arg.addr = alloca i32, align 4
    //   store i32 %arg, i32* %arg.addr, align 4, !tbaa !2
    //   %tobool = icmp ne i32 %arg, 0
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
    Instruction *IfInsertPt = APIInsertPt;
    Builder.SetInsertPoint(IfInsertPt);
    Value *Cmp = Builder.CreateICmpNE(VIf, ConstantInt::get(VIf->getType(), 0));
    Instruction *ThenTerm, *ElseTerm;
    VPOParoptUtils::buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, IfInsertPt,
                                        DT);
    APIInsertPt = ThenTerm;
    Call = genTargetInitCode(W, NewCall, RegionId, APIInsertPt);
    Builder.SetInsertPoint(ElseTerm);
    Builder.CreateStore(
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), -1),
        OffloadError);
    if (isa<WRNTargetDataNode>(W)) {
      // For target data directive, if the "if" clause is evaluated to false,
      // device is host, and the outlined function is called without mapping
      // data.
      SmallVector<Value *, 4> FnArgs(NewCall->args());
      Builder.CreateCall(NewF, FnArgs, "");
    }
  } else {
    Call = genTargetInitCode(W, NewCall, RegionId, APIInsertPt);
  }

  if (!hasOffloadCompilation()) {
    if (isa<WRNTargetNode>(W)) {
      Builder.SetInsertPoint(APIInsertPt);
      Builder.CreateStore(Call, OffloadError);

      Builder.SetInsertPoint(NewCall);
      LLVMContext &C = F->getContext();
      LoadInst *LastLoad =
          Builder.CreateLoad(OffloadError->getAllocatedType(), OffloadError);
      ConstantInt *ValueZero = ConstantInt::getSigned(Type::getInt32Ty(C), 0);
      Value *ErrorCompare = Builder.CreateICmpNE(LastLoad, ValueZero);
      DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);
      Instruction *Term = SplitBlockAndInsertIfThen(ErrorCompare, NewCall,
                                                    false, nullptr, &DTU, LI);
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

// Reset the expression value in Subdevice clause to be empty.
void VPOParoptTransform::resetValueInSubdeviceClause(WRegionNode* W) {
    if (!W->canHaveSubdevice())
        return;

    SubdeviceClause& Subdevice = W->getSubdevice();
    if (Subdevice.empty())
        return;
    assert(Subdevice.size() == 1 && "There should be only 1 Subdevice clause");
    SubdeviceItem* SubdeviceI = Subdevice.front();

    resetValueInOmpClauseGeneric(W, SubdeviceI->getStart());
    resetValueInOmpClauseGeneric(W, SubdeviceI->getLength());
    resetValueInOmpClauseGeneric(W, SubdeviceI->getStride());
}

// Returns the corresponding flag for a given map clause modifier.
uint64_t VPOParoptTransform::getMapTypeFlag(MapItem *MapI,
                                            bool AddrIsTargetParamFlag,
                                            bool IsFirstComponentFlag,
                                            bool IsTargetKernelArg) const {

  auto printAndReturn = [&](uint64_t RetVal) {
    LLVM_DEBUG(dbgs() << "genMapTypeFlag : Map-type for '";
               MapI->getOrig()->printAsOperand(dbgs());
               dbgs() << "': " << RetVal << " ("
                      << llvm::format_hex(RetVal, 18, true) << ").\n");
    return RetVal;
  };

  uint64_t Res = 0u;
  if (!AddrIsTargetParamFlag && IsFirstComponentFlag) {
    if (IsTargetKernelArg)
      Res |= TGT_MAP_TARGET_PARAM;
    return printAndReturn(Res);
  }

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

  if (MapI->getInUseDevicePtr())
    Res |= TGT_MAP_RETURN_PARAM;

  // Memberof is given by the 16 MSB of the flag, so rotate by 48 bits.
  // It is workaroud. Need more work.
  auto getMemberOfFlag = [&]() {
    return (uint64_t)1 << 48;
  };

  // The flag AddrIsTargetParamFlag indicates that the map clause is
  // not in a chain. If it is head of the chain, according to the logic at
  // the entry of function getMapTypeFlag, it returns TGT_MAP_TARGET_PARAM.
  if (AddrIsTargetParamFlag) {
    if (IsTargetKernelArg)
      Res |= TGT_MAP_TARGET_PARAM;
  } else
    Res |= TGT_MAP_PTR_AND_OBJ | getMemberOfFlag();

  return printAndReturn(Res);
}

// Generate the sizes and map type flags for the given map type, map
// modifier and the expression V.
void VPOParoptTransform::genTgtInformationForPtrs(
    WRegionNode *W, Value *V, SmallVectorImpl<Constant *> &ConstSizes,
    SmallVectorImpl<uint64_t> &MapTypes,
    SmallVectorImpl<GlobalVariable *> &Names, SmallVectorImpl<Value *> &Mappers,
#if INTEL_CUSTOMIZATION
    SmallVectorImpl<bool> &IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
    SmallVectorImpl<bool> &IsFunctionPtr, bool &hasRuntimeEvaluationCaptureSize,
    bool VIsTargetKernelArg) const {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTgtInformationForPtrs:"
                    << " ConstSizes.size()=" << ConstSizes.size()
                    << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");

  const DataLayout &DL = F->getParent()->getDataLayout();
  LLVMContext &C = F->getContext();

  bool ForceMapping =
      isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W) ||
      isa<WRNTargetVariantNode>(W) || isa<WRNDispatchNode>(W);

  GlobalVariable *MapNameUnknown = nullptr;
  auto getMapNameForVar = [&](Value *V) -> GlobalVariable * {
    if (!UseMapperAPI)
      return nullptr;

    bool IsDebugCompilation = F->getParent()->getNamedMetadata("llvm.dbg.cu");
    if (!IsDebugCompilation)
      return nullptr;

    IRBuilder<> Builder(W->getEntryBBlock());

    // TODO: Create the map name string using debug information for V.
#ifndef NDEBUG
    if (V->hasName())
      return Builder.CreateGlobalString(
          (";" + V->getName() + ";unknown;0;0;;").str(), ".mapname");
#endif

    if (!MapNameUnknown)
      MapNameUnknown =
          Builder.CreateGlobalString(";unknown;unknown;0;0;;", ".mapname");
    return MapNameUnknown;
  };

  MapClause const &MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    if (!ForceMapping &&
        (MapI->getOrig() != V || !MapI->getOrig()))
      continue;
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Working with Map Item: '";
               MapI->dump(); dbgs() << "'.\n");
    if (MapI->getIsMapChain()) {
      MapChainTy const &MapChain = MapI->getMapChain();
      MapAggrTy *AggrHead = MapChain[0];
      int CurrentIndexForBaseOfChain = MapTypes.size() + 1;
      int InitialIndexForBaseOfChain =
          AggrHead->hasInitialAggrIndex() ? AggrHead->getInitialAggrIndex() : 0;
#if INTEL_CUSTOMIZATION
      bool IsWILocal = false;
      if (FirstprivateItem *FprivI = MapI->getInFirstprivate())
        if (FprivI->getIsWILocal())
          IsWILocal = true;
#endif // INTEL_CUSTOMIZATION
      bool MapBaseIsFunctionPtr = MapI->getIsFunctionPointer();

      for (unsigned I = 0; I < MapChain.size(); ++I) {
        MapAggrTy *Aggr = MapChain[I];
        auto ConstValue = dyn_cast<ConstantInt>(Aggr->getSize());
        if (!ConstValue) {
          hasRuntimeEvaluationCaptureSize = true;
          ConstSizes.push_back(Constant::getNullValue(Type::getInt64Ty(C)));
        } else {
          // Sign extend the constant to signed 64-bit integer.
          // This is the format of arg_sizes passed to __tgt_target.
          ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                                ConstValue->getSExtValue()));
        }

        uint64_t MapType = Aggr->getMapType();
        if (MapType || Aggr->hasExplicitMapType()) {
          // MemberOf flag is in the 16 MSBs of the 64 bit MapType.
          int OrigMemberOfFlag = static_cast<int>(MapType >> 48);
          uint64_t NewMapType = MapType;

          if (OrigMemberOfFlag) {
            int NewMemberOfFlag = 0;
            if (!InitialIndexForBaseOfChain || ForceMemberofToPointToBase) {
              // TODO: Legacy code. Delete once deemed unnecessary.
              // Since we don't know the initial index of the map-chain's base,
              // we work off of the assumption that the member-of flag can only
              // point to the base of a map-chain.
              NewMemberOfFlag = CurrentIndexForBaseOfChain;
#if INTEL_CUSTOMIZATION
            } else if (F->isFortran()) {
              // TODO: Delete once CMPLRLLVM-27688 is fixed by FFE.
              // ifx does not currently set the appropriate member-of flag
              // based on index. They only set member-of(1).
              NewMemberOfFlag = CurrentIndexForBaseOfChain;
#endif // INTEL_CUSTOMIZATION
            } else {
              if (CurrentIndexForBaseOfChain != InitialIndexForBaseOfChain)
                LLVM_DEBUG(dbgs()
                           << __FUNCTION__
                           << ": Map index of base of chain shifted from '"
                           << InitialIndexForBaseOfChain << "' to '"
                           << CurrentIndexForBaseOfChain << "'.\n");

              NewMemberOfFlag = OrigMemberOfFlag + (CurrentIndexForBaseOfChain -
                                                    InitialIndexForBaseOfChain);
            }

            if (NewMemberOfFlag != OrigMemberOfFlag) {
              assert(NewMemberOfFlag < (1 << 16) &&
                     "Too many maps. MemberOf flag exceeding 16 bits.");
              uint64_t Mask = (~(0ull)) >> 16;
              NewMapType = NewMapType & Mask;
              NewMapType =
                  NewMapType | (static_cast<uint64_t>(NewMemberOfFlag) << 48);
              LLVM_DEBUG(dbgs()
                         << __FUNCTION__ << ": Updated MemberOf Flag from '"
                         << OrigMemberOfFlag << "' to '" << NewMemberOfFlag
                         << "'.\n");
            }
          }

          // For operands in a map chain, as well as use_device_ptr clause, we
          // need to add TGT_MAP_RETURN_PARAM to the map-type of the base of
          // the chain (if not already present).
          if (I == 0 && MapI->getInUseDevicePtr()) {
            NewMapType = NewMapType | TGT_MAP_RETURN_PARAM;
            LLVM_DEBUG(dbgs() << __FUNCTION__
                              << ": Added TGT_MAP_RETURN_PARAM flag.\n");
          }

          // It's possible that the frontend created a map-type for V
          // without TGT_PARAM because it's not used inside the region.
          // For now, we add it back so that the runtime doesn't complain
          // about it.
          // TODO: We can restructure to not pass V into the kernel instead.
          if (I == 0 && VIsTargetKernelArg) {
            NewMapType = NewMapType | TGT_MAP_TARGET_PARAM;
            LLVM_DEBUG(dbgs() << __FUNCTION__
                              << ": Added TGT_MAP_TARGET_PARAM flag.\n");
          }

          if (NewMapType & TGT_MAP_USE_HOST_MEM) {
            LLVM_DEBUG(dbgs() << __FUNCTION__ << ": MapType " << NewMapType
                              << " has HOST_MEM bit ");
            if (!MapI->getIsUsedInAtomicFreeReduction()) {
              // Do not honor the HOST_MEM bit if the mapped var is not
              // used in atomic-free reduction.
              NewMapType &= ~TGT_MAP_USE_HOST_MEM;
              LLVM_DEBUG(dbgs() << "but it is removed.\n");
            } else {
              LLVM_DEBUG(dbgs() << "and it is honored.\n");
            }
          }

          if (MapType != NewMapType) {
            LLVM_DEBUG(dbgs()
                       << __FUNCTION__ << ": MapType changed from '" << MapType
                       << " (" << llvm::format_hex(MapType, 18, true)
                       << ")' to '" << NewMapType << " ("
                       << llvm::format_hex(NewMapType, 18, true) << ")'.\n");
            MapType = NewMapType;
          }

          MapTypes.push_back(MapType);
        } else {
          MapTypes.push_back(getMapTypeFlag(MapI, MapChain.size() <= 1, I == 0,
                                            VIsTargetKernelArg));
        }
        bool IsMapChainHeadAndParam =
            (I == 0 && (MapTypes.back() & TGT_MAP_TARGET_PARAM));
#if INTEL_CUSTOMIZATION
        IsWILocalFirstprivate.push_back(IsMapChainHeadAndParam ? IsWILocal
                                                               : false);
#endif // INTEL_CUSTOMIZATION
        IsFunctionPtr.push_back(IsMapChainHeadAndParam ? MapBaseIsFunctionPtr
                                                       : false);

        // MapName looks like:
        //  @0 = private unnamed_addr constant [40 x i8]
        //       c";y[0][0:1];tgt_map_ptr_arrsec.cpp;7;7;;\00", align 1
        if (auto *AggrName = Aggr->getName()) {
          auto *MapName = cast<GlobalVariable>(AggrName);
          // LTO assumes globals have names.
          if (!MapName->hasName())
            MapName->setName(".mapname");
          Names.push_back(MapName);
        } else
          Names.push_back(getMapNameForVar(Aggr->getBasePtr()));

        Mappers.push_back(Aggr->getMapper());
      }
    } else {
      // TODO: this needs to be cleaned up, after the parsing
      //       code is removed from WRegion analysis code.
      llvm_unreachable("Paropt only supports map chains now.");
    }
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      if (FprivI->getOrig() != V)
        continue;
      if (FprivI->getInMap())
        continue;
      Type *ItemTy = V->getType();
      Names.push_back(getMapNameForVar(V));
      Mappers.push_back(nullptr);

      if (!isa<PointerType>(ItemTy)) {
        // Non-pointer firstprivate items must be mapped as literals
        // with size 0.
        ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C), 0));
        MapTypes.push_back(TGT_MAP_TARGET_PARAM | TGT_MAP_LITERAL);
      } else if (FprivI->getIsPointer()) {
        // firstprivate() pointers are mapped with zero size
        // and map type NONE.
        ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C), 0));
        MapTypes.push_back(TGT_MAP_TARGET_PARAM);
      } else {
        auto getFPItemConstSize = [&DL](FirstprivateItem *FprivI) {
          Type *ElementTy;
          Value *NumElements;
          std::tie(ElementTy, NumElements, std::ignore) =
              VPOParoptUtils::getItemInfo(FprivI);
          auto ElementSize = DL.getTypeAllocSize(ElementTy);
          if (!NumElements)
            return ElementSize;

          if (!isa<ConstantInt>(NumElements)) {
            // FIXME: This needs to be changed into an assert. It doesn't affect
            // user code since the frontends send in map-chains for VLAs, but
            // we have LIT tests that are entering this code path, which should
            // be updated in a separate NFC change.
            return ElementSize;
          }

          auto NumElementsV = cast<ConstantInt>(NumElements)->getZExtValue();
          return NumElementsV * ElementSize;
        };

        auto FPSize = getFPItemConstSize(FprivI);
        LLVM_DEBUG(dbgs() << __FUNCTION__
                          << ": map-size for firstprivate var '";
                   FprivI->getOrig()->printAsOperand(dbgs(), false);
                   dbgs() << "' = " << FPSize << ".\n");
        ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C), FPSize));
        MapTypes.push_back(TGT_MAP_TARGET_PARAM | TGT_MAP_TO | TGT_MAP_PRIVATE);
      }
#if INTEL_CUSTOMIZATION
      IsWILocalFirstprivate.push_back(FprivI->getIsWILocal());
#endif // INTEL_CUSTOMIZATION
      IsFunctionPtr.push_back(false); // FPTR is applicable to map-chains only.
    }
  }

  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca() == V) {
    Type *T = W->getParLoopNdInfoAlloca()->getAllocatedType();
    ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                          DL.getTypeAllocSize(T)));
    MapTypes.push_back(TGT_MAP_ND_DESC);
    Names.push_back(getMapNameForVar(V));
    Mappers.push_back(nullptr);
#if INTEL_CUSTOMIZATION
    IsWILocalFirstprivate.push_back(false);
#endif // INTEL_CUSTOMIZATION
    IsFunctionPtr.push_back(false); // FPTR is applicable to map-chains only.
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
  auto &UncollapsedNDRangeDims = W->getUncollapsedNDRangeDimensions();
  auto &UncollapsedNDRangeTypes = W->getUncollapsedNDRangeTypes();
  uint8_t DistributeDim = W->getNDRangeDistributeDim();

  if (UncollapsedNDRangeDims.empty())
    return nullptr;

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB = SplitBlock(EntryBB, &*(EntryBB->begin()), DT, LI);
  W->setEntryBBlock(NewEntryBB);

  unsigned NumLoops = UncollapsedNDRangeDims.size();
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
                      ArrayRef(CLLoopParameterRecTypeArgs.begin(),
                                   CLLoopParameterRecTypeArgs.end()),
                      false);
  // FIXME: Use getInsertionPtForAllocas() for this alloca.
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
      CloneUB = Builder.CreateLoad(UncollapsedNDRangeTypes[Idx],
                                   UncollapsedNDRangeDims[Idx]);

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

// Collect the data mapping information for the given region.
// Return the number of entries in the mapping data structures.
unsigned VPOParoptTransform::getTargetDataInfo(
    WRegionNode *W, const CallInst *Call,
    SmallVectorImpl<Constant *> &ConstSizes,
    SmallVectorImpl<uint64_t> &MapTypes,
    SmallVectorImpl<GlobalVariable *> &Names,
    SmallVectorImpl<Value *> &Mappers,
#if INTEL_CUSTOMIZATION
    SmallVectorImpl<bool> &IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
    SmallVectorImpl<bool> &IsFunctionPtr,
    bool &HasRuntimeEvaluationCaptureSize) const {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::getTargetDataInfo\n");
  auto Exiter = [&MapTypes]() {
    LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::getTargetDataInfo\n");
    return MapTypes.size();
  };

  HasRuntimeEvaluationCaptureSize = false;
  bool ForceMapping =
      // These regions will not have any real references to the mapped
      // items, but we still have to notify the runtime about the mappings.
      isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W);

  if (ForceMapping) {
    genTgtInformationForPtrs(W, nullptr, ConstSizes, MapTypes, Names, Mappers,
#if INTEL_CUSTOMIZATION
                             IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                             IsFunctionPtr, HasRuntimeEvaluationCaptureSize);
    return Exiter();
  }

  assert(Call && "Call is required to cross reference clauses with call args.");

  for (unsigned II = 0; II < Call->arg_size(); ++II) {
    Value *BPVal = Call->getArgOperand(II);
    genTgtInformationForPtrs(W, BPVal, ConstSizes, MapTypes, Names, Mappers,
#if INTEL_CUSTOMIZATION
                             IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                             IsFunctionPtr, HasRuntimeEvaluationCaptureSize,
                             /*VIsTargetKernelArg=*/isa<WRNTargetNode>(W));
  }

  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
    genTgtInformationForPtrs(W, W->getParLoopNdInfoAlloca(), ConstSizes,
                             MapTypes, Names, Mappers,
#if INTEL_CUSTOMIZATION
                             IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                             IsFunctionPtr, HasRuntimeEvaluationCaptureSize,
                             /*VIsTargetKernelArg=*/true);

  return Exiter();
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
CallInst *VPOParoptTransform::genTargetInitCode(
    WRegionNode *W, CallInst *Call, Value *RegionId, Instruction *InsertPt) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTargetInitCode\n");
  assert(!hasOffloadCompilation() &&
         "genTargetInitCode() called for device compilation.");

  TgDataInfo Info;
  SmallVector<Constant *, 16> ConstSizes;
  SmallVector<uint64_t, 16> MapTypes;
  SmallVector<GlobalVariable *, 16> Names;
  SmallVector<Value *, 16> Mappers;
#if INTEL_CUSTOMIZATION
  SmallVector<bool, 16> IsWILocalFirstprivate;
#endif // INTEL_CUSTOMIZATION
  SmallVector<bool, 16> IsFunctionPtr;
  bool HasRuntimeEvaluationCaptureSize = false;
  Info.NumberOfPtrs =
      getTargetDataInfo(W, Call, ConstSizes, MapTypes, Names, Mappers,
#if INTEL_CUSTOMIZATION
                        IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                        IsFunctionPtr, HasRuntimeEvaluationCaptureSize);

  if (Info.NumberOfPtrs)
    genOffloadArraysInit(W, &Info, Call, InsertPt, ConstSizes, MapTypes, Names,
                         Mappers, HasRuntimeEvaluationCaptureSize);

  genOffloadArraysArgument(&Info, InsertPt);

  CallInst *TgtCall = nullptr;
  if (isa<WRNTargetNode>(W)) {
    auto *IT = W->wrn_child_begin();
    if (IT != W->wrn_child_end() && isa<WRNTeamsNode>(*IT)) {
      WRNTeamsNode *TW = cast<WRNTeamsNode>(*IT);
      TgtCall = VPOParoptUtils::genTgtTargetTeams(
          TW, RegionId, Info.NumberOfPtrs, Info.ResBaseDataPtrs,
          Info.ResDataPtrs, Info.ResDataSizes, Info.ResDataMapTypes,
          Info.ResNames, Info.ResMappers, InsertPt);
    } else
      TgtCall = VPOParoptUtils::genTgtTarget(
          W, RegionId, Info.NumberOfPtrs, Info.ResBaseDataPtrs,
          Info.ResDataPtrs, Info.ResDataSizes, Info.ResDataMapTypes,
          Info.ResNames, Info.ResMappers, InsertPt);
  } else if (isa<WRNTargetDataNode>(W)) {
    TgtCall = VPOParoptUtils::genTgtTargetDataBegin(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, Info.ResNames, Info.ResMappers,
        InsertPt);
    genOffloadArraysArgument(&Info, InsertPt);
    VPOParoptUtils::genTgtTargetDataEnd(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, Info.ResNames, Info.ResMappers,
        InsertPt);
  } else if (isa<WRNTargetUpdateNode>(W))
    TgtCall = VPOParoptUtils::genTgtTargetDataUpdate(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, Info.ResNames, Info.ResMappers,
        InsertPt);
  else if (isa<WRNTargetEnterDataNode>(W))
    TgtCall = VPOParoptUtils::genTgtTargetDataBegin(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, Info.ResNames, Info.ResMappers,
        InsertPt);
  else if (isa<WRNTargetExitDataNode>(W))
    TgtCall = VPOParoptUtils::genTgtTargetDataEnd(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, Info.ResNames, Info.ResMappers,
        InsertPt);
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

  if (!(isa<WRNTargetDataNode>(W) || isa<WRNTargetVariantNode>(W) ||
        isa<WRNDispatchNode>(W)))
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
  uint64_t MapType = TGT_MAP_RETURN_PARAM;
  MapClause &MapC = W->getMap();
  for (UseDevicePtrItem *UDPI : UDPC.items()) {
    // There's already a map clause present for the use_device_ptr clause item.
    if (UDPI->getInMap())
      continue;

    Value *UDP = UDPI->getOrig();
    Value *MappedVal = UDP;
    if (UDPI->getIsPointerToPointer()) {
      Type *UDPLoadTy = nullptr;
      std::tie(UDPLoadTy, std::ignore, std::ignore) =
          VPOParoptUtils::getItemInfo(UDPI);
      MappedVal =
          LoadBuilder.CreateLoad(UDPLoadTy, UDP, UDP->getName() + ".load");
#if INTEL_CUSTOMIZATION
    } else if (UDPI->getIsF90DopeVector()) {
      // For F90_DVs, the map needs to be added for the data pointer, i.e.
      // load i32*, i32** (getelementptr (%dv, 0, 0)).
      Type *DVType = nullptr;
      Value *NumElements = nullptr;
      std::tie(DVType, NumElements, std::ignore) =
          VPOParoptUtils::getItemInfo(UDPI);
      assert(!NumElements && "use_device_ptr item cannot be an array.");

      auto *Zero = LoadBuilder.getInt32(0);
      auto *Addr0GEP = LoadBuilder.CreateInBoundsGEP(DVType, UDP, {Zero, Zero},
                                                     UDP->getName() + ".addr0");
      MappedVal = LoadBuilder.CreateLoad(
          cast<GEPOperator>(Addr0GEP)->getResultElementType(),
          Addr0GEP, Addr0GEP->getName() + ".load");
    } else if (UDPI->getIsCptr()) {
      // CPTR type is of form: "%cptr = type { i64 }". So, for "cptr*" operands,
      // we need to cast them to i8** before creating a load to be mapped, like:
      //   load i8*, i8** (bitcast cptr* %p to i8**)
      PointerType *Int8PtrTy = LoadBuilder.getInt8PtrTy();
      PointerType *Int8PtrPtrTy = Int8PtrTy->getPointerTo();

      auto *PtrPtrCast = LoadBuilder.CreateBitOrPointerCast(
          UDP, Int8PtrPtrTy, UDP->getName() + ".cast");

      MappedVal = LoadBuilder.CreateLoad(Int8PtrTy, PtrPtrCast,
                                         UDP->getName() + ".val");
#endif // INTEL_CUSTOMIZATION
    }
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

/// For [first]private clauses on \p W with non-constant number-of-elements
/// (or those marked with a VARLEN modifier), create a Map clause to let the
/// runtime handle their privatization. Any instructions generated for the map
/// clause are inserted before \p W's entry BasicBlock.
///
/// \code
/// (A)
/// --------------------------------------+------------------------------------
///         Before                        |      After
/// --------------------------------------+------------------------------------
/// %vla = alloca i32, i32 %n             |  %vla = alloca i32, i64 %n
///                                       |
///                                      (1) %n.cast = zext i32 %n to i64
///                                      (2) %n.in.bytes = mul i64 %n.cast, 4
///                                       |
/// "PRIVATE"(i32* %vla)                  |  "PRIVATE"(i32 %vla)
///                                      (3) "MAP"(i32* %vla, i32* %vla,
///                                       |        i64 %n.in.bytes,
///                                       |        PARAM|PRIVATE) // Not in IR
///                                       |
///
/// (B)
/// --------------------------------------+------------------------------------
///         Before                        |      After
/// --------------------------------------+------------------------------------
/// %vla = alloca i32, i32 %n             |  %vla = alloca i32, i64 %n
///                                       |
///                                      (1) %n.cast = zext i32 %n to i64
///                                      (2) %n.in.bytes = mul i64 %n.cast, 4
///                                       |
/// "FIRSTPRIVATE"(i32* %vla)             |  "FIRSTPRIVATE"(i32 %vla)
///                                      (3) "MAP"(i32* %vla, i32* %vla,
///                                       |        i64 %n.in.bytes,
///                                       |        PARAM|PRIVATE|TO)// Not in IR
///                                       |
///
/// \endcode
/// Note: the map itself is not added to the IR, only to the node \p W.
///
/// Note: This may not be safe to do for TYPED clauses, since the size is sent
/// as a bundle operand, which may get constant propagated in either host/device
/// compilation. That would cause one phase to treat it as a VLA, and the other
/// as a constant-sized array, causing a mismatch in the number of arguments.
/// It would be safer for the frontend to directly emit the additional map.
bool VPOParoptTransform::addMapForPrivateAndFPVLAs(WRNTargetNode *W) {
  assert(W && "Null WRegionNode.");

  Instruction *InsertPt = nullptr;
  auto SetupInsertPointIfNeeded = [&]() {
    // Create an empty BB before the entry BB, to insert size computation for
    // the new map clauses if InsertPt is null.
    if (InsertPt)
      return;

    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *NewEntryBB =
        SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
    W->setEntryBBlock(NewEntryBB);
    W->populateBBSet(true); // rebuild BBSet unconditionlly as EntryBB changed
    InsertPt = EntryBB->getTerminator();
  };

  auto AddMapForVLAIfNotPresent = [&, FunctionName =
                                          __FUNCTION__](auto *I) -> MapItem * {
    Value *Orig = I->getOrig();

    // If there's already a frontend-generated map for the item, we don't need
    // to do anything.
    if (I->getInMap())
      return nullptr;

    Type *ElementTy = nullptr;
    Value *NumElements = nullptr;
    std::tie(ElementTy, NumElements, std::ignore) =
        VPOParoptUtils::getItemInfo(I);

    // If the item is not for a VLA, and it's not marked with a VARLEN modifier,
    // we don't need to emit a map clause for it.
    if (!I->getIsVarLen() && (!NumElements || isa<ConstantInt>(NumElements)))
      return nullptr;

    // WILOCAL private VLAs can be allocated within the body of the target
    // region's outlined function, so we don't need to create a map for them.
    if (I->getIsWILocal())
      return nullptr;

    SetupInsertPointIfNeeded();
    IRBuilder<> MapBuilder(InsertPt);

    const DataLayout &DL = F->getParent()->getDataLayout();
    Type *I64Ty = MapBuilder.getInt64Ty();
    if (!NumElements)
      NumElements = ConstantInt::get(I64Ty, 1);

    Value *ElementSize =
        ConstantInt::get(I64Ty, DL.getTypeAllocSize(ElementTy));
    Value *NumElementsCast = MapBuilder.CreateZExtOrTrunc(
        NumElements, I64Ty, NumElements->getName() + ".cast"); //          (1)

    Value *MappedVal = Orig;
    Value *MapSize = MapBuilder.CreateMul(NumElementsCast, ElementSize, // (2)
                                          NumElements->getName() + ".in.bytes");
    uint64_t MapType = TGT_MAP_TARGET_PARAM | TGT_MAP_PRIVATE;
    if (isa<FirstprivateItem>(I))
      MapType |= TGT_MAP_TO;

    MapClause &MapC = W->getMap();
    MapAggrTy *MapAggr = new MapAggrTy(MappedVal, MappedVal, MapSize, MapType);
    MapItem *MapI = new MapItem(MapAggr);
    MapI->setOrig(MappedVal);
    MapI->setIsVarLen(I->getIsVarLen());
    MapC.add(MapI); //                                                     (3)
    I->setInMap(MapI);

    LLVM_DEBUG(dbgs() << FunctionName << ": Added map clause for VLA '";
               I->dump(); dbgs() << "'.\n");
    (void)FunctionName;
    return MapI;
  };

  bool Changed = false;
  for (PrivateItem *PrivI : W->getPriv().items()) {
    if (MapItem *MapI = AddMapForVLAIfNotPresent(PrivI)) {
      MapI->setInPrivate(PrivI);
      Changed = true;
    }
  }

  for (FirstprivateItem *FprivI : W->getFpriv().items()) {
    if (MapItem *MapI = AddMapForVLAIfNotPresent(FprivI)) {
      MapI->setInFirstprivate(FprivI);
      Changed = true;
    }
  }
  return Changed;
}

// Add globals for atomic-free GPU reduction global buffers (one per reduction
// item) and global teams counter (one per kernel)
bool VPOParoptTransform::createAtomicFreeReductionBuffers(WRegionNode *W) {
  assert(W && "Null WRegionNode.");

  // Global and local buffers stand for the different stages of the reduction
  // (global and local respectively), they are both global in terms of
  // a memory kind (i.e. addrspace(1) from the IR PoV).
  // AtomicFreeRedLocalBufSize == 0 indicates that no tree pattern is used for
  // local reduction, so we can reuse the buffer (red_buf right above) used
  // for global reduction.
  // NOTE: arrsec reduction doesn't support no-SLM local buffers yet.
  bool NeedsLocalBuffer = VPOParoptUtils::isAtomicFreeReductionLocalEnabled() &&
                          WRegionUtils::supportsLocalAtomicFreeReduction(W) &&
                          !AtomicFreeReductionUseSLM &&
                          AtomicFreeRedLocalBufSize;
  bool NeedsGlobalBuffer =
      VPOParoptUtils::isAtomicFreeReductionGlobalEnabled() &&
      WRegionUtils::supportsGlobalAtomicFreeReduction(W);

  if (!NeedsLocalBuffer && !NeedsGlobalBuffer)
    return false;

  auto &RedClause = W->getRed();
  if (RedClause.empty())
    return false;

  using ClauseBundleTy = SmallVector<Value *, 4>;
  SmallVector<std::pair<StringRef, ClauseBundleTy>, 8> NewClauses;
  StringRef MapClauseName = VPOAnalysisUtils::getClauseString(QUAL_OMP_MAP_TO);

  auto *WTarget = WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);
  MapClause &MapC = WTarget->getMap();

  auto addMapForValue = [&](Value *V, uint64_t MapType, Value *MapTypeVal,
                            Value *MapSize) {
    MapAggrTy *MapAggr = new MapAggrTy(V, V, MapSize, MapType);
    MapItem *MapI = new MapItem(MapAggr);
    MapI->setOrig(V);
    MapC.add(MapI);

    // FIXME: create 6-operand MAP clause with the variable name
    //        and the null mapper.
    NewClauses.push_back(
        {MapClauseName, ClauseBundleTy({V, V, MapSize, MapTypeVal})});

    // Some reduction LIT tests depend on this debug output
    LLVM_DEBUG(dbgs() << "createAtomicFreeReductionBuffers: Adding map-type ("
                      << *V << ", " << *V << ", " << *MapSize << ", "
                      << *MapTypeVal << ")\n");
  };
  const DataLayout &DL = F->getParent()->getDataLayout();

  CallInst *EntryCI = cast<CallInst>(WTarget->getEntryDirective());

  bool FoundProperItem = false;

  for (ReductionItem *RedI : RedClause.items()) {
    if (!VPOParoptUtils::supportsAtomicFreeReduction(RedI))
      continue;

    if (RedI->getIsArraySection())
      computeArraySectionTypeOffsetSize(WTarget, *RedI, EntryCI);

    Type *BufTy = nullptr;
    Value *NumElems = nullptr;

    std::tie(BufTy, NumElems, std::ignore) = VPOParoptUtils::getItemInfo(RedI);

    if (NumElems && !isa<ConstantInt>(NumElems))
      continue;

    FoundProperItem = true;

    assert(BufTy && "Found untyped reduction item");

    // For arrays/arrsects the type of the buffer is expected to be
    //    [M x [N * ElemTy]], where
    // M = {
    //  AtomicFreeRedLocalBufSize x AtomicFreeRedGlobalBufSize, local buffer
    //  AtomicFreeRedGlobalBufSize,                             global buffer
    // },
    // N = NumElems (i.e. section size),
    // ElemTy = BufTy returned by getItemInfo (i.e. item element type)
    if (NumElems)
      BufTy =
          ArrayType::get(BufTy, cast<ConstantInt>(NumElems)->getZExtValue());

    uint64_t MapType = TGT_MAP_PRIVATE | TGT_MAP_CLOSE;

    bool UseReductionDynamicBuffer =
        AtomicFreeReductionDynamicBuffer &&
        VPOParoptTransform::deviceTriplesHasSPIRV();

    if (UseReductionDynamicBuffer)
      MapType |= TGT_MAP_SIZE_TIMES_NUM_TEAMS;
    Value *MapTypeVal =
        ConstantInt::get(Type::getInt64Ty(F->getContext()), MapType);
    uint64_t Size = DL.getTypeSizeInBits(BufTy) / 8;

    Module *M = F->getParent();
    assert(M && "Function has no parent module.");
    Triple TT(M->getTargetTriple());

    GlobalValue::LinkageTypes BufLinkage =
        GlobalValue::LinkageTypes::ExternalWeakLinkage;
    Constant *Initializer = nullptr;
    // MSVC linker does not support multiple definitions of extern_weak
    // symbols with the same name in different modules, so we have to define
    // the variable with private linkage for the host compilation.
    //
    // FIXME: we need to think about not using global variables. These host
    //        variables are eventually just used to tell libomptarget know
    //        that it needs to allocate some memory on the device.
    //        I believe we could as well use nullptr for this purpose.
    if (TT.isOSWindows() && !hasOffloadCompilation()) {
      BufLinkage = GlobalValue::LinkageTypes::PrivateLinkage;
      Initializer = Constant::getNullValue(BufTy);
    }

    if (NeedsGlobalBuffer) {
      Value *MapGlobalSize = ConstantInt::get(
          Type::getInt64Ty(F->getContext()),
          Size * ((AtomicFreeRedGlobalBufSize && !UseReductionDynamicBuffer)
                      ? AtomicFreeRedGlobalBufSize
                      : 1));

      auto *GlobalBuf = new GlobalVariable(
          *F->getParent(), BufTy, false, BufLinkage, Initializer, "red_buf",
          nullptr, GlobalValue::NotThreadLocal,
          isTargetSPIRV() ? vpo::ADDRESS_SPACE_GLOBAL : 0);
      GlobalBuf->addAttribute(VPOParoptAtomicFreeReduction::GlobalBufferAttr);
      addMapForValue(GlobalBuf, MapType, MapTypeVal, MapGlobalSize);
    }

    if (NeedsLocalBuffer) {
      Value *MapLocalSize = ConstantInt::get(
          Type::getInt64Ty(F->getContext()),
          Size * AtomicFreeRedLocalBufSize *
              ((AtomicFreeRedGlobalBufSize && !UseReductionDynamicBuffer)
                   ? AtomicFreeRedGlobalBufSize
                   : 1));

      auto *LocalBuf = new GlobalVariable(
          *F->getParent(), BufTy, false, BufLinkage, Initializer,
          "red_local_buf", nullptr, GlobalValue::NotThreadLocal,
          isTargetSPIRV() ? vpo::ADDRESS_SPACE_GLOBAL : 0);
      LocalBuf->addAttribute(VPOParoptAtomicFreeReduction::LocalBufferAttr);
      addMapForValue(LocalBuf, MapType, MapTypeVal, MapLocalSize);
    }
  }

  if (!FoundProperItem)
    return false;

  if (NeedsGlobalBuffer) {
    Type *CounterTy = Type::getInt32Ty(F->getContext());
    uint64_t Size = DL.getTypeSizeInBits(CounterTy) / 8;
    // The teams_counter can either be made firstprivate (using MAP_TO), or we
    // can ask the runtime to allocate it from a zero-initialized memory pool,
    // to avoid host-to-device transfer associated with the MAP_TO.
    uint64_t MapType =
        TGT_MAP_PRIVATE | TGT_MAP_TARGET_PARAM |
        (AtomicFreeRedUseFPTeamsCounter ? TGT_MAP_TO
                                        : TGT_MAP_USE_ZERO_INIT_MEM);
    Value *MapTypeVal =
        ConstantInt::get(Type::getInt64Ty(F->getContext()), MapType);
    Value *MapSize = ConstantInt::get(Type::getInt64Ty(F->getContext()), Size);

    auto *GlobalCounter = new GlobalVariable(
        *(F->getParent()), CounterTy, false,
        GlobalValue::LinkageTypes::PrivateLinkage,
        ConstantInt::get(Type::getInt32Ty(F->getContext()), 0), "teams_counter",
        nullptr, GlobalValue::NotThreadLocal, isTargetSPIRV() ? 1 : 0);
    GlobalCounter->addAttribute(VPOParoptAtomicFreeReduction::TeamsCounterAttr);

    addMapForValue(GlobalCounter, MapType, MapTypeVal, MapSize);
  }

  SmallVector<std::pair<StringRef, ArrayRef<Value *>>> OpBundlesToAdd;
  for (auto &C : NewClauses)
    OpBundlesToAdd.emplace_back(C.first, C.second);

  EntryCI = VPOUtils::addOperandBundlesInCall(EntryCI, OpBundlesToAdd);
  WTarget->setEntryDirective(EntryCI);

  return true;
}

/// Convert 'IS_DEVICE_PTR' clauses in \p W to MAP, and
/// 'IS_DEVICE_PTR:PTR_TO_PTR' clauses to MAP + PRIVATE.
///
/// Note that 'HAS_DEVICE_ADDR' is parsed as 'IS_DEVICE_PTR'
/// so they are also transformed as shown below.
///
/// \code
/// (A)
/// --------------------------------------+------------------------------------
///         Before                        |      After
/// --------------------------------------+------------------------------------
///                                       |
/// "IS.DEVICE.PTR"(i32* %a.load)        (1) "MAP"(i32* %a.load, PARAM|LITERAL)
///                                       |
///
/// (B)
/// --------------------------------------+------------------------------------
///         Before                        |      After
/// --------------------------------------+------------------------------------
///                                      (2) %a.load = load i32*, i32** %a
///                                       |
/// "IS.DEVICE.PTR:PTR_TO_PTR"(i32** %a) (3) "MAP"(i32* %a.load, PARAM|LITERAL)
///                                      (4) "PRIVATE"(i32** %a)
///                                       |
///                                      (5) store i32* %a.load, i32** %a
///                                       |
#if INTEL_CUSTOMIZATION
///
/// (C)
/// --------------------------------------+------------------------------------
///         Before                        |      After
/// --------------------------------------+------------------------------------
///                                      (6) %a.cast1 = bitcast cptr* %a to i8**
///                                      (2) %a.load = load i*, i8** %a.cast1
///                                       |
/// "IS.DEVICE.PTR:CPTR"(cptr* %a)       (3) "MAP"(i8* %a.load, PARAM|LITERAL)
///                                      (4) "PRIVATE"(i32** %a)
///                                       |
///                                      (7) %a.cast2 = bitcast cptr* %a to i8**
///                                      (5) store i8* %a.load, i8** %a
///                                       |
///
/// (D)
/// --------------------------------------+------------------------------------
///         Before                        |      After
/// --------------------------------------+------------------------------------
/// "IS.DEVICE.PTR:F90_DV" (DV* %a)      (8) "FIRSTPRIVATE"(DV* %a)
///                                       |
#endif // INTEL_CUSTOMIZATION
/// \endcode
bool VPOParoptTransform::addMapAndPrivateForIsDevicePtr(WRegionNode *W) {
  assert(W && "Null WRegionNode.");

  if (!W->canHaveIsDevicePtr())
    return false;

  IsDevicePtrClause &IDPC = W->getIsDevicePtr();
  if (IDPC.empty())
    return false;

  StringRef MapClauseName = VPOAnalysisUtils::getClauseString(QUAL_OMP_MAP_TO);
  StringRef PrivateClauseName =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_PRIVATE);
  using ClauseBundleTy = SmallVector<Value *, 4>;
  SmallVector<std::pair<StringRef, ClauseBundleTy>, 8> NewClauses;

  const DataLayout &DL = F->getParent()->getDataLayout();
  uint64_t Size = DL.getPointerSizeInBits() / 8;
  uint64_t MapType = TGT_MAP_TARGET_PARAM | TGT_MAP_LITERAL;
  Value *MapSize = ConstantInt::get(Type::getInt64Ty(F->getContext()), Size);
  Value *MapTypeVal =
      ConstantInt::get(Type::getInt64Ty(F->getContext()), MapType);

  MapClause &MapC = W->getMap();
  FirstprivateClause &FirstprivateC = W->getFpriv();

  auto addMapForValue = [&](Value *V) {
    MapAggrTy *MapAggr = new MapAggrTy(V, V, MapSize, MapType);
    MapItem *MapI = new MapItem(MapAggr);
    MapI->setOrig(V);
    MapC.add(MapI);

    // FIXME: create 6-operand MAP clause with the variable name
    //        and the null mapper.
    NewClauses.push_back({MapClauseName,
                          ClauseBundleTy({V, V, MapSize, MapTypeVal})});
  };
#if INTEL_CUSTOMIZATION
  StringRef FPrivateClauseName =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_FIRSTPRIVATE);
  auto addFirstprivateForValue = [&](Value *V) {
    FirstprivateC.add(new FirstprivateItem(V));
    // TODO: OPAQUEPOINTER: We can remove this, since FFE is now emitting
    // FIRSTPRIVATE clause directly instead of IS_DEVICE_PTR:F90_DV for target
    // construct.
    NewClauses.push_back({FPrivateClauseName, ClauseBundleTy({V})});
    LLVM_DEBUG(dbgs() << "addFirstprivateForValue: Converted 'IS_DEVICE_PTR(";
               V->printAsOperand(dbgs()); dbgs() << ")' to 'FIRSTPRIVATE(";
               V->printAsOperand(dbgs()); dbgs() << "\n");
  };
#endif // INTEL_CUSTOMIZATION
  bool Changed = false;
  bool SeenOperandsWithModifiers = false;

  auto addMapIfNoModifier = [&](IsDevicePtrItem *IDPI) {
    if (IDPI->getIsPointerToPointer()) {
      SeenOperandsWithModifiers = true;
      return;
    }
#if INTEL_CUSTOMIZATION
    if (IDPI->getIsCptr()) {
      SeenOperandsWithModifiers = true;
      return;
    }
    if (IDPI->getIsF90DopeVector()) {
      SeenOperandsWithModifiers = true;
      return;
    }
#endif // INTEL_CUSTOMIZATION
    Value *IDP = IDPI->getOrig();
    addMapForValue(IDP); //                                             (1)
    LLVM_DEBUG(dbgs() << "addMapIfNoModifiers: Converted 'IS_DEVICE_PTR(";
               IDP->dump(); dbgs() << ")' to 'MAP(";
               IDP->printAsOperand(dbgs());
               dbgs() << ", TGT_PARAM | TGT_LITERAL)'.\n");
    Changed = true;
  };

  // (A) First, handle all "IS_DEVICE_PTR" clauses without PTR_TO_PTR modifiers.
  std::for_each(IDPC.items().begin(), IDPC.items().end(), addMapIfNoModifier);

  // Utility to update directive call, if any changes for is_device_ptr
  // clauses were made.
  auto UpdateIRIfChanged = [&]() {
    if (Changed) {
      for (IsDevicePtrItem *IDPI : IDPC.items())
        IDPI->setOrig(nullptr);
      CallInst *EntryCI = cast<CallInst>(W->getEntryDirective());
      EntryCI = VPOUtils::removeOpenMPClausesFromCall(
          EntryCI,
          {QUAL_OMP_IS_DEVICE_PTR,
           // has_device_addr() clause is parsed into IsDevicePtrItem.
           // We have to delete all has_device_addr() clauses
           // from the directive call, since they are "lowered" in this
           // function as well.
           QUAL_OMP_HAS_DEVICE_ADDR});

      SmallVector<std::pair<StringRef, ArrayRef<Value *>>> OpBundlesToAdd;
      for (auto &C : NewClauses)
        OpBundlesToAdd.emplace_back(C.first, C.second);

      EntryCI = VPOUtils::addOperandBundlesInCall(EntryCI, OpBundlesToAdd);
      W->setEntryDirective(EntryCI);
    }

    W->resetBBSetIfChanged(Changed); // Clear BBSet if transformed
    return Changed;
  };

  if (!SeenOperandsWithModifiers)
    return UpdateIRIfChanged();

  // (B) Handle clauses with PTR_TO_PTR modifiers next.

  // Create an empty BB before the entry BB, to insert the load (2).
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB =
      SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
  W->setEntryBBlock(NewEntryBB);
  W->populateBBSet(true); // rebuild BBSet unconditionally as EntryBB changed
  Instruction *LoadInsertPt = EntryBB->getTerminator();

  // Create an empty BB after the Entry BB to insert the store (5).
  BasicBlock *StoreBB = createEmptyPrivInitBB(W);

  IRBuilder<> LoadBuilder(LoadInsertPt);
  IRBuilder<> StoreBuilder(StoreBB->getTerminator());
  PrivateClause &PC = W->getPriv();

  auto addMapAndPrivateForIDP = [&](IsDevicePtrItem *IDPI, Type *LoadedValType,
                                    Value *LoadSrc, Value *StoreDst) {
    Value *IDP = IDPI->getOrig();
    Value *IDPLoad = LoadBuilder.CreateLoad(LoadedValType, LoadSrc, //  (2)
                                            IDP->getName() + ".load");
    addMapForValue(IDPLoad); //                                         (3)
    // TODO: OPAQUEPOINTER: This code is obsolete. We would have to add a TYPED
    // clause here for opaque pointers, but this code path is now obsolete, as
    // clang FE directly emits map-chains for the is_device_ptr clause.
    PC.add(IDP); //                                                     (4)
    StoreBuilder.CreateStore(IDPLoad, StoreDst); //                     (5)
    LLVM_DEBUG(dbgs() << "addMapAndPrivateForIDP: Converted 'IS_DEVICE_PTR:";
               IDPI->dump(); dbgs() << "' to 'MAP(";
               IDPLoad->printAsOperand(dbgs());
               dbgs() << ", TGT_PARAM | TGT_LITERAL) PRIVATE(";
               IDP->printAsOperand(dbgs()); dbgs() << ")'.\n");

    NewClauses.push_back({PrivateClauseName, ClauseBundleTy({IDP})});
    return true;
  };

  for (IsDevicePtrItem *IDPI : IDPC.items()) {
    Value *IDP = IDPI->getOrig();
#if INTEL_CUSTOMIZATION
    if (IDPI->getIsCptr()) {
      unsigned AddrSpace =
          cast<PointerType>(IDP->getType())->getAddressSpace();
      PointerType *Int8PtrTy = LoadBuilder.getInt8PtrTy(AddrSpace);
      PointerType *Int8PtrPtrTy = Int8PtrTy->getPointerTo(AddrSpace);

      Value *LoadSrc = LoadBuilder.CreateBitOrPointerCast(
          IDP, Int8PtrPtrTy, IDP->getName() + ".cast"); //              (6)
      Value *StoreDst = StoreBuilder.CreateBitOrPointerCast(
          IDP, Int8PtrPtrTy, IDP->getName() + ".cast"); //              (7)

      Changed |= addMapAndPrivateForIDP(IDPI, Int8PtrTy, LoadSrc, StoreDst);
      continue;
    }

    if (IDPI->getIsF90DopeVector()) {
      Changed = true;
      addFirstprivateForValue(IDP); //                                  (8)
      continue;
    }
#endif // INTEL_CUSTOMIZATION
    if (!IDPI->getIsPointerToPointer()) // Already handled above
      continue;

    Type *IDPLoadTy = nullptr;
    std::tie(IDPLoadTy, std::ignore, std::ignore) =
        VPOParoptUtils::getItemInfo(IDPI);
    Changed |= addMapAndPrivateForIDP(IDPI, IDPLoadTy, IDP, IDP);
  }

  return UpdateIRIfChanged();
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
#if INTEL_CUSTOMIZATION
// Case 3: Type of %udp is DV* (dope vector), and it's marked as F90_DV:
// Before:
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     call @outlined.funtion(...DV* %udp)  ; TgtDataOutlinedFunctionCall
//   call void @__tgt_target_data_end()
//
// After:
//   $udp.new = alloca DV                                            ; (2)
//   ...
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     memcpy(%udp.new, %udp, sizeof(DV));                           ; (7)
//     %udp.gep.cast = cast i8* %udp.gep to i32**                    ; (3)
//     %udp.updated.val = load i32*, i32** %udp.gep.cast             ; (4)
//     %udp.new.addr0.gep = getelementptr(%udp.new, 0, 0)            ; (8)
//     store i32* %udp.updated.val, i32** %udp.new.addr0.gep         ; (5)
//     call @outlined.funtion(...DV* %udp.new)                       ; (6)
//   call void @__tgt_target_data_end()
//
// Case 4: Type of %udp is cptr* (Fortran C_PTR), and it's marked as CPTR:
// Before:
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     call @outlined.funtion(...cptr* %udp)  ; TgtDataOutlinedFunctionCall
//   call void @__tgt_target_data_end()
//
// After:
//   $udp.new = alloca cptr                                          ; (2)
//   ...
//   %udp.gep = getelementptr(%offload.baseptrs...)                  ; (1)
//   ...
//   call void @__tgt_target_data_begin(...%offload.baseptrs...)
//     %udp.gep.cast = cast i8** %udp.gep to i8**           ;elided  ; (3)
//     %udp.updated.val = load i8*, i8** %udp.gep.cast               ; (4)
//     %udp.new.cast = bitcast %udp.new to i8**                      ; (9)
//     store i8* %udp.updated.val, i8** %udp.new.cast                ; (5)
//     call @outlined.funtion(...cptr* %udp.new)                     ; (6)
//   call void @__tgt_target_data_end()
//
#endif // INTEL_CUSTOMIZATION
void VPOParoptTransform::useUpdatedUseDevicePtrsInTgtDataRegion(
    WRegionNode *W, Instruction *TgtDataOutlinedFunctionCall) {
  assert(W && "Null WRegionNode.");
  assert(TgtDataOutlinedFunctionCall && "Null outlined function call.");
  // When handling WRNDispatchNode, there is no outlining involved,
  // and TgtDataOutlinedFunctionCall is just the dispatch call.

  if (!W->canHaveUseDevicePtr() && !isa<WRNDispatchNode>(W))
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

    Value *GepCast = Builder.CreateBitOrPointerCast(
        BasePtrGEP, MapI->getOrig()->getType()->getPointerTo(),
        BasePtrGEP->getName() + ".cast"); //                                (3)

    LoadInst *UpdatedUDPVal =
        Builder.CreateLoad(MapI->getOrig()->getType(), GepCast,
                           OrigV->getName() + ".updated.val"); //           (4)
    Value *NewV = UpdatedUDPVal;
    if (UDPI->getIsPointerToPointer()) {
      NewV = genPrivatizationAlloca(UDPI, AllocaInsertPt,
                                    ".new");                        //      (2)
      Builder.CreateStore(UpdatedUDPVal, NewV);                     //      (5)
    }
#if INTEL_CUSTOMIZATION
    else if (UDPI->getIsF90DopeVector()) {
      Type *DVType = nullptr;
      Value *NumElements = nullptr;
      std::tie(DVType, NumElements, std::ignore) =
          VPOParoptUtils::getItemInfo(UDPI);
      assert(!NumElements && "use_device_ptr item cannot be an array.");

      NewV = genPrivatizationAlloca(UDPI, AllocaInsertPt,
                                    ".new"); //                             (2)
      genCopyByAddr(UDPI, NewV, OrigV, &*Builder.GetInsertPoint()); //      (7)
      auto *Zero = Builder.getInt32(0);
      auto *Addr0GEP =
          Builder.CreateInBoundsGEP(DVType, NewV, {Zero, Zero}, //          (8)
                                    NewV->getName() + ".addr0");
      Builder.CreateStore(UpdatedUDPVal, Addr0GEP); //                      (5)
    } else if (UDPI->getIsCptr()) {
      NewV = genPrivatizationAlloca(UDPI, AllocaInsertPt,
                                    ".new"); //                             (2)
      PointerType *Int8PtrPtrTy = Builder.getInt8PtrTy()->getPointerTo();
      auto *NewVCast = Builder.CreateBitOrPointerCast(
          NewV, Int8PtrPtrTy, NewV->getName() + ".cast"); //                (9)

      Builder.CreateStore(UpdatedUDPVal, NewVCast); //                      (5)
    }
#endif // INTEL_CUSTOMIZATION

    TgtDataOutlinedFunctionCall->replaceUsesOfWith(OrigV, NewV); //         (6)

    LLVM_DEBUG(
        if (llvm::is_contained(TgtDataOutlinedFunctionCall->operands(), NewV)) {
          dbgs() << __FUNCTION__ << ": Replaced references to use_device_ptr '";
          OrigV->printAsOperand(dbgs());
          dbgs() << "', with '";
          NewV->printAsOperand(dbgs());
          dbgs() << "' in region.\n";
        } else {
          dbgs() << __FUNCTION__ << ": Privatized use_device_ptr operand '";
          OrigV->printAsOperand(dbgs());
          dbgs() << "', as: '";
          NewV->printAsOperand(dbgs());
          dbgs() << "'. But no uses of the original were replaced.\n";
        });
  }
}

// Utilities to construct the assignment to the base pointers, section
// pointers and size pointers if the flag hasRuntimeEvaluationCaptureSize is
// true.
void VPOParoptTransform::genOffloadArraysInitUtil(
    IRBuilder<> &Builder, Value *BasePtr, Value *SectionPtr, Value *Size,
    Value *Mapper, TgDataInfo *Info, SmallVectorImpl<Constant *> &ConstSizes,
    unsigned &Cnt, bool hasRuntimeEvaluationCaptureSize,
    Instruction **BasePtrGEPOut) {

  assert(BasePtr && "Unexpected: BasePtr is null");
  assert(SectionPtr && "Unexpected: SectionPtr is null");

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genOffloadArraysInitUtil:"
                    << " BasePtr=(" << *BasePtr << ") SectionPtr=("
                    << *SectionPtr << ")";
             if (Mapper) {
               dbgs() << " Mapper=(";
               Mapper->printAsOperand(dbgs());
               dbgs() << ")";
             } dbgs()
             << " Cnt=" << Cnt << " ConstSizes.size()=" << ConstSizes.size()
             << " hasRuntimeEvaluationCaptureSize="
             << hasRuntimeEvaluationCaptureSize << "\n");

  Value *NewBPVal, *BP, *P, *S, *SizeValue;
  auto *I8PTy = Builder.getInt8PtrTy();
  auto *I8PArrayTy = ArrayType::get(I8PTy, Info->NumberOfPtrs);

  NewBPVal = genCastforAddr(BasePtr, Builder);
  BP = Builder.CreateConstInBoundsGEP2_32(I8PArrayTy, Info->BaseDataPtrs, 0,
                                          Cnt);
  Builder.CreateStore(NewBPVal, BP);
  if (BasePtrGEPOut)
    *BasePtrGEPOut = cast<Instruction>(BP);

  P = Builder.CreateConstInBoundsGEP2_32(I8PArrayTy, Info->DataPtrs, 0, Cnt);
  NewBPVal = genCastforAddr(SectionPtr, Builder);
  Builder.CreateStore(NewBPVal, P);

  if (UseMapperAPI) {
    Value *MapperGEP =
        Builder.CreateConstInBoundsGEP2_32(I8PArrayTy, Info->Mappers, 0, Cnt);
    if (!Mapper)
      Mapper = ConstantPointerNull::get(I8PTy);
    else
      Info->FoundValidMapper = true;
    Value *MapperCast = genCastforAddr(Mapper, Builder);
    Builder.CreateStore(MapperCast, MapperGEP);
  }

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
    WRegionNode *W, TgDataInfo *Info, Instruction *InsertPt,
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
      isa<WRNTargetVariantNode>(W) || isa<WRNDispatchNode>(W);

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
        // TODO: Use mapper from map-chain
        genOffloadArraysInitUtil(
            Builder, Aggr->getBasePtr(), Aggr->getSectionPtr(), Aggr->getSize(),
            Aggr->getMapper(), Info, ConstSizes, Cnt,
            hasRuntimeEvaluationCaptureSize, I == 0 ? &BasePtrGEP : nullptr);
      }
    } else {
      // TODO: this needs to be cleaned up, after the parsing
      //       code is removed from WRegion analysis code.
      llvm_unreachable("Paropt only supports map chains now.");
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
    SmallVectorImpl<GlobalVariable *> &Names, SmallVectorImpl<Value *> &Mappers,
    bool hasRuntimeEvaluationCaptureSize, Instruction *InsertPtForAllocas) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genOffloadArraysInit:"
                    << " ConstSizes.size()=" << ConstSizes.size()
                    << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");

  Value *BPVal;
  // Insert allocas for offload arrays in a parent region or the parent
  // function's entry block based on whether there are any parent regions that
  // may be outlined. See target-task.ll, target_map_in_loop.ll for examples.
  // If InsertPtForAllocas is not provided (null), get it from
  // VPOParoptUtils::getInsertionPtForAllocas().
  if (!InsertPtForAllocas)
    InsertPtForAllocas =
        VPOParoptUtils::getInsertionPtForAllocas(W, F, /*OutsideRegion=*/true);
  IRBuilder<> AllocaBuilder(InsertPtForAllocas);
  IRBuilder<> Builder(InsertPt);
  unsigned Cnt = 0;
  bool Match = false;
  Value *SizesArray;
  LLVMContext &C = F->getContext();
  Type *I8PTy = Builder.getInt8PtrTy();

  // Build the alloca defs of the target parms.
  if (hasRuntimeEvaluationCaptureSize)
    SizesArray = AllocaBuilder.CreateAlloca(
        ArrayType::get(Type::getInt64Ty(C), Info->NumberOfPtrs), nullptr,
        ".offload_sizes");
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

  AllocaInst *TgBasePointersArray = AllocaBuilder.CreateAlloca(
      ArrayType::get(I8PTy, Info->NumberOfPtrs), nullptr, ".offload_baseptrs");

  AllocaInst *TgPointersArray = AllocaBuilder.CreateAlloca(
      ArrayType::get(I8PTy, Info->NumberOfPtrs), nullptr, ".offload_ptrs");

  Constant *MapTypesArrayInit =
      ConstantDataArray::get(AllocaBuilder.getContext(), MapTypes);
  auto *MapTypesArrayGbl =
      new GlobalVariable(*(F->getParent()), MapTypesArrayInit->getType(),
                         true, GlobalValue::PrivateLinkage, MapTypesArrayInit,
                         ".offload_maptypes", nullptr);
  MapTypesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  GlobalVariable *NamesArray = nullptr;
  if (UseMapperAPI)
    if (llvm::any_of(Names, [](GlobalVariable *C) { return C; })) {
      SmallVector<Constant *, 16> NamesCasted;
      // Create a global mapnames array from individual names:
      // @0 = private unnamed_addr constant [13 x i8] c";y;t.c;4;7;;\00" // (1)
      //
      // @.offload_mapnames = private constant [1 x i8*] [               // (3)
      //   i8* getelementptr inbounds ([13 x i8], [13 x i8]* @0,
      //                               i32 0, i32 0)                     // (2)
      //   ]
      //
      llvm::transform(
          Names, std::back_inserter(NamesCasted),
          [&C](GlobalVariable *N) { //                                      (1)
            assert(N && "Name is null.");
            Constant *Zero = ConstantInt::get(Type::getInt32Ty(C), 0);
            Constant *Indices[] = {Zero, Zero};
            return ConstantExpr::getInBoundsGetElementPtr(N->getValueType(), N,
                                                          Indices); //      (2)
          });

      auto *NamesArrayInit = ConstantArray::get(
          ArrayType::get(I8PTy, NamesCasted.size()), NamesCasted);
      GlobalVariable *NamesArrayGbl =
          new GlobalVariable(*(F->getParent()), NamesArrayInit->getType(), true,
                             GlobalValue::PrivateLinkage, NamesArrayInit,
                             ".offload_mapnames", nullptr); //              (3)
      NamesArray = NamesArrayGbl;
    }

  AllocaInst *TgMappersArray = nullptr;
  if (UseMapperAPI)
    TgMappersArray = AllocaBuilder.CreateAlloca(
        ArrayType::get(I8PTy, Info->NumberOfPtrs), nullptr, ".offload_mappers");

  Info->BaseDataPtrs = TgBasePointersArray;
  Info->DataPtrs = TgPointersArray;
  Info->DataSizes = SizesArray;
  Info->DataMapTypes = MapTypesArrayGbl;
  Info->Names = NamesArray;
  Info->Mappers = TgMappersArray;

  if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W) ||
      isa<WRNTargetVariantNode>(W) || isa<WRNDispatchNode>(W)) {
    genOffloadArraysInitForClause(W, Info, InsertPt, ConstSizes,
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
  for (unsigned II = 0; II < Call->arg_size(); ++II) {
    BPVal = Call->getArgOperand(II);

    Match = false;
    genOffloadArraysInitForClause(W, Info, InsertPt, ConstSizes,
                                  hasRuntimeEvaluationCaptureSize, BPVal, Match,
                                  Builder, Cnt);

    if (!Match)
      genOffloadArraysInitUtil(Builder, BPVal, BPVal,
                               /*Size=*/nullptr, /*Mapper=*/nullptr, Info,
                               ConstSizes, Cnt,
                               hasRuntimeEvaluationCaptureSize);
  }
  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
    genOffloadArraysInitUtil(Builder, W->getParLoopNdInfoAlloca(),
                             W->getParLoopNdInfoAlloca(),
                             /*Size=*/nullptr, /*Mapper=*/nullptr, Info,
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

  auto *I8PTy = Builder.getInt8PtrTy();
  auto *I64Ty = Type::getInt64Ty(C);
  auto *NullI8PP = ConstantPointerNull::get(PointerType::getUnqual(I8PTy));
  auto *NullI64P = ConstantPointerNull::get(PointerType::getUnqual(I64Ty));

  if (Info->NumberOfPtrs) {
    auto *I8PArrayTy = ArrayType::get(I8PTy, Info->NumberOfPtrs);
    auto *I64ArrayTy = ArrayType::get(I64Ty, Info->NumberOfPtrs);

    Info->ResBaseDataPtrs = Builder.CreateConstInBoundsGEP2_32(
        I8PArrayTy, Info->BaseDataPtrs, 0, 0);
    Info->ResDataPtrs =
        Builder.CreateConstInBoundsGEP2_32(I8PArrayTy, Info->DataPtrs, 0, 0);
    Info->ResDataSizes =
        Builder.CreateConstInBoundsGEP2_32(I64ArrayTy, Info->DataSizes, 0, 0);
    Info->ResDataMapTypes = Builder.CreateConstInBoundsGEP2_32(
        I64ArrayTy, Info->DataMapTypes, 0, 0);
    if (UseMapperAPI) {
      // If there are no non-null names/mappers, pass "i8** null" to RTL.
      Info->ResNames = Info->Names ? Builder.CreateConstInBoundsGEP2_32(
                                         I8PArrayTy, Info->Names, 0, 0)
                                   : NullI8PP;

      Info->ResMappers = Info->FoundValidMapper
                             ? Builder.CreateConstInBoundsGEP2_32(
                                   I8PArrayTy, Info->Mappers, 0, 0)
                             : NullI8PP;
    }
  } else {
    Info->ResBaseDataPtrs = NullI8PP;
    Info->ResDataPtrs = NullI8PP;
    if (UseMapperAPI) {
      Info->ResNames = NullI8PP;
      Info->ResMappers = NullI8PP;
    }
    Info->ResDataSizes = NullI64P;
    Info->ResDataMapTypes = NullI64P;
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

  if (LaunderIntrinsicsForRegion.count(W) == 0)
    return false;

  auto &LaunderIntrinsics = LaunderIntrinsicsForRegion[W];
  auto NumLaunderIntrinsicsForW = LaunderIntrinsics.size();
  if (NumLaunderIntrinsicsForW == 0)
    return false;

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Number of launder intrinsics for the region is "
                    << NumLaunderIntrinsicsForW << ".\n");

  DenseMap<Value *, Value *> RenameMap;
  bool Changed = false;

  auto replaceWithOperandZero = [&](CallInst *CI) -> Value * {
    LLVM_DEBUG(dbgs() << "clearLaunderIntrinBeforeRegion: Replacing "
                         "launder intrinsic '";
               CI->printAsOperand(dbgs()); dbgs() << "' with its operand.\n");

    Value *NewV = CI->getOperand(0);
    CI->replaceAllUsesWith(NewV);
    CI->eraseFromParent();
    Changed = true;
    return NewV;
  };

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
    Value *NewV = replaceWithOperandZero(CI);
      RenameMap.insert({V, NewV});
      LaunderIntrinsics.erase(CI);

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

  if (W->canHaveShared()) {
    SharedClause &ShaClause = W->getShared();
    for (SharedItem *ShaI : ShaClause.items()) {
      NewV = removeLaunderIntrinsic(ShaI->getOrig(), true);
      ShaI->setOrig(NewV);
    }
  }

  if (W->canHaveUseDevicePtr()) {
    UseDevicePtrClause const &UDPClause = W->getUseDevicePtr();
    for (UseDevicePtrItem *UDPI : UDPClause.items()) {
      NewV = removeLaunderIntrinsic(UDPI->getOrig(), true);
      UDPI->setOrig(NewV);
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

  // Clear any unhandled launder intrinsics left for the region
  if (LaunderIntrinsics.size() > 0) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Clearing "
                      << LaunderIntrinsics.size()
                      << " unhandled intrinsics.\n");
    for (auto *I : LaunderIntrinsics) {
      auto *CI = cast<CallInst>(I);
      assert(isFenceCall(CI) && "Unexpected value in Launder intrinsics map");
      replaceWithOperandZero(CI);
    }

    LaunderIntrinsicsForRegion[W].clear();
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
/// The same holds for any other constructs requiring function outlining, that
/// use global variables privatized in their parent constructs.
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
bool VPOParoptTransform::genGlobalPrivatizationLaunderIntrin(
    WRegionNode *W, const std::unordered_set<Value *> *ValuesToChange) {
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB =
      SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
  W->setEntryBBlock(NewEntryBB);
  bool Changed = false;
  W->populateBBSet(true); // rebuild BBSet unconditionlly as EntryBB changed

  // For the example in the header comment, this function will create the
  // renamed %1 for the gep on @a, and %a for @a.
  auto createRenamedValueForV = [&](Value *V) {
    IRBuilder<> Builder(EntryBB->getTerminator());
    Value *NewV = Builder.CreateLaunderInvariantGroup(V);
    LaunderIntrinsicsForRegion[W].insert(NewV->stripPointerCasts());
    NewV->setName(V->getName());
    LLVM_DEBUG(dbgs() << "createRenamedValueForV : Renamed '";
               V->printAsOperand(dbgs());
               dbgs() << "' (via launder intrinsic) to: '";
               NewV->printAsOperand(dbgs()); dbgs() << "'.\n");
    return NewV;
  };

  DenseMap<Value *, Value *> RenamedNonPointerCEsMap;
  auto renameNonPointerConstExprVInEntryDirective = [&](Value *V) -> Value * {
    if (!V)
      return V;

    if (ValuesToChange != nullptr &&
        ValuesToChange->find(V) == ValuesToChange->end())
      return V;

    auto VOrigAndNew = RenamedNonPointerCEsMap.find(V);
    if (VOrigAndNew != RenamedNonPointerCEsMap.end())
      return VOrigAndNew->second;

    auto *VCast = dyn_cast<ConstantExpr>(V);
    if (!VCast) {
      RenamedNonPointerCEsMap.insert({V, V});
      return V;
    }

    Instruction *VInst = VCast->getAsInstruction();
    VInst->setName("cexpr.inst");
    VInst->insertBefore(EntryBB->getTerminator());
    W->getEntryDirective()->replaceUsesOfWith(V, VInst);
    RenamedNonPointerCEsMap.insert({V, VInst});

    LLVM_DEBUG(dbgs() << "renameNonPointerConstExprVInEntryDirective: Expr '";
               VCast->printAsOperand(dbgs());
               dbgs() << "' hoisted to Instruction '";
               VInst->printAsOperand(dbgs()); dbgs() << "'.\n");
    return VInst;
  };

  // Map between Original Value V and the renamed value NewV. If no renaming
  // happens, The map will have {V, V}. Use MapVector so that the replacement
  // happens in the same order as the generation of renamed values.
  MapVector<Value *, Value *> RenameMap;
  // Maintain a set of Vs that will be replaced within the region with their
  // corresponding NewVs from RenameMap. For renamed Vs not in this set, the
  // replacement will happen only on the entry directive.
  SmallPtrSet<Value *, 32> ReplaceInFullRegion;

  // Create a renamed value for V if it's a Global or ConstantExpr.
  auto createRenamedValueForGlobalsAndConstExprs =
      [&](Value *V, bool MarkForReplacementInRegion = true) {
        // If the set of ValuesToChange is provided, only change the Values in
        // the Set.
        if (ValuesToChange != nullptr &&
            ValuesToChange->find(V) == ValuesToChange->end())
          return V;

        if (MarkForReplacementInRegion)
          ReplaceInFullRegion.insert(V);

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

      if (ReplaceInFullRegion.find(V) == ReplaceInFullRegion.end()) {
        W->getEntryDirective()->replaceUsesOfWith(V, NewV);
        continue;
      }

      genPrivatizationReplacement(W, V, NewV);
    }
  };

  Value *VNew = nullptr;

  // First, rename If clause and map-size ConstantExprs in the entry directive.
  // Otherwise, when breaking other operands, or map base/section pointers,
  // references to broken const-expr map-sizes in clause items may become
  // obsolete. e.g.
  // -------------------------------------+-------------------------------------
  //         Before                       |      After
  // -------------------------------------+-------------------------------------
  //                                      |  %cexpr.inst =
  //                                      |   i64 ptrtoint(i32* @y to i64)
  //                                      |
  //                                      |  %cexpr.inst1 =
  //                                      |   i1 icmp ne i32* @z, i32* null
  //                                      |
  // "MAP"(i32* @w, i32* %1,              |  "MAP"(i32* @w, i32* %1,
  //      i64 ptrtoint(i32* @y to i64))   |        i64 %cexpr.inst)
  // "MAP"(i32* @x, i32* %1,              |  "MAP"(i32* @x, i32* %1,
  //      i64 ptrtoint(i32* @y to i64))   |        i64 %cexpr.inst)
  //  "IF"(i1 icmp ne i32* @z, i32* null) |  "IF"(i1 %cexpr.inst1)
  //                                      |
  // -------------------------------------+-------------------------------------
  // TODO: If we find other cases that need this, like for array-section bounds/
  // linear step, we'll need to add code for handling them here. However, with
  // the current pipeline, we haven't seen any case for non-pointer
  // constant-exprs that would need laundering via
  // genLaunderIntrinIfPrivatizedInAncestor(). That's because renaming of
  // operands in prepare pass breaks all const-exprs in inner regions that use
  // global operands from outer regions.
  if (W->canHaveIf()) {
    VNew = renameNonPointerConstExprVInEntryDirective(W->getIf());
    W->setIf(VNew);
  }

  VPOParoptUtils::executeForEachItemInClause(
      W->getMapIfSupported(), [&](MapItem *MapI) {
        if (MapI->getIsMapChain())
          for (auto *Aggr : MapI->getMapChain()) {
            VNew = renameNonPointerConstExprVInEntryDirective(Aggr->getSize());
            Aggr->setSize(VNew);
            Changed = true;
          }
      });

  if (W->canHaveMap()) {
    MapClause &MpClause = W->getMap();
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
          VNew = createRenamedValueForGlobalsAndConstExprs(
              Aggr->getSectionPtr(), /*MarkForReplacementInRegion=*/false);
          Aggr->setSectionPtr(VNew);
          VNew = createRenamedValueForGlobalsAndConstExprs(
              Aggr->getBasePtr(), /*MarkForReplacementInRegion=*/false);
          Aggr->setBasePtr(VNew);
        }
      }

      VNew = createRenamedValueForGlobalsAndConstExprs(MapI->getOrig());
      MapI->setOrig(VNew);
    }
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

  if (W->canHaveUseDevicePtr()) {
    UseDevicePtrClause &UseDevPtrClause = W->getUseDevicePtr();
    for (UseDevicePtrItem *Item : UseDevPtrClause.items()) {
      VNew = createRenamedValueForGlobalsAndConstExprs(Item->getOrig());
      Item->setOrig(VNew);
    }
  }

  if (W->canHaveShared()) {
    SharedClause &ShaClause = W->getShared();
    for (SharedItem *ShaI : ShaClause.items()) {
      VNew = createRenamedValueForGlobalsAndConstExprs(ShaI->getOrig());
      ShaI->setOrig(VNew);
    }
  }

  if (W->canHaveLastprivate()) {
    LastprivateClause &LprivClause = W->getLpriv();
    for (LastprivateItem *I : LprivClause.items()) {
      VNew = createRenamedValueForGlobalsAndConstExprs(I->getOrig());
      I->setOrig(VNew);
    }
  }

  if (W->canHaveReduction()) {
    ReductionClause &RedClause = W->getRed();
    for (ReductionItem *I : RedClause.items()) {
      VNew = createRenamedValueForGlobalsAndConstExprs(I->getOrig());
      I->setOrig(VNew);
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

Function *VPOParoptTransform::getOmpGetNumThreadsFunctionIfPresent() {
  return F->getParent()->getFunction("omp_get_num_threads");
}

void VPOParoptTransform::collectOmpNumThreadsCallerInfo() {
  Function *OmpGetNumThreadsFunction = getOmpGetNumThreadsFunctionIfPresent();
  if (!OmpGetNumThreadsFunction)
    return;

  if (NumThreadsCallerInfo.Computed)
    return;

  NumThreadsCallerInfo.Computed = true;

  SmallSetVector<Function *, 16> ToProcess;
  SmallPtrSet<Function *, 16> Processed;

  ToProcess.insert(OmpGetNumThreadsFunction);

  while (!ToProcess.empty()) {
    Function *Current = ToProcess.pop_back_val();

    // We first need to drop dead constant users of the function to avoid
    // false positives when checking whether it's address-taken.
    Current->removeDeadConstantUsers();

    // If any of the candidate function is address-taken, it may not be
    // reasonable to analyze which functions call it. So we can save this
    // information, and skip further analysis.
    const User *AddrTakenBy = nullptr;
    if (Current->hasAddressTaken(&AddrTakenBy)) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Function ";
                 Current->printAsOperand(dbgs(), false);
                 dbgs() << " is address-taken");
      if (AddrTakenBy)
        LLVM_DEBUG(dbgs() << " by: '" << *AddrTakenBy);
      LLVM_DEBUG(dbgs() << "'.\n");

      // No need to compute further.
      NumThreadsCallerInfo.AddressTaken = true;
      return;
    }

    Processed.insert(Current);

    for (const Use &CU : Current->uses()) {
      User *CUU = CU.getUser();

      auto *Call = dyn_cast<CallBase>(CUU);
      if (!Call)
        continue;

      Function *Caller = Call->getFunction();
      if (Processed.count(Caller) != 0 || ToProcess.count(Caller) != 0)
        continue;

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": ";
                 Caller->printAsOperand(dbgs(), false);
                 dbgs() << " may call omp_get_num_threads.\n");

      ToProcess.insert(Caller);
      NumThreadsCallerInfo.PotentialCallers.insert(Caller);
    }
  }
}

bool VPOParoptTransform::mayCallOmpGetNumThreads(WRegionNode *W) {

  auto logAndReturn = [&](bool Flag) {
    LLVM_DEBUG(dbgs() << "mayCallOmpGetNumThreads: Region #" << W->getNumber()
                      << " (" << W->getName()
                      << ") may call omp_get_num_threads: "
                      << (Flag ? "Yes" : "No") << ".\n");
    return Flag;
  };

  Function *OmpGetNumThreadsFunction = getOmpGetNumThreadsFunctionIfPresent();
  if (!OmpGetNumThreadsFunction)
    return logAndReturn(false);

  if (!FrugalNumThreadsSimulation)
    return logAndReturn(true);

  // First we collect the potential callers of omp_get_num_threads. This
  // shouldn't take much compile time since we just walk the list of its users.
  collectOmpNumThreadsCallerInfo();

  // Next, we walk through the region to check if can call either
  // omp_get_num_threads, or one of its callers.

  W->populateBBSet();

  SmallPtrSet<Function *, 8> FunctionsCalledFromRegion;

  for (auto *BB : make_range(W->bbset_begin() + 1, W->bbset_end() - 1)) {
    for (Instruction &I : *BB) {
      auto *CB = dyn_cast<CallBase>(&I);
      if (!CB)
        continue;

      auto *CI = dyn_cast<CallInst>(CB);
      if (!CI) {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Cannot analyze the call '"
                          << *CB << "'.\n");
        return logAndReturn(true);
      }

      if (isa<IntrinsicInst>(CI))
        continue; // Ignore calls to llvm intrinsics.

      auto *CalledF =
          dyn_cast<Function>(CI->getCalledOperand()->stripPointerCasts());
      if (!CalledF) {
        LLVM_DEBUG(dbgs() << __FUNCTION__
                          << ": Cannot get the called function for '" << *CI
                          << "'.\n");
        return logAndReturn(true);
      }

      if (FunctionsCalledFromRegion.count(CalledF))
        continue; // Already seen.

      FunctionsCalledFromRegion.insert(CalledF);

      LibFunc LF;
      if (TLI->getLibFunc(*CalledF, LF)) {
        if (LF == LibFunc_omp_get_num_threads) {
          LLVM_DEBUG(dbgs()
                     << __FUNCTION__
                     << ": The region calls omp_get_num_threads directly.\n");
          return logAndReturn(true);
        }
        LLVM_DEBUG(dbgs() << __FUNCTION__
                          << ": Ignoring the call to library function '"
                          << CalledF->getName() << "' from the region.\n");
        continue; // Ignore library function calls other than
                  // omp_get_num_threads itself. TODO: Check if some other kmpc
                  // library function calls omp_get_num_thread internally.
      }

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": The region calls ";
                 CalledF->printAsOperand(dbgs(), false); dbgs() << ".\n");

      if (CalledF->isDeclaration()) {
        // If we cannot see the definition of a function, we assume that it may
        // call omp_get_num_threads. If this is not sufficient, this kind of
        // optimization would probably need to happen in the openmpopt pass
        // (which is in the inliner's pipeline).
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": ";
                   CalledF->printAsOperand(dbgs(), false);
                   dbgs() << " does not have a definition. It may call "
                             "omp_get_num_threads.\n");
        return logAndReturn(true);
      }

      if (NumThreadsCallerInfo.AddressTaken) {
        LLVM_DEBUG(dbgs() << __FUNCTION__
                          << ": omp_get_num_threads or one of its callers is "
                             "address-taken. ";
                   CalledF->printAsOperand(dbgs(), false);
                   dbgs() << " might be calling it.\n");
        return logAndReturn(true);
      }

      if (NumThreadsCallerInfo.PotentialCallers.count(CalledF) != 0) {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": ";
                   CalledF->printAsOperand(dbgs(), false);
                   dbgs() << " may call omp_get_num_threads.\n");
        return logAndReturn(true);
      }
    }
  }

  LLVM_DEBUG(
      dbgs() << __FUNCTION__
             << ": Didn't find any potential caller of omp_get_num_threads "
                "in the region.\n");
  return logAndReturn(false);
}

bool VPOParoptTransform::isFunctionOpenMPTargetDeclare() const {
  return (F->hasFnAttribute("openmp-target-declare"));
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
  Instruction *AllocaInsertPt =
      VPOParoptUtils::getInsertionPtForAllocas(W, F, /*OutsideRegion=*/true);
  IRBuilder<> AllocaBuilder(AllocaInsertPt);
  IRBuilder<> Builder(F->getContext());

  auto InsertArtificialUseForValue = [&](Value *V) {
    // Emit a sequence, which is easily optimizable by
    // SROA and CSE:
    //     %promote.uses = alloca i8
    //     store i8 ptrtoint (type* @G to i8), i8* %promote.uses
    //
    // Insert the artificial alloca in the entry block of the first parent
    // region what would be outlined, if any, otherwise of the parent function.
    if (!ArtificialAlloca)
      ArtificialAlloca = AllocaBuilder.CreateAlloca(
          Builder.getInt8Ty(), nullptr, "promoted.clause.args");

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
// The construct to match is given by MatchConstruct, and at least one of the
// device architectures must be supported (ie, in enum DeviceArch).
//
// On a match, besides returning the variant function name, it also stores in
// the output parameter 'DeviceArchs' a bit vector representing supported
// device architectures that are listed in the string attribute.  If a match is
// not found, it returns a null string and 'DeviceArchs' may be undefined.
//
// To support OMP5.1 declare variant, it also returns strings for
// need_device_ptr and interop.
//
// TODO: For the case of MatchConstruct=="dispatch", we will support
//       multiple variants and multiple interop objs. For that purpose, the
//       outputs DeviceArchs, NeedDevicePtrStr and InteropStr will become
//       SmallVectors to represent the variants.
#if INTEL_CUSTOMIZATION
//       This support is currently not needed by MKL.
#endif // INTEL_CUSTOMIZATION
StringRef VPOParoptTransform::getVariantInfo(
    WRegionNode *W, CallInst *BaseCall, StringRef &MatchConstruct,
    uint64_t &DeviceArchsOut, std::optional<uint64_t> &InteropPositionOut,
    StringRef &NeedDevicePtrStrOut, StringRef &InteropStrOut) {

  assert(BaseCall && "BaseCall is null");
  Function *BaseFunc = BaseCall->getCalledFunction();
  if (!BaseFunc) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": BaseCall->getCalledFunction() returned null\n");
    return "";
  }

  StringRef VariantAttributeString =
      BaseFunc->getFnAttribute("openmp-variant").getValueAsString();

  auto emitWarning = [](WRegionNode *W, const Twine &Message) {
    Function *F = W->getEntryDirective()->getFunction();
    DiagnosticInfoOptimizationFailure DI("openmp", "implementation-warning",
                                         W->getEntryDirective()->getDebugLoc(),
                                         W->getEntryBBlock());
    DI << Message.str();
    F->getContext().diagnose(DI);
  };

  // Counts the number of declare variants that match the 'dispatch'
  // or 'target variant dispatch' construct specified in MatchConstruct
  auto CountDeclareVariantsForDispatch =
      VariantAttributeString.count("construct:" + MatchConstruct.str());

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
  //   "name:foo_gpu;construct:target_variant_dispatch;arch:gen9,XeHP;;
  //    name:foo_xxx;construct:parallel;arch:xxx"
  //
  // For dispatch construct, multiple dispatch variants may occur. E.g.,
  //   "name:foo_gpu1;construct:dispatch;arch:gen9;need_device_ptr:F,F;;
  //    name:foo_gpu2;construct:dispatch;arch:XeLP;need_device_ptr:T,T"
  //
  // We want to find the <variant> whose "construt" field's <value> is
  // MatchConstruct and whose "arch" field's <value> contains at least one
  // architecture supported in enum DeviceArch.
  // If such a <variant> is found, return the string from its "name" field
  // and update the output parameter 'DeviceArchs'.

  SmallVector<StringRef, 1> Variants;  // holds <variant> substrings
  SmallVector<StringRef, 3> Fields;    // holds <field>:<value> substrings
  SmallVector<StringRef, 2> FV;        // FV[0]= <field>; FV[1]= <value>
  StringRef VariantNameOut;            // string to return
  bool FoundConstruct = false;
  bool FoundArch = false;
  bool FoundName = false;
  uint64_t DeviceArchs = 0u;

  auto matchDeviceArch = [&DeviceArchs](StringRef &ArchList) {
    // ArchList is of the form <arch>[,<arch>[,<arch>...]]
    // Split it to extract each arch, and update DeviceArchs.
    SmallVector<StringRef, 2> Arch;
    ArchList.split(Arch, ",");
    DeviceArchs = 0;
    for (unsigned I = 0; I < Arch.size(); ++I) {
      if (Arch[I] == "gen")
        DeviceArchs |= DeviceArch_Gen9 | DeviceArch_XeLP | DeviceArch_XeHP |
                       DeviceArch_XeHPG;  // 0xf
      else if (Arch[I] == "gen9")
        DeviceArchs |= DeviceArch_Gen9;   // 0x1
      else if (Arch[I] == "XeLP")
        DeviceArchs |= DeviceArch_XeLP;   // 0x2
      else if (Arch[I] == "XeHP")
        DeviceArchs |= DeviceArch_XeHP;   // 0x4
      else if (Arch[I] == "XeHPG")
        DeviceArchs |= DeviceArch_XeHPG;  // 0x8
      else if (Arch[I] == "x86_64")
        DeviceArchs |= DeviceArch_x86_64; // 0x100
    }
    return DeviceArchs;
  };

  // Split VariantAttributeString so that each <variant> substring is
  // separately stored in the Variants vector
  VariantAttributeString.split(Variants, ";;");

  // Inspect each <variant> to find the "construct" and "arch" of interest.
  for (StringRef &Variant : Variants) {

    // LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Variant: " << Variant << "\n");

    // We only support one interop in append_args. Abort if >1 is found.
    // TODO: support multiple interop objs
    auto CountInterop = Variant.count("interop:");
    if (CountInterop > 1) {
      F->getContext().diagnose(DiagnosticInfoUnsupported(*F,
          "Found multiple interop in append_args. This is still unsupported.",
          W->getEntryDirective()->getDebugLoc()));
      return "";
    }

    // Split Variant so that each <field>:<value> substring is separately
    // stored in the Fields vector
    Fields.clear();
    Variant.split(Fields, ";");

    FoundConstruct = false;
    FoundArch = false;
    FoundName = false;
    StringRef VariantName;
    StringRef InteropStr;
    StringRef NeedDevicePtrStr;
    DeviceArchs = 0u;
    uint64_t InteropPosition = 0u;

    for (StringRef &Field : Fields) {
      // LLVM_DEBUG(dbgs() << __FUNCTION__ << ":   Field: " << Field << "\n");

      // Split Field so that FV[0] has <field> and FV[1] has <value>
      FV.clear();
      Field.split(FV, ":");
      assert(FV.size() == 2 &&
             "Malformed <field>:<value> in openmp-variant attribute");
      if (FV[0] == "construct") {
        // The same function can have multiple variants matching different
        // constructs (e.g. dispatch and variant dispatch).
        if (FV[1] != MatchConstruct)
          break;
        FoundConstruct = true;
      } else if (FV[0] == "arch" && matchDeviceArch(FV[1])) {
        FoundArch = true;
      } else if (FV[0] == "name") {
        VariantName = FV[1];
        FoundName = true;
      } else if (FV[0] == "need_device_ptr") {
        NeedDevicePtrStr = FV[1];
      } else if (FV[0] == "interop") {
        InteropStr = FV[1];
      } else if (FV[0] == "interop_position") {
        uint64_t Position = 0u;
        if (FV[1].getAsInteger(10, Position)) {
          // getAsInteger() returns true on error
          llvm_unreachable("Interop position must be an unsigned integer");
        } else if (Position > 0) {
          InteropPosition = Position;
        } else {
          llvm_unreachable("Interop position must be positive");
        }
      } else {
        F->getContext().diagnose(DiagnosticInfoUnsupported(
            *F, "Found unsupported field in the openmp-variant attribute.",
            W->getEntryDirective()->getDebugLoc()));
        llvm_unreachable(
            "Found unsupported field in the openmp-variant attribute.");
      }
    } // for (StringRef &Field : Fields)

    if (FoundConstruct) {
      if (FoundName) {
        // Found a variant matching the construct. Update the output fields.
        VariantNameOut = VariantName;
        DeviceArchsOut = DeviceArchs;
        NeedDevicePtrStrOut = NeedDevicePtrStr;
        InteropStrOut = InteropStr;
        if (InteropPosition > 0)
          InteropPositionOut.emplace(InteropPosition);

        if (CountDeclareVariantsForDispatch > 1) {
          emitWarning(W, "Found multiple variants for " + MatchConstruct +
                             ". Only one will be used in the current "
                             "implementation. The variant to be used is '" +
                             VariantName + "'");
        }
        break;
      }
      // found <variant> with matching construct, but without a
      // "name" field. It must be corrupt.
      llvm_unreachable("No variant function name in openmp-variant attribute");
    }
  } // for (StringRef &Variant : Variants)

  if (FoundConstruct && FoundName) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Found variant function: " << VariantNameOut);
    if (FoundArch)
      LLVM_DEBUG(dbgs() << " and device bits "
                        << llvm::format_hex(DeviceArchsOut, 6, true));
    else
      LLVM_DEBUG(dbgs() << " with no device arch specified");
  } else {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Variant function not found");
  }
  LLVM_DEBUG(dbgs() << " for construct '" << MatchConstruct << "'\n");
  return VariantNameOut;
}

// This interface is for target variant dispatch, which does not need
// NeedDevicePtrStr and InteropStr
StringRef VPOParoptTransform::getVariantInfo(
    WRegionNode *W, CallInst *BaseCall, StringRef &MatchConstruct,
    uint64_t &DeviceArchs, std::optional<uint64_t> &InteropPositionOut) {

  StringRef NeedDevicePtrStr; // unused
  StringRef InteropStr;       // unused
  return getVariantInfo(W, BaseCall, MatchConstruct, DeviceArchs,
                        InteropPositionOut, NeedDevicePtrStr, InteropStr);
}

static Value *genDeviceNum(WRegionNode *W, Instruction *InsertPt,
                           Value *InteropClauseObj) {
  Value *DeviceNum = W->getDevice();

  if (DeviceNum) {
    assert(!DeviceNum->getType()->isPointerTy() &&
           "DeviceID should not be a pointer");
    return VPOParoptUtils::encodeSubdevice(W, InsertPt, DeviceNum);
  }
  // else device clause is unspecified.
  //  - If interop clause is specified, get device num from it;
  //  - otherwise, use the default device.

  if (InteropClauseObj)
    return VPOParoptUtils::genOmpGetInteropDeviceNum(InteropClauseObj,
                                                     InsertPt);
  IRBuilder<> Builder(InsertPt);
  IntegerType *Int64Ty = Builder.getInt64Ty();
  return Builder.CreateZExt(VPOParoptUtils::genOmpGetDefaultDevice(InsertPt),
                            Int64Ty);
}

// Emit code to check for device availability, and return %available:
//
//   %call = call i32 @__tgt_is_device_available(i64 %0, i8* DeviceType) (1)
//   %available  = icmp ne i32 %call, 0                                  (2)
//
// For dispatch construct with a NOVARIANTs(i8 %novariants) clause, emit
//
//   %call = call i32 @__tgt_is_device_available(i64 %0, i8* DeviceType) (1)
//   %1  = icmp ne i32 %call, 0                                          (2)
//   %dovariants = icmp eq i1 %tobool1, false                            (3)
//   %available = and i1 %1, %dovariants                                 (4)
//
// The second argument of __tgt_is_device_available() is a void* that
// carries device type information. Currently it is a bit vector
// representing devices listed in enum DeviceArch.
// Having a null DeviceType will match any device type.
static Value *genDeviceAvailable(WRegionNode *W, Instruction *InsertPt,
                                 Value *DeviceNum, uint64_t DeviceArchs) {
  IRBuilder<> Builder(InsertPt);
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  IntegerType *Int32Ty = Builder.getInt32Ty();
  ConstantInt *ValueZero = ConstantInt::get(Int32Ty, 0);

  Value *DeviceType;
  if (DeviceArchs) {
    const DataLayout &DL = InsertPt->getModule()->getDataLayout();
    const unsigned PtrSz = DL.getPointerSizeInBits();
    Value *DeviceArchVal;
    if (PtrSz >= 64) {
      DeviceArchVal = Builder.getInt64(DeviceArchs);
    } else { // PtrSz <= 63
      assert(DeviceArchs < (1LLu << PtrSz) &&
             "Bit vector size exceeds pointer size");
      DeviceArchVal = Builder.getIntN(PtrSz, DeviceArchs);
    }
    DeviceType = Builder.CreateIntToPtr(DeviceArchVal, Int8PtrTy);
  } else {
    // when no device arch specified, using a null ptr for DeviceType
    // tells __tgt_is_device_available to match any device type
    DeviceType = ConstantPointerNull::get(Int8PtrTy);
  }

  CallInst *IsDeviceAvailable =            //                               (1)
      VPOParoptUtils::genTgtIsDeviceAvailable(DeviceNum, DeviceType, InsertPt);
  Value *Available =                       //                               (2)
      Builder.CreateICmpNE(IsDeviceAvailable, ValueZero);

  if (isa<WRNDispatchNode>(W)) {
    Value *Novariants = W->getNovariants();
    if (Novariants != nullptr) {
      unsigned BitWidth = Novariants->getType()->getIntegerBitWidth();
      // dovariants = (novariants==0)
      Value *Dovariants = Builder.CreateICmpEQ(
          Novariants, Builder.getIntN(BitWidth, 0), "dovariants"); //       (3)
      Available = Builder.CreateAnd(Available, Dovariants); //              (4)
    }
  }

  Available->setName("available");
  return Available;
}

// To support asynchronous execution for a TARGET VARIANT DISPATCH NOWAIT
// construct, an AsyncObj is created and passed to @__tgt_create_interop_obj().
// Its type is that of a struct of this form:
//
//      struct AsyncObjTy { // struct size = 8+8+4 = 20 on 64bit arch
//        void* shareds;    // unused
//        void* task_entry; // unused
//        int   part_id;    // unused
//      };
//
// The AsyncObjTy is actually a __kmp_task_t struct.
//
// The AsyncObj is allocated by calling __kmpc_omp_task_alloc, with
// the proxy flag bit set (WRNTaskFlag::Proxy is 0x10).
static Value *createAsyncObj(WRegionNode *W, Value *DeviceNum,
                             StructType *IdentTy, Instruction *InsertPt) {
  Function *F = InsertPt->getFunction();
  LLVMContext &C = F->getContext();
  const DataLayout &DL = F->getParent()->getDataLayout();

  IRBuilder<> Builder(InsertPt);
  IntegerType *Int32Ty = Builder.getInt32Ty();
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  Value *Zero = Builder.getInt32(0);
  ConstantPointerNull *NullPtr = ConstantPointerNull::get(Int8PtrTy);

  // Build the struct for AsyncObjTy:
  //
  //   %__struct.AsyncObj = type { i8*, i8*, i32 }

  Type *AsyncObjTyFields[] = {Int8PtrTy, // 0: unused: shareds
                              Int8PtrTy, // 1: unused: task_entry
                              Int32Ty};  // 2: unused: part_id
  StructType *AsyncObjTy =
      StructType::create(C, AsyncObjTyFields, "__struct.AsyncObj", false);
  int AsyncObjTySize = DL.getTypeAllocSize(AsyncObjTy);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": AsyncObjTy: " << *AsyncObjTy
                    << "; Size = " << AsyncObjTySize << " bytes\n");

  // Create AsyncObj by calling __kmpc_omp_task_alloc(). The call looks like:
  //
  //   %asyncobj = call i8* @__kmpc_omp_task_alloc(
  //                        %__struct.ident_t* @.kmpc_loc.0.0,
  //                        i32 0,      // unused
  //                        i32 16,     // "proxy" flag 0x10
  //                        i64 20,     // sizeof(AsyncObjTy) = 8+8+4 = 20
  //                        i64 0,      // sizeof(shareds_t) = 0
  //                        i8* null)   // unused

  CallInst *AsyncObj = VPOParoptUtils::genKmpcTaskAllocForAsyncObj(
      W, IdentTy, AsyncObjTySize, InsertPt);
  assert(AsyncObj && "AsyncObj not created for Target Variant Dispatch Nowait");
  AsyncObj->setName("asyncobj"); // void* pointer
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": AsyncObj: " << *AsyncObj << "\n");

  // Create a base pointer to AsyncObj by casting the void* ptr to AsyncObjTy*
  // like this:
  //   %asyncobj.ptr = bitcast i8* %asyncobj to %__struct.AsyncObj*
  Value *AsyncObjPtr = Builder.CreateBitCast( // base pointer to AsyncObjTy
      AsyncObj, PointerType::getUnqual(AsyncObjTy), "asyncobj.ptr");

  // Initialize fields 1 and 2 of AsyncObj. Field 0 is already initialized
  // by the runtime.

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

  assert(InteropObj && "InteropObj not created for [Target Variant] Dispatch");

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": InteropObj: " << *InteropObj << "\n");

  return InteropObj;
}

bool VPOParoptTransform::genInteropCode(WRegionNode* W) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genInteropCode\n");

  W->populateBBSet();

  Value *DeviceNum = W->getDevice();
  InteropActionClause &ActionClause = W->getInteropAction();
  DependClause const &DepClause = W->getDepend();
  bool IsAsync = W->getNowait();

  BasicBlock *BranchBB = createEmptyPrivInitBB(W);

  Instruction *InsertPt = BranchBB->getTerminator();
  IRBuilder<> Builder(InsertPt);
  IntegerType *Int64Ty = Builder.getInt64Ty();

  if (IsAsync) {
    OptimizationRemarkMissed R("openmp", "Interop", W->getEntryDirective());
    R << "Nowait clause on interop construct was ignored (not yet "
         "supported).";
    ORE.emit(R);
  }

  if (!DeviceNum)
    DeviceNum = VPOParoptUtils::genOmpGetDefaultDevice(InsertPt);

  DeviceNum = Builder.CreateZExtOrTrunc(DeviceNum, Int64Ty);
  assert(DeviceNum->getType()->isIntegerTy(64) && "DeviceID is not an i64.");

  CallInst *TaskAllocCI = VPOParoptUtils::genKmpcTaskAllocWithoutCallback(
      W, IdentTy, TidPtrHolder, InsertPt);

  if (!DepClause.empty() || W->getDepArray()) {
    AllocaInst *DummyTaskTDependRec = genDependInitForTask(W, InsertPt);
    genTaskDeps(W, IdentTy, TidPtrHolder, /*TaskAlloc=*/nullptr,
                DummyTaskTDependRec, InsertPt, true);
  }

  assert(!ActionClause.empty() && "Interop construct has no action clause");
  VPOParoptUtils::genKmpcTaskBeginIf0(W, IdentTy, TidPtrHolder, TaskAllocCI,
                                      InsertPt);

  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  PointerType *Int8PtrPtrTy = Int8PtrTy->getPointerTo();

  for (auto *Item : ActionClause.items()) {
    Value *InteropVarAddr = Item->getOrig();
    auto *InteropVarAddrCast = Builder.CreateBitOrPointerCast(
        InteropVarAddr, Int8PtrPtrTy,
        InteropVarAddr->getName() + "interop.addr.cast");
    if (Item->getIsInit()) {
      CallInst *Interop = VPOParoptUtils::genTgtCreateInterop(
          DeviceNum, Item->getIsTarget() ? 0 : 1, Item->getPreferList(),
          InsertPt);
      Builder.CreateStore(Interop, InteropVarAddrCast);
    } else if (Item->getIsDestroy() || Item->getIsUse()) {
      Value *InteropVar =
          Builder.CreateLoad(Int8PtrTy, InteropVarAddrCast,
                             InteropVarAddr->getName() + "interop.obj.val");
      if (Item->getIsDestroy()) {
        VPOParoptUtils::genTgtReleaseInterop(
            InteropVar, InsertPt, /*EmitTgtReleaseInteropObj=*/false);
        Builder.CreateStore(ConstantPointerNull::get(Int8PtrTy),
                            InteropVarAddrCast);
      } else { // Item->getIsUse() is true
        if (DispatchCodegenVersion == 0)
          VPOParoptUtils::genTgtUseInterop(InteropVar, InsertPt);
        else // DispatchCodegenVersion > 0
          VPOParoptUtils::genTgtInteropUseAsync(W, IdentTy, TidPtrHolder,
                                                InteropVar, IsAsync, InsertPt);
      }
    } else {
      llvm_unreachable("Unexpected interop action clause item type");
    }
  }
  VPOParoptUtils::genKmpcTaskCompleteIf0(W, IdentTy, TidPtrHolder, TaskAllocCI,
                                         InsertPt);
  return true;
}

/// Reuse target data logic to get device pointers from runtime.
///   (A) Emit call to __tgt_target_data_begin/end() to get device pointers
///       corresponding to use_device_ptr operands.
///   (B) Replace args in the foo_variant() call to use the above mentioned
///       device pointers.
/// For WRNTargetVariantNode, VariantCall is its VariantWrapperCall
/// For WRNDispatchNode, VariantCall is the variant dispatch call
/// Task (A) is done here in VPOParoptTransform::genTargetVariantDispatchCode()
/// Task (B) is done in useUpdatedUseDevicePtrsInTgtDataRegion()
//  See the header comment of useUpdatedUseDevicePtrsInTgtDataRegion() for
//  examples of the replacement of use_device_ptr operands with a private version
//  containing the device value.
void VPOParoptTransform::getAndReplaceDevicePtrs(
    WRegionNode *W, CallInst *VariantCall, Instruction *InsertPtForAllocas) {

  assert((isa<WRNTargetVariantNode>(W) || isa<WRNDispatchNode>(W)) &&
         "getAndReplaceDevicePtrs called for non-dispatch region.");

  UseDevicePtrClause &UDPtrClause = W->getUseDevicePtr();
  if (UDPtrClause.empty())
    return;

  TgDataInfo Info;
  Info.NumberOfPtrs = UDPtrClause.size();
  bool hasRuntimeEvaluationCaptureSize = false;
  SmallVector<Constant *, 16> ConstSizes;
  SmallVector<uint64_t, 16> MapTypes;
  SmallVector<GlobalVariable *, 16> Names;
  SmallVector<Value *, 16> Mappers;
#if INTEL_CUSTOMIZATION
  SmallVector<bool, 16> IsWILocalFirstprivate;
#endif // INTEL_CUSTOMIZATION
  SmallVector<bool, 16> IsFunctionPtr;

  (void)addMapForUseDevicePtr(W, VariantCall);

  genTgtInformationForPtrs(W, nullptr, ConstSizes, MapTypes, Names, Mappers,
#if INTEL_CUSTOMIZATION
                           IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                           IsFunctionPtr, hasRuntimeEvaluationCaptureSize);

  CallInst *DummyCall = nullptr;
  genOffloadArraysInit(W, &Info, DummyCall, VariantCall, ConstSizes, MapTypes,
                       Names, Mappers, hasRuntimeEvaluationCaptureSize,
                       InsertPtForAllocas);

  genOffloadArraysArgument(&Info, VariantCall);

  (void)VPOParoptUtils::genTgtTargetDataBegin(
      W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
      Info.ResDataSizes, Info.ResDataMapTypes, Info.ResNames, Info.ResMappers,
      VariantCall); //                                                      (A)

  (void)VPOParoptUtils::genTgtTargetDataEnd( //                             (A)
      W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
      Info.ResDataSizes, Info.ResDataMapTypes, Info.ResNames, Info.ResMappers,
      VariantCall->getNextNonDebugInstruction());

  useUpdatedUseDevicePtrsInTgtDataRegion(W, VariantCall); //                (B)
}

// Find the dispatch call in the dispatch construct,
// remove its QUAL_OMP_DISPATCH_CALL marker, and save it with W->setCall()
static bool findDispatchCall(WRegionNode *W) {
  bool found = false;
  for (auto *BB : make_range(W->bbset_begin() + 1, W->bbset_end() - 1)) {
    if (found)
      break;
    for (Instruction &I : *BB) {
      if (auto *CI = dyn_cast<CallInst>(&I)) {
        CallInst *NewCI =
            VPOUtils::removeOpenMPClausesFromCall(CI, {QUAL_OMP_DISPATCH_CALL});
        if (NewCI != CI) {
          // CI was the dispatch call, now replaced by
          // NewCI which has QUAL_OMP_DISPATCH_CALL removed
          W->setCall(NewCI);
          found = true;
          break;
        }
      }
    }
  }
  // TODO: remove the check for WRNDispatchNode in the assert below
  //       once TARGET VARIANT DISPATCH also has the the base call always
  //       marked with QUAL_OMP_DISPATCH_CALL
  assert((!isa<WRNDispatchNode>(W) || found) && "Dispatch call not found");
  return found;
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
/// If the device clause is absent, the default dnum is
/// omp_get_default_device().
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
///   i32 dispatch = __tgt_is_device_available(dnum, nullptr)
///   if (dispatch == true)
///      foo_variant(<args>);
///   else
///      foo(<args>);
/// \endcode
///
/// IR:
///    define internal void @foo_gpu.wrapper(<args>) {
///      ...
///      %interop = call i8* @__tgt_create_interop_obj(...)                (7)
///      %variant = call i32 @foo_gpu(<args>);                             (8)
///      call i32 @__tgt_release_interop_obj(%interop)                     (9)
///      ...
///    }
///
///    %0 = load i32, i32* @dnum, align 4
///    %1 = sext i32 %0 to i64
///    %call = call i32 @__tgt_is_device_available(i64 %0, i8* null)       (1)
///    %available = icmp ne i32 %call, 0                                   (2)
///    br i1 %available, label %variant.call, label %base.call             (3)
///
///  variant.call:                                                         (4)
///    call @foo_gpu.wrapper(<args>)                                       (5)
///    br label %if.end
///
///  base.call:                                                            (6)
///    %call = call i32 @foo(<args>)
///    br label %if.end
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
/// The compiler emits extra code (on top of Case1) as follows:
///
///   (A) Emit call to __tgt_target_data_begin/end() to get device pointers
///       corresponding to use_device_ptr operands.
///   (B) Replace args in the foo_variant() call to use the above mentioned
///       device pointers.
/// Pseudocode for the extra code looks like this:
/// \code
///
///   @.offload_maptypes = ... [64, 64] // TGT_RETURN_PARAM
///   @.offload_sizes = ... [0, 0]
///   ...
///   define internal void @foo_gpu.wrapper(i8* %a, i8* %b) {
///     ...
///     %variant = call i32 @foo_gpu(%a, %b);                              (8)
///     ...
///   }
///   ...
///   %a.map.gep = getelementptr(%offload_baseptrs, 0, 0)
///   %b.map.gep = getelementptr(%offload_baseptrs, 0, 1)
///   store i8* %a, i8** %a.map.gep
///   store i8* %b, i8** %b.map.gep
///
///   call void @__tgt_target_data_begin(..., @.offload_sizes,
///                                           @.offload_maptypes)
///   %a_updated = load i8*, i8** %a.map.gep
///   %b_updated = load i8*, i8** %a.map.gep
///
///   call foo_gpu.wrapper(%a_updated, %b_updated);                        (5)
///   call void @__tgt_target_data_end(...)
/// \endcode
///
/// Task (A) and (B) are done in getAndReplaceDevicePtrs()
bool VPOParoptTransform::genTargetVariantDispatchCode(WRegionNode *W) {
  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genTargetVariantDispatchCode\n");
  W->populateBBSet();

  // The first and last BasicBlocks contain the region.entry/exit calls.
  // There may be many call instructions in this region like memcpy but there
  // should be only one call instruction with the following string attribute.
  // "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen"
  // where the variant name is "foo_gpu" in this example.
  // All other instructions in the region are ignored.

  StringRef MatchConstruct("target_variant_dispatch");
  StringRef VariantName;
  uint64_t DeviceArchs = 0u; // bit vector of device architectures
  std::optional<uint64_t> InteropPosition =
      std::nullopt; // position of interop arg in variant call

  CallInst *BaseCall = nullptr;

  // find the dispatch call and remove its QUAL_OMP_DISPATCH_CALL OB from IR
  findDispatchCall(W);
  BaseCall = W->getCall();
  if (BaseCall) {
    VariantName = getVariantInfo(W, BaseCall, MatchConstruct, DeviceArchs,
                                 InteropPosition);
  } else {
    // TODO: remove this ELSE part and the IF-check above when the
    //       dispatch call is always marked with "QUAL.OMP.DISPATCH.CALL".
    for (auto *BB : make_range(W->bbset_begin()+1, W->bbset_end()-1)) {
      for (Instruction &I : *BB) {
        if (auto *TempCallInst = dyn_cast<CallInst>(&I)) {
          BaseCall = TempCallInst;
          VariantName = getVariantInfo(W, BaseCall, MatchConstruct, DeviceArchs,
                                       InteropPosition);
          if (!VariantName.empty()) {
            break;
          }
        }
      }
      if (!VariantName.empty())
        break;
    }
  }

  auto emitRemark = [&](const StringRef &Message) {
    OptimizationRemarkMissed R("openmp", "Region", W->getEntryDirective());
    R << ore::NV("Construct", W->getName()) << Message;
    ORE.emit(R);
  };

  assert(BaseCall && "Base call not found in Target Variant Dispatch");
  if (!BaseCall) {
    emitRemark(" Could not find a valid function call in the region");
    return false;
  }

  if (VariantName.empty()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Variant function not found\n");
    emitRemark(" Could not find a matching variant function");
    return false;
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Found variant function name: "
                    << VariantName << "\n");

  // Initialize types and constants
  BasicBlock *BranchBB = createEmptyPrivInitBB(W);
  BasicBlock *EndVariantsBB = createEmptyPrivFiniBB(W);

  Instruction *InsertPt = BranchBB->getTerminator();
  IRBuilder<> Builder(InsertPt);

  Value *DeviceNum = genDeviceNum(W, InsertPt, /*InteropClauseObj=*/ nullptr);

  // Emit dispatch condition:
  //   %call = call i32 @__tgt_is_device_available(i64 %0, i8* DeviceType)  (1)
  //   %available  = icmp ne i32 %call, 0                                   (2)
  Value *Available = genDeviceAvailable(W, InsertPt, DeviceNum, DeviceArchs);

  // Emit dispatch code:
  //
  //   br i1 %available, label %variant.call, label %base.call              (3)
  //
  // variant.call:                                                          (4)
  //   < call to target_data_begin, and maps to get device pointers >       (A)
  //   call void @foo_gpu.wrapper(<args>)                               (5) (B)
  //   < call to target_data_end() >                                        (A)
  //   br label %if.end
  //
  // base.call:                                                             (6)
  //   %call = call i32 @foo(<args>)
  //   br label %if.end
  //
  ValueToValueMapTy VMap;
  SmallVector<BasicBlock *, 32> BBSet;
  VPOUtils::singleRegionMultiVersioning(BranchBB->getSingleSuccessor(),
                                        EndVariantsBB, BBSet, VMap, Available,
                                        DT); //                     (3) (4) (6)

  // Create the interop object for
  // (1) Asynchronous case (i.e., NOWAIT is present), or
  // (2) Synchronous case when "UseInterop" flag is true.
  //     If the flag is false, then the synchronous case will revert to
  //     the old implementation without using interop_obj.
  //     TODO: remove the old implementation when the new one if fully tested.
  Value *InteropObj = nullptr;
  if (UseInterop || W->getNowait())
    InteropObj = createInteropObj(W, DeviceNum, IdentTy, BaseCall); //      (7)

  bool IsVoidType = (BaseCall->getType() == Builder.getVoidTy());
  uint64_t InteropPositionIfEmitted = 0; // will be updated by genVariantCall()
  CallInst *VariantCall = VPOParoptUtils::genVariantCall(
      BaseCall, VariantName, InteropObj, InteropPosition,
      InteropPositionIfEmitted, BaseCall, W); //                            (8)
  if (!IsVoidType)
    VariantCall->setName("variant");

  // Release the interop object for synchronous execution (no NOWAIT clause).
  // Don't do this for the asynchronous case; the async_handler will do it.
  if (InteropObj != nullptr && W->getNowait() == false)
    VPOParoptUtils::genTgtReleaseInterop(InteropObj, BaseCall, true); //    (9)

  // BaseCall's original arguments before outlining the call in WrapperFn are
  // used to find corresponding arguments in VariantWrapperCall in order to
  // propagate the ByVal and alignment attributes.
  SmallVector<Value *, 4> BaseArgs(BaseCall->args());

  BaseCall->replaceAllUsesWith(VariantCall);
  assert(BaseCall->use_empty());

  Function *WrapperFn = VPOParoptUtils::genOutlineFunction(
      *W, DT, AC, ArrayRef(BBSet), (VariantName + ".wrapper").str()); //    (5)
  CallInst *VariantWrapperCall = cast<CallInst>(WrapperFn->user_back());

  // BaseCall may have arguments with the ByVal attribute, with or without an
  // alignment attribute. Propagate such attributes to the corresponding
  // arguments in VariantWrapperCall and the corresponding formal parameters in
  // WrapperFn. Example:
  // - BaseCall:
  //          call void @foo1(%struct.A* byval(%struct.A) align 8 %AAA)
  // - WrapperCall before:
  //          call void @foo2.wrapper(%struct.A* %AAA)
  // - WrapperCall after:
  //          call void @foo2.wrapper(%struct.A* byval(%struct.A) align 8 %AAA)
  LLVMContext &C = Builder.getContext();
  for (unsigned ArgNum = 0; ArgNum < BaseCall->arg_size(); ++ArgNum) {
    if (BaseCall->isByValArgument(ArgNum)) {
      Value *BaseArg = BaseArgs[ArgNum];
      MaybeAlign MayAln = BaseCall->getParamAlign(ArgNum);
      Align Aln = MayAln.valueOrOne();
      unsigned Alignment = Aln.value();
      // Find BaseArg in VariantWrapperCall to propagate its ByVal and Alignment
      // attributes to both the wrapper call and the wrapper function.
      for (unsigned WrapperArgNum = 0;
           WrapperArgNum < VariantWrapperCall->arg_size();
           ++WrapperArgNum) {
        Value *WrapperArg = VariantWrapperCall->getArgOperand(WrapperArgNum);
        if (BaseArg == WrapperArg) {
          assert(isa<PointerType>(
                     VariantWrapperCall->getFunctionType()->getParamType(
                         WrapperArgNum)) &&
                 "Byval expects a pointer type");
          VariantWrapperCall->addParamAttr(
              WrapperArgNum, Attribute::getWithByValType(
                                 C, BaseCall->getParamByValType(ArgNum)));
          WrapperFn->addParamAttr(WrapperArgNum,
                                  Attribute::getWithByValType(
                                      C, BaseCall->getParamByValType(ArgNum)));
          if (Alignment > 1) {
            VariantWrapperCall->addParamAttr(
                WrapperArgNum,
                Attribute::getWithAlignment(C, Align(Alignment)));
            WrapperFn->addParamAttr(WrapperArgNum, Attribute::getWithAlignment(
                                                       C, Align(Alignment)));
          }
          break; // skip remaining args in VariantWrapperCall
        }
      } // for WrapperArgNum
    }   // if
  }     // for ArgNum

  BaseCall->eraseFromParent();
  getAndReplaceDevicePtrs(W, VariantWrapperCall);

  UseDevicePtrClause &UDPC = W->getUseDevicePtr();
  UDPC.clear(); // Clear UDPC of old info from the use_device_ptr clause,
                // then populate it based on the need_device_ptr clause.
  processNeedDevicePtrClause(W, VariantCall, InteropPositionIfEmitted);

  // The target info arrays emitted by genOffloadArraysInit() to handle the
  // need_device_ptr clause should go inside the wrapper function. We have to
  // pass the insertion point explicitly; otherwise, genOffloadArraysInit()
  // will call getInsertionPtForAllocas() to get the insertion point, and
  // it will result in the function containing the TARGET VARIANT DISPATCH
  // construct, which is wrong.
  Instruction *InsertPtForAllocas = WrapperFn->getEntryBlock().getFirstNonPHI();

  getAndReplaceDevicePtrs(W, VariantCall, InsertPtForAllocas);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Wrapper Call:" << *VariantWrapperCall
                    << "\n");
  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genTargetVariantDispatchCode\n");
  W->resetBBSet(); // Invalidate BBSet after transformations
  return true;
}

// NeedDevicePtrStr is a comma-separated list of substrings
// like this: "T,F,CPTR,F90_DV,PTR_TO_PTR"
// Each substring describes a fn arg in the corresponding position.
// "F" means no processing needed for the fn arg. Anything else means
// the fn arg is a host pointer that we need to replace with its
// device pointer.
//
// We artificially allow the DISPATCH construct to take use_device_ptr
// and map clauses so we can reuse the code in getAndReplaceDevicePtrs().
//
// Before calling getAndReplaceDevicePtrs(), we populate the artificial
// use_device_ptr clause with fn args based on NeedDevicePtrStr.
void VPOParoptTransform::processNeedDevicePtr(WRegionNode *W,
                                              CallInst *VariantCall,
                                              StringRef &NeedDevicePtrStr) {
  if (NeedDevicePtrStr.empty())
    return;

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::processNeedDevicePtr\n");

  SmallVector<Value *, 4> FnArgs(VariantCall->args());
  SmallVector<StringRef, 4> Substr;
  NeedDevicePtrStr.split(Substr, ",");

  assert(Substr.size() <= FnArgs.size() &&
         "number of need_device_ptr operands cannot exceed number of fn args");

  UseDevicePtrClause &UDPC = W->getUseDevicePtr();
#if INTEL_CUSTOMIZATION

  // Extract the struct type from StructTyName, and use that to populate the
  // type fields in the item UDPI.
  auto AddF90DVDVInfoToUDPIUsingStructName = [&](UseDevicePtrItem *UDPI,
                                                 StringRef StructTyName) {
    assert(UDPI && "Null use_device_ptr item.");
    LLVMContext &C = F->getContext();

    auto *DVTy = StructType::getTypeByName(C, StructTyName);
    if (!DVTy)
      llvm_unreachable("Couldn't find type of F90_DV need_device_ptr operand.");

    UDPI->setIsF90DopeVector(true);
    UDPI->setIsTyped(true);
    UDPI->setOrigItemElementTypeFromIR(DVTy);
    UDPI->setNumElements(ConstantInt::get(Type::getInt32Ty(C), 1));
    // Pointee element type is not needed for use_device_ptr codegen.
    UDPI->setPointeeElementTypeFromIR(nullptr);
  };
#endif // INTEL_CUSTOMIZATION

  NeedDevicePtrSet &NDP = W->getNeedDevicePtr();
  NeedDevicePtrSet &NDPTP = W->getNeedDevicePtrToPtr();

  for (unsigned I = 0; I < Substr.size(); ++I) {
    auto Arg = FnArgs[I];
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": need_device_ptr for ";
               Arg->printAsOperand(dbgs());
               dbgs() << " is " << Substr[I] << "\n");
    if (Substr[I] == "F")
      continue;
    if (NDP.contains(I) || NDPTP.contains(I))
      continue; // skip if also in a need_device_ptr clause, handled separately
    assert(isa<PointerType>(Arg->getType()) &&
           "need_device_ptr expects a pointer-type fn argument");
    UDPC.add(Arg);
    if (Substr[I] == "T")
      continue;
    if (Substr[I] == "PTR_TO_PTR")
      UDPC.back()->setIsPointerToPointer(true);
#if INTEL_CUSTOMIZATION
    else if (Substr[I] == "F90_DV")
      UDPC.back()->setIsF90DopeVector(true);
    else if (Substr[I].consume_front("F90_DV."))
      AddF90DVDVInfoToUDPIUsingStructName(UDPC.back(), Substr[I]);
    else if (Substr[I] == "CPTR")
      UDPC.back()->setIsCptr(true);
#endif // INTEL_CUSTOMIZATION
    else
      llvm_unreachable("Unknown need_device_ptr marker");
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::processNeedDevicePtr\n");
}

// To handle need_device_ptr clause for dispatch and target variant dispatch,
// we populate artificial use_device_ptr clauses with fn args based on the
// need_device_ptr clauses. The information stored in use_device_ptr clause
// will be used by getAndReplaceDevicePtrs() to emit code.
void VPOParoptTransform::processNeedDevicePtrClause(WRegionNode *W,
                                                    CallInst *VariantCall,
                                                    uint64_t InteropPosition) {
  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::processNeedDevicePtrClause\n");

  NeedDevicePtrSet &NDP = W->getNeedDevicePtr();
  NeedDevicePtrSet &NDPTP = W->getNeedDevicePtrToPtr();
  UseDevicePtrClause &UDPC = W->getUseDevicePtr();
  SmallVector<Value *, 4> FnArgs(VariantCall->args());

  // InteropPosition is used to adjust the need_device_pointer clause arguments
  // that are after the interop obj. A variadic function declared as
  //   void foo(int*, ...)
  // and called as
  //   #pragma omp [target variant] dispatch need_device_ptr(1,2)
  //     foo(p1, p2)
  // needs its arguments "p1" and "p2" translated. However, if an interop obj
  // is emitted, the actual variant call may look like
  //   foo_var(p1, interop, p2)
  // so we need to adjust need_device_ptr(1,2) --> need_device_ptr(1,3)
  //
  // If InteropPosition is 0, then an interop obj was not used. To simplify
  // implementation, in this case we set it to be number-of-args + 1, so it's
  // always greater than any argument in the need_device_ptr clause.
  //
  // TODO: When multiple interop objs are supported, we will also need to pass
  // in the number of interop objs injected into the variant call.
  if (InteropPosition == 0)
    InteropPosition = FnArgs.size() + 1;

  auto populateUDPC = [&](NeedDevicePtrSet &NDPClause, bool IsPtrToPtr) {
    for (unsigned N : NDPClause) {
      assert(N <= FnArgs.size() && "need_device_ptr clause argument value "
                                   "cannot exceed the number of function args");
      if (N >= InteropPosition)
        N++;                    // adjust N to account for the interop obj
      auto Arg = FnArgs[N - 1]; // N is 1-based, but FnArgs indices are 0-based
      assert(isa<PointerType>(Arg->getType()) &&
             "need_device_ptr clause expects a pointer-type fn argument");
      UDPC.add(Arg);
      UDPC.back()->setIsPointerToPointer(IsPtrToPtr);
    }
  };

  populateUDPC(NDP, false);
  populateUDPC(NDPTP, true);

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::processNeedDevicePtrClause\n");
}

// Handle the depend clause of the dispatch construct by emitting these calls
// around the VariantCall:
//   dummytaskthunk = @__kmpc_omp_task_alloc(loc, tid, ... )               (1)
//   @__kmpc_omp_wait_deps(loc, tid, ...)                                  (2)
//   @__kmpc_omp_task_begin_if0(loc, tid, dummytaskthunk)                  (3)
//   VariantCall(...)
//   @__kmpc_omp_task_complete_if0(loc, tid, dummytaskthunk)               (4)
//
// The calls to (1), (3) and (4) are for OMPT tracing only;
// do not emit them if SupportOMPTTracing==false
void VPOParoptTransform::genDependForDispatch(WRegionNode *W,
                                              CallInst *VariantCall,
                                              bool SupportOMPTTracing) {
  // Currently the depend clause of the dispatch construct is attached
  // to the implicit parent task
  WRegionNode *ParentTask = W->getParent();
  if (!(ParentTask && isa<WRNTaskNode>(ParentTask) &&
        ParentTask->getIsImplicit()))
    return;

  DependClause const &DepClause = ParentTask->getDepend();
  if (DepClause.empty() && !ParentTask->getDepArray())
    return;

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genDependForDispatch\n");

  // Insert code before VariantCall;
  Instruction *InsertPt = VariantCall;
  CallInst *TaskAllocCI = nullptr;

  if (SupportOMPTTracing)
    TaskAllocCI = VPOParoptUtils::genKmpcTaskAllocWithoutCallback(
        W, IdentTy, TidPtrHolder, InsertPt); //                            (1)

  AllocaInst *DummyTaskTDependRec = genDependInitForTask(ParentTask, InsertPt);
  genTaskDeps(ParentTask, IdentTy, TidPtrHolder, /*TaskAlloc=*/nullptr,
              DummyTaskTDependRec, InsertPt, true); //                     (2)

  if (SupportOMPTTracing) {
    VPOParoptUtils::genKmpcTaskBeginIf0(W, IdentTy, TidPtrHolder, TaskAllocCI,
                                        InsertPt); //                      (3)

    // Insert code after VariantCall
    VPOParoptUtils::genKmpcTaskCompleteIf0(
        W, IdentTy, TidPtrHolder, TaskAllocCI,
        InsertPt->getNextNonDebugInstruction()); //                        (4)
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genDependForDispatch\n");
}

// The input InteropStr is a string for a comma-separated list of items,
// where each item can be one of these:
//   - target
//   - targetsync
//   - an integer between 1 and 6, inclusive (the prefer type)
// At least one of the items must be target or targetsync.
// Examples of InteropStr:
//   "targetsync"
//   "target,targetsync"
//   "targetsync,4,6,1"
//
// Input parm: the InteropStr string
// Output parm: PreferList is populated with integers in the same order as they
//              appear in the InteropStr
// Returns 1 if targetsync is seen. Otherwise returns 0.
static unsigned getInteropOperation(StringRef InteropStr,
                                    SmallVectorImpl<unsigned> &PreferList) {
  SmallVector<StringRef, 3> Items;
  InteropStr.split(Items, ",");
  int TargetOrTargetsync = -1; // return value; will be 0 or 1
  int PreferItem = -1;
  for (StringRef &Item : Items) {
    if (Item == "target") {
      TargetOrTargetsync = std::max(TargetOrTargetsync, 0);
    } else if (Item == "targetsync") {
      TargetOrTargetsync = std::max(TargetOrTargetsync, 1);
    } else if (!Item.getAsInteger(/*radix=*/ 10, PreferItem)) {
      // getAsInteger() returns true on error
      assert(1 <= PreferItem && PreferItem <= 6 &&
             "Invalid Prefer Type. Expected to be in [1,6]");
      PreferList.push_back((unsigned)PreferItem);
    } else {
      llvm_unreachable("getInteropOperation: Unknown string in InteropStr");
    }
  }

  assert(TargetOrTargetsync > -1 &&
         "getInteropOperation: Target/Targetsync was not seen ");

  return (unsigned)TargetOrTargetsync;
}

bool VPOParoptTransform::genDispatchCode(WRegionNode *W) {
  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::genDispatchCode\n");
  W->populateBBSet();

  // find the dispatch call and remove its QUAL_OMP_DISPATCH_CALL OB from IR
  findDispatchCall(W);
  CallInst *BaseCall = W->getCall();

  // TODO: make BaseCall's arguments unique temps. In most cases the args
  //       are already unique temps out of the FE, so we can do this later.
  // makeFuncArgsUnique(Basecall);

  StringRef MatchConstruct("dispatch");
  uint64_t DeviceArchs = 0u; // bit vector of device architectures
  std::optional<uint64_t> InteropPosition =
      std::nullopt; // position of interop arg in variant call
  StringRef NeedDevicePtrStr;
  StringRef InteropStr;
  StringRef VariantName =
      getVariantInfo(W, BaseCall, MatchConstruct, DeviceArchs, InteropPosition,
                     NeedDevicePtrStr, InteropStr);

  auto emitRemark = [&](const StringRef &Message) {
    OptimizationRemarkMissed R("openmp", "Region", W->getEntryDirective());
    R << ore::NV("Construct", W->getName()) << Message;
    ORE.emit(R);
  };

  assert(BaseCall && "Base call not found in Dispatch");
  if (!BaseCall) {
    emitRemark(" Could not find a valid function call in the region");
    return false;
  }

  if (VariantName.empty()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Variant function not found\n");
    emitRemark(" Could not find a matching variant function");
    return false;
  }

  InteropClause &IOClause = W->getInterop();
  if (IOClause.size() > 1) {
    // TODO: support multiple interop objs
    F->getContext().diagnose(DiagnosticInfoUnsupported(
        *F, "Multiple interop objs in INTEROP clause is not yet supported.",
        W->getEntryDirective()->getDebugLoc()));
    return false;
  }

  if (InteropStr.empty() && !IOClause.empty()) {
    // TODO: when supporting multiple interop objs, InteropStr will be a vector
    //       and the check becomes: if (InteropStrVec.size() < IOClause.size())
    F->getContext().diagnose(DiagnosticInfoUnsupported(
        *F,
        "Number of interop objs in INTEROP clause cannot exceed number of "
        "interop operations in APPEND_ARGS clause.",
        W->getEntryDirective()->getDebugLoc()));
    return false;
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Found variant function name: " << VariantName);
  if (!NeedDevicePtrStr.empty())
    LLVM_DEBUG(dbgs() << " ; need_device_ptr: " << NeedDevicePtrStr);
  if (!InteropStr.empty())
    LLVM_DEBUG(dbgs() << " ; interop: " << InteropStr);
  LLVM_DEBUG(dbgs() << "\n");

  Instruction *InsertPt = BaseCall;

  assert((W->getDevice() || IOClause.size() <= 1) &&
         "If INTEROP clause has multiple objs then device clause is required");
         // FE guarantees this

  Value *InteropClauseObj = nullptr;
  if (!IOClause.empty()) {
    InteropItem *IOItem = IOClause.front();
    InteropClauseObj = IOItem->getOrig();
    assert(InteropClauseObj && "Interop clause item is null");
    // TODO: when we support multiple interop objs, use a loop
    //       for (InteropItem *IOItem : IOClause.items()) ...
  }
  Value *DeviceNum = genDeviceNum(W, InsertPt, InteropClauseObj);

  // Emit dispatch condition:
  //   %call = call i32 @__tgt_is_device_available(i64 %0, i8* DeviceType)  (1)
  //   %available  = icmp ne i32 %call, 0                                   (2)
  Value *Available = genDeviceAvailable(W, InsertPt, DeviceNum, DeviceArchs);

  // Emit CFG to hold dispatch code:
  //   if (%available)
  //     // code to setup variant call goes here:
  //     //  1. create interop obj (if not already in an interop clause)
  //     //  2. process need_device_ptr
  //     //  3. handle depend clause
  //     call variant function
  //     //  4. sync call for NOWAIT (only for DispatchCodegenVersion > 0)
  //   else
  //     call base function
  //
  // There are currently two codegen implementations of 1-4 above.
  // When they differ, they are guarded with DispatchCodegenVersion.
  //
  // A. DispatchCodegenVersion == 0 is for the original implementation:
  //     A1. Call  __tgt_create_interop_obj to create interop objs.
  //     A2. Processing of need_device_ptr is the same in both versions.
  //     A3. Call __kmpc_omp_wait_deps, __kmpc_omp_task_begin_if0,
  //         and __kmpc_omp_task_complete_if0 to handle DEPEND.
  //     A4. Does not call __tgt_target_sync or __tgt_interop_use_async
  //
  // B. DispatchCodegenVersion > 0 is for the newer implementation that
  //    supports creation of interop obj that honors the prefer_type
  //    specified in append_args. This implementation also invokes a
  //    runtime that is more efficient at handling asynchronous execution
  //    (ie when NOWAIT is specified):
  //     B1. Call  __kmpc_get_current_task and __tgt_get_interop_obj
  //         to create interop objs.
  //     B2. Processing of need_device_ptr is the same in both versions.
  //     B3. Call __kmpc_omp_wait_deps to handle DEPEND, but not
  //         __kmpc_omp_task_begin_if0 and  __kmpc_omp_task_complete_if0.
  //     B4. if NOWAIT is not specified
  //        a) Call __tgt_target_sync if INTEROP is not specified
  //        b) Call __tgt_interop_use_async if INTEROP is specified
  Instruction *ThenTerm, *ElseTerm;
  VPOParoptUtils::buildCFGForIfClause(Available, ThenTerm, ElseTerm, InsertPt, DT);

  IRBuilder<> Builder(ThenTerm);

  LoadInst *Tid = nullptr;
  CallInst *CurrentTask = nullptr;

  // Initialize Tid and CurrentTask if still uninitialized
  auto LoadTidAndCurrentTask = [&]() {
    Type *Int32Ty = Builder.getInt32Ty();
    Type *Int8PtrTy = Builder.getInt8PtrTy();

    if (!Tid) // Load Tid from TidPtrHolder
      Tid =
          Builder.CreateAlignedLoad(Int32Ty, TidPtrHolder, Align(4), "my.tid");

    if (!CurrentTask) { // Emit call to __kmpc_get_current_task
      CurrentTask = VPOParoptUtils::genCall(
          "__kmpc_get_current_task", Int8PtrTy, {Tid}, {Int32Ty}, ThenTerm);
      CurrentTask->setName("current.task");
    }
  };

  // Current we only support one interop obj, either interop(target) or
  // interop(targetsync). We use the old-stype interop obj from
  // createInteropObj() and don't distinguish between target and targetsync.
  // TODO: Use the new-style interop obj created by #pragma omp interop
  Value *InteropObj = nullptr;
  if (InteropStr.empty()) {
    assert(!W->getNowait() &&
           "Expected an interop obj for dispatch nowait");
  } else if (InteropClauseObj) {
    InteropObj = InteropClauseObj;
  } else {
    SmallVector<unsigned> PreferList;
    unsigned OmpInteropContext = getInteropOperation(InteropStr, PreferList);

    if (DispatchCodegenVersion == 0) {
      // (A1)
      assert(PreferList.size() == 0 &&
             "prefer_type is unsupported in append_args with "
             "vpo-paropt-dispatch-codegen-version=0. "
             "Set the flag to 1 to enable support.");
      InteropObj = createInteropObj(W, DeviceNum, IdentTy, ThenTerm); //    (7)
    }
    else {
      // (B1)
      // For DispatchCodegenVersion > 0, emit this call to get the interop obj:
      //   InteropObj = __tgt_get_interop_obj(loc, interop_op, num_prefers,
      //                            prefer_list, device_id, gtid, CurrentTask);
      LoadTidAndCurrentTask();
      InteropObj = VPOParoptUtils::genTgtGetInteropObj(
          W, IdentTy, OmpInteropContext, PreferList, DeviceNum, Tid,
          CurrentTask, ThenTerm);
      InteropObj->setName("interop.obj");
    }
  }

  // Create and insert Variant call before ThenTerm
  bool IsVoidType = (BaseCall->getType() == Builder.getVoidTy());
  uint64_t InteropPositionIfEmitted = 0; // will be updated by genVariantCall()
  CallInst *VariantCall = VPOParoptUtils::genVariantCall(
      BaseCall, VariantName, InteropObj, InteropPosition,
      InteropPositionIfEmitted, ThenTerm, W); //                            (8)
  if (!IsVoidType)
    VariantCall->setName("variant");

  // (A2,B2) Handle need_device_ptr
  processNeedDevicePtr(W, VariantCall, NeedDevicePtrStr); // from decl variant
  processNeedDevicePtrClause(
      W, VariantCall,
      InteropPositionIfEmitted); // from need_device_ptr clause
  getAndReplaceDevicePtrs(W, VariantCall);

  // (A3,B3) Handle depend clause
  // If DispatchCodegenVersion > 0, do not emit the calls to
  //   __kmpc_omp_task_begin_if0
  //   __kmpc_omp_task_complete_if0
  // which are only used for tracing.
  bool SupportOMPTTracing = (DispatchCodegenVersion == 0);
  genDependForDispatch(W, VariantCall, SupportOMPTTracing);

  // (B4) If DispatchCodegenVersion > 0 and the NOWAIT clause is not specified,
  // if the INTEROP clause was specified emit a call to
  //   __tgt_interop_use_async(loc, gtid, interop_op, false)
  // if the INTEROP clause was not specified emit a call to
  //   __tgt_target_sync(loc, gtid, CurrentTask, nullptr);
  if (DispatchCodegenVersion > 0 && !W->getNowait()) {
    LoadTidAndCurrentTask();
    if (InteropClauseObj)
      VPOParoptUtils::genTgtInteropUseAsync(W, IdentTy, TidPtrHolder,
                                            InteropObj, false, ThenTerm);
    else
      VPOParoptUtils::genTgtTargetSync(W, IdentTy, Tid, CurrentTask, ThenTerm);
  }

  // Move BaseCall to before ElseTerm
  InsertPt = BaseCall->getNextNode(); // insert PHI before this point later
  assert(InsertPt && "Corrupt IR: BaseCall cannot be last instruction in BB");
  BaseCall->moveBefore(ElseTerm);

  // If BaseCall has users, then insert a PHI before InsertPt
  // and replace all uses of BaseCall with PHI
  if (BaseCall->getNumUses() > 0) {
    Builder.SetInsertPoint(InsertPt);
    PHINode *Phi = Builder.CreatePHI(BaseCall->getType(), 2, "callphi");
    Phi->addIncoming(VariantCall, ThenTerm->getParent());
    Phi->addIncoming(BaseCall, ElseTerm->getParent());
    BaseCall->replaceUsesWithIf(Phi, [Phi](Use &U) {
      if (auto *UI = dyn_cast<Instruction>(U.getUser()))
        if (UI != Phi) // don't replace in Phi
          return true;
      return false;
    });
  }

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genDispatchCode\n");
  W->resetBBSet(); // Invalidate BBSet after transformations
  return true;
}
#endif // INTEL_COLLAB
