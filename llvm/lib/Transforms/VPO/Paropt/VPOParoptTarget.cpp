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
#include "llvm/Support/Debug.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/InferAddressSpacesUtils.h"

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target"

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

  bool ForceMapping =
      isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W);

  IRBuilder<> Builder(W->getEntryBBlock()->getFirstNonPHI());

  for (auto *Item : MpClause.items()) {
    if (!Item->getIsMapChain())
      continue;
    MapChainTy const &MapChain = Item->getMapChain();
    for (int I = MapChain.size() - 1; I >= 0; --I) {
      MapAggrTy *Aggr = MapChain[I];
      Value *SectionPtr = Aggr->getSectionPtr();
      resetValueInIntelClauseGeneric(W, SectionPtr);

      if (deviceTriplesHasSPIRV() && MapChain.size() > 1 &&
          I != 0 && !ForceMapping)
        Builder.CreateStore(SectionPtr, Aggr->getBasePtr());

      Value *Size = Aggr->getSize();
      if (!dyn_cast<ConstantInt>(Size))
        resetValueInIntelClauseGeneric(W, Size);
    }
  }
}

Function *VPOParoptTransform::finalizeKernelFunction(WRegionNode *W,
                                                     Function *Fn,
                                                     CallInst *&Call) {

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
  Function *NFn = Function::Create(NFnTy, GlobalValue::ExternalLinkage);
  NFn->copyAttributesFrom(Fn);
  NFn->setCallingConv(CallingConv::SPIR_KERNEL);
  NFn->addFnAttr("target.declare", "true");

  Fn->getParent()->getFunctionList().insert(Fn->getIterator(), NFn);
  NFn->takeName(Fn);
  NFn->getBasicBlockList().splice(NFn->begin(), Fn->getBasicBlockList());

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
      NewArgV = Builder.CreatePointerBitCastOrAddrSpaceCast(ArgV, I->getType());
    }
    I->replaceAllUsesWith(NewArgV);
    NewArgI->takeName(&*I);
    ++NewArgI;
  }

  DenseMap<const Function *, DISubprogram *> FunctionDIs;

  auto DI = FunctionDIs.find(Fn);
  if (DI != FunctionDIs.end()) {
    DISubprogram *SP = DI->second;

    FunctionDIs.erase(DI);
    FunctionDIs[NFn] = SP;
  }
  if (VPOAnalysisUtils::isTargetSPIRV(NFn->getParent()) &&
      hasOffloadCompilation())
    InferAddrSpaces(*TTI, 0, *NFn);

  return NFn;
}
// Generate the code for the directive omp target
bool VPOParoptTransform::genTargetOffloadingCode(WRegionNode *W) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTargetOffloadingCode\n");

  W->populateBBSet();

  resetValueInIntelClauseGeneric(W, W->getIf());
  resetValueInIntelClauseGeneric(W, W->getDevice());
  resetValueInIsDevicePtrClause(W);
  resetValueInPrivateClause(W);
  resetValueInMapClause(W);

  // Set up Fn Attr for the new function
  Function *NewF = VPOParoptUtils::genOutlineFunction(*W, DT, AC);

  if (!VPOAnalysisUtils::isTargetSPIRV(F->getParent()))
    NewF->addFnAttr("target.declare", "true");

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  // Add "target.entry" attribute to the outlined function.
  NewF->addFnAttr("omp.target.entry");
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  CallInst *NewCall = cast<CallInst>(NewF->user_back());

  Constant *RegionId = nullptr;
  if (isa<WRNTargetNode>(W)) {
    assert(MT && "target region with no module transform");
    RegionId = MT->registerTargetRegion(W, NewF);
  }

  // Please note that the name of NewF is updated in the
  // function registerTargetRegion.
  if (VPOAnalysisUtils::isTargetSPIRV(F->getParent()) &&
      hasOffloadCompilation())
    finalizeKernelFunction(W, NewF, NewCall);

  IRBuilder<> Builder(F->getEntryBlock().getTerminator());
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
    buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, InsertPt);
    InsertPt = ThenTerm;
    Call = genTargetInitCode(W, NewCall, RegionId, InsertPt);
    Builder.SetInsertPoint(ElseTerm);
    Builder.CreateStore(
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), -1),
        OffloadError);
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
    } else if (isa<WRNTargetEnterDataNode>(W) ||
               isa<WRNTargetExitDataNode>(W) ||
               isa<WRNTargetUpdateNode>(W)) {
      NewCall->eraseFromParent();
      // We cannot erase the function right now, because it now contains
      // the region's entry/exit calls, which we will try to erase later.
      NewF->removeFromParent();
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
      resetValueInIntelClauseGeneric(W, NumTeamsPtr);

    if (auto *ThreadLimitPtr = W->getThreadLimit())
      resetValueInIntelClauseGeneric(W, ThreadLimitPtr);

    return;
  }

  if (W->getIsPar())
    if (auto *NumThreadsPtr = W->getNumThreads())
      resetValueInIntelClauseGeneric(W, NumThreadsPtr);
}

