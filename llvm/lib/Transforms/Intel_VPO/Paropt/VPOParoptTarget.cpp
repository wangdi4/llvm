#if INTEL_COLLAB
//===- VPOParoptTarget.cpp - Transformation of W-Region for offloading ----===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTarget.cpp implements the omp target feature.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

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
#include "llvm/Transforms/Utils/Intel_InferAddressSpacesUtils.h"

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target"

// Reset the value in the Map clause to be empty.
void VPOParoptTransform::resetValueInMapClause(WRegionNode *W) {
  if (!W->canHaveMap())
    return;

  MapClause const &MpClause = W->getMap();
  if (MpClause.empty())
    return;

  IRBuilder<> Builder(W->getEntryBBlock()->getFirstNonPHI());

  for (auto *Item : MpClause.items()) {
    if (Item->getOrig())
      resetValueInIntelClauseGeneric(W, Item->getOrig());
    if (!Item->getIsMapChain())
      continue;
    MapChainTy const &MapChain = Item->getMapChain();
    for (int I = MapChain.size() - 1; I >= 0; --I) {
      MapAggrTy *Aggr = MapChain[I];
      Value *BasePtr = Aggr->getBasePtr();
      if (I != 0)
        resetValueInIntelClauseGeneric(W, BasePtr);
      Value *SectionPtr = Aggr->getSectionPtr();
      resetValueInIntelClauseGeneric(W, SectionPtr);

      if (deviceTriplesHasSPIRV() && MapChain.size() > 1 &&
          I != 0)
        Builder.CreateStore(SectionPtr, BasePtr);

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
  resetValueInIsDevicePtrClause(W);
  resetValueInPrivateClause(W);
  resetValueInMapClause(W);

  bool Changed = false;

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
    } else if (isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W)) {
      NewCall->removeFromParent();
      NewCall->insertAfter(Call);
    } else if (isa<WRNTargetEnterDataNode>(W) ||
               isa<WRNTargetExitDataNode>(W)) {
      NewCall->removeFromParent();
      NewF->removeFromParent();
    }
  }

  W->resetBBSet(); // Invalidate BBSet after transformations

  Changed = true;

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genTargetOffloadingCode\n");
  return Changed;
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
  else if (MapI->getIsMapTo() || MapI->getInFirstprivate())
    Res = TGT_MAP_TO;
  else if (MapI->getIsMapFrom())
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
  const DataLayout DL = F->getParent()->getDataLayout();
  LLVMContext &C = F->getContext();

  MapClause const &MpClause = W->getMap();
  bool Match = false;
  for (MapItem *MapI : MpClause.items()) {
    if (!isa<WRNTargetEnterDataNode>(W) && !isa<WRNTargetExitDataNode>(W) &&
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
        if (deviceTriplesHasSPIRV() && MapChain.size() > 1 &&
            I == 0)
          break;
      }
    } else {
      ConstSizes.push_back(ConstantInt::get(Type::getInt64Ty(C),
                                            DL.getTypeAllocSize(T)));
      MapTypes.push_back(getMapTypeFlag(MapI, !IsFirstExprFlag, true, true));
    }
    IsFirstExprFlag = false;
  }
  if (deviceTriplesHasSPIRV() && Match == false &&
      !isa<WRNTargetEnterDataNode>(W) && !isa<WRNTargetExitDataNode>(W)) {
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
  if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W))
    Info.NumberOfPtrs = 1;

  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
    Info.NumberOfPtrs++;

  if (Info.NumberOfPtrs) {

    SmallVector<Constant *, 16> ConstSizes;
    SmallVector<uint64_t, 16> MapTypes;
    bool IsFirstExprFlag = true;

    if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W))
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

    if (Size && !dyn_cast<ConstantInt>(Size))
      SizeValue = Size;
    else
      SizeValue = ConstSizes[Cnt];
    Builder.CreateStore(
        Builder.CreateSExt(SizeValue, Type::getInt64Ty(C)), S);
  }
  Cnt++;
}

// Generate the target intialization code for the pointers based
// on the order of the map clause.
void VPOParoptTransform::genOffloadArraysInitForClause(
    WRegionNode *W, TgDataInfo *Info, CallInst *Call, Instruction *InsertPt,
    SmallVectorImpl<Constant *> &ConstSizes,
    bool hasRuntimeEvaluationCaptureSize, Value *BPVal, bool &Match,
    IRBuilder<> &Builder, unsigned &Cnt) {
  MapClause const &MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    if (!isa<WRNTargetEnterDataNode>(W) && !isa<WRNTargetExitDataNode>(W) &&
        (MapI->getOrig() != BPVal || !MapI->getOrig()))
      continue;
    if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W))
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
            I == 0)
          break;
      }
    } else
      genOffloadArraysInitUtil(Builder, BPVal, BPVal, nullptr, Info, ConstSizes,
                               Cnt, hasRuntimeEvaluationCaptureSize);
  }

  if (deviceTriplesHasSPIRV() && Match == false &&
      !isa<WRNTargetEnterDataNode>(W) && !isa<WRNTargetExitDataNode>(W)) {
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
}

