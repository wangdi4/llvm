#if INTEL_COLLAB
//===- VPOParoptTarget.cpp - Transformation of W-Region for offloading --===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Utils/CodeExtractor.h"
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

  for (auto *Item : MpClause.items()) {
    if (Item->getOrig())
      resetValueInIntelClauseGeneric(W, Item->getOrig());
    if (!Item->getIsMapChain())
      continue;
    MapChainTy const &MapChain = Item->getMapChain();
    for (unsigned I = 0; I < MapChain.size(); ++I) {
      MapAggrTy *Aggr = MapChain[I];
      Value *SectionPtr = Aggr->getSectionPtr();
      resetValueInIntelClauseGeneric(W, SectionPtr);
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

  // extract a W-Region to generate a function
  CodeExtractor CE(makeArrayRef(W->bbset_begin(), W->bbset_end()), DT, false,
                   nullptr, nullptr, false, true);

  assert(CE.isEligible());

  // Set up Fn Attr for the new function
  Function *NewF = CE.extractCodeRegion();

  assert(NewF != nullptr && "Expect non-empty outline function");

  // Set up the Calling Convention used by OpenMP Runtime Library
  CallingConv::ID CC = CallingConv::C;
  if (!VPOAnalysisUtils::isTargetSPIRV(F->getParent()))
    NewF->addFnAttr("target.declare", "true");

  DT->verify(DominatorTree::VerificationLevel::Full);

  // Adjust the calling convention for both the function and the
  // call site.
  NewF->setCallingConv(CC);

  // Remove @llvm.dbg.declare, @llvm.dbg.value intrinsics from NewF
  // to prevent verification failures. This is due due to the
  // CodeExtractor not properly handling them at the moment.
  VPOUtils::stripDebugInfoInstrinsics(*NewF);

  assert(NewF->hasOneUse() && "New function should have one use");
  User *U = NewF->user_back();
  CallInst *NewCall = cast<CallInst>(U);
  NewCall->setCallingConv(CC);

  if (VPOAnalysisUtils::isTargetSPIRV(F->getParent()) &&
      hasOffloadCompilation())
    finalizeKernelFunction(W, NewF, NewCall);

  Constant *RegionId = nullptr;
  if (isa<WRNTargetNode>(W)) {
    assert(MT && "target region with no module transform");
    RegionId = MT->registerTargetRegion(W, NewF);
  }

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
    Value *Cmp = Builder.CreateICmpNE(VIf, ConstantInt::get(VIf->getType(), 0));
    TerminatorInst *ThenTerm, *ElseTerm;
    buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, InsertPt);
    InsertPt = ThenTerm;
    Call = genTargetInitCode(W, NewCall, RegionId, InsertPt);
    Builder.SetInsertPoint(ElseTerm);
    Builder.CreateStore(
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), -1),
        OffloadError);
  } else
    Call = genTargetInitCode(W, NewCall, RegionId, InsertPt);

  if (isa<WRNTargetNode>(W)) {
    Builder.SetInsertPoint(InsertPt);
    Builder.CreateStore(Call, OffloadError);

    Builder.SetInsertPoint(NewCall);
    LoadInst *LastLoad = Builder.CreateLoad(OffloadError);
    ConstantInt *ValueZero =
        ConstantInt::getSigned(Type::getInt32Ty(F->getContext()), 0);
    Value *ErrorCompare = Builder.CreateICmpNE(LastLoad, ValueZero);
    TerminatorInst *Term = SplitBlockAndInsertIfThen(ErrorCompare, NewCall,
                                                     false, nullptr, DT, LI);
    Term->getParent()->setName("omp_offload.failed");
    LastLoad->getParent()->getTerminator()->getSuccessor(1)->setName(
        "omp_offload.cont");
    NewCall->removeFromParent();
    NewCall->insertBefore(Term->getParent()->getTerminator());
  } else if (isa<WRNTargetDataNode>(W) || isa<WRNTargetUpdateNode>(W)) {
    NewCall->removeFromParent();
    NewCall->insertAfter(Call);
  } else if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W)) {
    NewCall->removeFromParent();
    NewF->removeFromParent();
  }

  W->resetBBSet(); // Invalidate BBSet after transformations

  Changed = true;

  LLVM_DEBUG(dbgs() << "\nExit VPOParoptTransform::genTargetOffloadingCode\n");
  return Changed;
}