// Reset the expression value in IsDevicePtr clause to be empty.
void VPOParoptTransform::resetValueInIsDevicePtrClause(WRegionNode *W) {
  if (!W->canHaveIsDevicePtr())
    return;

  IsDevicePtrClause &IDevicePtrClause = W->getIsDevicePtr();
  if (IDevicePtrClause.empty())
    return;

  for (auto *I : IDevicePtrClause.items()) {
    resetValueInIntelClauseGeneric(W, I->getOrig());
  }
}

// Returns the corresponding flag for a given map clause modifier.
uint64_t VPOParoptTransform::getMapTypeFlag(MapItem *MapI, bool AddrPtrFlag,
                                            bool AddrIsTargetParamFlag,
                                            bool IsFirstComponentFlag) {
  uint64_t Res = 0u;
  if (!AddrIsTargetParamFlag && IsFirstComponentFlag)
    return TGT_MAP_TARGET_PARAM;

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
  else if (AddrPtrFlag)
    Res |= TGT_MAP_PTR_AND_OBJ | getMemberOfFlag();

  return Res;
}

// Return the map modifiers for the firstprivate.
uint64_t VPOParoptTransform::getMapModifiersForFirstPrivate() {
  return TGT_MAP_TO;
}

// Return the defaut map information.
uint64_t VPOParoptTransform::generateDefaultMap() {
  return getMapModifiersForFirstPrivate() | TGT_MAP_TARGET_PARAM |
         TGT_MAP_IMPLICIT;
}