// Pass the data to the array of base pointer as well as  array of
// section pointers. If the flag hasRuntimeEvaluationCaptureSize is true,
// the compiler needs to generate the init code for the size array.
void VPOParoptTransform::genOffloadArraysInit(
    WRegionNode *W, TgDataInfo *Info, CallInst *Call, Instruction *InsertPt,
    SmallVectorImpl<Constant *> &ConstSizes,
    bool hasRuntimeEvaluationCaptureSize) {
  Value *BPVal;
  IRBuilder<> Builder(InsertPt);
  unsigned Cnt = 0;
  bool Match = false;

  if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W)) {
    genOffloadArraysInitForClause(W, Info, Call, InsertPt, ConstSizes,
                                  hasRuntimeEvaluationCaptureSize, nullptr,
                                  Match, Builder, Cnt);
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
}

// Generate the pointers pointing to the array of base pointer, the
// array of section pointers, the array of sizes, the array of map types.
void VPOParoptTransform::genOffloadArraysArgument(
    TgDataInfo *Info, Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);

  LLVMContext &C = F->getContext();

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

// Given a global variable reference in a OMP target construct, the
// corresponding target outline function needs to pass the address of
// global variable as one of its arguments. The utility CodeExtractor which
// is used by the paropt cannot generate such argument since the global
// variable is live in the module. In order to help the CodeExtractor to
// achieve this, the following utilty is used to generate place holder for
// global variable. The later outline function can have corresponding
// argument for this global variable.
//
// %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
//      "QUAL.OMP.FIRSTPRIVATE"(i32* @pvtPtr) ]
// ===>
// entry:
//   %0 = call i8* @llvm.launder.invariant.group.p0i8
//                  (i8* bitcast (i32* @pvtPtr to i8*))
//   %1 = bitcast i8* %0 to i32*
//   br label %entry.split
// ...
// %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
//      "QUAL.OMP.FIRSTPRIVATE"(i32* %1) ]
Value *VPOParoptTransform::genRenamePrivatizationImpl(WRegionNode *W,
                                                      Value *V,
                                                      BasicBlock *EntryBB,
                                                      BasicBlock *NextExitBB,
                                                      Item *IT) {
  IRBuilder<> Builder(EntryBB->getFirstNonPHI());
  Value *NewPrivInst = Builder.CreateLaunderInvariantGroup(V);
  genPrivatizationReplacement(W, V, NewPrivInst, IT);
  return NewPrivInst;
}

// Replace the new generated local variables with global variables
// in the target initialization code.
bool VPOParoptTransform::finalizeGlobalPrivatizationCode(WRegionNode *W) {

  // Clean up the fence call and replace the use of its return value
  // with call's operand.
  auto propagateGlobals = [&](Value *Orig) {
    if (!Orig)
      return;
    BitCastInst *BI = dyn_cast<BitCastInst>(Orig);
    if (!BI)
      return;
    Value *V = BI->getOperand(0);
    CallInst *CI = dyn_cast<CallInst>(V);
    if (CI && isFenceCall(CI)) {
      CI->replaceAllUsesWith(CI->getOperand(0));
      CI->eraseFromParent();
    }
  };

  MapClause const &MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    Value *Orig = MapI->getOrig();
    propagateGlobals(Orig);
    if (MapI->getIsMapChain()) {
      MapChainTy const &MapChain = MapI->getMapChain();
      for (int I = MapChain.size() - 1; I >= 0; --I) {
        MapAggrTy *Aggr = MapChain[I];
        Value *SectionPtr = Aggr->getSectionPtr();
        propagateGlobals(SectionPtr);
      }
    }
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *Orig = FprivI->getOrig();
      propagateGlobals(Orig);
    }
  }
  return false;
}

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