// Set the value in num_teams and thread_limit clause to be empty.
void VPOParoptTransform::resetValueInNumTeamsAndThreadsClause(WRegionNode *W) {
  if (!W->getIsTeams())
    return;

  Value *NumTeamsPtr = W->getNumTeams();
  if (NumTeamsPtr)
    resetValueInIntelClauseGeneric(W, NumTeamsPtr);

  Value *ThreadLimitPtr = W->getThreadLimit();
  if (ThreadLimitPtr)
    resetValueInIntelClauseGeneric(W, ThreadLimitPtr);
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
unsigned VPOParoptTransform::getMapTypeFlag(MapItem *MapI, bool IsFirstExprFlag,
                                            bool IsFirstComponentFlag) {
  unsigned Res = 0u;

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

  if (IsFirstExprFlag)
    Res |= TGT_MAP_IS_PTR;
  if (IsFirstComponentFlag)
    Res |= TGT_MAP_FIRST_REF;

  return Res;
}

// Generate the sizes and map type flags for the given map type, map
// modifier and the expression V.
void VPOParoptTransform::GenTgtInformationForPtrs(
    WRegionNode *W, Value *V, SmallVectorImpl<Constant *> &ConstSizes,
    SmallVectorImpl<uint64_t> &MapTypes,
    bool &hasRuntimeEvaluationCaptureSize) {
  const DataLayout DL = F->getParent()->getDataLayout();

  bool IsFirstExprFlag = true;

  MapClause const &MpClause = W->getMap();
  for (MapItem *MapI : MpClause.items()) {
    if (!isa<WRNTargetEnterDataNode>(W) && !isa<WRNTargetExitDataNode>(W) &&
        (MapI->getNew() != V || !MapI->getOrig()))
      continue;
    Type *T = MapI->getOrig()->getType()->getPointerElementType();
    if (MapI->getIsMapChain()) {
      MapChainTy const &MapChain = MapI->getMapChain();
      for (unsigned I = 0; I < MapChain.size(); ++I) {
        MapAggrTy *Aggr = MapChain[I];
        auto ConstValue = dyn_cast<ConstantInt>(Aggr->getSize());
        if (!ConstValue) {
          hasRuntimeEvaluationCaptureSize = true;
          ConstSizes.push_back(ConstantInt::get(
              IntelGeneralUtils::getSizeTTy(F), DL.getTypeAllocSize(T)));
        } else
          ConstSizes.push_back(ConstValue);
        MapTypes.push_back(
            getMapTypeFlag(MapI, !IsFirstExprFlag, I == 0 ? true : false));
      }
    } else {
      ConstSizes.push_back(ConstantInt::get(IntelGeneralUtils::getSizeTTy(F),
                                            DL.getTypeAllocSize(T)));
      MapTypes.push_back(getMapTypeFlag(MapI, !IsFirstExprFlag, true));
    }

    IsFirstExprFlag = false;
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      if (FprivI->getOrig() != V)
        continue;
      if (FprivI->getInMap())
        continue;
      Type *T = FprivI->getOrig()->getType()->getPointerElementType();
      ConstSizes.push_back(ConstantInt::get(IntelGeneralUtils::getSizeTTy(F),
                                            DL.getTypeAllocSize(T)));
      MapTypes.push_back(TGT_MAP_TO);
    }
  }

  if (W->canHaveIsDevicePtr()) {
    IsDevicePtrClause &IDevicePtrClause = W->getIsDevicePtr();
    for (IsDevicePtrItem *IsDevicePtrI : IDevicePtrClause.items()) {
      if (IsDevicePtrI->getNew() != V)
        continue;
      Type *T = IntelGeneralUtils::getSizeTTy(F);
      ConstSizes.push_back(ConstantInt::get(T, DL.getTypeAllocSize(T)));
      MapTypes.push_back(TGT_MAP_PRIVATE_VAL | TGT_MAP_FIRST_REF);
    }
  }
  if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca() == V) {
    Type *T = W->getParLoopNdInfoAlloca()->getType()->getPointerElementType();
    ConstSizes.push_back(ConstantInt::get(IntelGeneralUtils::getSizeTTy(F),
                                          DL.getTypeAllocSize(T)));
    MapTypes.push_back(TGT_MAPTYPE_ND_DESC);
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

    if (isa<WRNTargetEnterDataNode>(W) || isa<WRNTargetExitDataNode>(W))
      GenTgtInformationForPtrs(W, nullptr, ConstSizes, MapTypes,
                               hasRuntimeEvaluationCaptureSize);
    else {
      for (unsigned II = 0; II < Call->getNumArgOperands(); ++II) {
        Value *BPVal = Call->getArgOperand(II);
        GenTgtInformationForPtrs(W, BPVal, ConstSizes, MapTypes,
                                 hasRuntimeEvaluationCaptureSize);
      }
      if (isa<WRNTargetNode>(W) && W->getParLoopNdInfoAlloca())
        GenTgtInformationForPtrs(W, W->getParLoopNdInfoAlloca(), ConstSizes,
                                 MapTypes, hasRuntimeEvaluationCaptureSize);
    }

    Info.NumberOfPtrs = MapTypes.size();

    Value *SizesArray;

    if (hasRuntimeEvaluationCaptureSize)
      SizesArray = Builder.CreateAlloca(
          ArrayType::get(Builder.getInt8PtrTy(), Info.NumberOfPtrs), nullptr,
          ".offload_sizes");
    else {
      auto *SizesArrayInit = ConstantArray::get(
          ArrayType::get(IntelGeneralUtils::getSizeTTy(F), ConstSizes.size()),
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

  genOffloadArraysArgument(&Info, InsertPt, hasRuntimeEvaluationCaptureSize);

  CallInst *TgtCall;
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
    genOffloadArraysArgument(&Info, Call, hasRuntimeEvaluationCaptureSize);
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
    S = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->DataSizes, 0, Cnt);

    if (Size && !dyn_cast<ConstantInt>(Size))
      SizeValue = Size;
    else
      SizeValue = ConstSizes[Cnt];
    Builder.CreateStore(genCastforAddr(SizeValue, Builder), S);
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
        (MapI->getNew() != BPVal || !MapI->getOrig()))
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
      }
    } else
      genOffloadArraysInitUtil(Builder, BPVal, BPVal, nullptr, Info, ConstSizes,
                               Cnt, hasRuntimeEvaluationCaptureSize);
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

    if (!Match)
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
    TgDataInfo *Info, Instruction *InsertPt,
    bool hasRuntimeEvaluationCaptureSize) {
  IRBuilder<> Builder(InsertPt);

  if (Info->NumberOfPtrs) {
    Info->ResBaseDataPtrs = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->BaseDataPtrs, 0, 0);
    Info->ResDataPtrs = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(Builder.getInt8PtrTy(), Info->NumberOfPtrs),
        Info->DataPtrs, 0, 0);
    Info->ResDataSizes = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(hasRuntimeEvaluationCaptureSize
                           ? Builder.getInt8PtrTy()
                           : IntelGeneralUtils::getSizeTTy(F),
                       Info->NumberOfPtrs),
        Info->DataSizes, 0, 0);
    Info->ResDataMapTypes = Builder.CreateConstInBoundsGEP2_32(
        ArrayType::get(IntelGeneralUtils::getSizeTTy(F), Info->NumberOfPtrs),
        Info->DataMapTypes, 0, 0);
  } else {
    Info->ResBaseDataPtrs = ConstantPointerNull::get(
        PointerType::getUnqual(Builder.getInt8PtrTy()));
    Info->ResDataPtrs = ConstantPointerNull::get(
        PointerType::getUnqual(Builder.getInt8PtrTy()));
    Info->ResDataSizes = ConstantPointerNull::get(PointerType::getUnqual(
                               IntelGeneralUtils::getSizeTTy(F)));
    Info->ResDataMapTypes = ConstantPointerNull::get(
        PointerType::getUnqual(IntelGeneralUtils::getSizeTTy(F)));
  }
}