// Generate the sizes and map type flags for the given map type, map
// modifier and the expression V.
void VPOParoptTransform::genTgtInformationForPtrs(
    WRegionNode *W, Value *V, SmallVectorImpl<Constant *> &ConstSizes,
    SmallVectorImpl<uint64_t> &MapTypes,
    bool &hasRuntimeEvaluationCaptureSize,
    bool &IsFirstExprFlag) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genTgtInformationForPtrs:"
                    << " ConstSizes.size()=" << ConstSizes.size()
                    << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");

  const DataLayout DL = F->getParent()->getDataLayout();
  LLVMContext &C = F->getContext();

  bool ForceMapping =
      isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W);

  MapClause const &MpClause = W->getMap();
  bool Match = false;
  for (MapItem *MapI : MpClause.items()) {
    if (!ForceMapping &&
        (MapI->getOrig() != V || !MapI->getOrig()))
      continue;
    Match = true;
    Type *T = MapI->getOrig()->getType()->getPointerElementType();
    if (MapI->getIsMapChain()) {
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
        MapTypes.push_back(
            getMapTypeFlag(MapI, !IsFirstExprFlag,
                MapChain.size() > 1 ? false : true,
                I == 0 ? true : false));
        IsFirstExprFlag = false;
        // FIXME: this has to be fixed. Code generation on the host must not
        //        depend on the fact that user passed -fopenmp-targets=spirv64
        //        or not. There are several calls to deviceTriplesHasSPIRV()
        //        in this file, and they all relate to a single change-set.
        //        I am not yet sure what is going on here, but basically
        //        we need to configure map entries for all base and section
        //        pointers. In case of WRNTargetNode, we may ignore those
        //        map items that do not end up in the parameter list
        //        of the outline function, unless they are mapped ALWAYS.
        //        Right now we iterate through all parameters of the outline
        //        function, but it looks like we should iterate through
        //        the map clauses and match them to the parameters:
        //        if a map item is ALWAYS and it does not match any parameter,
        //        we must still create a map entry for it.
        if (deviceTriplesHasSPIRV() && MapChain.size() > 1 &&
            I == 0 && !ForceMapping)
          break;
      }
    } else {
      assert(!MapI->getIsArraySection() &&
             "Map with an array section must have a map chain.");
      ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                            DL.getTypeAllocSize(T)));
      MapTypes.push_back(getMapTypeFlag(MapI, !IsFirstExprFlag, true, true));
    }
    IsFirstExprFlag = false;
  }
  if (deviceTriplesHasSPIRV() && Match == false && !ForceMapping) {
    for (MapItem *MapI : MpClause.items()) {
      if (MapI->getIsMapChain()) {
        MapChainTy const &MapChain = MapI->getMapChain();
        for (unsigned I = 1; I < MapChain.size(); ++I) {
          MapAggrTy *Aggr = MapChain[I];
          if (Aggr->getSectionPtr() != V)
            continue;
          auto ConstValue = dyn_cast<ConstantInt>(Aggr->getSize());
          if (!ConstValue) {
            hasRuntimeEvaluationCaptureSize = true;
            Type *T = MapI->getOrig()->getType()->getPointerElementType();
            ConstSizes.push_back(ConstantInt::get(
              Type::getInt64Ty(C), DL.getTypeAllocSize(T)));
          } else {
            // Sign extend the constant to signed 64-bit integer.
            // This is the format of arg_sizes passed to __tgt_target.
            ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                                  ConstValue->getSExtValue()));
          }
          MapTypes.push_back(
            getMapTypeFlag(MapI, !IsFirstExprFlag,
            true,
            I == 0 ? true : false));
          IsFirstExprFlag = false;
        }
      }
    }
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      if (FprivI->getOrig() != V)
        continue;
      if (FprivI->getInMap())
        continue;
      Type *T = FprivI->getOrig()->getType()->getPointerElementType();
      ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                            DL.getTypeAllocSize(T)));
      MapTypes.push_back(generateDefaultMap());
    }
  }

  if (W->canHaveIsDevicePtr()) {
    IsDevicePtrClause &IDevicePtrClause = W->getIsDevicePtr();
    for (IsDevicePtrItem *IsDevicePtrI : IDevicePtrClause.items()) {
      if (IsDevicePtrI->getOrig() != V)
        continue;
      Type *T = Type::getInt64Ty(C);
      ConstSizes.push_back(ConstantInt::get(T, DL.getTypeAllocSize(T)));
      MapTypes.push_back(TGT_MAP_LITERAL | TGT_MAP_TARGET_PARAM | TGT_MAP_IMPLICIT);
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
AllocaInst *VPOParoptTransform::genTgtLoopParameter(WRegionNode *W,
                                                    WRegionNode *WL) {
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB = SplitBlock(EntryBB, &*(EntryBB->begin()), DT, LI);
  W->setEntryBBlock(NewEntryBB);
  LLVMContext &C = F->getContext();
  IntegerType *Int64Ty = Type::getInt64Ty(C);
  Instruction *InsertPt = EntryBB->getTerminator();
  IRBuilder<> Builder(InsertPt);
  SmallVector<Type *, 4> CLLoopParameterRecTypeArgs;
  CLLoopParameterRecTypeArgs.push_back(Int64Ty);
  for (unsigned I = 0; I < WL->getWRNLoopInfo().getNormIVSize(); I++) {
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
  Value *BaseGep =
      Builder.CreateInBoundsGEP(CLLoopParameterRecType, DummyCLLoopParameterRec,
                                {Builder.getInt32(0), Builder.getInt32(0)});

  Builder.CreateStore(
      Builder.CreateSExtOrTrunc(
          Builder.getInt32(WL->getWRNLoopInfo().getNormIVSize()), Int64Ty),
      BaseGep);

  for (unsigned I = 0; I < WL->getWRNLoopInfo().getNormIVSize(); I++) {
    Loop *L = WL->getWRNLoopInfo().getLoop(I);
    Value *LowerBndGep = Builder.CreateInBoundsGEP(
        CLLoopParameterRecType, DummyCLLoopParameterRec,
        {Builder.getInt32(0), Builder.getInt32(3 * I + 1)});
    Builder.CreateStore(Builder.getInt64(0), LowerBndGep);

    Value *UpperBndGep = Builder.CreateInBoundsGEP(
        CLLoopParameterRecType, DummyCLLoopParameterRec,
        {Builder.getInt32(0), Builder.getInt32(3 * I + 2)});
    Value *CloneUB = VPOParoptUtils::cloneInstructions(
        WRegionUtils::getOmpLoopUpperBound(L), InsertPt);
    assert(CloneUB && "genTgtLoopParameter: unexpected null CloneUB");
    Builder.CreateStore(Builder.CreateSExtOrTrunc(CloneUB, Int64Ty),
                        UpperBndGep);

    Value *StrideGep = Builder.CreateInBoundsGEP(
        CLLoopParameterRecType, DummyCLLoopParameterRec,
        {Builder.getInt32(0), Builder.getInt32(3 * I + 3)});
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
  LLVMContext &C = F->getContext();
  IRBuilder<> Builder(F->getEntryBlock().getFirstNonPHI());
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
    bool IsFirstExprFlag = true;

    if (ForceMapping)
      genTgtInformationForPtrs(W, nullptr, ConstSizes, MapTypes,
                               hasRuntimeEvaluationCaptureSize,
                               IsFirstExprFlag);
    else {
      for (unsigned II = 0; II < Call->getNumArgOperands(); ++II) {
        Value *BPVal = Call->getArgOperand(II);
        genTgtInformationForPtrs(W, BPVal, ConstSizes, MapTypes,
                                 hasRuntimeEvaluationCaptureSize,
                                 IsFirstExprFlag);
      }
      if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
        genTgtInformationForPtrs(W, W->getParLoopNdInfoAlloca(), ConstSizes,
                                 MapTypes, hasRuntimeEvaluationCaptureSize,
                                 IsFirstExprFlag);
    }

    Info.NumberOfPtrs = MapTypes.size();

    Value *SizesArray;

    if (hasRuntimeEvaluationCaptureSize)
      SizesArray = Builder.CreateAlloca(
          ArrayType::get(Type::getInt64Ty(C), Info.NumberOfPtrs),
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
        ArrayType::get(Builder.getInt8PtrTy(), Info.NumberOfPtrs), nullptr,
        ".offload_baseptrs");

    AllocaInst *TgPointersArray = Builder.CreateAlloca(
        ArrayType::get(Builder.getInt8PtrTy(), Info.NumberOfPtrs), nullptr,
        ".offload_ptrs");

    Constant *MapTypesArrayInit =
        ConstantDataArray::get(Builder.getContext(), MapTypes);
    auto *MapTypesArrayGbl =
        new GlobalVariable(*(F->getParent()), MapTypesArrayInit->getType(),
                           true, GlobalValue::PrivateLinkage, MapTypesArrayInit,
                           ".offload_maptypes", nullptr);
    MapTypesArrayGbl->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Info.BaseDataPtrs = TgBasePointersArray;
    Info.DataPtrs = TgPointersArray;
    Info.DataSizes = SizesArray;
    Info.DataMapTypes = MapTypesArrayGbl;

    genOffloadArraysInit(W, &Info, Call, InsertPt, ConstSizes,
                         hasRuntimeEvaluationCaptureSize);
  }

  genOffloadArraysArgument(&Info, InsertPt);
  if (hasOffloadCompilation())
    return Call;

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
    genOffloadArraysArgument(&Info, Call);
    VPOParoptUtils::genTgtTargetDataEnd(
        W, Info.NumberOfPtrs, Info.ResBaseDataPtrs, Info.ResDataPtrs,
        Info.ResDataSizes, Info.ResDataMapTypes, Call);
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

// Utilities to construct the assignment to the base pointers, section
// pointers and size pointers if the flag hasRuntimeEvaluationCaptureSize is
// true.
void VPOParoptTransform::genOffloadArraysInitUtil(
    IRBuilder<> &Builder, Value *BasePtr, Value *SectionPtr, Value *Size,
    TgDataInfo *Info, SmallVectorImpl<Constant *> &ConstSizes, unsigned &Cnt,
    bool hasRuntimeEvaluationCaptureSize) {

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
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W);

  MapClause const &MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    if (!ForceMapping &&
        (MapI->getOrig() != BPVal || !MapI->getOrig()))
      continue;
    if (ForceMapping)
      BPVal = MapI->getOrig();
    Match = true;
    if (MapI->getIsMapChain()) {
      MapChainTy const &MapChain = MapI->getMapChain();
      for (unsigned I = 0; I < MapChain.size(); ++I) {
        MapAggrTy *Aggr = MapChain[I];
        genOffloadArraysInitUtil(
            Builder, Aggr->getBasePtr(), Aggr->getSectionPtr(), Aggr->getSize(),
            Info, ConstSizes, Cnt, hasRuntimeEvaluationCaptureSize);
        if (deviceTriplesHasSPIRV() && MapChain.size() > 1 &&
            I == 0 && !ForceMapping)
          break;
      }
    } else {
      assert(!MapI->getIsArraySection() &&
             "Map with an array section must have a map chain.");
      genOffloadArraysInitUtil(Builder, BPVal, BPVal, nullptr,
                               Info, ConstSizes,
                               Cnt, hasRuntimeEvaluationCaptureSize);
    }
  }

  if (deviceTriplesHasSPIRV() && Match == false &&
      !ForceMapping) {
    for (MapItem *MapI : MpClause.items()) {
      if (MapI->getIsMapChain()) {
        MapChainTy const &MapChain = MapI->getMapChain();
        for (unsigned I = 1; I < MapChain.size(); ++I) {
          MapAggrTy *Aggr = MapChain[I];
          if (Aggr->getSectionPtr() != BPVal)
            continue;
          genOffloadArraysInitUtil(
            Builder, Aggr->getSectionPtr(), Aggr->getSectionPtr(), Aggr->getSize(),
            Info, ConstSizes, Cnt, hasRuntimeEvaluationCaptureSize);
          Match = true;
          break;
        }
      }
      if (Match)
        break;
    }
  }

  LLVM_DEBUG(
      dbgs() << "\nExit VPOParoptTransform::genOffloadArraysInitForClause:"
             << " ConstSizes.size()=" << ConstSizes.size()
             << " Match=" << Match << " Cnt=" << Cnt << "\n");
}

// Pass the data to the array of base pointer as well as  array of
// section pointers. If the flag hasRuntimeEvaluationCaptureSize is true,
// the compiler needs to generate the init code for the size array.
void VPOParoptTransform::genOffloadArraysInit(
    WRegionNode *W, TgDataInfo *Info, CallInst *Call, Instruction *InsertPt,
    SmallVectorImpl<Constant *> &ConstSizes,
    bool hasRuntimeEvaluationCaptureSize) {

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::genOffloadArraysInit:"
                    << " ConstSizes.size()=" << ConstSizes.size()
                    << " hasRuntimeEvaluationCaptureSize="
                    << hasRuntimeEvaluationCaptureSize << "\n");

  Value *BPVal;
  IRBuilder<> Builder(InsertPt);
  unsigned Cnt = 0;
  bool Match = false;

  if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
      isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W)) {
    genOffloadArraysInitForClause(W, Info, Call, InsertPt, ConstSizes,
                                  hasRuntimeEvaluationCaptureSize, nullptr,
                                  Match, Builder, Cnt);
    LLVM_DEBUG(dbgs() << "\nExit1 VPOParoptTransform::genOffloadArraysInit:"
                      << " ConstSizes.size()=" << ConstSizes.size() << "\n");
    return;
  }

  for (unsigned II = 0; II < Call->getNumArgOperands(); ++II) {
    BPVal = Call->getArgOperand(II);

    Match = false;
    genOffloadArraysInitForClause(W, Info, Call, InsertPt, ConstSizes,
                                  hasRuntimeEvaluationCaptureSize, BPVal, Match,
                                  Builder, Cnt);

    // For target data don't add BPVals that don't match the map clauses.
    // They should not be sent to the __tgt_target_data_* runtime.
    if (!Match && !isa<WRNTargetDataNode>(W))
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

/// For a given Value \p V, capture it, and rename all its occurrences within
/// the WRegion \p W (including the region entry directive).
///
/// Before:
/// \code
///   %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
///        "QUAL.OMP.FIRSTPRIVATE"(i32* @V) ]
/// \endcode
///
/// After:
/// \code
///   %0 = bitcast i32* @V to i8*
///   %1 = call i8* @llvm.launder.invariant.group.p0i8(i8 * %0)
///   %V1 = bitcast i8* %1 to i32*
///   %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
///        "QUAL.OMP.FIRSTPRIVATE"(i32* %V1) ]
/// \endcode
Value *VPOParoptTransform::genRenamePrivatizationImpl(WRegionNode *W, Value *V,
                                                      BasicBlock *EntryBB,
                                                      Item *IT) {
  IRBuilder<> Builder(EntryBB->getTerminator());
  Value *NewPrivInst = Builder.CreateLaunderInvariantGroup(V);
  NewPrivInst->setName(V->getName());
  genPrivatizationReplacement(W, V, NewPrivInst, IT);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Captured via launder intrinsic: '";
             V->printAsOperand(dbgs()); dbgs() << "'.\n");

  return NewPrivInst;
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
/// This renaming is done using genRenamePrivatizationImpl().
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

  // Map between Original Value V and the renamed value NewV. If no renaming
  // happens, The map will have {V, V}.
  DenseMap<Value *, Value *> RenameMap;
  auto renameGlobalsAndConstExprs = [&](Value *V, Item *It) {
    auto VOrigAndNew = RenameMap.find(V);
    if (VOrigAndNew != RenameMap.end())
      return VOrigAndNew->second;

    if (!isa<GlobalVariable>(V) && !isa<ConstantExpr>(V)) {
      RenameMap.insert({V, V});
      return V;
    }

    Value *VNew = genRenamePrivatizationImpl(W, V, EntryBB, It);
    RenameMap.insert({V, VNew});
    Changed = true;
    return VNew;
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
        VNew = renameGlobalsAndConstExprs(Aggr->getSectionPtr(), MapI);
        Aggr->setSectionPtr(VNew);
        VNew = renameGlobalsAndConstExprs(Aggr->getBasePtr(), MapI);
        Aggr->setBasePtr(VNew);
      }
    }

    VNew = renameGlobalsAndConstExprs(MapI->getOrig(), MapI);
    MapI->setOrig(VNew);
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      VNew = renameGlobalsAndConstExprs(FprivI->getOrig(), FprivI);
      FprivI->setOrig(VNew);
    }
  }

  if (W->canHaveIsDevicePtr()) {
    IsDevicePtrClause &IsDevPtrClause = W->getIsDevicePtr();
    for (IsDevicePtrItem *Item : IsDevPtrClause.items()) {
      VNew = renameGlobalsAndConstExprs(Item->getOrig(), Item);
      Item->setOrig(VNew);
    }
  }

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
#endif // INTEL_COLLAB