// If the incoming data is global variable, Create the stack variable and
// replace the the global variable with the stack variable.
bool VPOParoptTransform::genGlobalPrivatizationCode(WRegionNode *W) {
  MapClause const &MpClause = W->getMap();
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewEntryBB =
      SplitBlock(EntryBB, EntryBB->getFirstNonPHI(), DT, LI);
  W->setEntryBBlock(NewEntryBB);
  BasicBlock *ExitBB = W->getExitBBlock();
  BasicBlock *NextExitBB = SplitBlock(ExitBB, ExitBB->getTerminator(), DT, LI);
  bool Changed = false;

  for (MapItem *MapI : MpClause.items()) {
    Value *Orig = MapI->getOrig();
    // The map section pointer needs to renamed so that the following cse
    // optimization can be inhibited.
    // Before early-cse:
    //
    //entry:
    //  %sbox = alloca i32*, align 8
    //  store i32* getelementptr inbounds ([256 x i32],
    //  [256 x i32]* @g_sbox, i32 0, i32 0), i32** %sbox
    //  %1 = load i32*, i32** %sbox, align 8
    //  %arrayidx = getelementptr inbounds i32, i32* %1, i64 0
    //  br label %DIR.OMP.TARGET.1
    //
    //DIR.OMP.TARGET.1:
    //  %2 = call token @llvm.directive.region.entry() [
    //         "DIR.OMP.TARGET"(),
    //         "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    //         "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %sbox,
    //         i32** %sbox, i64 8),
    //         "QUAL.OMP.MAP.TO:AGGR"(i32** %sbox,
    //         i32* %arrayidx, i64 1024),
    //         ... ]
    //
    //After early-cse:
    //
    //entry:
    //  %sbox = alloca i32*, align 8
    //  store i32* getelementptr inbounds ([256 x i32],
    //  [256 x i32]* @g_sbox, i32 0, i32 0), i32** %sbox
    //  %1 = load i32*, i32** %sbox, align 8
    //  %arrayidx = getelementptr inbounds i32, i32* %1, i64 0
    //  br label %DIR.OMP.TARGET.1
    //
    //DIR.OMP.TARGET.1:
    //  %2 = call token @llvm.directive.region.entry() [
    //         "DIR.OMP.TARGET"(),
    //         "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    //         "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %sbox, i32** %sbox, i64 8),
    //         "QUAL.OMP.MAP.TO:AGGR"(i32** %sbox,
    //         i32* getelementptr inbounds ([256 x i32],
    //         [256 x i32]* @g_sbox, i32 0, i32 0), i64 1024)
    //         ... ]
    // The solution is to rename the value %arrayidx to inbit this early-cse
    // optimizaiton.

    if (MapI->getIsMapChain()) {
      MapChainTy const &MapChain = MapI->getMapChain();
      for (int I = MapChain.size() - 1; I >= 0; --I) {
        MapAggrTy *Aggr = MapChain[I];
        Value *SectionPtr = Aggr->getSectionPtr();
        if (auto *GEP = dyn_cast<GetElementPtrInst>(SectionPtr)) {
          if (!Changed)
            W->populateBBSet();
          genRenamePrivatizationImpl(W, GEP, EntryBB, NextExitBB, MapI);
          Changed = true;
        }
      }
    }
    if (!Orig)
      continue;
    GlobalVariable *G = dyn_cast<GlobalVariable>(Orig);

    if (G) {
      if (!Changed)
        W->populateBBSet();
      genRenamePrivatizationImpl(W, G, EntryBB, NextExitBB, MapI);
      Changed = true;
    }
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *Orig = FprivI->getOrig();
      MapItem *MapI = FprivI->getInMap();
      if (MapI)
        continue;

      if (GlobalVariable *G = dyn_cast<GlobalVariable>(Orig)) {
        if (!Changed)
          W->populateBBSet();

        genRenamePrivatizationImpl(W, G, EntryBB, NextExitBB, FprivI);

        Changed = true;
      }
    }
  }

  if (Changed)
    W->resetBBSet();
  return Changed;
}

// Pass the value of the DevicePtr to the outlined function.
bool VPOParoptTransform::genDevicePtrPrivationCode(WRegionNode *W) {
  bool Changed = false;
  if (!W->canHaveIsDevicePtr())
    return Changed;
  IsDevicePtrClause &IDevicePtrClause = W->getIsDevicePtr();
  if (!IDevicePtrClause.empty()) {
    W->populateBBSet();
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *PrivInitEntryBB = nullptr;
    Changed = true;
    IRBuilder<> Builder(W->getPredBBlock()->getTerminator());
    for (IsDevicePtrItem *IsDevicePtrI : IDevicePtrClause.items()) {
      Value *Orig = IsDevicePtrI->getOrig();
      Value *NewPrivInst = genPrivatizationAlloca(
          IsDevicePtrI, EntryBB->getFirstNonPHI(), ".isdeviceptr");
      genPrivatizationReplacement(W, Orig, NewPrivInst, IsDevicePtrI);
      IsDevicePtrI->setNew(NewPrivInst);
      createEmptyPrvInitBB(W, PrivInitEntryBB);
      Builder.SetInsertPoint(W->getPredBBlock()->getTerminator());
      LoadInst *Load = Builder.CreateLoad(IsDevicePtrI->getOrig());
      Builder.SetInsertPoint(PrivInitEntryBB->getTerminator());
      Builder.CreateStore(Load, NewPrivInst);
      IsDevicePtrI->setNew(Load);
    }
  }
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
#endif // INTEL_COLLAB
