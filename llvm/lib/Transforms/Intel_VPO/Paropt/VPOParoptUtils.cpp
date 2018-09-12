#if INTEL_COLLAB
//==-- VPOParoptUtils.cpp - Utilities for VPO Paropt Transforms -*- C++ -*--==//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Nov 2015: Initial Implementation of OpenMP runtime APIs (Xinmin Tian)
//
//==------------------------------------------------------------------------==//
///
/// \file
/// This file provides a set of utilities for VPO Paropt Transformations to
/// generate OpenMP runtime API call instructions.
///
//==------------------------------------------------------------------------==//

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptUtils.h"
#include <string>

#define DEBUG_TYPE "vpo-paropt-utils"

using namespace llvm;
using namespace llvm::vpo;

static const unsigned StackAdjustedAlignment = 16;

// This function generates a runtime library call to __kmpc_begin(&loc, 0)
CallInst *VPOParoptUtils::genKmpcBeginCall(Function *F, Instruction *AI,
                                           StructType *IdentTy) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *KmpcLoc =
      genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, &B, &E);

  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

  Constant *FnC = M->getOrInsertFunction("__kmpc_begin", Type::getVoidTy(C),
                                         PointerType::getUnqual(IdentTy),
                                         Type::getInt32Ty(C));

  Function *FnKmpcBegin = cast<Function>(FnC);

  FnKmpcBegin->setCallingConv(CallingConv::C);

  std::vector<Value *> FnKmpcBeginArgs;
  FnKmpcBeginArgs.push_back(KmpcLoc);
  FnKmpcBeginArgs.push_back(ValueZero);

  CallInst *KmpcBeginCall = CallInst::Create(FnKmpcBegin, FnKmpcBeginArgs);
  KmpcBeginCall->setCallingConv(CallingConv::C);

  return KmpcBeginCall;
}

// This function generates a runtime library call to __kmpc_end(&loc)
CallInst *VPOParoptUtils::genKmpcEndCall(Function *F, Instruction *AI,
                                         StructType *IdentTy) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *KmpcLoc =
      genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, &B, &E);

  Constant *FnC = M->getOrInsertFunction("__kmpc_end", Type::getVoidTy(C),
                                         PointerType::getUnqual(IdentTy));

  Function *FnKmpcEnd = cast<Function>(FnC);

  FnKmpcEnd->setCallingConv(CallingConv::C);

  std::vector<Value *> FnKmpcEndArgs;
  FnKmpcEndArgs.push_back(KmpcLoc);

  CallInst *KmpcEndCall = CallInst::Create(FnKmpcEnd, FnKmpcEndArgs);
  KmpcEndCall->setCallingConv(CallingConv::C);

  return KmpcEndCall;
}


// This function generates a runtime library call to __kmpc_ok_to_fork(&loc)
CallInst *VPOParoptUtils::genKmpcForkTest(WRegionNode *W, StructType *IdentTy,
                                          Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();

  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  FunctionType *FnForkTestTy = FunctionType::get(
      Type::getInt32Ty(C), PointerType::getUnqual(IdentTy), false);

  Function *FnForkTest = M->getFunction("__kmpc_ok_to_fork");

  if (!FnForkTest) {
    FnForkTest = Function::Create(FnForkTestTy, GlobalValue::ExternalLinkage,
                                  "__kmpc_ok_to_fork", M);
    FnForkTest->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnForkTestArgs;
  FnForkTestArgs.push_back(Loc);

  CallInst *ForkTestCall = CallInst::Create(FnForkTest, FnForkTestArgs,
                                            "fork.test", InsertPt);
  ForkTestCall->setCallingConv(CallingConv::C);
  ForkTestCall->setTailCall(true);

  return ForkTestCall;
}

/// Update loop scheduling kind based on ordered clause and chunk
/// size information
WRNScheduleKind VPOParoptUtils::genScheduleKind(WRNScheduleKind Kind,
                                                bool IsOrdered, int Chunk)
{
  if (IsOrdered) {
    switch (Kind) {
    case WRNScheduleStatic:
      if (Chunk == 0)
        return WRNScheduleOrderedStaticEven;
      else
        return WRNScheduleOrderedStatic;
    case WRNScheduleStaticEven:
      return WRNScheduleOrderedStaticEven;
    case WRNScheduleDynamic:
      return WRNScheduleOrderedDynamic;
    case WRNScheduleGuided:
      return WRNScheduleOrderedGuided;
    case WRNScheduleRuntime:
      return WRNScheduleOrderedRuntime;
    case WRNScheduleAuto:
      return WRNScheduleOrderedAuto;
    case WRNScheduleTrapezoidal:
      return WRNScheduleOrderedTrapezoidal;
    case WRNScheduleStaticGreedy:
      return WRNScheduleOrderedStaticGreedy;
    case WRNScheduleStaticBalanced:
      return WRNScheduleOrderedStaticBalanced;
    case WRNScheduleGuidedIterative:
      return WRNScheduleOrderedGuidedIterative;
    case WRNScheduleGuidedAnalytical:
      return WRNScheduleOrderedGuidedAnalytical;
    default:
      return WRNScheduleOrderedStaticEven;
    }
  }
  else if (Chunk == 0 && Kind == WRNScheduleStatic)
    return WRNScheduleStaticEven;

  return Kind;
}

// Query scheduling type based on ordered clause and chunk size
// information. The values of the enums are used to invoke the
// RTL, so do not change them.
//
// typedef enum WRNScheduleKind {
//    WRNScheduleCrewloop                = 18,
//    WRNScheduleStatic                  = 33,
//    WRNScheduleStaticEven              = 34,
//    WRNScheduleDynamic                 = 35,
//    WRNScheduleGuided                  = 36,
//    WRNScheduleRuntime                 = 37,
//    WRNScheduleAuto                    = 38,
//    WRNScheduleTrapezoidal             = 39,
//    WRNScheduleStaticGreedy            = 40,
//    WRNScheduleStaticBalanced          = 41,
//    WRNScheduleGuidedIterative         = 42,
//    WRNScheduleGuidedAnalytical        = 43,
//
//    WRNScheduleOrderedStatic           = 65,
//    WRNScheduleOrderedStaticEven       = 66,
//    WRNScheduleOrderedDynamic          = 67,
//    WRNScheduleOrderedGuided           = 68,
//    WRNScheduleOrderedRuntime          = 69,
//    WRNScheduleOrderedAuto             = 70,
//
//    WRNScheduleOrderedTrapezoidal      = 71,
//    WRNScheduleOrderedStaticGreedy     = 72,
//    WRNScheduleOrderedStaticBalanced   = 73,
//    WRNScheduleOrderedGuidedIterative  = 74,
//    WRNScheduleOrderedGuidedAnalytical = 75,
//
//    WRNScheduleDistributeStatic        = 91,
//    WRNScheduleDistributeStaticEven    = 92
// } WRNScheduleKind;
WRNScheduleKind VPOParoptUtils::getLoopScheduleKind(WRegionNode *W)
{
  if (W->canHaveSchedule()) {
    // E.g., W could be WRNParallelLoop or WRNWksLoop
    auto Schedule  = W->getSchedule();

    auto Kind   = Schedule.getKind();
    auto Chunk  = Schedule.getChunk();

    bool IsOrdered = (W->getOrdered() == 0);
    return VPOParoptUtils::genScheduleKind(Kind, IsOrdered, Chunk);
  }
  else
    // else W could be WRNParallelSections or WRNSections
    return WRNScheduleStaticEven;
}

// Query scheduling type based on dist_schedule clause and chunk size
// information. The values of the enums are used to invoke the RTL, so
// do not change them.
WRNScheduleKind VPOParoptUtils::getDistLoopScheduleKind(WRegionNode *W)
{
  if (W->canHaveDistSchedule()) {
    auto DistSchedule = W->getDistSchedule();

    auto Kind   = DistSchedule.getKind();
    auto Chunk  = DistSchedule.getChunk();

    if (Chunk != 0) return Kind;
  }
  return WRNScheduleDistributeStaticEven;
}

// Generate a call to set `num_teams` for the `teams` region. Example:
// \code
//  call void @__kmpc_push_num_teams(%ident_t* %loc, i32 %tid, i32 %ntms,
//                                   i32 %nths)
// \endcode
CallInst *VPOParoptUtils::genKmpcPushNumTeams(WRegionNode *W,
                                              StructType *IdentTy, Value *Tid,
                                              Value *NumTeams,
                                              Value *NumThreads,
                                              Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  Module *M = F->getParent();

  StringRef FnName;

  FnName = "__kmpc_push_num_teams";

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");
  Type *Int32Ty = Type::getInt32Ty(C);
  Value *Zero = ConstantInt::get(Int32Ty, 0);

  SmallVector<Value *, 3> FnArgs;
  FnArgs.push_back(Loc);
  FnArgs.push_back(Tid);
  if (NumTeams)
    FnArgs.push_back(NumTeams);
  else
    FnArgs.push_back(Zero);

  if (NumThreads)
    FnArgs.push_back(NumThreads);
  else
    FnArgs.push_back(Zero);

  Type *RetTy = Type::getVoidTy(C);

  // Generate __kmpc_push_num_teams(loc, tid, num_teams, num_threads) in IR
  CallInst *PushNumTeams = genCall(M, FnName, RetTy, FnArgs);
  PushNumTeams->insertBefore(InsertPt);

  return PushNumTeams;
}

// This function generates a call to set num_threads for the parallel
// region and parallel loop/sections
//
// call void @__kmpc_push_num_threads(%ident_t* %loc, i32
// Builder.CreateBitCast(SharedGep, PointerType::getUnqual(%tid, i32 %nths)
CallInst *VPOParoptUtils::genKmpcPushNumThreads(WRegionNode *W,
                                                StructType *IdentTy, Value *Tid,
                                                Value *NumThreads,
                                                Instruction *InsertPt) {
  BasicBlock  *B = W->getEntryBBlock();
  BasicBlock  *E = W->getExitBBlock();

  Function    *F = B->getParent();
  LLVMContext &C = F->getContext();

  Module *M = F->getParent();

  StringRef FnName;

  FnName = "__kmpc_push_num_threads";

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *Loc =
    genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  SmallVector<Value *, 3> FnArgs {Loc, Tid, NumThreads};

  Type *RetTy = Type::getVoidTy(C);

  // Generate __kmpc_push_num_threads(loc, tid, num_threads) in IR
  CallInst *PushNumThreads = genCall(M, FnName, RetTy, FnArgs);
  PushNumThreads->insertBefore(InsertPt);

  return PushNumThreads;
}

// This function generates a call as follows.
//    i8* @__kmpc_task_reduction_get_th_data(i32, i8*, i8*)
CallInst *VPOParoptUtils::genKmpcRedGetNthData(WRegionNode *W, Value *TidPtr,
                                               Value *SharedGep,
                                               Instruction *InsertPt,
                                               bool UseTbb) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = W->getEntryBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  Value *RedGetNthDataArgs[] = {
      Builder.CreateLoad(TidPtr),
      ConstantPointerNull::get(Type::getInt8PtrTy(C)),
      Builder.CreateBitCast(SharedGep, Type::getInt8PtrTy(C))};

  Type *TypeParams[] = {Type::getInt32Ty(C), Type::getInt8PtrTy(C),
                        Type::getInt8PtrTy(C)};
  FunctionType *FnTy =
      FunctionType::get(Type::getInt8PtrTy(C), TypeParams, false);

  StringRef FnName = UseTbb ? "__tbb_omp_task_reduction_get_th_data" :
                              "__kmpc_task_reduction_get_th_data";

  Function *FnRedGetNthData = M->getFunction(FnName);

  if (!FnRedGetNthData) {
    FnRedGetNthData =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);
    FnRedGetNthData->setCallingConv(CallingConv::C);
  }

  CallInst *RedGetNthDataCall =
      CallInst::Create(FnRedGetNthData, RedGetNthDataArgs, "", InsertPt);
  RedGetNthDataCall->setCallingConv(CallingConv::C);
  RedGetNthDataCall->setTailCall(false);

  return RedGetNthDataCall;
}

// This function generates a call as follows.
//   void @__kmpc_omp_taskwait({ i32, i32, i32, i32, i8* }*, i32)
CallInst *VPOParoptUtils::genKmpcTaskWait(WRegionNode *W, StructType *IdentTy,
                                          Value *TidPtr,
                                          Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  int Flags = KMP_IDENT_KMPC;
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  Value *TaskArgs[] = {Loc, Builder.CreateLoad(TidPtr)};
  Type *TypeParams[] = {Loc->getType(), Type::getInt32Ty(C)};
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), TypeParams, false);

  StringRef FnName = "__kmpc_omp_taskwait";
  Function *FnTaskWait = M->getFunction(FnName);

  if (!FnTaskWait) {
    FnTaskWait =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);
    FnTaskWait->setCallingConv(CallingConv::C);
  }

  CallInst *TaskWaitCall = CallInst::Create(FnTaskWait, TaskArgs, "", InsertPt);
  TaskWaitCall->setCallingConv(CallingConv::C);
  TaskWaitCall->setTailCall(false);

  return TaskWaitCall;
}

/// \brief Build int32_t __tgt_target_data_begin(int64_t  device_id,
///                                              int32_t  num_args,
///                                              void**   args_base,
///                                              void**   args,
///                                              int64_t* args_size,
///                                              int64_t* args_maptype)
///
CallInst *VPOParoptUtils::genTgtTargetDataBegin(WRegionNode *W, int NumArgs,
                                                Value *ArgsBase, Value *Args,
                                                Value *ArgsSize,
                                                Value *ArgsMaptype,
                                                Instruction *InsertPt) {
  assert((isa<WRNTargetDataNode>(W) || isa<WRNTargetEnterDataNode>(W)) &&
         "Expected a WRNTargetDataNode or WRNTargetEnterDataNode");
  Value *DeviceIDPtr = W->getDevice();
  CallInst *Call= genTgtCall("__tgt_target_data_begin", DeviceIDPtr, NumArgs,
                             ArgsBase, Args, ArgsSize, ArgsMaptype, InsertPt);
  return Call;
}