// The utility to generate the stack variable to pass the value of
// global variable.
Value *VPOParoptTransform::genGlobalPrivatizationImpl(WRegionNode *W,
                                                      GlobalVariable *G,
                                                      BasicBlock *EntryBB,
                                                      BasicBlock *NextExitBB,
                                                      Item *IT) {
  G->setTargetDeclare(true);
  Instruction *InsertPt = EntryBB->getFirstNonPHI();
  const DataLayout &DL = InsertPt->getModule()->getDataLayout();
  auto NewPrivInst = genPrivatizationAlloca(W, G, InsertPt, ".priv.mp");
  genPrivatizationReplacement(W, G, NewPrivInst, IT);

  if (!VPOUtils::canBeRegisterized(NewPrivInst->getAllocatedType(), DL)) {
    VPOUtils::genMemcpy(NewPrivInst, G, DL, NewPrivInst->getAlignment(),
                        InsertPt->getParent());
    VPOUtils::genMemcpy(G, NewPrivInst, DL, NewPrivInst->getAlignment(),
                        NextExitBB);
  } else {
    LoadInst *Load = new LoadInst(G);
    Load->insertAfter(cast<Instruction>(NewPrivInst));
    StoreInst *Store = new StoreInst(Load, NewPrivInst);
    Store->insertAfter(Load);
    IRBuilder<> Builder(NextExitBB->getTerminator());
    Builder.CreateStore(Builder.CreateLoad(NewPrivInst), G);
  }
  return NewPrivInst;
}