/// \brief Build int32_t __tgt_target_data_end(int64_t  device_id,
///                                            int32_t  num_args,
///                                            void**   args_base,
///                                            void**   args,
///                                            int64_t* args_size,
///                                            int64_t* args_maptype)
///
CallInst *VPOParoptUtils::genTgtTargetDataEnd(WRegionNode *W, int NumArgs,
                                              Value *ArgsBase, Value *Args,
                                              Value *ArgsSize,
                                              Value *ArgsMaptype,
                                              Instruction *InsertPt) {
  assert((isa<WRNTargetDataNode>(W) || isa<WRNTargetExitDataNode>(W)) &&
         "Expected a WRNTargetDataNode or WRNTargetExitDataNode");
  Value *DeviceIDPtr = W->getDevice();
  CallInst *Call= genTgtCall("__tgt_target_data_end", DeviceIDPtr, NumArgs,
                             ArgsBase, Args, ArgsSize, ArgsMaptype, InsertPt);
  return Call;
}

/// \brief Build int32_t __tgt_target_data_update(int64_t  device_id,
///                                               int32_t  num_args,
///                                               void**   args_base,
///                                               void**   args,
///                                               int64_t* args_size,
///                                               int64_t* args_maptype)
///
CallInst *VPOParoptUtils::genTgtTargetDataUpdate(WRegionNode *W, int NumArgs,
                                                 Value *ArgsBase, Value *Args,
                                                 Value *ArgsSize,
                                                 Value *ArgsMaptype,
                                                 Instruction *InsertPt) {
  assert(isa<WRNTargetUpdateNode>(W) && "Expected a WRNTargetUpdateNode");
  Value *DeviceIDPtr = W->getDevice();
  CallInst *Call= genTgtCall("__tgt_target_data_update", DeviceIDPtr, NumArgs,
                             ArgsBase, Args, ArgsSize, ArgsMaptype, InsertPt);
  return Call;
}

/// \brief Build int32_t __tgt_target(int64_t  device_id,
///                                   void*    host_addr,
///                                   int32_t  num_args,
///                                   void**   args_base,
///                                   void**   args,
///                                   int64_t* args_size,
///                                   int64_t* args_maptype)
///
CallInst *VPOParoptUtils::genTgtTarget(WRegionNode *W, Value *HostAddr,
                                       int NumArgs, Value *ArgsBase,
                                       Value *Args, Value *ArgsSize,
                                       Value *ArgsMaptype,
                                       Instruction *InsertPt) {
  assert(isa<WRNTargetNode>(W) && "Expected a WRNTargetNode");
  Value *DeviceIDPtr = W->getDevice();
  CallInst *Call= genTgtCall("__tgt_target", DeviceIDPtr, NumArgs, ArgsBase,
                             Args, ArgsSize, ArgsMaptype, InsertPt, HostAddr);
  return Call;
}

/// \brief Build int32_t __tgt_target_teams(int64_t  device_id,
///                                         void*    host_addr,
///                                         int32_t  num_args,
///                                         void**   args_base,
///                                         void**   args,
///                                         int64_t* args_size,
///                                         int64_t* args_maptype,
///                                         int32_t  num_teams,
///                                         int32_t  thread_limit)
///
CallInst *VPOParoptUtils::genTgtTargetTeams(WRegionNode *W, Value *HostAddr,
                                            int NumArgs, Value *ArgsBase,
                                            Value *Args, Value *ArgsSize,
                                            Value *ArgsMaptype,
                                            Instruction *InsertPt) {
  // This call supports the target teams construct.
  // Its WRN representation is a WRNTeamsNode enclosed in a WRNTargetNode.

  assert(isa<WRNTeamsNode>(W) && "Expected a WRNTeamsNode");

  WRegionNode *WTarget = W->getParent();
  assert(isa<WRNTargetNode>(WTarget) && "Expected parent to be WRNTargetNode");

  Value *DeviceIDPtr    = WTarget->getDevice();
  Value *NumTeamsPtr    = W->getNumTeams();
  Value *ThreadLimitPtr = W->getThreadLimit();
  CallInst *Call= genTgtCall("__tgt_target_teams", DeviceIDPtr, NumArgs,
                             ArgsBase, Args, ArgsSize, ArgsMaptype, InsertPt,
                             HostAddr, NumTeamsPtr, ThreadLimitPtr);
  return Call;
}

/// \brief Base routine to create one of these libomptarget calls:
/// \code
///   void    __tgt_target_data_begin( int64_t device_id, <common>)
///   void    __tgt_target_data_end(   int64_t device_id, <common>)
///   void    __tgt_target_data_update(int64_t device_id, <common>)
///   int32_t __tgt_target(int64_t device_id, void *host_addr, <common>)
///   int32_t __tgt_target_teams(int64_t device_id, void *host_addr,
///                              <common>, int32_t num_teams,
///                              int32_t thread_limit)
/// \endcode
/// where <common> represents these 5 arguments:
/// \code
///   int32_t  num_args,    // number of pointers being mapped
///   void**   args_base,   // array of base pointers being mapped
///   void**   args,        // array of section pointers (base+offset)
///   int64_t* args_size,   // array of sizes (bytes) of each mapped datum
///   int64_t* args_maptype // array of map attributes for each mapping
/// \endcode
CallInst *VPOParoptUtils::genTgtCall(StringRef FnName, Value *DeviceIDPtr,
                                     int NumArgsCount, Value *ArgsBase,
                                     Value *Args, Value *ArgsSize,
                                     Value *ArgsMaptype, Instruction *InsertPt,
                                     Value *HostAddr, Value *NumTeamsPtr,
                                     Value *ThreadLimitPtr) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);
  Type *ReturnTy; // void for _tgt_target_data_*(); i32 otherwise

  Value *NumTeams = nullptr;
  Value *ThreadLimit = nullptr;

  // First parm: "int64_t device_id"
  Value *DeviceID;
  if (DeviceIDPtr == nullptr) {
    // user did not specify device; default is -1
    DeviceID = ConstantInt::get(Int64Ty, -1);
  } else if (isa<Constant>(DeviceIDPtr))
    DeviceID = Builder.CreateSExtOrBitCast(DeviceIDPtr, Int64Ty);
  else {
    DeviceID = new LoadInst(DeviceIDPtr, "deviceID", InsertPt);
    DeviceID = Builder.CreateSExtOrBitCast(DeviceID, Int64Ty);
  }

  SmallVector<Value *, 9> FnArgs    = { DeviceID };
  SmallVector<Type *, 9> FnArgTypes = {Int64Ty};

  if (HostAddr) {
    // HostAddr!=null means FnName is __tgt_target or __tgt_target_teams
    ReturnTy = Int32Ty;

    // Handle the "void *host_addr" parm of __tgt_target and __tgt_target_teams
    FnArgs.push_back(HostAddr);
    FnArgTypes.push_back(HostAddr->getType());
    if (FnName == "__tgt_target_teams") {
      // __tgt_target_teams has two more parms: "int32_t num_teams" and
      // "int32_t thread_limit".  Initialize them here.
      if (NumTeamsPtr == nullptr)
        NumTeams = Builder.getInt32(0);
      else
        NumTeams =
            Builder.CreateSExtOrTrunc(Builder.CreateLoad(NumTeamsPtr), Int32Ty);

      if (ThreadLimitPtr == nullptr)
        ThreadLimit = Builder.getInt32(0);
      else
        ThreadLimit = Builder.CreateSExtOrTrunc(
            Builder.CreateLoad(ThreadLimitPtr), Int32Ty);
    }
  } else {
    // HostAddr==null means FnName is not __tgt_target or __tgt_target_teams
    ReturnTy = Type::getVoidTy(C);
  }

  // Five common parms needed by all __tgt_target* calls :
  //     int32_t  num_args,
  //     void**   args_base,
  //     void**   args,
  //     int64_t* args_size,
  //     int32_t* args_maptype
  Value *NumArgs = ConstantInt::get(Int32Ty, NumArgsCount);
  FnArgs.push_back(NumArgs);
  FnArgTypes.push_back(Int32Ty);

  FnArgs.push_back(ArgsBase);
  FnArgTypes.push_back(ArgsBase->getType());

  FnArgs.push_back(Args);
  FnArgTypes.push_back(Args->getType());

  FnArgs.push_back(ArgsSize);
  FnArgTypes.push_back(ArgsSize->getType());

  FnArgs.push_back(ArgsMaptype);
  FnArgTypes.push_back(ArgsMaptype->getType());

  // Add the two parms for __tgt_target_teams
  if (NumTeams != nullptr) {
    FnArgs.push_back(NumTeams);
    FnArgTypes.push_back(Int32Ty);

    FnArgs.push_back(ThreadLimit);
    FnArgTypes.push_back(Int32Ty);
  }

  // LLVM_DEBUG(dbgs() << "FnArgs.size= "<< FnArgs.size());
  // LLVM_DEBUG(dbgs() << "FnArgTypes.size() = "<< FnArgTypes.size());
  CallInst *Call = genCall(FnName, ReturnTy, FnArgs, FnArgTypes,
                           InsertPt);
  return Call;
}

// Call to i32 __tgt_unregister_lib(__tgt_bin_desc *desc);
CallInst *VPOParoptUtils::genTgtUnregisterLib(Value *Desc,
                                              Instruction *InsertPt) {
  return genTgtRegGeneric(Desc, InsertPt, "__tgt_unregister_lib");
}

// The document says this is VOID, but Clang emits it as i32.
// For now, let's be consistent with Clang.
//
// Call to i32 __tgt_register_lib(__tgt_bin_desc *desc);
CallInst *VPOParoptUtils::genTgtRegisterLib(Value *Desc,
                                            Instruction *InsertPt) {
  return genTgtRegGeneric(Desc, InsertPt, "__tgt_register_lib");
}

// Call to generic function to support the generation of
// __tgt_register_lib and __tgt_unregister_lib.
CallInst *VPOParoptUtils::genTgtRegGeneric(Value *Desc, Instruction *InsertPt,
                                           StringRef FnName) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Value *Args[] = { Desc };
  Type *ArgTypes[] = { Desc->getType() };
  CallInst *Call = genCall(FnName, Type::getInt32Ty(C), Args, ArgTypes,
                           InsertPt);
  return Call;
}

// Generate a generic call to `get_global_id, get_local_id...`.
// Example
//   call i64 @_Z14get_local_sizej(i32 0)
//          where the value 0 is the dimension number.
//
// Here are the list of OpenCL functions generated by this util.
//   "_Z13get_global_idj"
//   "_Z12get_local_idj"
//   "_Z14get_local_sizej"
//   "_Z14get_num_groupsj"
//   "_Z12get_group_idj"
//   "_Z18work_group_barrierj"
//   "_Z9mem_fencej"
//   "_Z14read_mem_fencej"
//   "_Z15write_mem_fencej".
CallInst *VPOParoptUtils::genOCLGenericCall(StringRef FnName,
                                            ArrayRef<Value *> FnArgs,
                                            Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Type *ArgTypes[] = {Type::getInt32Ty(C)};
  CallInst *Call =
      genCall(FnName, Type::getInt64Ty(C), FnArgs, ArgTypes, InsertPt);
  return Call;
}

// Call to i32 __cxa_atexit(void (i8*)*
//   @.omp_offloading.descriptor_unreg, i8* bitcast (%struct.__tgt_bin_desc*
//   @.omp_offloading.descriptor to i8*), i8* @__dso_handle)
CallInst *VPOParoptUtils::genCxaAtExit(Value *TgtDescUnregFn, Value *Desc,
                                       Value *Handle, Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *Int8PtrTy = Type::getInt8PtrTy(C);
  Value *BitCast = Builder.CreateBitCast(Desc, Int8PtrTy);

  SmallVector<Value *, 3> Args    = { TgtDescUnregFn, BitCast, Handle };
  SmallVector<Type *, 3> ArgTypes = { TgtDescUnregFn->getType(), Int8PtrTy,
                                      Int8PtrTy };
  CallInst *Call = genCall("__cxa_atexit", Type::getInt32Ty(C), Args, ArgTypes,
                           InsertPt);
  return Call;
}

// This function generates a call as follows.
//    void @__kmpc_omp_task_begin_if0(
//          { i32, i32, i32, i32, i8* }* /* &loc */,
//          i32 /* tid */,
//          i8* /*thunk_temp */)
CallInst *VPOParoptUtils::genKmpcTaskBeginIf0(WRegionNode *W,
                                              StructType *IdentTy,
                                              Value *TidPtr, Value *TaskAlloc,
                                              Instruction *InsertPt) {

  return genKmpcTaskGeneric(W, IdentTy, TidPtr, TaskAlloc, InsertPt,
                            "__kmpc_omp_task_begin_if0");
}

// This function generates a call as follows.
//    void @__kmpc_omp_task_complete_if0(
//          { i32, i32, i32, i32, i8* }* /* &loc */,
//          i32 /* tid */,
//          i8* /*thunk_temp */)
CallInst *VPOParoptUtils::genKmpcTaskCompleteIf0(WRegionNode *W,
                                                 StructType *IdentTy,
                                                 Value *TidPtr,
                                                 Value *TaskAlloc,
                                                 Instruction *InsertPt) {

  return genKmpcTaskGeneric(W, IdentTy, TidPtr, TaskAlloc, InsertPt,
                            "__kmpc_omp_task_complete_if0");
}

// This function generates a call as follows.
//    void @__kmpc_task({ i32, i32, i32, i32, i8* }*, i32, i8*)
CallInst *VPOParoptUtils::genKmpcTask(WRegionNode *W, StructType *IdentTy,
                                      Value *TidPtr, Value *TaskAlloc,
                                      Instruction *InsertPt) {
  return genKmpcTaskGeneric(W, IdentTy, TidPtr, TaskAlloc, InsertPt,
                            "__kmpc_omp_task");
}

// This function generates a call as follows.
//    void @__kmpc_omp_task_with_deps(
//           { i32, i32, i32, i32, i8* }* /* &loc */,
//           i32 /* tid */,
//           i8* /*thunk_temp */,
//           i32 /* depend_count */,
//           i8* /* &depend_record
//           i32 /* 0 */,
//           i8* /* 0 */)
CallInst *VPOParoptUtils::genKmpcTaskWithDeps(WRegionNode *W,
                                              StructType *IdentTy,
                                              Value *TidPtr, Value *TaskAlloc,
                                              Value *Dep, int DepNum,
                                              Instruction *InsertPt) {
  return genKmpcTaskDepsGeneric(W, IdentTy, TidPtr, TaskAlloc, Dep, DepNum,
                                InsertPt, "__kmpc_omp_task_with_deps");
}

// This function generates a call as follows.
//    void @__kmpc_omp_wait_deps(
//           { i32, i32, i32, i32, i8* }* /* &loc */,
//           i32 /* tid */,
//           i32 /* depend_count */,
//           i8* /* &depend_record
//           i32 /* 0 */,
//           i8* /* 0 */)
CallInst *VPOParoptUtils::genKmpcTaskWaitDeps(WRegionNode *W,
                                              StructType *IdentTy,
                                              Value *TidPtr, Value *Dep,
                                              int DepNum,
                                              Instruction *InsertPt) {
  return genKmpcTaskDepsGeneric(W, IdentTy, TidPtr, nullptr, Dep, DepNum,
                                InsertPt, "__kmpc_omp_wait_deps");
}

// Generic routine to generate __kmpc_omp_task_with_deps or
//  __kmpc_omp_wait_deps.
CallInst *VPOParoptUtils::genKmpcTaskDepsGeneric(
    WRegionNode *W, StructType *IdentTy, Value *TidPtr, Value *TaskAlloc,
    Value *Dep, int DepNum, Instruction *InsertPt, StringRef FnName) {

  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  int Flags = KMP_IDENT_KMPC;
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  std::vector<Value *> TaskArgs;
  TaskArgs.push_back(Loc);
  TaskArgs.push_back(Builder.CreateLoad(TidPtr));
  if (TaskAlloc)
    TaskArgs.push_back(TaskAlloc);
  TaskArgs.push_back(Builder.getInt32(DepNum));
  TaskArgs.push_back(Dep);
  TaskArgs.push_back(Builder.getInt32(0));
  TaskArgs.push_back(ConstantPointerNull::get(Type::getInt8PtrTy(C)));

  std::vector<Type *> TypeParams;
  TypeParams.push_back(Loc->getType());
  TypeParams.push_back(Type::getInt32Ty(C));
  if (TaskAlloc)
    TypeParams.push_back(Type::getInt8PtrTy(C));
  TypeParams.push_back(Type::getInt32Ty(C));
  TypeParams.push_back(Type::getInt8PtrTy(C));
  TypeParams.push_back(Type::getInt32Ty(C));
  TypeParams.push_back(Type::getInt8PtrTy(C));

  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), TypeParams, false);

  Function *FnTask = M->getFunction(FnName);

  if (!FnTask) {
    FnTask = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);
    FnTask->setCallingConv(CallingConv::C);
  }

  CallInst *TaskCall = CallInst::Create(FnTask, TaskArgs, "", InsertPt);
  TaskCall->setCallingConv(CallingConv::C);
  TaskCall->setTailCall(false);

  return TaskCall;
}

// This is a generic function to support the generation of
//   __kmpc_task, __kmpc_omp_task_begin_if0 and __kmpc_omp_task_complete_if0.
CallInst *VPOParoptUtils::genKmpcTaskGeneric(WRegionNode *W,
                                             StructType *IdentTy, Value *TidPtr,
                                             Value *TaskAlloc,
                                             Instruction *InsertPt,
                                             StringRef FnName) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  int Flags = KMP_IDENT_KMPC;
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  Value *TaskArgs[] = {Loc, Builder.CreateLoad(TidPtr), TaskAlloc};
  Type *TypeParams[] = {Loc->getType(), Type::getInt32Ty(C),
                        Type::getInt8PtrTy(C)};
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), TypeParams, false);

  Function *FnTask = M->getFunction(FnName);

  if (!FnTask) {
    FnTask = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);
    FnTask->setCallingConv(CallingConv::C);
  }

  CallInst *TaskCall = CallInst::Create(FnTask, TaskArgs, "", InsertPt);
  TaskCall->setCallingConv(CallingConv::C);
  TaskCall->setTailCall(false);

  return TaskCall;
}

// This function generates a call as follows.
// void __kmpc_copyprivate(
//    ident_t *loc, kmp_int32 global_tid, kmp_int32 cpy size, void *cpy data,
//    void(*cpy func)(void *, void *), kmp_int32 didit );
CallInst *VPOParoptUtils::genKmpcCopyPrivate(WRegionNode *W,
                                             StructType *IdentTy, Value *TidPtr,
                                             unsigned Size, Value *CpyData,
                                             Function *FnCopyPriv,
                                             Value *IsSingleThread,
                                             Instruction *InsertPt) {

  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = W->getEntryBBlock();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Value *CprivArgs[] = {
      Builder.getInt32(Size),
      Builder.CreateBitCast(CpyData, Type::getInt8PtrTy(C)),
      Builder.CreateBitCast(FnCopyPriv, Type::getInt8PtrTy(C)), IsSingleThread};
  CallInst *CprivCall =
      genKmpcCallWithTid(W, IdentTy, TidPtr, InsertPt, "__kmpc_copyprivate",
                         Type::getVoidTy(C), CprivArgs);
  CprivCall->insertBefore(InsertPt);
  return CprivCall;
}

// This function generates a call as follows.
//    void @__kmpc_taskloop({ i32, i32, i32, i32, i8* }*, i32, i8*, i32,
//    i64*, i64*, i64, i32, i32, i64, i8*)
// The generated code is based on the assumption that the loop is normalized
// and the stride is 1.
CallInst *VPOParoptUtils::genKmpcTaskLoop(WRegionNode *W, StructType *IdentTy,
                                          Value *TidPtr, Value *TaskAlloc,
                                          Value *Cmp, Value *LBPtr,
                                          Value *UBPtr, Value *STPtr,
                                          StructType *KmpTaskTTWithPrivatesTy,
                                          Instruction *InsertPt, bool UseTbb,
                                          Function *FnTaskDup) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  int Flags = KMP_IDENT_KMPC;
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  Value *Cast = Builder.CreateBitCast(
      TaskAlloc, PointerType::getUnqual(KmpTaskTTWithPrivatesTy));

  SmallVector<Value *, 4> Indices;
  Indices.push_back(Builder.getInt32(0));
  Indices.push_back(Builder.getInt32(0));
  Value *TaskTTyGep =
      Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Cast, Indices);

  assert(isa<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0)) &&
         "TaskT is not Struct Type.");
  StructType *KmpTaskTTy =
      cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0));

  Indices.pop_back();
  Indices.push_back(Builder.getInt32(5));
  Value *LBGep = Builder.CreateInBoundsGEP(KmpTaskTTy, TaskTTyGep, Indices);
  Value *LBVal = Builder.CreateLoad(LBPtr);
  if (LBVal->getType() != KmpTaskTTy->getElementType(5))
    LBVal = Builder.CreateSExtOrTrunc(LBVal, KmpTaskTTy->getElementType(5));

  Builder.CreateStore(LBVal, LBGep);

  Indices.pop_back();
  Indices.push_back(Builder.getInt32(6));
  Value *UBGep = Builder.CreateInBoundsGEP(KmpTaskTTy, TaskTTyGep, Indices);
  Value *UBVal = Builder.CreateLoad(UBPtr);
  if (UBVal->getType() != KmpTaskTTy->getElementType(6))
    UBVal = Builder.CreateSExtOrTrunc(UBVal, KmpTaskTTy->getElementType(6));
  Builder.CreateStore(UBVal, UBGep);

  Indices.pop_back();
  Indices.push_back(Builder.getInt32(7));
  Value *STGep = Builder.CreateInBoundsGEP(KmpTaskTTy, TaskTTyGep, Indices);
  Value *STVal = Builder.CreateLoad(STPtr);
  if (STVal->getType() != KmpTaskTTy->getElementType(7))
    STVal = Builder.CreateSExtOrTrunc(STVal, KmpTaskTTy->getElementType(7));
  Builder.CreateStore(STVal, STGep);
  Value *STLoad = Builder.CreateLoad(STGep);

  Value *GrainSizeV = nullptr;
  switch (W->getSchedCode()) {
  case 0:
    GrainSizeV = Builder.getInt64(0);
    break;
  case 1:
    GrainSizeV =
        Builder.CreateSExtOrTrunc(W->getGrainsize(), Type::getInt64Ty(C));
    break;
  case 2:
    GrainSizeV =
        Builder.CreateSExtOrTrunc(W->getNumTasks(), Type::getInt64Ty(C));
    break;
  default:
    llvm_unreachable("genKmpcTaskLoop: unexpected SchedCode");
  }

  Value *TaskLoopArgs[] = {
      Loc,
      Builder.CreateLoad(TidPtr),
      TaskAlloc,
      Cmp == nullptr ? Builder.getInt32(1)
                     : Builder.CreateSExtOrTrunc(Cmp, Type::getInt32Ty(C)),
      LBGep,
      UBGep,
      STLoad,
      Builder.getInt32(0),
      Builder.getInt32(W->getSchedCode()),
      GrainSizeV,
      (FnTaskDup == nullptr)
          ? ConstantPointerNull::get(Type::getInt8PtrTy(C))
          : Builder.CreateBitCast(FnTaskDup, Type::getInt8PtrTy(C))};
  Type *TypeParams[] = {Loc->getType(),
                        Type::getInt32Ty(C),
                        Type::getInt8PtrTy(C),
                        Type::getInt32Ty(C),
                        PointerType::getUnqual(Type::getInt64Ty(C)),
                        PointerType::getUnqual(Type::getInt64Ty(C)),
                        Type::getInt64Ty(C),
                        Type::getInt32Ty(C),
                        Type::getInt32Ty(C),
                        Type::getInt64Ty(C),
                        Type::getInt8PtrTy(C)};
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), TypeParams, false);

  StringRef FnName = UseTbb ? "__tbb_omp_taskloop" : "__kmpc_taskloop";
  Function *FnTaskLoop = M->getFunction(FnName);

  if (!FnTaskLoop) {
    FnTaskLoop =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);
    FnTaskLoop->setCallingConv(CallingConv::C);
  }

  CallInst *TaskLoopCall =
      CallInst::Create(FnTaskLoop, TaskLoopArgs, "", InsertPt);
  TaskLoopCall->setCallingConv(CallingConv::C);
  TaskLoopCall->setTailCall(false);

  return TaskLoopCall;
}

// This function generates a call as follows.
//    i8* @__kmpc_task_reduction_init(i32, i32, i8*)
CallInst *VPOParoptUtils::genKmpcTaskReductionInit(WRegionNode *W,
                                                   Value *TidPtr, int ParmNum,
                                                   Value *RedRecord,
                                                   Instruction *InsertPt,
                                                   bool UseTbb) {
  BasicBlock *B = W->getEntryBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  IRBuilder<> Builder(InsertPt);
  Value *TaskRedInitArgs[] = {
      Builder.CreateLoad(TidPtr), Builder.getInt32(ParmNum),
      Builder.CreatePointerCast(RedRecord, Builder.getInt8PtrTy())};
  Type *TypeParams[] = {Type::getInt32Ty(C), Type::getInt32Ty(C),
                        Type::getInt8PtrTy(C)};
  FunctionType *FnTy =
      FunctionType::get(Type::getInt8PtrTy(C), TypeParams, false);

  StringRef FnName = UseTbb ? "__tbb_omp_task_reduction_init" :
                              "__kmpc_task_reduction_init";

  Function *FnTaskRedInit = M->getFunction(FnName);

  if (!FnTaskRedInit) {
    FnTaskRedInit =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);
    FnTaskRedInit->setCallingConv(CallingConv::C);
  }

  CallInst *TaskRedInitCall =
      CallInst::Create(FnTaskRedInit, TaskRedInitArgs, "", InsertPt);
  TaskRedInitCall->setCallingConv(CallingConv::C);
  TaskRedInitCall->setTailCall(false);

  return TaskRedInitCall;
}

// This function generates a call as follows.
//    i8* @__kmpc_omp_task_alloc({ i32, i32, i32, i32, i8* }*, i32, i32,
//    i64, i64, i32 (i32, i8*)*)
CallInst *VPOParoptUtils::genKmpcTaskAlloc(WRegionNode *W, StructType *IdentTy,
                                           Value *TidPtr,
                                           int KmpTaskTTWithPrivatesTySz,
                                           int KmpSharedTySz,
                                           PointerType *KmpRoutineEntryPtrTy,
                                           Function *MicroTaskFn,
                                           Instruction *InsertPt, bool UseTbb) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  int Flags = KMP_IDENT_KMPC;
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  auto *TaskFlags = ConstantInt::get(Type::getInt32Ty(C), W->getTaskFlag());
  auto *KmpTaskTWithPrivatesTySize =
      ConstantInt::get(Type::getInt64Ty(C), KmpTaskTTWithPrivatesTySz);
  auto *SharedsSize = ConstantInt::get(Type::getInt64Ty(C), KmpSharedTySz);
  IRBuilder<> Builder(InsertPt);
  Value *AllocArgs[] = {
      Loc,         Builder.CreateLoad(TidPtr),
      TaskFlags,   KmpTaskTWithPrivatesTySize,
      SharedsSize, Builder.CreateBitCast(MicroTaskFn, KmpRoutineEntryPtrTy)};
  Type *TypeParams[] = {Loc->getType(),      Type::getInt32Ty(C),
                        Type::getInt32Ty(C), Type::getInt64Ty(C),
                        Type::getInt64Ty(C), KmpRoutineEntryPtrTy};
  FunctionType *FnTy =
      FunctionType::get(Type::getInt8PtrTy(C), TypeParams, false);

  StringRef FnName = UseTbb? "__tbb_omp_task_alloc" : "__kmpc_omp_task_alloc";
  Function *FnTaskAlloc = M->getFunction(FnName);

  if (!FnTaskAlloc) {
    FnTaskAlloc =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);
    FnTaskAlloc->setCallingConv(CallingConv::C);
  }

  CallInst *TaskAllocCall =
      CallInst::Create(FnTaskAlloc, AllocArgs, "", InsertPt);
  TaskAllocCall->setCallingConv(CallingConv::C);
  TaskAllocCall->setTailCall(false);

  return TaskAllocCall;
}