// Replace the new generated local variables with global variables
// in the target initialization code.
bool VPOParoptTransform::finalizeGlobalPrivatizationCode(WRegionNode *W) {
  MapClause const &MpClause = W->getMap();

  for (MapItem *MapI : MpClause.items()) {
    Value *Orig = MapI->getOrig();
    if (!Orig)
      continue;
    GlobalVariable *G = dyn_cast<GlobalVariable>(Orig);
    if (!G) {
      LoadInst *LI = dyn_cast<LoadInst>(Orig);
      if (LI)
        G = dyn_cast<GlobalVariable>(LI->getPointerOperand());
    }
    if (G) {
      MapI->getNew()->replaceAllUsesWith(G);
    }
  }
  return false;
}

// If the incoming data is global variable, Create the stack variable and
// replace the the global variable with the stack variable.
bool VPOParoptTransform::genGlobalPrivatizationCode(WRegionNode *W) {
  MapClause const &MpClause = W->getMap();
  BasicBlock *EntryBB = &(F->getEntryBlock());
  BasicBlock *ExitBB = W->getExitBBlock();
  BasicBlock *NextExitBB = SplitBlock(ExitBB, ExitBB->getTerminator(), DT, LI);
  bool Changed = false;


  for (MapItem *MapI : MpClause.items()) {
    Value *Orig = MapI->getOrig();
    if (!Orig)
      continue;
    GlobalVariable *G = dyn_cast<GlobalVariable>(Orig);
    if (!G) {
      LoadInst *LI = dyn_cast<LoadInst>(Orig);
      if (LI)
        G = dyn_cast<GlobalVariable>(LI->getPointerOperand());
    }
    if (G) {
      if (!Changed)
        W->populateBBSet();
      Value *NewPrivInst =
          genGlobalPrivatizationImpl(W, G, EntryBB, NextExitBB, MapI);

      MapI->setNew(NewPrivInst);
      Changed = true;
    } else
      // The New is set to be the same as Orig for local firstprivate so that
      // global/local firstprivate can be processed in a unified way in the
      // later OMP code generation.
      MapI->setNew(Orig);
  }

  if (W->canHaveFirstprivate()) {
    FirstprivateClause &FprivClause = W->getFpriv();
    for (FirstprivateItem *FprivI : FprivClause.items()) {
      Value *Orig = FprivI->getOrig();
      MapItem *MapI = FprivI->getInMap();
      if (MapI) {
        FprivI->setNew(MapI->getNew());
        continue;
      }
      if (GlobalVariable *G = dyn_cast<GlobalVariable>(Orig)) {
        if (!Changed)
          W->populateBBSet();

        Value *NewPrivInst =
            genGlobalPrivatizationImpl(W, G, EntryBB, NextExitBB, FprivI);

        FprivI->setNew(NewPrivInst);
        // The Orig of the firstprivate is set to the new private alloca
        // instruction so that the corresponding firstprivate item will
        // not be processed later.
        FprivI->setOrig(NewPrivInst);
        Changed = true;
      } else
        FprivI->setNew(Orig);
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
          W, Orig, EntryBB->getFirstNonPHI(), ".isdeviceptr");
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
  for (const auto &T : TgtDeviceTriples) {
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