// This function generates a call to notify the runtime system that the static
// distribute loop scheduling for teams is started
//
//   call void @__kmpc_team_static_init_4(%ident_t* %loc, i32 %tid,
//               i32* %islast, i32* %lb, i32* %ub, i32* %st,
//               i32 inc, i32 chunk)
//   call void @__kmpc_team_static_init_8(%ident_t* %loc, i32 %tid,
//               i64* %islast, i64* %lb, i64* %ub, i64* %st,
//               i64 inc, i64 chunk)
//
// Note: The type of the LCV (i32/i64) determines if the 4- or 8-byte version
// is used. The parameter 'Chunk' has to be cast to the matching type when
// needed. For example, in
//
//     int chksize;
//     long long jjj;
//     #pragma omp distribute parallel for dist_schedule(static, chksize)
//       for (jjj=1; jjj<100; jjj++) ...
//
// the LCV jjj is i64, so chksize is cast from i32 to i64 in the call:
//
//     %chunk.cast = sext i32 %chksize to i64
//     call void @__kmpc_team_static_init_8( ... , i64 %chunk.cast)
//
// The cast instruction is not needed if the type is already matching. For
// example, if "long long jjj" above is changed to "int jjj", then we get
//
//     ; no casting of "i32 %chksize" as it is the correct type
//     call void @__kmpc_team_static_init_4( ... , i32 %chksize)
//
// The cast instruction is not emitted if chunk is a constant and the compiler
// can convert it directly. For example, given:
//
//     long long jjj;
//     #pragma omp distribute parallel for schedule(static, 17)
//       for (jjj=1; jjj<100; jjj++) ...
//
// The original "i32 17" is directly converted to "i64 17" by the compiler
// without needing a sext instruction:
//
//     call void @__kmpc_team_static_init_8( ... , i64 17)
//
CallInst *VPOParoptUtils::genKmpcTeamStaticInit(WRegionNode *W,
                                            StructType *IdentTy,
                                            Value *Tid, Value *IsLastVal,
                                            Value *LB, Value *UB, Value *ST,
                                            Value *Inc, Value *Chunk,
                                            int Size, bool IsUnsigned,
                                            Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  Module   *M = F->getParent();

  LLVMContext &C = F->getContext();

  int Flags = KMP_IDENT_KMPC;
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);

  Type *IntArgTy = (Size == 32) ? Int32Ty : Int64Ty;

  // If Chunk's type != IntArgTy, cast it to IntArgTy
  IRBuilder<> Builder(InsertPt);
  Chunk = Builder.CreateSExtOrTrunc(Chunk, IntArgTy, "team.chunk.cast");

  StringRef FnName;

  if (IsUnsigned)
    FnName = (Size == 32) ? "__kmpc_team_static_init_4u" :
                            "__kmpc_team_static_init_8u" ;
  else
    FnName = (Size == 32) ? "__kmpc_team_static_init_4" :
                            "__kmpc_team_static_init_8" ;


  Type *ParamsTy[] = {PointerType::getUnqual(IdentTy),
                      Int32Ty, PointerType::getUnqual(Int32Ty),
                      PointerType::getUnqual(IntArgTy),
                      PointerType::getUnqual(IntArgTy),
                      PointerType::getUnqual(IntArgTy),
                      IntArgTy, IntArgTy};
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);

  Function *FnTeamStaticInit = M->getFunction(FnName);

  if (!FnTeamStaticInit) {
    FnTeamStaticInit = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                    FnName, M);
    FnTeamStaticInit->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnTeamStaticInitArgs;

  FnTeamStaticInitArgs.push_back(Loc);
  FnTeamStaticInitArgs.push_back(Tid);
  FnTeamStaticInitArgs.push_back(IsLastVal);
  FnTeamStaticInitArgs.push_back(LB);
  FnTeamStaticInitArgs.push_back(UB);
  FnTeamStaticInitArgs.push_back(ST);
  FnTeamStaticInitArgs.push_back(Inc);
  FnTeamStaticInitArgs.push_back(Chunk);

  CallInst *TeamStaticInitCall = CallInst::Create(FnTeamStaticInit,
                                           FnTeamStaticInitArgs, "", InsertPt);
  TeamStaticInitCall->setCallingConv(CallingConv::C);
  TeamStaticInitCall->setTailCall(false);

  return TeamStaticInitCall;
}

// This function generates a call to notify the runtime system that the static
// loop scheduling is started
//
//   call void @__kmpc_for_static_init_4(%ident_t* %loc, i32 %tid,
//               i32 schedtype, i32* %islast, i32* %lb, i32* %ub, i32* %st,
//               i32 inc, i32 chunk)
//   call void @__kmpc_for_static_init_8(%ident_t* %loc, i32 %tid,
//               i32 schedtype, i64* %islast, i64* %lb, i64* %ub, i64* %st,
//               i64 inc, i64 chunk)
//
// Note: The type of the LCV (i32/i64) determines if the 4- or 8-byte version
// is used. The parameter 'Chunk' has to be cast to the matching type when
// needed. For example, in
//
//     int chksize;
//     long long jjj;
//     #pragma omp for schedule(static, chksize)
//       for (jjj=1; jjj<100; jjj++) ...
//
// the LCV jjj is i64, so chksize is cast from i32 to i64 in the call:
//
//     %chunk.cast = sext i32 %chksize to i64
//     call void @__kmpc_for_static_init_8( ... , i64 %chunk.cast)
//
// The cast instruction is not needed if the type is already matching. For
// example, if "long long jjj" above is changed to "int jjj", then we get
//
//     ; no casting of "i32 %chksize" as it is the correct type
//     call void @__kmpc_for_static_init_4( ... , i32 %chksize)
//
// The cast instruction is not emitted if chunk is a constant and the compiler
// can convert it directly. For example, given:
//
//     long long jjj;
//     #pragma omp for schedule(static, 17)
//       for (jjj=1; jjj<100; jjj++) ...
//
// The original "i32 17" is directly converted to "i64 17" by the compiler
// without needing a sext instruction:
//
//     call void @__kmpc_for_static_init_8( ... , i64 17)
//
CallInst *VPOParoptUtils::genKmpcStaticInit(WRegionNode *W,
                                            StructType *IdentTy,
                                            Value *Tid, Value *SchedType,
                                            Value *IsLastVal, Value *LB,
                                            Value *UB, Value *DistUB, Value *ST,
                                            Value *Inc, Value *Chunk,
                                            int Size, bool IsUnsigned,
                                            Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  Module   *M = F->getParent();

  LLVMContext &C = F->getContext();

  int Flags = KMP_IDENT_KMPC;
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  bool IsDistChunkedParLoop = false;

  if (isa<WRNDistributeParLoopNode>(W))  {
    WRNScheduleKind DistSchedKind = VPOParoptUtils::getDistLoopScheduleKind(W);
    if (DistSchedKind == WRNScheduleDistributeStatic)
      IsDistChunkedParLoop = true;
  }

  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);

  Type *IntArgTy = (Size == 32) ? Int32Ty : Int64Ty;

  // If Chunk's type != IntArgTy, cast it to IntArgTy
  IRBuilder<> Builder(InsertPt);
  Chunk = Builder.CreateSExtOrTrunc(Chunk, IntArgTy, "chunk.cast");

  StringRef FnName;

  if (IsUnsigned) {
    if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop)
      FnName = (Size == 32) ? "__kmpc_dist_for_static_init_4u" :
                              "__kmpc_dist_for_static_init_8u" ;
    else
      FnName = (Size == 32) ? "__kmpc_for_static_init_4u" :
                              "__kmpc_for_static_init_8u" ;
  }
  else {
    if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop)
      FnName = (Size == 32) ? "__kmpc_dist_for_static_init_4" :
                              "__kmpc_dist_for_static_init_8" ;
    else
      FnName = (Size == 32) ? "__kmpc_for_static_init_4" :
                              "__kmpc_for_static_init_8" ;
  }

  FunctionType *FnTy;

  if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop) {
    Type *ParamsTy[] = {PointerType::getUnqual(IdentTy),
                        Int32Ty, Int32Ty, PointerType::getUnqual(Int32Ty),
                        PointerType::getUnqual(IntArgTy),
                        PointerType::getUnqual(IntArgTy),
                        PointerType::getUnqual(IntArgTy),
                        PointerType::getUnqual(IntArgTy),
                        IntArgTy, IntArgTy};
    FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);
  }
  else {
    Type *ParamsTy[] = {PointerType::getUnqual(IdentTy),
                        Int32Ty, Int32Ty, PointerType::getUnqual(Int32Ty),
                        PointerType::getUnqual(IntArgTy),
                        PointerType::getUnqual(IntArgTy),
                        PointerType::getUnqual(IntArgTy),
                        IntArgTy, IntArgTy};
    FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);
  }

  Function *FnStaticInit = M->getFunction(FnName);

  if (!FnStaticInit) {
    FnStaticInit = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                    FnName, M);
    FnStaticInit->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnStaticInitArgs;

  FnStaticInitArgs.push_back(Loc);
  FnStaticInitArgs.push_back(Tid);
  FnStaticInitArgs.push_back(SchedType);
  FnStaticInitArgs.push_back(IsLastVal);
  FnStaticInitArgs.push_back(LB);
  FnStaticInitArgs.push_back(UB);

  if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop)
    FnStaticInitArgs.push_back(DistUB);

  FnStaticInitArgs.push_back(ST);
  FnStaticInitArgs.push_back(Inc);
  FnStaticInitArgs.push_back(Chunk);

  CallInst *StaticInitCall = CallInst::Create(FnStaticInit,
                                              FnStaticInitArgs, "", InsertPt);
  StaticInitCall->setCallingConv(CallingConv::C);
  StaticInitCall->setTailCall(false);

  return StaticInitCall;
}

// This function generates a call to notify the runtime system that the static
// loop scheduling is done
//   call void @__kmpc_for_static_fini(%ident_t* %loc, i32 %tid)
CallInst *VPOParoptUtils::genKmpcStaticFini(WRegionNode *W,
                                            StructType *IdentTy,
                                            Value *Tid,
                                            Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  Module   *M = F->getParent();
  LLVMContext &C = F->getContext();

  int Flags = KMP_IDENT_KMPC;

  Type *IntTy = Type::getInt32Ty(C);

  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);
  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  Type *ParamsTy[] = {PointerType::getUnqual(IdentTy), IntTy};

  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);

  Function *FnStaticFini = M->getFunction("__kmpc_for_static_fini");

  if (!FnStaticFini) {
    FnStaticFini = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                  "__kmpc_for_static_fini", M);
    FnStaticFini->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnStaticFiniArgs;

  FnStaticFiniArgs.push_back(Loc);
  FnStaticFiniArgs.push_back(Tid);

  CallInst *StaticFiniCall = CallInst::Create(FnStaticFini,
                                              FnStaticFiniArgs, "", InsertPt);
  StaticFiniCall->setCallingConv(CallingConv::C);
  StaticFiniCall->setTailCall(false);

  return StaticFiniCall;
}

// This function generates a call to notify the runtime system that the
// guided/runtime/dynamic loop scheduling is started
//
//   call void @__kmpc_dispatch_init_4{u}(%ident_t* %loc, i32 %tid,
//               i32 schedtype, i32 %lb, i32 %ub, i32 %st, i32 chunk)
//
//   call void @__kmpc_dispatch_init_8{u}(%ident_t* %loc, i32 %tid,
//               i32 schedtype, i64 %lb, i64 %ub, i64 %st, i64 chunk)
CallInst *VPOParoptUtils::genKmpcDispatchInit(WRegionNode *W,
                                              StructType *IdentTy,
                                              Value *Tid, Value *SchedType,
                                              Value *IsLastVal,
                                              Value *LB, Value *UB,
                                              Value *ST, Value *Chunk,
                                              int Size, bool IsUnsigned,
                                              Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  Module   *M = F->getParent();

  LLVMContext &C = F->getContext();

  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);

  Type *IntArgTy = (Size == 32) ? Int32Ty : Int64Ty;

  bool IsDistChunkedParLoop = false;

  if (isa<WRNDistributeParLoopNode>(W))  {
    WRNScheduleKind DistSchedKind = VPOParoptUtils::getDistLoopScheduleKind(W);
    if (DistSchedKind == WRNScheduleDistributeStatic)
      IsDistChunkedParLoop = true;
  }

  // If Chunk's type != IntArgTy, cast it to IntArgTy
  IRBuilder<> Builder(InsertPt);
  Chunk = Builder.CreateSExtOrTrunc(Chunk, IntArgTy, "chunk.cast");

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  StringRef FnName;

  if (IsUnsigned)
   if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop)
     FnName = (Size == 32) ? "__kmpc_dist_dispatch_init_4u" :
                             "__kmpc_dist_dispatch_init_8u" ;
   else
     FnName = (Size == 32) ? "__kmpc_dispatch_init_4u" :
                             "__kmpc_dispatch_init_8u" ;
  else
   if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop)
     FnName = (Size == 32) ? "__kmpc_dist_dispatch_init_4" :
                             "__kmpc_dist_dispatch_init_8" ;
   else
    FnName = (Size == 32) ? "__kmpc_dispatch_init_4" :
                            "__kmpc_dispatch_init_8" ;


  FunctionType *FnTy;

  if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop) {
    Type *ParamsTy[] = {PointerType::getUnqual(IdentTy),
                        Int32Ty, Int32Ty,
                        IntArgTy, IntArgTy, IntArgTy, IntArgTy, IntArgTy};
    FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);
  }
  else {
    Type *ParamsTy[] = {PointerType::getUnqual(IdentTy),
                        Int32Ty, Int32Ty,
                        IntArgTy, IntArgTy, IntArgTy, IntArgTy};
    FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);
  }

  Function *FnDispatchInit = M->getFunction(FnName);

  if (!FnDispatchInit) {
    FnDispatchInit = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                      FnName, M);
    FnDispatchInit->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnDispatchInitArgs;

  FnDispatchInitArgs.push_back(Loc);
  FnDispatchInitArgs.push_back(Tid);
  FnDispatchInitArgs.push_back(SchedType);

  if (isa<WRNDistributeParLoopNode>(W) && !IsDistChunkedParLoop)
    FnDispatchInitArgs.push_back(IsLastVal);

  FnDispatchInitArgs.push_back(LB);
  FnDispatchInitArgs.push_back(UB);
  FnDispatchInitArgs.push_back(ST);
  FnDispatchInitArgs.push_back(Chunk);

  CallInst *DispatchInitCall = CallInst::Create(FnDispatchInit,
                                         FnDispatchInitArgs, "", InsertPt);
  DispatchInitCall->setCallingConv(CallingConv::C);
  DispatchInitCall->setTailCall(false);

  return DispatchInitCall;
}

// This function generates a call to the runtime system that performs
// loop partitioning for guided/runtime/dynamic/auto scheduling.
//
//   call void @__kmpc_dispatch_next_4{u}(%ident_t* %loc, i32 %tid,
//               i32 *isLast, i32 *%lb, i32 *%ub, i32 *%st)
//
//   call void @__kmpc_dispatch_next_8{u}(%ident_t* %loc, i32 %tid,
//               i32 *isLast, i64 *%lb, i64 *%ub, i64 *%st)
CallInst *VPOParoptUtils::genKmpcDispatchNext(WRegionNode *W,
                                              StructType *IdentTy,
                                              Value *Tid, Value *IsLastVal,
                                              Value *LB, Value *UB, Value *ST,
                                              int Size, bool IsUnsigned,
                                              Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  Module   *M = F->getParent();

  LLVMContext &C = F->getContext();

  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);

  Type *IntArgTy = (Size == 32) ? Int32Ty : Int64Ty;

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  StringRef FnName;

  if (IsUnsigned)
    FnName = (Size == 32) ? "__kmpc_dispatch_next_4u" :
                            "__kmpc_dispatch_next_8u" ;
  else
    FnName = (Size == 32) ? "__kmpc_dispatch_next_4" :
                            "__kmpc_dispatch_next_8" ;

  Type *ParamsTy[] = {PointerType::getUnqual(IdentTy),
                      Int32Ty, PointerType::getUnqual(Int32Ty),
                      PointerType::getUnqual(IntArgTy),
                      PointerType::getUnqual(IntArgTy),
                      PointerType::getUnqual(IntArgTy)};

  FunctionType *FnTy = FunctionType::get(Int32Ty, ParamsTy, false);

  Function *FnDispatchNext = M->getFunction(FnName);

  if (!FnDispatchNext) {
    FnDispatchNext = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                      FnName, M);
    FnDispatchNext->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnDispatchNextArgs;

  FnDispatchNextArgs.push_back(Loc);
  FnDispatchNextArgs.push_back(Tid);
  FnDispatchNextArgs.push_back(IsLastVal);
  FnDispatchNextArgs.push_back(LB);
  FnDispatchNextArgs.push_back(UB);
  FnDispatchNextArgs.push_back(ST);

  CallInst *DispatchNextCall = CallInst::Create(FnDispatchNext,
                                         FnDispatchNextArgs, "", InsertPt);
  DispatchNextCall->setCallingConv(CallingConv::C);
  DispatchNextCall->setTailCall(false);
  return DispatchNextCall;
}

// This function generates a call to the runtime system that informs
// guided/runtime/dynamic/auto scheduling is done.
//
//   call void @__kmpc_dispatch_fini_4{u}(%ident_t* %loc, i32 %tid)
//   call void @__kmpc_dispatch_fini_8{u}(%ident_t* %loc, i32 %tid)
CallInst *VPOParoptUtils::genKmpcDispatchFini(WRegionNode *W,
                                              StructType *IdentTy,
                                              Value *Tid, int Size,
                                              bool IsUnsigned,
                                              Instruction *InsertPt) {
  BasicBlock  *B = W->getEntryBBlock();
  BasicBlock  *E = W->getExitBBlock();

  Function    *F = B->getParent();
  LLVMContext &C = F->getContext();

  Module *M = F->getParent();

  StringRef FnName;

  if (IsUnsigned)
    FnName = (Size == 32) ? "__kmpc_dispatch_fini_4u" :
                            "__kmpc_dispatch_fini_8u" ;
  else
    FnName = (Size == 32) ? "__kmpc_dispatch_fini_4" :
                            "__kmpc_dispatch_fini_8" ;

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *Loc =
    genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  SmallVector<Value *, 2> FnArgs {Loc, Tid};

  Type *RetTy = Type::getVoidTy(C);

  // Generate __kmpc_dispatch_fini4{u}/8{u} in IR
  CallInst *DispatchFini = genCall(M, FnName, RetTy, FnArgs);
  DispatchFini->insertBefore(InsertPt);

  return DispatchFini;
}


// This function generates OpenMP runtime __kmpc_threadprivate_cached call.
CallInst *VPOParoptUtils::genKmpcThreadPrivateCachedCall(
                   Function *F,
                   Instruction *AI,
                   StructType *IdentTy,
                   Value *Tid,
                   Value *GV,
                   Value *GVSize,
                   Value *TpvGV) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *KmpcLoc =
      genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, &B, &E);

  SmallVector<Value *, 6> FnGetTpvArgs;
  FnGetTpvArgs.push_back(KmpcLoc);
  FnGetTpvArgs.push_back(Tid);
  FnGetTpvArgs.push_back(GV);
  FnGetTpvArgs.push_back(GVSize);
  FnGetTpvArgs.push_back(TpvGV);

  auto ReturnTy = Type::getInt8PtrTy(C);
  return genCall(M, "__kmpc_threadprivate_cached", ReturnTy, FnGetTpvArgs);
}

bool VPOParoptUtils::isKmpcGlobalThreadNumCall(Instruction *I) {
  return VPOAnalysisUtils::isCallOfName(I, "__kmpc_global_thread_num");
}

CallInst *VPOParoptUtils::findKmpcGlobalThreadNumCall(BasicBlock *BB) {
  for (Instruction &II : *BB) {
    if (isKmpcGlobalThreadNumCall(&II))
      return dyn_cast<CallInst>(&II);
  }
  return nullptr;
}

// This function generates a runtime library call to get global OpenMP thread
// ID - __kmpc_global_thread_num(&loc)
CallInst *VPOParoptUtils::genKmpcGlobalThreadNumCall(Function *F,
                                                     Instruction *AI,
                                                     StructType *IdentTy) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  if (!IdentTy)
    IdentTy = StructType::get(C, {Type::getInt32Ty(C),
                                  Type::getInt32Ty(C),
                                  Type::getInt32Ty(C),
                                  Type::getInt32Ty(C),
                                  Type::getInt8PtrTy(C)});

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  GlobalVariable *KmpcLoc =
      genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, &B, &E);

  FunctionType *FnGetTidTy = FunctionType::get(
      Type::getInt32Ty(C), PointerType::getUnqual(IdentTy), false);

  Function *FnGetTid = M->getFunction("__kmpc_global_thread_num");

  if (!FnGetTid) {
    FnGetTid = Function::Create(FnGetTidTy, GlobalValue::ExternalLinkage,
                                "__kmpc_global_thread_num", M);
    FnGetTid->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnGetTidArgs;
  FnGetTidArgs.push_back(KmpcLoc);

  CallInst *GetTidCall = CallInst::Create(FnGetTid, FnGetTidArgs, "tid.val");
  GetTidCall->setCallingConv(CallingConv::C);
  GetTidCall->setTailCall(true);

  return GetTidCall;
}

// This function collects path, file name, line, column information for
// generating kmpc_location struct needed for OpenMP runtime library
GlobalVariable *
VPOParoptUtils::genKmpcLocfromDebugLoc(Function *F, Instruction *AI,
                                       StructType *IdentTy, int Flags,
                                       BasicBlock *BS, BasicBlock *BE) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  std::string LocString;

  StringRef Path("");
  StringRef File("unknown");
  StringRef FnName("unknown");
  unsigned SLine = 0;
  unsigned ELine = 0;

  int VpoEmitSourceLocation = 1;

  for (int K = 0; K < 2; ++K) {
    BasicBlock::iterator I = (K == 0) ? BS->begin() : BE->begin();
    if (Instruction *Inst = dyn_cast<Instruction>(I)) {
      if (DILocation *Loc = Inst->getDebugLoc()) {
        if (K == 0) {
          Path = Loc->getDirectory();
          File = Loc->getFilename();
          FnName = Loc->getScope()->getSubprogram()->getName();
          SLine = Loc->getLine();
        } else {
          ELine = Loc->getLine();
        }
      }
    }
  }

  // Source location string for OpenMP runtime library call
  // LocString = ";pathfilename;routinename;sline;eline;;"
  switch (VpoEmitSourceLocation) {

  case 1:
    LocString = (";unknown;" + FnName + ";" + Twine(SLine) + ";" +
                 Twine(ELine) + ";;\00")
                    .str();
    break;

  case 2:
    LocString = (";" + Path + "/" + File + ";" + FnName + ";" + Twine(SLine) +
                 ";" + Twine(ELine) + ";;\00")
                    .str();
    break;

  case 0:
  default:
    LocString = ";unknown;unknown;0;0;;\00";
    break;
  }

  // Constant Definitions
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
  ConstantInt *ValueFlags = ConstantInt::get(Type::getInt32Ty(C), Flags);

  // Define the type of loc string. It is an array of i8/char type.
  ArrayType *LocStringTy = ArrayType::get(Type::getInt8Ty(C), LocString.size());

  // Create a global variable containing the loc string. Example:
  // @.source.0.0.9 = private unnamed_addr constant [22 x i8]
  // c";unknown;unknown;0;0;;"
  Constant *LocStringInit = ConstantDataArray::getString(C, LocString, false);
  GlobalVariable *LocStringVar = new GlobalVariable(
      *M, LocStringTy, true, GlobalValue::PrivateLinkage, LocStringInit,
      ".source." + Twine(SLine) + "." + Twine(ELine));
  // Allows merging of variables with same content.
  LocStringVar->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  LLVM_DEBUG(dbgs() << "\nSource Location Info: " << LocString << "\n");

  // Get a pointer to the global variable containing loc string.
  Constant *Zeros[] = {ValueZero, ValueZero};
  Constant *LocStringPtr =
      ConstantExpr::getGetElementPtr(LocStringTy, LocStringVar, Zeros);

  // We now have values of all loc struct elements.
  // IdentTy:       {i32,   i32,    i32,    i32,    i8*         }
  // Loc struct:    {0,     Flags,  0,      0,      LocStringPtr}
  // So, we finally create a global variable to hold the struct. Example:
  // @.kmpc_loc.0.0.10 = private unnamed_addr constant { i32, i32, i32, i32, i8*
  // } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22
  // x i8]* @.source.0.0.9, i32 0, i32 0) }
  Constant *StructInit = ConstantStruct::get(
      IdentTy, {ValueZero, ValueFlags, ValueZero, ValueZero, LocStringPtr});
  GlobalVariable *KmpcLoc = new GlobalVariable(
      *M, IdentTy, /*isConstant=*/false, GlobalValue::PrivateLinkage,
      StructInit, ".kmpc_loc." + Twine(SLine) + "." + Twine(ELine));
  // Allows merging of variables with same content.
  KmpcLoc->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  return KmpcLoc;
}

// Generate source location information for Explicit barrier
GlobalVariable *VPOParoptUtils::genKmpcLocforExplicitBarrier(
                   Instruction *AI, StructType *IdentTy, BasicBlock *BB) {
  int Flags = KMP_IDENT_KMPC | KMP_IDENT_BARRIER_EXPL; // bits 0x2 | 0x20

#if 0
  if (VPOParopt_openmp_dvsm)
    flags |= KMP_IDENT_CLOMP;  // bit 0x4
#endif

  Function *F = BB->getParent();
  GlobalVariable *KmpcLoc =
      VPOParoptUtils::genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, BB, BB);
  return KmpcLoc;
}

// Generate source location information for Implicit barrier
GlobalVariable *VPOParoptUtils::genKmpcLocforImplicitBarrier(
    WRegionNode *W, Instruction *AI, StructType *IdentTy, BasicBlock *BB) {
  int Flags = 0;

  switch (W->getWRegionKindID()) {

  case WRegionNode::WRNParallelLoop:
  case WRegionNode::WRNWksLoop:
    Flags = KMP_IDENT_BARRIER_IMPL_FOR;
    break;

  case WRegionNode::WRNParallelSections:
  case WRegionNode::WRNSections:
    Flags = KMP_IDENT_BARRIER_IMPL_SECTIONS;
    break;

  case WRegionNode::WRNTask:
  case WRegionNode::WRNTaskloop:
    break;

  case WRegionNode::WRNSingle:
    Flags = KMP_IDENT_BARRIER_IMPL_SINGLE;
    break;

  default:
    Flags = KMP_IDENT_BARRIER_IMPL;
    break;
  }

  Flags |= KMP_IDENT_KMPC; // bit 0x2

#if 0
  if (PAROPT_openmp_dvsm)
    Flags |= KMP_IDENT_CLOMP;  // bit 0x4
#endif

  Function *F = BB->getParent();
  GlobalVariable *KmpcLoc =
      VPOParoptUtils::genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, BB, BB);
  return KmpcLoc;
}

// Insert kmpc_[cancel_]barrier(...) call before InsertPt. If the call emitted
// is a __kmpc_cancel_barrier(...), add it to the parent WRegionNode's
// CancellationPoints.
CallInst *VPOParoptUtils::genKmpcBarrier(WRegionNode *W, Value *Tid,
                                         Instruction *InsertPt,
                                         StructType *IdentTy, bool IsExplicit) {

  WRegionNode *WParent = W->getParent();
  bool IsCancelBarrier = WParent && WParent->canHaveCancellationPoints() &&
                         WRegionUtils::hasCancelConstruct(WParent);

  auto *BarrierCall = VPOParoptUtils::genKmpcBarrierImpl(
      W, Tid, InsertPt, IdentTy, IsExplicit, IsCancelBarrier);

  if (IsCancelBarrier)
    WParent->addCancellationPoint(BarrierCall);

  return BarrierCall;
}

// Insert kmpc_[cancel_]barrier(...) call before InsertPt.
CallInst *VPOParoptUtils::genKmpcBarrierImpl(WRegionNode *W, Value *Tid,
                                             Instruction *InsertPt,
                                             StructType *IdentTy,
                                             bool IsExplicit,
                                             bool IsCancelBarrier) {
  BasicBlock  *B = InsertPt->getParent();
  Function    *F = B->getParent();
  Module      *M = F->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy;
  StringRef FnName;


  if (IsCancelBarrier) {
    RetTy = Type::getInt32Ty(C);
    FnName = "__kmpc_cancel_barrier";
  } else {
    RetTy = Type::getVoidTy(C);
    FnName = "__kmpc_barrier";
  }

  // Create the arg for Loc
  GlobalVariable *Loc;
  if (IsExplicit)
    Loc = genKmpcLocforExplicitBarrier(InsertPt, IdentTy, B);
  else // Implicit
    Loc = genKmpcLocforImplicitBarrier(W, InsertPt, IdentTy, B);

  // Create the arg for Tid
  LoadInst *LoadTid = new LoadInst(Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  // Create the argument list
  SmallVector<Value *, 3> FnArgs = {Loc, LoadTid};

  CallInst *BarrierCall = genCall(M, FnName, RetTy, FnArgs);
  BarrierCall->insertBefore(InsertPt);

  return BarrierCall;
}

// Generates a critical section around the middle BasicBlocks of `W`
// by emitting calls to `__kmpc_critical` before `BeginInst`, and
// `__kmpc_end_critical` after `EndInst`.
bool VPOParoptUtils::genKmpcCriticalSection(WRegionNode *W, StructType *IdentTy,
                                            Constant *TidPtr,
                                            const StringRef &LockNameSuffix) {
  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");

  assert(W->getBBSetSize() >= 3 &&
         "Critical Node is expected to have at least 3 BBlocks.");

  // W should have entry and exit BBlocks with the directive
  // intrinsic calls, and some middle BBlocks. We intend on inserting the
  // critical calls at the places marked below:
  //
  //    EntryBB:
  //      call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
  //      call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  // +------< begin critical >
  // |    br label %BB1
  // |
  // |  BB1:
  // |    ...
  // |  ...
  // |    br label %ExitBB
  // |
  // |  ExitBB:
  // |    call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
  // |    call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  // +------< end critical >
  //      br label %..

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();
  assert(EntryBB->size() >= 2 && "Entry BBlock has invalid size.");
  assert(ExitBB->size() >= 2 && "Exit BBlock has invalid size.");

  // BeginInst: `br label %BB1` (EntryBB) in the above example.
  Instruction *BeginInst = &(*(EntryBB->rbegin()));
  // EndInst: `call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")`
  // (ExitBB) in the above example.
  Instruction *EndInst = &(*(++(ExitBB->rbegin())));

  assert(BeginInst != nullptr && "BeginInst is null.");
  assert(EndInst != nullptr && "EndInst is null.");

  return genKmpcCriticalSection(W, IdentTy, TidPtr, BeginInst, EndInst,
                                LockNameSuffix);
}

// Wraps the above function for case when the caller does not provide a lock
// name suffix, and uses a default lock name suffix.
bool VPOParoptUtils::genKmpcCriticalSection(WRegionNode *W, StructType *IdentTy,
                                            Constant *TidPtr) {
  return genKmpcCriticalSection(W, IdentTy, TidPtr, "");
}

// Generates a KMPC call to IntrinsicName with Tid obtained using TidPtr.
CallInst *VPOParoptUtils::genKmpcCallWithTid(
    WRegionNode *W, StructType *IdentTy, Value *TidPtr, Instruction *InsertPt,
    StringRef IntrinsicName, Type *ReturnTy, ArrayRef<Value *> Args) {
  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(InsertPt != nullptr && "InsertPt is null.");
  assert(!IntrinsicName.empty() && "IntrinsicName is empty.");
  assert(TidPtr != nullptr && "TidPtr is null.");

  // The KMPC call is of form:
  //     __kmpc_atomic_<type>(loc, tid, args).
  // We have the Intrinsic name, its return type and other function args. The
  // loc argument is obtained using the IdentTy struct inside genKmpcCall. But
  // we need a valid Tid, which we can load from memory using TidPtr.
  LoadInst *LoadTid = new LoadInst(TidPtr, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  // Now bundle all the function arguments together.
  SmallVector<Value*, 3> FnArgs = {LoadTid};

  if (!Args.empty())
    FnArgs.append(Args.begin(), Args.end());

  // And then try to generate the KMPC call.
  return VPOParoptUtils::genKmpcCall(W, IdentTy, InsertPt, IntrinsicName,
                                     ReturnTy, FnArgs);
}

// This function generates a call as follows.
// void @__kmpc_taskgroup(%ident_t* %loc.addr.11.12, i32 %my.tid)
CallInst *VPOParoptUtils::genKmpcTaskgroupCall(WRegionNode *W,
                                               StructType *IdentTy, Value *Tid,
                                               Instruction *InsertPt) {
  return genKmpcTaskgroupOrEndTaskgroupCall(W, IdentTy, Tid, InsertPt, true);
}

// This function generates a call as follows.
// void @__kmpc_end_taskgroup(%ident_t* %loc.addr.11.12, i32 %my.tid)
CallInst *VPOParoptUtils::genKmpcEndTaskgroupCall(WRegionNode *W,
                                                  StructType *IdentTy,
                                                  Value *Tid,
                                                  Instruction *InsertPt) {
  return genKmpcTaskgroupOrEndTaskgroupCall(W, IdentTy, Tid, InsertPt, false);
}

// This function generates calls for the taskgroup region.
//
//   call void @__kmpc_taskgroup(%ident_t* %loc, i32 %tid)
//      or
//   call void @__kmpc_end_taskgroup(%ident_t* %loc, i32 %tid)
CallInst *VPOParoptUtils::genKmpcTaskgroupOrEndTaskgroupCall(
    WRegionNode *W, StructType *IdentTy, Value *Tid, Instruction *InsertPt,
    bool IsTaskgroupStart) {

  BasicBlock *B = W->getEntryBBlock();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = nullptr;
  StringRef FnName;

  if (IsTaskgroupStart)
    FnName = "__kmpc_taskgroup";
  else
    FnName = "__kmpc_end_taskgroup";

  RetTy = Type::getVoidTy(C);

  LoadInst *LoadTid = new LoadInst(Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  // Now bundle all the function arguments together.
  SmallVector<Value *, 3> FnArgs = {LoadTid};

  CallInst *TaskgroupOrEndCall =
      VPOParoptUtils::genKmpcCall(W, IdentTy, InsertPt, FnName, RetTy, FnArgs);
  return TaskgroupOrEndCall;
}

// This function generates a call to query the current thread if it is a master
// thread. Or, generates a call to end_master callfor the team of threads.
//   %master = call @__kmpc_master(%ident_t* %loc, i32 %tid)
//      or
//   call void @__kmpc_end_master(%ident_t* %loc, i32 %tid)
CallInst *VPOParoptUtils::genKmpcMasterOrEndMasterCall(WRegionNode *W,
                            StructType *IdentTy, Value *Tid,
                            Instruction *InsertPt, bool IsMasterStart) {

  BasicBlock  *B = W->getEntryBBlock();
  Function    *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = nullptr;
  StringRef FnName;

  if (IsMasterStart) {
    FnName = "__kmpc_master";
    RetTy = Type::getInt32Ty(C);
  }
  else {
    FnName = "__kmpc_end_master";
    RetTy = Type::getVoidTy(C);
  }

  LoadInst *LoadTid = new LoadInst(Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  // Now bundle all the function arguments together.
  SmallVector<Value *, 3> FnArgs = {LoadTid};

  CallInst *MasterOrEndCall = VPOParoptUtils::genKmpcCall(W,
                                IdentTy, InsertPt, FnName, RetTy, FnArgs);
  return MasterOrEndCall;
}

// This function generates calls to guard the single thread execution for the
// single/end single region.
//
//   call single = @__kmpc_single(%ident_t* %loc, i32 %tid)
//      or
//   call void @__kmpc_end_single(%ident_t* %loc, i32 %tid)
CallInst *VPOParoptUtils::genKmpcSingleOrEndSingleCall(WRegionNode *W,
                            StructType *IdentTy, Value *Tid,
                            Instruction *InsertPt, bool IsSingleStart) {

  BasicBlock  *B = W->getEntryBBlock();
  Function    *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = nullptr;
  StringRef FnName;

  if (IsSingleStart) {
    FnName = "__kmpc_single";
    RetTy = Type::getInt32Ty(C);
  }
  else {
    FnName = "__kmpc_end_single";
    RetTy = Type::getVoidTy(C);
  }

  LoadInst *LoadTid = new LoadInst(Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  // Now bundle all the function arguments together.
  SmallVector<Value *, 3> FnArgs = {LoadTid};

  CallInst *SingleOrEndCall = VPOParoptUtils::genKmpcCall(W,
                                IdentTy, InsertPt, FnName, RetTy, FnArgs);
  return SingleOrEndCall;
}

// This function generates calls to guard the ordered thread execution for the
// ordered/end ordered region.
//
//   call void @__kmpc_ordered(%ident_t* %loc, i32 %tid)
//      or
//   call void @__kmpc_end_ordered(%ident_t* %loc, i32 %tid)
CallInst *VPOParoptUtils::genKmpcOrderedOrEndOrderedCall(WRegionNode *W,
                            StructType *IdentTy, Value *Tid,
                            Instruction *InsertPt, bool IsOrderedStart) {

  BasicBlock  *B = W->getEntryBBlock();
  Function    *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = Type::getVoidTy(C);

  StringRef FnName;

  if (IsOrderedStart)
    FnName = "__kmpc_ordered";
  else
    FnName = "__kmpc_end_ordered";

  LoadInst *LoadTid = new LoadInst(Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  // Now bundle all the function arguments together.
  SmallVector<Value *, 3> FnArgs = {LoadTid};

  CallInst *OrderedOrEndCall = VPOParoptUtils::genKmpcCall(W,
                                 IdentTy, InsertPt, FnName, RetTy, FnArgs);
  return OrderedOrEndCall;
}

// This function generates and inserts calls to kmpc_doacross_wait/post for
// '#pragma omp ordered depend(source/sink)'.
//
//   %dep.vec = alloca i64, align 8                           ; (1)
//   %dv.val = load i32, i32* %<dep.vec.value>                ; (2)
//   %conv = sext i32 %dv.val to i64                          ; (3)
//   store i64 %conv, i64* %dep.vec, align 8                  ; (4)
//   %tid = load i32, i32* %tidptr, align 4
//   call void @__kmpc_doacross_wait/post({ i32, i32, i32, i32, i8* }* %loc, i32
//   %tid, i64* %dims)
//
// If DepVecValue is an alloca, a load is first done from it before
// storing it to 'dims' for the runtime.
//
// The call is inserted before InsertPt.
//
CallInst *VPOParoptUtils::genDoacrossWaitOrPostCall(
    WRNOrderedNode *W, StructType *IdentTy, Value *TidPtr,
    Instruction *InsertPt, Value *DepVecValue, bool IsDoacrossPost) {

  assert(DepVecValue && "Null Dependence Vector Value for doacross wait/post.");

  BasicBlock *BB = InsertPt->getParent();
  assert(BB && "InsertPt has no parent BB.");
  Function *F = BB->getParent();
  assert(F != nullptr && "insertpt has no parent Function");
  Module *M = F->getParent();
  assert(M != nullptr && ") has no parent Module");
  LLVMContext &C = F->getContext();

  Type *Int64Ty = Type::getInt64Ty(C);

  // Since the frontend already collapses the loopnest, the depend vector (third
  // argument of the runtime call), is just an i64 pointer.
  // (1) Allocate space for the Dependence Vector.
  const DataLayout &DL = M->getDataLayout();
  AllocaInst *DepVec = new AllocaInst(
      Int64Ty, DL.getAllocaAddrSpace(), nullptr, 8, "dep.vec");
  DepVec->insertBefore(InsertPt);

  // (2) If DepVecValue is an alloca, generate a load from it.
  if (isa<AllocaInst>(DepVecValue))
    DepVecValue = new LoadInst(DepVecValue, "dv.val", InsertPt);

  // (3) Cast the value of the depend vector to I64.
  CastInst *DepVecValueCast =
      CastInst::CreateSExtOrBitCast(DepVecValue, Int64Ty, "conv", InsertPt);

  // (4) Store to the dependence vector.
  StoreInst *DepVecAssignment = new StoreInst(DepVecValueCast, DepVec);
  DepVecAssignment->insertBefore(InsertPt);

  CallInst *Call = genKmpcCallWithTid(W, IdentTy, TidPtr, InsertPt,
                                      IsDoacrossPost ? "__kmpc_doacross_post"
                                                     : "__kmpc_doacross_wait",
                                      nullptr, {DepVec});

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Doacross wait/post call emitted.\n");

  Call->insertBefore(InsertPt);
  return Call;
}

// This function generates and inserts a call to kmpc_doacross_init,
// for '#pragma omp [parallel] for ordered(n)'
//
//   %dims.vec = alloca { i64, i64, i64 }, align 16                      ; (1)
//   %6 = load i32, i32* %lower.bnd                                      ; (2)
//   %7 = sext i32 %6 to i64                                             ; (3)
//   %8 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }*
//   %dims.vec, i32 0, i32 0                                             ; (4)
//
//   store i64 %7, i64* %8                                               ; (5)
//
//   %9 = load i32, i32* %upper.bnd                                      ; (6)
//   %10 = sext i32 %9 to i64                                            ; (7)
//   %11 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }*
//   %dims.vec, i32 0, i32 1                                             ; (8)
//
//   store i64 %10, i64* %11                                             ; (9)
//
//   %12 = load i32, i32* %stride                                        ; (10)
//   %13 = sext i32 %12 to i64                                           ; (11)
//   %14 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }*
//   %dims.vec, i32 0, i32 2                                             ; (12)
//
//   store i64 %13, i64* %14                                             ; (13)
//
//   %my.tid32 = load i32, i32* %tid, align 4                            ; Tid
//   call void @__kmpc_doacross_init({ i32, i32, i32, i32, i8* }*        ; (14)
//   @.kmpc_loc.0.0.8, i32 %my.tid32, i32 1, { i64, i64, i64 }* %dims.vec)
//
//   The call along with the above initializations of dims.vec, are inserted
//   before InsertPt.
CallInst *VPOParoptUtils::genKmpcDoacrossInit(WRegionNode *W,
                                              StructType *IdentTy, Value *Tid,
                                              Instruction *InsertPt,
                                              Value *LBound, Value *UBound,
                                              Value *Stride) {

  assert(LBound && "Null Lower Bound for doacross init.");
  assert(UBound && "Null Upper Bound for doacross init.");
  assert(Stride && "Null Stride for doacross init.");

  BasicBlock *BB = InsertPt->getParent();
  assert(BB && "InsertPt has no parent BB.");

  Function *F = BB->getParent();
  assert(F != nullptr && "InsertPt has no parent Function");

  Module *M = F->getParent();
  assert(M != nullptr && ") has no parent Module");

  LLVMContext &C = F->getContext();

  Type *Int64Ty = Type::getInt64Ty(C);
  Type *Int32Ty = Type::getInt32Ty(C);
  Value *Zero = ConstantInt::get(Int32Ty, 0);
  Value *One = ConstantInt::get(Int32Ty, 1);
  Value *Two = ConstantInt::get(Int32Ty, 2);

  // The dims vector needs to be a struct of 3 i64s,  which should contain
  // lb, ub and stride respectively. Note that since the incoming loop is
  // already collapsed, there is only one struct needed, not an array of
  // these structs.
  StructType *DimsVecTy = StructType::get(C, {Int64Ty, Int64Ty, Int64Ty});

  // (1) Create alloca for the struct.
  const DataLayout &DL = M->getDataLayout();
  AllocaInst *DimsVec = new AllocaInst(
      DimsVecTy, DL.getAllocaAddrSpace(), nullptr, 16, "dims.vec");
  DimsVec->insertBefore(InsertPt);

  auto populateDimsVecAtIndex = [&](Value *Index, Value *ValPtr) {
    assert(isa<AllocaInst>(ValPtr) &&
           "populateDimsVecAtIndex: ValPtr is not an Alloca.");

    Value *ValLoad = new LoadInst(ValPtr, "", InsertPt);   // (2), (6), (10)
    CastInst *ValCast = CastInst::CreateSExtOrBitCast(
        ValLoad, Int64Ty, "", InsertPt);                   // (3), (7), (11)
    GetElementPtrInst *DstPtr = GetElementPtrInst::CreateInBounds(
        DimsVec, {Zero, Index}, "", InsertPt);             // (4), (8), (12)
    StoreInst *DstStore = new StoreInst(ValCast, DstPtr);  // (5), (9), (13)
    DstStore->insertBefore(InsertPt);
  };

  // Store lbound to index 0 of the struct.
  populateDimsVecAtIndex(Zero, LBound);  // (2), (3), (4), (5)

  // Store ubound to index 1 of the struct.
  populateDimsVecAtIndex(One, UBound);   // (6), (7), (8), (9)

  // Store stride to index 2 of the struct.
  populateDimsVecAtIndex(Two, Stride);   // (10), (11), (12), (13)

  // Emit the call to __kmpc_doacross_init.
  CallInst *Call = genKmpcCall(W, IdentTy, InsertPt, "__kmpc_doacross_init",
                               nullptr, {Tid, One, DimsVec}); // (14)

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Doacross init call emitted.\n");

  Call->insertBefore(InsertPt);
  return Call;
}

// This function generates and inserts a call to kmpc_doacross_fini,
// for '#pragma omp for ordered(n)'
//
//   call void @__kmpc_doacross_fini({ i32, i32, i32, i32, i8* }* @.kmpc_loc,
//   i32 %my.tid)
//
//   The call is inserted \en after \p InsertPt.
CallInst *VPOParoptUtils::genKmpcDoacrossFini(WRegionNode *W,
                                              StructType *IdentTy, Value *Tid,
                                              Instruction *InsertPt) {

  CallInst *Fini = VPOParoptUtils::genKmpcCall(
      W, IdentTy, InsertPt, "__kmpc_doacross_fini", nullptr, {Tid});

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Doacross fini call emitted.\n");

  Fini->insertAfter(InsertPt);
  return Fini;
}

// Create this call
//
//   call void @__kmpc_flush(%ident_t* %loc)
//
// and insert it at InsertPt. Then, return it.
CallInst *VPOParoptUtils::genKmpcFlush(WRegionNode *W, StructType *IdentTy,
                                       Instruction *InsertPt) {
  BasicBlock  *B = W->getEntryBBlock();
  Function    *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = Type::getVoidTy(C);

  CallInst *Flush = VPOParoptUtils::genKmpcCall(W, IdentTy, InsertPt,
                                                "__kmpc_flush", RetTy, {},
                                                true /*insert call*/ );
  return Flush;
}


// Private helper methods for generation of a KMPC call.

// Generates KMPC calls to the intrinsic `IntrinsicName`.
//
// If \p Insert is true (default is false), then insert the call into the IR
// \b before \p InsertPt
//
// Note: \p InsertPt is also used to get the Loc, so it cannot be nullptr
// when calling genKmpcCall, even when \p Insert is false.
CallInst *VPOParoptUtils::genKmpcCall(WRegionNode *W, StructType *IdentTy,
                                      Instruction *InsertPt,
                                      StringRef IntrinsicName, Type *ReturnTy,
                                      ArrayRef<Value *> Args, bool Insert) {
  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(InsertPt != nullptr && "InsertPt is null.");
  assert(!IntrinsicName.empty() && "IntrinsicName is empty.");

  // Obtain Loc info
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  Module *M = F->getParent();

  int Flags = KMP_IDENT_KMPC;

  // Before emitting the KMPC call, we need the Loc information.
  GlobalVariable *Loc =
      genKmpcLocfromDebugLoc(F, InsertPt, IdentTy, Flags, B, E);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Loc: " << *Loc << "\n");

  // At this point, we have all the function args: loc + incoming Args. We bind
  // them together as FnArgs.
  SmallVector<Value *, 9> FnArgs = {Loc};
  FnArgs.append(Args.begin(), Args.end());

  // Next, for return type, we use ReturnType if it is not null, otherwise we
  // use VoidTy.
  LLVMContext &C = F->getContext();
  ReturnTy = (ReturnTy != nullptr) ? ReturnTy : Type::getVoidTy(C);

  if (!Insert)
    InsertPt = nullptr; // do not emit the call into the IR

  return genCall(M, IntrinsicName, ReturnTy, FnArgs, InsertPt);
}

// Genetates a CallInst for the given Function* Fn and its argument list.
// Fn is assumed to be already declared.
CallInst *VPOParoptUtils::genCall(Function *Fn, ArrayRef<Value *> FnArgs,
                                  ArrayRef<Type*> FnArgTypes,
                                  Instruction *InsertPt,
                                  bool IsTail, bool IsVarArg) {
  assert(Fn != nullptr && "Function Declaration is null.");
  CallInst *Call = CallInst::Create(Fn, FnArgs, "", InsertPt);
  // Note: if InsertPt!=nullptr, Call is emitted into the IR as well.
  assert(Call != nullptr && "Failed to generate Function Call");

  Call->setCallingConv(CallingConv::C);
  Call->setTailCall(IsTail);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Function call: " << *Call << "\n");

  return Call;
}

// Genetates a CallInst for a function with name `FnName`.
CallInst *VPOParoptUtils::genCall(Module *M, StringRef FnName, Type *ReturnTy,
                                  ArrayRef<Value *> FnArgs,
                                  ArrayRef<Type*> FnArgTypes,
                                  Instruction *InsertPt,
                                  bool IsTail, bool IsVarArg) {
  assert(M != nullptr && "Module is null.");
  assert(!FnName.empty() && "Function name is empty.");
  assert(FunctionType::isValidReturnType(ReturnTy) && "Invalid Return Type");

  // Create the function type from the return type and argument types.
  FunctionType *FnTy = FunctionType::get(ReturnTy, FnArgTypes, IsVarArg);

  // Get the function prototype from the module symbol table. If absent,
  // create and insert it into the symbol table first.
  Constant *FnC = M->getOrInsertFunction(FnName, FnTy);
  Function *Fn = cast<Function>(FnC);

  CallInst *Call = genCall(Fn, FnArgs, FnArgTypes, InsertPt, IsTail, IsVarArg);
  return Call;
}

// A genCall() interface where FunArgTypes is omitted; it will be computed from
// FnArgs.
CallInst *VPOParoptUtils::genCall(Module *M, StringRef FnName, Type *ReturnTy,
                                  ArrayRef<Value *> FnArgs,
                                  Instruction *InsertPt, bool IsTail,
                                  bool IsVarArg) {
  // Before creating a call to the function, we first need to insert the
  // function prototype of the intrinsic into the Module's symbol table. To
  // create the prototype, we need the FunctionType, which requires the types
  // of all formal paramters. We create a list of such types below from the
  // function arguments.
  SmallVector<Type *, 9> FnArgTypes;
  for (Value *Arg : FnArgs) {
    Type *ArgType = Arg->getType();
    assert(FunctionType::isValidArgumentType(ArgType) && "Invalid Argument.");
    FnArgTypes.insert(FnArgTypes.end(), ArgType);
  }
  CallInst *Call = genCall(M, FnName, ReturnTy, FnArgs, FnArgTypes, InsertPt,
                           IsTail, IsVarArg);
  return Call;
}

// A genCall() interface where the Module is omitted; it will be computed from
// the insertion point.
CallInst *VPOParoptUtils::genCall(StringRef FnName, Type *ReturnTy,
                                  ArrayRef<Value*> FnArgs,
                                  ArrayRef<Type*> FnArgTypes,
                                  Instruction *InsertPt, bool IsTail,
                                  bool IsVarArg) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();

  CallInst *Call = genCall(M, FnName, ReturnTy, FnArgs, FnArgTypes, InsertPt,
                           IsTail, IsVarArg);
  return Call;
}

// Private helper methods for generation of a critical section.

// Creates a prefix for the name of the lock var to be used in KMPC critical
// calls.
SmallString<64> VPOParoptUtils::getKmpcCriticalLockNamePrefix(WRegionNode *W) {

  assert(W != nullptr && "WRegionNode is null.");

  if (isa<WRNAtomicNode>(W))
    return SmallString<64>("_kmpc_atomic_");

  // For Critical, the lock name is determined based on OS and architecture.
  BasicBlock *BB = W->getEntryBBlock();
  assert(BB && "WRegionNode has no Entry BB.");
  Function *F = BB->getParent();
  assert(F != nullptr && "BB has no parent Function");
  Module *M = F->getParent();
  assert(M != nullptr && "Function has no parent Module");

  Triple TargetTriple(M->getTargetTriple());

  if (TargetTriple.isOSWindows()) {
    Triple::ArchType Arch = TargetTriple.getArch();
    if (Arch == Triple::x86)
      return SmallString<64>("_$vcomp$critsect$");

    if (Arch == Triple::x86_64)
      return SmallString<64>("$vcomp$critsect$");

    // TODO: Check if we need to check for other architectures here.
  }

  // TODO: Check if we need to check for Architectre/ OS types here.
  return SmallString<64>(".gomp_critical_user_");
}


// Returns the lock variable to be used in KMPC critical calls.
GlobalVariable *
VPOParoptUtils::genKmpcCriticalLockVar(WRegionNode *W,
                                       const StringRef &LockNameSuffix) {

  assert(W != nullptr && "WRegionNode is null.");

  // We first get the lock name prefix for the lock var based on the target.
  SmallString<64> LockName = getKmpcCriticalLockNamePrefix(W);
  LockName += LockNameSuffix;
  LockName += ".var";

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Lock name:" << LockName << ".\n");

  // Now, the type for lock variable is an array of eight 32-bit integers.
  BasicBlock *BB = W->getEntryBBlock();
  assert(BB && "WRegionNode has no Entry BB.");
  Function *F = BB->getParent();
  Module *M = F->getParent();
  LLVMContext &C = M->getContext();

  ArrayType *LockVarTy = ArrayType::get(Type::getInt32Ty(C), 8);

  // See if a lock object already exists, if so, reuse it.
  GlobalVariable *GV =
      M->getGlobalVariable(LockName);
  if (GV != nullptr) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Reusing existig lock var: " << *GV
                      << ".\n");

    assert(GV->getType()->getContainedType(0) == LockVarTy &&
           "Lock variable name conflicts with an existing variable.");
    return GV;
  }

  // Otherwise, Create a new lock object. CommonLinkage is used so that multiple
  // lock variables with the same name (across modules) get merged into a single
  // one at link time.
  GV = new GlobalVariable(*M, LockVarTy, false, GlobalValue::CommonLinkage,
                          ConstantAggregateZero::get(LockVarTy), LockName);

  assert(GV != nullptr && "Unable to generate Kmpc critical lock var.");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Lock var generated: " << *GV
                    << ".\n");

  return GV;
}

// Generates a critical section around Instructions `begin` and `end`.
bool VPOParoptUtils::genKmpcCriticalSectionImpl(
    WRegionNode *W, StructType *IdentTy, Constant *TidPtr,
    Instruction *BeginInst, Instruction *EndInst, GlobalVariable *LockVar) {

  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(BeginInst != nullptr && "BeginInst is null.");
  assert(EndInst != nullptr && "EndInst is null.");
  assert(LockVar != nullptr && "LockVar is null.");

  CallInst *BeginCritical =
      genKmpcCallWithTid(W, IdentTy, TidPtr, BeginInst, "__kmpc_critical",
                         nullptr, {LockVar});
  assert(BeginCritical != nullptr && "Could not call __kmpc_critical");

  if (BeginCritical == nullptr)
      return false;

  CallInst *EndCritical =
      genKmpcCallWithTid(W, IdentTy, TidPtr, EndInst, "__kmpc_end_critical",
                         nullptr, {LockVar});
  assert(EndCritical != nullptr && "Could not call __kmpc_end_critical");

  if (BeginCritical == nullptr)
      return false;

  // Now insert the calls in the IR.
  BeginCritical->insertBefore(BeginInst);
  if (EndInst->isTerminator())
    EndCritical->insertBefore(EndInst);
  else
    EndCritical->insertAfter(EndInst);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Critical Section generated.\n");
  return true;
}

// Generates a critical section around Instructions `BeginInst` and `Endinst`,
// by emitting calls to `__kmpc_critical` before `BeginInst`, and
// `__kmpc_end_critical` after `EndInst`.
bool VPOParoptUtils::genKmpcCriticalSection(WRegionNode *W, StructType *IdentTy,
                                            Constant *TidPtr,
                                            Instruction *BeginInst,
                                            Instruction *EndInst,
                                            const StringRef &LockNameSuffix) {
  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(BeginInst != nullptr && "BeginInst is null.");
  assert(EndInst != nullptr && "EndInst is null.");

  // Generate the Lock object for critical section.
  GlobalVariable *Lock = genKmpcCriticalLockVar(W, LockNameSuffix);
  assert(Lock != nullptr && "Could not create critical section lock variable.");

  return genKmpcCriticalSectionImpl(W, IdentTy, TidPtr, BeginInst, EndInst,
                                    Lock);
}

// Generates and inserts a 'kmpc_cancel[lationpoint]' CallInst.
CallInst *VPOParoptUtils::genKmpcCancelOrCancellationPointCall(
    WRegionNode *W, StructType *IdentTy, Constant *TidPtr,
    Instruction *InsertPoint, WRNCancelKind CancelKind,
    bool IsCancellationPoint) {

  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(InsertPoint != nullptr && "InsertionPoint is null.");

  StringRef FnName;

  if (IsCancellationPoint)
    FnName = "__kmpc_cancellationpoint";
  else
    FnName = "__kmpc_cancel";

  BasicBlock *B = W->getEntryBBlock();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  CallInst *CancelCall = genKmpcCallWithTid(
      W, IdentTy, TidPtr, InsertPoint, FnName, Type::getInt32Ty(C),
      {ConstantInt::get(Type::getInt32Ty(C), CancelKind)});

  CancelCall->insertBefore(InsertPoint);

  WRegionNode *WParent = W->getParent();
  assert(WParent && "genKmpcCancelOrCancellationPointCall: Orphaned "
                    "cancel/cancellation point construct");
  WParent->addCancellationPoint(CancelCall);

  return CancelCall;
}

// Emit Constructor call and insert it after PrivAlloca
CallInst *VPOParoptUtils::genConstructorCall(Function *Ctor, Value *V,
                                             Value* PrivAlloca) {
  if (Ctor == nullptr)
    return nullptr;

  Type *ValType = V->getType();
  CallInst *Call = genCall(Ctor, {V}, {ValType}, nullptr);
  Instruction *InsertAfterPt = cast<Instruction>(PrivAlloca);
  Call->insertAfter(InsertAfterPt);
  LLVM_DEBUG(dbgs() << "CONSTRUCTOR: " << *Call << "\n");
  return Call;
}

// Emit Destructor call and insert it before InsertBeforePt
CallInst *VPOParoptUtils::genDestructorCall(Function *Dtor, Value *V,
                                            Instruction *InsertBeforePt) {
  if (Dtor == nullptr)
    return nullptr;

  Type *ValType = V->getType();
  CallInst *Call = genCall(Dtor, {V}, {ValType}, nullptr);
  Call->insertBefore(InsertBeforePt);
  LLVM_DEBUG(dbgs() << "DESTRUCTOR: " << *Call << "\n");
  return Call;
}

// Emit Copy Constructor call and insert it before InsertBeforePt
CallInst *VPOParoptUtils::genCopyConstructorCall(Function *Cctor, Value *D,
                                  Value *S, Instruction *InsertBeforePt) {
  if (Cctor == nullptr)
    return nullptr;

  Type *DTy = D->getType();
  Type *STy = S->getType();

  CallInst *Call = genCall(Cctor, {D,S}, {DTy, STy}, nullptr);
  Call->insertBefore(InsertBeforePt);
  LLVM_DEBUG(dbgs() << "COPY CONSTRUCTOR: " << *Call << "\n");
  return Call;
}

// Emit Copy Assign call and insert it before InsertBeforePt
CallInst *VPOParoptUtils::genCopyAssignCall(Function *Cp, Value *D, Value *S,
                                            Instruction *InsertBeforePt) {
  if (Cp == nullptr)
    return nullptr;

  Type *DTy = D->getType();
  Type *STy = S->getType();

  CallInst *Call = genCall(Cp, {D,S}, {DTy, STy}, nullptr);
  Call->insertBefore(InsertBeforePt);
  LLVM_DEBUG(dbgs() << "COPY ASSIGN: " << *Call << "\n");
  return Call;
}

// Computes the OpenMP loop upper bound so that the iteration space can be
// closed interval.
Value *VPOParoptUtils::computeOmpUpperBound(WRegionNode *W,
                                            Instruction* InsertPt) {
  assert(W->getIsOmpLoop() && "computeOmpUpperBound: not a loop-type WRN");

  Loop *L = W->getWRNLoopInfo().getLoop();

  Value *RightValue = WRegionUtils::getOmpLoopUpperBound(L);
  bool IsLeft = true;
  CmpInst::Predicate PD = WRegionUtils::getOmpPredicate(L, IsLeft);
  IntegerType *UpperBoundTy =
    cast<IntegerType>(RightValue->getType());
  ConstantInt *ValueOne  = ConstantInt::get(UpperBoundTy, 1);

  Value *Res = RightValue;
  IRBuilder<> Builder(InsertPt);

  BasicBlock *LatchBB = L->getLoopLatch();
  BasicBlock *HeaderBB = L->getHeader();
  BranchInst *BR = dyn_cast<BranchInst>(LatchBB->getTerminator());

  // -------------------------------+----------------------------------
  // Non-Reversed Branch            | Reversed Branch
  // -------------------------------+----------------------------------
  //  for ( i = 0; i < 10; i++) {...}
  // -------------------------------+----------------------------------
  // (A)                            | (B)
  //                                |
  //  L1:                     <loop header>   L1:
  //                                |
  //  i < 10 ? goto L1: goto L2;    |         i > 9 ? goto L2: goto L1;
  //                                |
  //  L2:                      <loop exit>    L2:
  //                                |
  // -------------------------------+----------------------------------
  //  No need to swap               | Need to swap and invert the predicate
  // -------------------------------+----------------------------------
  //
  //  for ( i = 10; i > 0; i--) {...}
  // -------------------------------+----------------------------------
  // (C)                            | (D)
  //                                |
  //  L3:                     <loop header>   L3:
  //                                |
  //  i > 0 ? goto L3: goto L4;     |         i < 1 ? goto L4: goto L3;
  //                                |
  //  L4:                      <loop exit>    L4:
  // -------------------------------+----------------------------------
  //  No need to swap               | Need to swap and invert the predicate
  // -------------------------------+----------------------------------
  //
  // The compiler transforms the loop as follows.
  //   If the first edge is not a back edge, swap the edges and
  //   invert the predicate.
  assert(BR && "computeOmpUpperBound: Expect non-empty branch instruction");
  if (BR->getSuccessor(0) != HeaderBB) {
    BR->swapSuccessors();
    ICmpInst *Cond = dyn_cast<ICmpInst>(BR->getCondition());
    assert(Cond && "computeOmpUpperBound: Expect non-empty cmp instruction");
    Cond->setPredicate(ICmpInst::getInversePredicate(PD));
    PD = WRegionUtils::getOmpPredicate(L, IsLeft);
  }

  if (PD == ICmpInst::ICMP_SLT ||
      PD == ICmpInst::ICMP_ULT) {
    if (IsLeft)
      Res = Builder.CreateSub(RightValue, ValueOne);
    else
      Res = Builder.CreateAdd(RightValue, ValueOne);
  }
  else if (PD == ICmpInst::ICMP_SGT ||
           PD == ICmpInst::ICMP_UGT) {
    if (IsLeft)
      Res = Builder.CreateAdd(RightValue, ValueOne);
    else
      Res = Builder.CreateSub(RightValue, ValueOne);
  }

  return Res;

}

// Returns the predicate which includes equal for the zero trip test.
CmpInst::Predicate VPOParoptUtils::computeOmpPredicate(CmpInst::Predicate PD) {
  if (CmpInst::isSigned(PD))
    return ICmpInst::ICMP_SLE;
  else {
    assert(CmpInst::isUnsigned(PD) &&
         "computeOmpPredicate: Expect unsigned predicate");
    return ICmpInst::ICMP_ULE;
  }
}

// Updates the bottom test predicate to include equal predicate.
void VPOParoptUtils::updateOmpPredicateAndUpperBound(WRegionNode *W,
                                                     Value *UB,
                                                     Instruction* InsertPt) {

  assert(W->getIsOmpLoop() && "computeOmpUpperBound: not a loop-type WRN");

  Loop *L = W->getWRNLoopInfo().getLoop();
  ICmpInst* IC = WRegionUtils::getOmpLoopBottomTest(L);
  bool IsLeft = true;
  CmpInst::Predicate PD = WRegionUtils::getOmpPredicate(L, IsLeft);
  Value *PredOperand;
  if (IsLeft)
    PredOperand = IC->getOperand(1);
  else
    PredOperand = IC->getOperand(0);

  if (UB->getType()->getIntegerBitWidth() !=
      PredOperand->getType()->getIntegerBitWidth()) {
    IRBuilder<> B(InsertPt);
    UB = B.CreateSExtOrTrunc(UB, PredOperand->getType());
  }

  if (IsLeft)
    IC->setOperand(1, UB);
  else {
    IC->setOperand(0, UB);
    IC->swapOperands();
  }

  IC->setPredicate(computeOmpPredicate(PD));

  BasicBlock *LatchBB = L->getLoopLatch();
  BasicBlock *HeaderBB = L->getHeader();
  BranchInst *BR = dyn_cast<BranchInst>(LatchBB->getTerminator());
  assert(BR &&
      "updateOmpPredicateAndUpperBound: Expect non-empty branch instruction");

  assert(BR->getSuccessor(0) == HeaderBB &&
         "updateOmpPredicateAndUpperBound: Expect the first target of the \
      branch instruction is the loop header");
  (void) HeaderBB;
  (void) BR;
}

static Value *findChainToLoad(Value *V,
                              SmallVectorImpl<Instruction *> &ChainToBase) {
  if (TruncInst *Trunc = dyn_cast<TruncInst>(V)) {
    ChainToBase.push_back(Trunc);
    return findChainToLoad(Trunc->getOperand(0), ChainToBase);
  } else if (SExtInst *SI = dyn_cast<SExtInst>(V)) {
    ChainToBase.push_back(SI);
    return findChainToLoad(SI->getOperand(0), ChainToBase);
  } else if (ZExtInst *ZI = dyn_cast<ZExtInst>(V)) {
    ChainToBase.push_back(ZI);
    return findChainToLoad(ZI->getOperand(0), ChainToBase);
  } else if (LoadInst *LI = dyn_cast<LoadInst>(V)) {
    ChainToBase.push_back(LI);
    return LI;
  }
  llvm_unreachable("findChainToLoad: unhandled instruction");
}

// Creates a clone of CI, adds OpBundlesToAdd to it, and returns it.
CallInst *VPOParoptUtils::addOperandBundlesInCall(
    CallInst *CI,
    ArrayRef<std::pair<StringRef, ArrayRef<Value *>>> OpBundlesToAdd) {

  assert(CI && "addOperandBundlesInCall: Null CallInst");

  if (OpBundlesToAdd.empty())
    return CI;

  SmallVector<Value *, 8> Args;
  for (auto AI = CI->arg_begin(), AE = CI->arg_end(); AI != AE; AI++) {
    Args.insert(Args.end(), *AI);
  }

  SmallVector<OperandBundleDef, 1> OpBundles;
  CI->getOperandBundlesAsDefs(OpBundles);

  for (auto &StrValVec : OpBundlesToAdd) {
    OperandBundleDef B(StrValVec.first, StrValVec.second);
    OpBundles.push_back(B);
  }

  auto NewI = CallInst::Create(CI->getCalledValue(), Args, OpBundles, "", CI);

  NewI->takeName(CI);
  NewI->setCallingConv(CI->getCallingConv());
  NewI->setAttributes(CI->getAttributes());
  NewI->setDebugLoc(CI->getDebugLoc());

  CI->replaceAllUsesWith(NewI);
  CI->eraseFromParent();

  return NewI;
}

// Clones the load instruction and inserts before the InsertPt.
Value *VPOParoptUtils::cloneInstructions(Value *V, Instruction *InsertBefore) {
  if (isa<Constant>(V))
    return V;

  SmallVector<Instruction *, 3> ChainToBase;
  findChainToLoad(V, ChainToBase);
  std::reverse(ChainToBase.begin(), ChainToBase.end());

  Instruction *LastClonedValue = nullptr;
  Instruction *LastValue = nullptr;

  for (Instruction *Instr : ChainToBase) {
    Instruction *ClonedValue = Instr->clone();
    ClonedValue->insertBefore(InsertBefore);
    ClonedValue->setName(Instr->getName() + ".remat");
    if (LastClonedValue)
      ClonedValue->replaceUsesOfWith(LastValue, LastClonedValue);
    LastClonedValue = ClonedValue;
    LastValue = Instr;
  }
  return LastClonedValue;

  /*
    if (auto *LI = dyn_cast<LoadInst>(V)) {
      auto NewLI = LI->clone();
      NewLI->insertBefore(InsertPt);
      return NewLI;
    }
    else
      return V;
  */
}

// Generate the pointer pointing to the head of the array.
Value *VPOParoptUtils::genArrayLength(AllocaInst *AI, Value *BaseAddr,
                                      Instruction *InsertPt,
                                      IRBuilder<> &Builder, Type *&ElementTy,
                                      Value *&ArrayBegin) {
  Type *AllocaTy = AI->getAllocatedType();
  Type *ScalarTy = AllocaTy->getScalarType();
  ArrayType *ArrTy = dyn_cast<ArrayType>(ScalarTy);
  assert(ArrTy && "Expect array type. ");

  SmallVector<llvm::Value *, 8> GepIndices;
  ConstantInt *Zero = Builder.getInt32(0);
  GepIndices.push_back(Zero);
  uint64_t CountFromCLAs = 1;

  ArrayType *ArrayT = ArrTy;
  while (ArrayT) {
    GepIndices.push_back(Zero);
    CountFromCLAs *= ArrayT->getNumElements();
    ElementTy = ArrayT->getElementType();
    ArrayT = dyn_cast<ArrayType>(ElementTy);
  }

  LLVMContext &C = InsertPt->getParent()->getParent()->getContext();
  ArrayBegin = Builder.CreateInBoundsGEP(BaseAddr, GepIndices, "array.begin");
  Value *numElements = ConstantInt::get(Type::getInt32Ty(C), CountFromCLAs);

  return numElements;
}

Constant* VPOParoptUtils::getMinMaxIntVal(LLVMContext &C, Type *Ty,
                                             bool IsUnsigned, bool GetMax) {
  IntegerType *IntTy = dyn_cast<IntegerType>(Ty->getScalarType());
  assert(IntTy && "getMinMaxIntVal: Expected Interger type");

  unsigned BitWidth = IntTy->getBitWidth();
  assert(BitWidth <= 64 && "getMinMaxIntVal: Expected BitWidth <= 64");

  APInt MinMaxAPInt;

  if (GetMax)
    MinMaxAPInt = IsUnsigned ? APInt::getMaxValue(BitWidth) :
                               APInt::getSignedMaxValue(BitWidth);
  else
    MinMaxAPInt = IsUnsigned ? APInt::getMinValue(BitWidth) :
                               APInt::getSignedMinValue(BitWidth);

  ConstantInt *MinMaxVal = ConstantInt::get(C, MinMaxAPInt);
  if (VectorType *VTy = dyn_cast<VectorType>(Ty))
    return ConstantVector::getSplat(VTy->getNumElements(), MinMaxVal);
  return MinMaxVal;
}

#if 0
uint64_t VPOParoptUtils::getMaxInt(Type *Ty, bool IsUnsigned) {

  uint64_t MaxInt;
  IntegerType *IntTy = dyn_cast<IntegerType>(Ty);

  assert(IntTy && "getMaxInt: Expected Interger type");
  assert(IntTy->getBitWidth() <= 64 && "getMaxInt: Expected BitWidth <= 64");

  if (IsUnsigned)
    MaxInt = IntTy->getBitMask();  // binary 11...111 for n-bit integers
  else { // signed
    MaxInt = IntTy->getSignBit();  // binary 10...000 for n-bit integers
    MaxInt -= 1;                   // binary 01...111 for n-bit integers
  }

  return MaxInt;
}

uint64_t VPOParoptUtils::getMinInt(Type *Ty, bool IsUnsigned) {

  uint64_t MinInt;
  IntegerType *IntTy = dyn_cast<IntegerType>(Ty);

  assert(IntTy && "getMinInt: Expected Interger type");
  assert(IntTy->getBitWidth() <= 64 && "getMinInt: Expected BitWidth <= 64");

  if (IsUnsigned)
    MinInt = 0;
  else // signed
    MinInt = IntTy->getSignBit();  // binary 10...000 for n-bit integers

  return MinInt;
}
#endif // if 0

#endif // INTEL_COLLAB
