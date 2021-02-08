#if INTEL_COLLAB
//==-- VPOParoptUtils.cpp - Utilities for VPO Paropt Transforms -*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include <string>

#define DEBUG_TYPE "vpo-paropt-utils"

using namespace llvm;
using namespace llvm::vpo;

// Disable outline verification by default, because it fails in many tests
// currently.
static cl::opt<bool> EnableOutlineVerification(
    "vpo-paropt-enable-outline-verification", cl::Hidden, cl::init(true),
    cl::desc("Enable checking that all arguments of outlined routines "
             "are pointers or have pointer-sized types."));

static cl::opt<bool> StrictOutlineVerification(
    "vpo-paropt-strict-outline-verification", cl::Hidden, cl::init(false),
    cl::desc("Only allow pointers to be arguments of outlined routines."));

static cl::opt<bool> SPIRVTargetHasEUFusion(
    "vpo-paropt-spirv-target-has-eu-fusion", cl::Hidden, cl::init(true),
    cl::desc("Generate code for SPIR-V target with EU fusion."));

// Undocumented option to control execution scheme for SPIR targets.
// This option has to have the same value for the host and the target
// compilations to work properly.
static cl::opt<spirv::ExecutionSchemeTy> SPIRExecutionScheme(
    spirv::ExecutionSchemeOptionName, cl::Hidden,
    cl::init(spirv::ImplicitSIMDSPMDES),
    cl::desc(""),
    cl::values(
        clEnumValN(spirv::ImplicitSIMDES,
            "0", "Default"),               // Implicit SIMD
        clEnumValN(spirv::ImplicitSIMDSPMDES,
            "1", "<undocumented>"),        // Implicit SIMD with SPMD
        clEnumValN(spirv::ExplicitSIMDES,
            "2", "<undocumented>")));      // Explicit SIMD

// Control whether "omp target parallel for" may be executed
// with multiple teams/WGs.
// This option has to have the same value for the host and the target
// compilations to work properly.
static cl::opt<bool> SPIRImplicitMultipleTeams(
    "vpo-implicit-multiple-teams", cl::Hidden, cl::init(true),
    cl::desc("Allow creation of multiple WGs for executing omp target "
             "parallel for. Note that it is not OpenMP conformant."));

// Control the level of detail of the source location info passed to OpenMP
// via the kmp_ident_t* argument of most kmp calls.
static cl::opt<uint32_t> ParallelSourceInfoMode(
    "parallel-source-info", cl::Hidden, cl::init(1),
    cl::desc("Control source location info in OpenMP code.  0 = none; "
             "1 (default) = function + line; 2 = path + function + line."));

// Enable target compilation mode expliclty, e.g. for LIT tests.
// This option is currently not used by any driver.
static cl::opt<bool> SwitchToOffload(
    "switch-to-offload", cl::Hidden, cl::init(false),
    cl::desc("switch to offload mode (default = false)"));

// Enables the emission of tgt_push_code_location calls.
static cl::opt<bool> PushCodeLocation(
    "vpo-paropt-enable-push-code-location", cl::Hidden, cl::init(true),
    cl::desc("Emit calls to __tgt_push_code_location()"));

// If module M has a StructType of name Name, and element types ElementTypes,
// return it.
StructType *VPOParoptUtils::getStructTypeWithNameAndElementsFromModule(
    Module *M, StringRef Name, ArrayRef<Type *> ElementTypes) {

  assert(M && "Null module pointer.");
  assert(!Name.empty() && "Empty struct name.");
  assert(!ElementTypes.empty() && "Empty element types array.");

  StructType *StructTypeWithMatchingName =
      StructType::getTypeByName(M->getContext(), Name);
  if (!StructTypeWithMatchingName)
    return nullptr;

  if (StructTypeWithMatchingName->elements().equals(ElementTypes))
    return StructTypeWithMatchingName;

  llvm_unreachable("Struct type with matching name has different elements.");
}

// If in the module of F, a StructType with name Name and types
// ElementTypes exists, then return it, otherwise create that struct and
// return it.
StructType *
VPOParoptUtils::getOrCreateStructType(Function *F, StringRef Name,
                                      ArrayRef<Type *> ElementTypes) {

  assert(F && "Null function pointer.");
  assert(!Name.empty() && "Empty struct name.");
  assert(!ElementTypes.empty() && "Empty element types array.");

  LLVMContext &C = F->getContext();
  Module *M = F->getParent();

  StructType *ExistingStructType =
      getStructTypeWithNameAndElementsFromModule(M, Name, ElementTypes);

  if (ExistingStructType)
    return ExistingStructType;

  return StructType::create(C, ElementTypes, Name, false);
}

// Find any existing ident_t (LOC) struct in the module of F, and if found,
// return it. If not, create an ident_t struct and return it.
StructType *VPOParoptUtils::getIdentStructType(Function *F) {

  assert(F && "Null function pointer.");

  LLVMContext &C = F->getContext();
  unsigned AS = VPOAnalysisUtils::isTargetSPIRV(F->getParent())
                    ? vpo::ADDRESS_SPACE_GENERIC
                    : 0;
  Type *IdentTyArgs[] = {Type::getInt32Ty(C),        // reserved_1
                         Type::getInt32Ty(C),        // flags
                         Type::getInt32Ty(C),        // reserved_2
                         Type::getInt32Ty(C),        // reserved_3
                         Type::getInt8PtrTy(C, AS)}; // *psource

  return VPOParoptUtils::getOrCreateStructType(F, "struct.ident_t",
                                               IdentTyArgs);
}

// This function generates a runtime library call to __kmpc_begin(&loc, 0)
CallInst *VPOParoptUtils::genKmpcBeginCall(Function *F, Instruction *AI,
                                           StructType *IdentTy) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  Constant *KmpcLoc = genKmpcLocfromDebugLoc(IdentTy, Flags, &B, &E);

  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

  FunctionCallee FnC = M->getOrInsertFunction("__kmpc_begin",
                                              Type::getVoidTy(C),
                                              PointerType::getUnqual(IdentTy),
                                              Type::getInt32Ty(C));

  Function *FnKmpcBegin = cast<Function>(FnC.getCallee());

  std::vector<Value *> FnKmpcBeginArgs;
  FnKmpcBeginArgs.push_back(KmpcLoc);
  FnKmpcBeginArgs.push_back(ValueZero);

  CallInst *KmpcBeginCall =
      CallInst::Create(FnC.getFunctionType(), FnKmpcBegin, FnKmpcBeginArgs);
  setFuncCallingConv(KmpcBeginCall, M);

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

  Constant *KmpcLoc = genKmpcLocfromDebugLoc(IdentTy, Flags, &B, &E);

  FunctionCallee FnC = M->getOrInsertFunction("__kmpc_end", Type::getVoidTy(C),
                                              PointerType::getUnqual(IdentTy));

  Function *FnKmpcEnd = cast<Function>(FnC.getCallee());

  std::vector<Value *> FnKmpcEndArgs;
  FnKmpcEndArgs.push_back(KmpcLoc);

  CallInst *KmpcEndCall =
      CallInst::Create(FnC.getFunctionType(), FnKmpcEnd, FnKmpcEndArgs);
  setFuncCallingConv(KmpcEndCall, M);

  return KmpcEndCall;
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

// Returns Value of a clause argument, which may be passed either
// via a pointer or as a Constant.
// The resulting value is casted to the given type.
Value *VPOParoptUtils::getOrLoadClauseArgValueWithSext(
    Value *Arg, Type *Ty, IRBuilder<> &Builder) {
  if (!Arg)
    return nullptr;

  assert(Ty && Ty->isIntegerTy() && "Expected integer type.");

  if (Arg->getType()->isPointerTy())
    Arg = Builder.CreateLoad(Arg->getType()->getPointerElementType(), Arg);
  else
    assert(isa<Constant>(Arg) &&
           "The clause argument must be either pointer or Constant Value.");

  return Builder.CreateSExtOrTrunc(Arg, Ty);
}

/// This function generates a call to set num_teams and num_threads
/// (from thread_limit clause) for the teams region.
///
/// \code
/// call void @__kmpc_push_num_teams(ident_t *loc,
///                                  kmp_int32 global_tid,
///                                  kmp_int32 num_teams,
///                                  kmp_int32 num_threads)
/// \endcode
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

  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  // Assert that we used the right type for internally created
  // thread ID.
  assert(Tid->getType()->isIntegerTy(32) &&
         "Thread ID must be 4-byte integer.");

  // Cast num_teams() and thread_limit() values to 4-byte integer.
  // This has to be done by FE, but we can handle it here, if FE failed
  // to insert a bitcast.
  IRBuilder<> Builder(InsertPt);
  Type *Int32Ty = Type::getInt32Ty(C);

  if (NumTeams)
    NumTeams = getOrLoadClauseArgValueWithSext(NumTeams, Int32Ty, Builder);
  else
    NumTeams = ConstantInt::get(Int32Ty, 0);

  if (NumThreads)
    NumThreads = getOrLoadClauseArgValueWithSext(NumThreads, Int32Ty, Builder);
  else
    NumThreads = ConstantInt::get(Int32Ty, 0);

  SmallVector<Value *, 4> FnArgs {Loc, Tid, NumTeams, NumThreads};
  Type *RetTy = Type::getVoidTy(C);

  // Generate __kmpc_push_num_teams(loc, tid, num_teams, num_threads) in IR
  CallInst *PushNumTeams = genCall(M, FnName, RetTy, FnArgs);
  PushNumTeams->insertBefore(InsertPt);

  return PushNumTeams;
}

/// This function generates a call to set num_threads for the parallel
/// region and parallel loop/sections
//
/// \code
/// call void @__kmpc_push_num_threads(ident_t *loc,
///                                    kmp_int32 global_tid,
///                                    kmp_int32 num_threads)
/// \endcode
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

  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  // Assert that we used the right type for internally created
  // thread ID.
  assert(Tid->getType()->isIntegerTy(32) &&
         "Thread ID must be 4-byte integer.");

  // Cast num_threads() value to 4-byte integer.  This has to be done
  // by FE, but we can handle it here, if FE failed to insert a bitcast.
  IRBuilder<> Builder(InsertPt);
  NumThreads = Builder.CreateSExtOrTrunc(NumThreads, Type::getInt32Ty(C));

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
      Builder.CreateLoad(Builder.getInt32Ty(), TidPtr),
      ConstantPointerNull::get(Type::getInt8PtrTy(C)),
      Builder.CreateBitCast(SharedGep, Type::getInt8PtrTy(C))};

  Type *TypeParams[] = {Type::getInt32Ty(C), Type::getInt8PtrTy(C),
                        Type::getInt8PtrTy(C)};
  FunctionType *FnTy =
      FunctionType::get(Type::getInt8PtrTy(C), TypeParams, false);

  StringRef FnName = UseTbb ? "__tbb_omp_task_reduction_get_th_data" :
                              "__kmpc_task_reduction_get_th_data";

  Function *FnRedGetNthData = M->getFunction(FnName);

  if (!FnRedGetNthData)
    FnRedGetNthData =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *RedGetNthDataCall =
      CallInst::Create(FnTy, FnRedGetNthData, RedGetNthDataArgs, "", InsertPt);
  setFuncCallingConv(RedGetNthDataCall, M);
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
  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  Value *TaskArgs[] = {Loc, Builder.CreateLoad(Builder.getInt32Ty(), TidPtr)};
  Type *TypeParams[] = {Loc->getType(), Type::getInt32Ty(C)};
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), TypeParams, false);

  StringRef FnName = "__kmpc_omp_taskwait";
  Function *FnTaskWait = M->getFunction(FnName);

  if (!FnTaskWait)
    FnTaskWait =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *TaskWaitCall =
      CallInst::Create(FnTy, FnTaskWait, TaskArgs, "", InsertPt);
  setFuncCallingConv(TaskWaitCall, M);
  TaskWaitCall->setTailCall(false);

  return TaskWaitCall;
}

CallInst *VPOParoptUtils::genTgtTargetDataBegin(
    WRegionNode *W, int NumArgs, Value *ArgsBase, Value *Args, Value *ArgsSize,
    Value *ArgsMaptype, Value *ArgsNames, Value *ArgsMappers,
    Instruction *InsertPt) {
  assert((isa<WRNTargetDataNode>(W) || isa<WRNTargetEnterDataNode>(W) ||
                                        isa<WRNTargetVariantNode>(W)) &&
         "Expected a WRNTargetDataNode or WRNTargetEnterDataNode"
                                        "or WRNTargetVariantNode");
  Value *DeviceID = W->getDevice();
  CallInst *Call =
      genTgtCall("__tgt_target_data_begin", W, DeviceID, NumArgs, ArgsBase,
                 Args, ArgsSize, ArgsMaptype, ArgsNames, ArgsMappers, InsertPt);
  return Call;
}

CallInst *VPOParoptUtils::genTgtTargetDataEnd(
    WRegionNode *W, int NumArgs, Value *ArgsBase, Value *Args, Value *ArgsSize,
    Value *ArgsMaptype, Value *ArgsNames, Value *ArgsMappers,
    Instruction *InsertPt) {
  assert((isa<WRNTargetDataNode>(W) || isa<WRNTargetExitDataNode>(W) ||
          isa<WRNTargetVariantNode>(W)) &&
         "Expected a WRNTargetDataNode or WRNTargetExitDataNode"
                                       "or WRNTargetVariantNode");

  Value *DeviceID = W->getDevice();
  CallInst *Call =
      genTgtCall("__tgt_target_data_end", W, DeviceID, NumArgs, ArgsBase, Args,
                 ArgsSize, ArgsMaptype, ArgsNames, ArgsMappers, InsertPt);
  return Call;
}

CallInst *VPOParoptUtils::genTgtTargetDataUpdate(
    WRegionNode *W, int NumArgs, Value *ArgsBase, Value *Args, Value *ArgsSize,
    Value *ArgsMaptype, Value *ArgsNames, Value *ArgsMappers,
    Instruction *InsertPt) {
  assert(isa<WRNTargetUpdateNode>(W) && "Expected a WRNTargetUpdateNode");
  Value *DeviceID = W->getDevice();
  CallInst *Call =
      genTgtCall("__tgt_target_data_update", W, DeviceID, NumArgs, ArgsBase,
                 Args, ArgsSize, ArgsMaptype, ArgsNames, ArgsMappers, InsertPt);
  return Call;
}

CallInst *VPOParoptUtils::genTgtTarget(WRegionNode *W, Value *HostAddr,
                                       int NumArgs, Value *ArgsBase,
                                       Value *Args, Value *ArgsSize,
                                       Value *ArgsMaptype, Value *ArgsNames,
                                       Value *ArgsMappers,
                                       Instruction *InsertPt) {
  assert(isa<WRNTargetNode>(W) && "Expected a WRNTargetNode");
  Value *DeviceID = W->getDevice();
  CallInst *Call =
      genTgtCall("__tgt_target", W, DeviceID, NumArgs, ArgsBase, Args, ArgsSize,
                 ArgsMaptype, ArgsNames, ArgsMappers, InsertPt, HostAddr);
  return Call;
}

CallInst *
VPOParoptUtils::genTgtTargetTeams(WRegionNode *W, Value *HostAddr, int NumArgs,
                                  Value *ArgsBase, Value *Args, Value *ArgsSize,
                                  Value *ArgsMaptype, Value *ArgsNames,
                                  Value *ArgsMappers, Instruction *InsertPt) {
  // This call supports the target teams construct.
  // Its WRN representation is a WRNTeamsNode enclosed in a WRNTargetNode.

  assert(isa<WRNTeamsNode>(W) && "Expected a WRNTeamsNode");

  WRegionNode *WTarget = W->getParent();
  assert(isa<WRNTargetNode>(WTarget) && "Expected parent to be WRNTargetNode");

  Value *DeviceID            = WTarget->getDevice();
  SubdeviceClause& Subdevice = WTarget->getSubdevice();
  SubdeviceItem* SubdeviceI  = nullptr;
  if (Subdevice.size() != 0)
    SubdeviceI  = Subdevice.front();

  Value *NumTeamsPtr = W->getNumTeams();

  assert((!useSPMDMode(W) || !NumTeamsPtr) &&
         "SPMD mode cannot be used with num_teams.");

  Value *ThreadLimitPtr = W->getThreadLimit();
  CallInst *Call =
      genTgtCall("__tgt_target_teams", W, DeviceID, NumArgs, ArgsBase, Args,
                 ArgsSize, ArgsMaptype, ArgsNames, ArgsMappers, InsertPt,
                 HostAddr, NumTeamsPtr, ThreadLimitPtr, SubdeviceI);
  return Call;
}

void VPOParoptUtils::encodeSubdeviceConstants(const ConstantInt *Param,
                                             uint64_t &ConstantDeviceID,
                                             int Shift, uint64_t MaskSize) {
  uint64_t Mask = (~(~(0ull) << MaskSize));
  uint64_t UIParam = Param->getZExtValue();
  LLVM_DEBUG(dbgs() << "before shift: " << llvm::format_hex(UIParam, 18, true)
                    << ", ");
  UIParam &= Mask;
  UIParam <<= Shift;
  LLVM_DEBUG(dbgs() << "after shift: " << llvm::format_hex(UIParam, 18, true)
                    << "\n");
  ConstantDeviceID |= UIParam;
}

Value *VPOParoptUtils::genEncodingSubdeviceNonConstants(Instruction *InsertPt,
                                                        Value *Param,
                                                        int Shift,
                                                        uint64_t MaskSize) {
  IRBuilder<> Builder(InsertPt);
  Type *Int64Ty = Builder.getInt64Ty();

  LLVM_DEBUG(dbgs() << "is not constant.\n");
  uint64_t Mask = (~(~(0ull) << MaskSize));
  Param = Builder.CreateZExt(Param, Int64Ty);
  Param = Builder.CreateAnd(Param, Mask);
  Param = Builder.CreateShl(Param, Shift);
  return Param;
}

Value *VPOParoptUtils::encodeSubdevice(WRegionNode *W, Instruction *InsertPt,
                                       Value *DeviceID,
                                       SubdeviceItem *SubdeviceI) {
  IRBuilder<> Builder(InsertPt);
  Type *Int64Ty = Builder.getInt64Ty();

  DeviceID = Builder.CreateZExtOrBitCast(DeviceID, Int64Ty);

  if (SubdeviceI == nullptr) {
    if (isa<WRNTeamsNode>(W))
      return DeviceID;
    assert(W->canHaveSubdevice() &&
           "W must support subdevice, since it supports Device");
    SubdeviceClause &Subdevice = W->getSubdevice();
    if (Subdevice.empty())
      return DeviceID;
    assert(Subdevice.size() == 1 && "There should be only 1 Subdevice clause");
    SubdeviceI = Subdevice.front();
  }

  const ConstantInt *CIDevice = dyn_cast<ConstantInt>(DeviceID);
  Value *Stride = SubdeviceI->getStride();
  const ConstantInt *CIStride = dyn_cast<ConstantInt>(Stride);
  Value *Length = SubdeviceI->getLength();
  const ConstantInt *CILength = dyn_cast<ConstantInt>(Length);
  Value *Start = SubdeviceI->getStart();
  const ConstantInt *CIStart = dyn_cast<ConstantInt>(Start);
  Value *Level = SubdeviceI->getLevel();
  const ConstantInt *CILevel = dyn_cast<ConstantInt>(Level);

  assert(CILevel && "Subdevice Level must be Constant");

  // MSB is set to 1 if Subdevice clause exists
  uint64_t ConstantDeviceID = 1ull << 63;

  // Device Num uses the least significant 32 bits of DeviceID
  LLVM_DEBUG(dbgs() << "Subdevice encoding : Device ");
  if (CIDevice)
    encodeSubdeviceConstants(CIDevice, ConstantDeviceID, 0, 32);
  else
    DeviceID = genEncodingSubdeviceNonConstants(InsertPt, DeviceID, 0, 32);

  // Level is 2 bits and is encoded between bits 57..56 of DeviceID
  LLVM_DEBUG(dbgs() << "Subdevice encoding : Level  ");
  encodeSubdeviceConstants(CILevel, ConstantDeviceID, 56, 2);

  // Start is 8 bits and is encoded between bits 55..48 of DeviceID
  LLVM_DEBUG(dbgs() << "Subdevice encoding : Start  ");
  if (CIStart)
    encodeSubdeviceConstants(CIStart, ConstantDeviceID, 48, 8);
  else
    Start = genEncodingSubdeviceNonConstants(InsertPt, Start, 48, 8);

  // Length is 8 bits and is encoded between bits 47..40 of DeviceID
  LLVM_DEBUG(dbgs() << "Subdevice encoding : Length ");
  if (CILength)
    encodeSubdeviceConstants(CILength, ConstantDeviceID, 40, 8);
  else
    Length = genEncodingSubdeviceNonConstants(InsertPt, Length, 40, 8);

  // Stride is 8 bits and is encoded between bits 39..32 of DeviceID
  LLVM_DEBUG(dbgs() << "Subdevice encoding : Stride ");
  if (CIStride)
    encodeSubdeviceConstants(CIStride, ConstantDeviceID, 32, 8);
  else
    Stride = genEncodingSubdeviceNonConstants(InsertPt, Stride, 32, 8);

  Value* Encoding = ConstantInt::get(Int64Ty, ConstantDeviceID);
  if (CIDevice && CIStride && CILength && CIStart) {
    // All fields are constants; no runtime IR needed
    LLVM_DEBUG(dbgs() << "DeviceID after Subdevice encoding: "
                      << llvm::format_hex(ConstantDeviceID, 18, true) << "\n");
    return Encoding;
  }

  // Emit IR to OR in the nonconstant fields
  if (!CIDevice)
    Encoding = Builder.CreateOr(DeviceID, Encoding);
  if (!CIStride)
    Encoding = Builder.CreateOr(Stride, Encoding);
  if (!CILength)
    Encoding = Builder.CreateOr(Length, Encoding);
  if (!CIStart)
    Encoding = Builder.CreateOr(Start, Encoding);
  return Encoding;
}

CallInst *VPOParoptUtils::genTgtCall(StringRef FnName, WRegionNode *W,
                                     Value *DeviceID, int NumArgsCount,
                                     Value *ArgsBase, Value *Args,
                                     Value *ArgsSize, Value *ArgsMaptype,
                                     Value *ArgsNames, Value *ArgsMappers,
                                     Instruction *InsertPt, Value *HostAddr,
                                     Value *NumTeamsPtr, Value *ThreadLimitPtr,
                                     SubdeviceItem *SubdeviceI) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);
  Type *ReturnTy; // void for _tgt_target_data_*(); i32 otherwise
  Type *Int8PtrTy = Type::getInt8PtrTy(C);

  Value *NumTeams = nullptr;
  Value *ThreadLimit = nullptr;

  // First parm: "int64_t device_id"
  if (DeviceID == nullptr)
    DeviceID = Builder.CreateZExt(genOmpGetDefaultDevice(InsertPt), Int64Ty);

  DeviceID = encodeSubdevice(W, InsertPt, DeviceID, SubdeviceI);

  SmallVector<Value *, 12> FnArgs;
  SmallVector<Type *, 12> FnArgTypes;

  if (UseMapperAPI) {
    StructType *IdentTy = VPOParoptUtils::getIdentStructType(F);
    BasicBlock *EntryBB = W->getEntryBBlock();
    BasicBlock *ExitBB = W->getExitBBlock();
    int Flags = KMP_IDENT_KMPC;
    Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, EntryBB, ExitBB);
    FnArgs.push_back(Loc);
    FnArgTypes.push_back(Loc->getType());
  }

  FnArgs.push_back(DeviceID);
  FnArgTypes.push_back(Int64Ty);

  if (HostAddr) {
    // HostAddr!=null means FnName is __tgt_target or __tgt_target_teams
    ReturnTy = Int32Ty;

    // Handle the "void *host_addr" parm of __tgt_target and __tgt_target_teams
    Value *BitCast = Builder.CreateBitCast(HostAddr, Int8PtrTy);
    FnArgs.push_back(BitCast);
    FnArgTypes.push_back(BitCast->getType());
    if (FnName == "__tgt_target_teams") {
      // __tgt_target_teams has two more parms: "int32_t num_teams" and
      // "int32_t thread_limit".  Initialize them here.
      if (NumTeamsPtr == nullptr)
        NumTeams = Builder.getInt32(0);
      else
        NumTeams =
            getOrLoadClauseArgValueWithSext(NumTeamsPtr, Int32Ty, Builder);

      if (ThreadLimitPtr == nullptr)
        ThreadLimit = Builder.getInt32(0);
      else
        ThreadLimit =
            getOrLoadClauseArgValueWithSext(ThreadLimitPtr, Int32Ty, Builder);
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

  if (UseMapperAPI) {
    FnArgs.push_back(ArgsNames);
    FnArgTypes.push_back(ArgsNames->getType());
    FnArgs.push_back(ArgsMappers);
    FnArgTypes.push_back(ArgsMappers->getType());
  }

  // Add the two parms for __tgt_target_teams
  if (NumTeams != nullptr) {
    FnArgs.push_back(NumTeams);
    FnArgTypes.push_back(Int32Ty);

    FnArgs.push_back(ThreadLimit);
    FnArgTypes.push_back(Int32Ty);
  }

  // LLVM_DEBUG(dbgs() << "FnArgs.size= "<< FnArgs.size());
  // LLVM_DEBUG(dbgs() << "FnArgTypes.size() = "<< FnArgTypes.size());
  auto Name = UseMapperAPI ? (FnName + "_mapper").str() : FnName.lower();
  CallInst *Call = genCall(Name, ReturnTy, FnArgs, FnArgTypes, InsertPt);
  // TODO: Disable this extra call for code-location once ident_t sent
  // via the mapper APIs provides enough information, and is stable.
  if(PushCodeLocation) {
    CallInst *CallPushCodeLocation = genTgtPushCodeLocation(InsertPt, Call);
    LLVM_DEBUG(dbgs() << "\nGenerating: " << *CallPushCodeLocation << "\n");
    (void)CallPushCodeLocation;
  }
  return Call;
}

VPOParoptUtils::SrcLocMode getSrcLocMode() {
  switch (ParallelSourceInfoMode) {
  case 0:
    return VPOParoptUtils::SRC_LOC_NONE;
  case 1:
    return VPOParoptUtils::SRC_LOC_FUNC;
  case 2:
    return VPOParoptUtils::SRC_LOC_PATH;
  default:
    return VPOParoptUtils::SRC_LOC_FUNC;
  }
}

/// Generate tgt_push_code_location call which pushes source code location
/// and the pointer to the tgt_target*() function.
/// Generated function looks as follows:
/// void __tgt_push_code_location (void *Location,
///                                 void *FunctionPtr)
CallInst *VPOParoptUtils::genTgtPushCodeLocation(Instruction *Location,
                                                 CallInst *TgtTargetCall) {
  IRBuilder<> Builder(Location);
  BasicBlock *B = Location->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Type *Int8PtrTy = Type::getInt8PtrTy(C);

  DILocation *Loc1 = Location->getDebugLoc();
  DILocation *Loc2 = nullptr;
  SrcLocMode Mode = getSrcLocMode();

  GlobalVariable *LocStr = genLocStrfromDebugLoc(F, Loc1, Loc2, Mode);

  //  Create the following function call
  // call void @__tgt_push_code_location(i8* getelementptr inbounds ([18 x i8],
  // [18 x i8]* @.source.3.4, i32 0, i32 0), i8* bitcast (i32 (i64, i8*, i32,
  // i8**, i8**, i64*, i64*)* @__tgt_target to i8*))
  Type *ReturnTy = Type::getVoidTy(C);
  SmallVector<Value *, 2> Args;
  SmallVector<Type *, 2> ArgTypes;
  Function *Fn = TgtTargetCall->getCalledFunction();
  Value *Cast = Builder.CreateBitCast(Fn, Int8PtrTy);
  Value *LocStrCast = Builder.CreateBitCast(LocStr, Int8PtrTy);
  Args.push_back(LocStrCast);
  ArgTypes.push_back(LocStrCast->getType());
  Args.push_back(Cast);
  ArgTypes.push_back(Int8PtrTy);
  CallInst *Call = genCall("__tgt_push_code_location", ReturnTy, Args, ArgTypes,
                           TgtTargetCall);
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
                                            Type *RetType,
                                            ArrayRef<Value *> FnArgs,
                                            Instruction *InsertPt) {
  BasicBlock *B  = InsertPt->getParent();
  Function *F    = B->getParent();
  LLVMContext &C = F->getContext();

  SmallVector<Type *, 1> ArgType = { Type::getInt32Ty(C) };

  if (FnArgs.empty())
    ArgType.clear();

  CallInst *Call =
      genCall(FnName, RetType, FnArgs, ArgType, InsertPt);
  setFuncCallingConv(Call, Call->getModule());
  return Call;
}

// Set SPIR_FUNC calling convention for SPIR-V targets, otherwise,
// set C calling convention.
void VPOParoptUtils::setFuncCallingConv(CallInst *CI, Module *M) {
  CallingConv::ID Conv = CallingConv::C;

  if (VPOAnalysisUtils::isTargetSPIRV(M))
    Conv = CallingConv::SPIR_FUNC;

  CI->setCallingConv(Conv);
  if (auto *CF = CI->getCalledFunction())
    CF->setCallingConv(Conv);
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

// Generate a call to
//   int32_t __tgt_is_device_available(int64_t device_num, void *device_type)
CallInst *VPOParoptUtils::genTgtIsDeviceAvailable(Value *DeviceNum,
                                                  Value *DeviceType,
                                                  Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);
  Type *Int8PtrTy = Type::getInt8PtrTy(C);

  assert(DeviceType && DeviceType->getType()->isPointerTy() &&
         "DeviceType expected to be pointer");
  DeviceNum = IRBuilder<>(InsertPt).CreateSExt(DeviceNum, Int64Ty);
  Value *Args[] = {DeviceNum, DeviceType};
  Type *ArgTypes[] = {Int64Ty, Int8PtrTy};
  CallInst *Call =
      genCall("__tgt_is_device_available", Int32Ty, Args, ArgTypes, InsertPt);
  return Call;
}

// Generate a call to
//   void *__tgt_create_buffer(int64_t device_num, void *host_ptr)
CallInst *VPOParoptUtils::genTgtCreateBuffer(Value *DeviceNum, Value *HostPtr,
                                             Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Type *Int64Ty = Type::getInt64Ty(C);
  Type *Int8PtrTy = Type::getInt8PtrTy(C);

  assert(HostPtr && HostPtr->getType() == Int8PtrTy &&
         "HostPtr expected to be void*");
  DeviceNum = IRBuilder<>(InsertPt).CreateSExt(DeviceNum, Int64Ty);
  Value *Args[] = {DeviceNum, HostPtr};
  Type *ArgTypes[] = {Int64Ty, Int8PtrTy};
  CallInst *Call =
      genCall("__tgt_create_buffer", Int8PtrTy, Args, ArgTypes, InsertPt);
  return Call;
}

// Generate a call to
//   int __tgt_release_buffer(int64_t device_num, void *tgt_buffer)
CallInst *VPOParoptUtils::genTgtReleaseBuffer(Value *DeviceNum,
                                              Value *TgtBuffer,
                                              Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);
  Type *Int8PtrTy = Type::getInt8PtrTy(C);

  assert(TgtBuffer && TgtBuffer->getType() == Int8PtrTy &&
         "TgtBuffer expected to be void*");
  DeviceNum = IRBuilder<>(InsertPt).CreateSExt(DeviceNum, Int64Ty);
  Value *Args[] = {DeviceNum, TgtBuffer};
  Type *ArgTypes[] = {Int64Ty, Int8PtrTy};
  CallInst *Call =
      genCall("__tgt_release_buffer", Int32Ty, Args, ArgTypes, InsertPt);
  return Call;
}

// Generate a call to
//   void *__tgt_create_interop_obj(int64_t device_id,
//                                  bool    is_async,
//                                  void   *async_obj)
CallInst *VPOParoptUtils::genTgtCreateInteropObj(Value *DeviceNum, bool IsAsync,
                                                 Value *AsyncObj,
                                                 Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Type *Int8Ty = Type::getInt8Ty(C);
  Type *Int64Ty = Type::getInt64Ty(C);
  Type *Int8PtrTy = Type::getInt8PtrTy(C);

  assert((IsAsync && AsyncObj || !IsAsync && !AsyncObj) &&
         "AsyncObj must be null iff IsAsync is false");
  assert((AsyncObj == nullptr || AsyncObj->getType() == Int8PtrTy) &&
         "AsyncObj expected to be void*");

  DeviceNum = IRBuilder<>(InsertPt).CreateSExt(DeviceNum, Int64Ty);

  Value *IsAsyncVal = ConstantInt::get(Int8Ty, IsAsync ? 1 : 0);

  if (AsyncObj == nullptr)    // create a void* nullptr argument
    AsyncObj = Constant::getNullValue(Int8PtrTy);

  Value *Args[] = {DeviceNum, IsAsyncVal, AsyncObj};
  Type *ArgTypes[] = {Int64Ty, Int8Ty, Int8PtrTy};
  CallInst *Call =
      genCall("__tgt_create_interop_obj", Int8PtrTy, Args, ArgTypes, InsertPt);

  if (IsAsync)
    Call->setName("interop.obj.async");
  else
    Call->setName("interop.obj.sync");

  return Call;
}

// Generate a call to
//   int __tgt_release_interop_obj(void *interop_obj)
CallInst *VPOParoptUtils::genTgtReleaseInteropObj(Value *InteropObj,
                                                  Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();
  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int8PtrTy = Type::getInt8PtrTy(C);

  assert(InteropObj && InteropObj->getType() == Int8PtrTy &&
         "InteropObj expected to be void*");

  CallInst *Call = genCall("__tgt_release_interop_obj", Int32Ty, {InteropObj},
                           {Int8PtrTy}, InsertPt);
  return Call;
}

// Generate a call to
//   int omp_get_num_devices()
CallInst *VPOParoptUtils::genOmpGetNumDevices(Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  Type *Int32Ty = Type::getInt32Ty(C); // return type

  CallInst *Call = genEmptyCall(M, "omp_get_num_devices", Int32Ty, InsertPt);
  return Call;
}

// Generate a call to
//   int omp_get_default_device()
CallInst* VPOParoptUtils::genOmpGetDefaultDevice(Instruction* InsertPt) {
    BasicBlock* B = InsertPt->getParent();
    Function* F = B->getParent();
    Module* M = F->getParent();
    LLVMContext& C = F->getContext();

    Type* Int32Ty = Type::getInt32Ty(C); // return type

    CallInst* Call = genEmptyCall(M, "omp_get_default_device", Int32Ty, InsertPt);
    return Call;
}

// Generate a call to
//   omp_allocator_handle_t omp_get_default_allocator()
CallInst *VPOParoptUtils::genOmpGetDefaultAllocator(Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();

  // return type omp_allocator_handle_t is IntPtr
  Type *IntPtrTy = GeneralUtils::getSizeTTy(F);

  CallInst *Call =
      genEmptyCall(M, "omp_get_default_allocator", IntPtrTy, InsertPt);
  Call->setName("default_allocator");
  return Call;
}

// Generate a call to
//   void* omp_alloc(size_t Size, omp_allocator_handle_t Handle)
// where omp_allocator_handle_t is uintptr_t, so the call is one of these:
//   %call = call i8* @omp_alloc(i64 Size, i64 Handle) // in P64 arch
//   %call = call i8* @omp_alloc(i32 Size, i32 Handle) // in P32 arch
//
// If the input 'Handle' is null, then use omp_get_default_allocator()
// as the second parameter of omp_alloc().
CallInst *VPOParoptUtils::genOmpAlloc(Value *Size, Value *Handle,
                                      Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();

  // size_t and omp_allocator_handle_t are both IntPtr
  Type *IntPtrTy = GeneralUtils::getSizeTTy(F);
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();

  auto *SizeCast = Builder.CreateZExtOrBitCast(Size, IntPtrTy);
  if (Handle)
    Handle = Builder.CreateZExtOrBitCast(Handle, IntPtrTy);
  else
    Handle = genOmpGetDefaultAllocator(InsertPt);
  CallInst *Call = genCall("omp_alloc", Int8PtrTy, {SizeCast, Handle},
                           {IntPtrTy, IntPtrTy}, InsertPt);
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
  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  std::vector<Value *> TaskArgs;
  TaskArgs.push_back(Loc);
  TaskArgs.push_back(Builder.CreateLoad(Builder.getInt32Ty(), TidPtr));
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

  if (!FnTask)
    FnTask = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *TaskCall = CallInst::Create(FnTy, FnTask, TaskArgs, "", InsertPt);
  setFuncCallingConv(TaskCall, M);
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
  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  Value *TaskArgs[] = {
      Loc, Builder.CreateLoad(Builder.getInt32Ty(), TidPtr), TaskAlloc};
  Type *TypeParams[] = {Loc->getType(), Type::getInt32Ty(C),
                        Type::getInt8PtrTy(C)};
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), TypeParams, false);

  Function *FnTask = M->getFunction(FnName);

  if (!FnTask)
    FnTask = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *TaskCall = CallInst::Create(FnTy, FnTask, TaskArgs, "", InsertPt);
  setFuncCallingConv(TaskCall, M);
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
                                          Value *Cmp, AllocaInst *LBPtr,
                                          AllocaInst *UBPtr, AllocaInst *STPtr,
                                          StructType *KmpTaskTTWithPrivatesTy,
                                          Instruction *InsertPt, bool UseTbb,
                                          Function *FnTaskDup) {
  IRBuilder<> Builder(InsertPt);
  Value *Zero = Builder.getInt32(0);
  Type *Int64Ty = Builder.getInt64Ty();
  Type *Int32Ty = Builder.getInt32Ty();
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  PointerType *Int64PtrTy = PointerType::getUnqual(Int64Ty);

  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  int Flags = KMP_IDENT_KMPC;
  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  Value *Cast = Builder.CreateBitCast(
      TaskAlloc, PointerType::getUnqual(KmpTaskTTWithPrivatesTy),
      ".taskt.with.privates");

  Value *TaskTTyGep = Builder.CreateInBoundsGEP(KmpTaskTTWithPrivatesTy, Cast,
                                                {Zero, Zero}, ".taskt");

  assert(isa<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0)) &&
         "TaskT is not Struct Type.");
  StructType *KmpTaskTTy =
      cast<StructType>(KmpTaskTTWithPrivatesTy->getElementType(0));

  Value *LBGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, TaskTTyGep, {Zero, Builder.getInt32(5)}, ".lb.gep");
  Value *LBVal = Builder.CreateLoad(LBPtr->getAllocatedType(), LBPtr, ".lb");
  if (LBVal->getType() != KmpTaskTTy->getElementType(5))
    LBVal = Builder.CreateSExtOrTrunc(LBVal, KmpTaskTTy->getElementType(5),
                                      ".lb.cast");

  Builder.CreateStore(LBVal, LBGep);

  Value *UBGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, TaskTTyGep, {Zero, Builder.getInt32(6)}, ".ub.gep");
  Value *UBVal = Builder.CreateLoad(UBPtr->getAllocatedType(), UBPtr, ".ub");
  if (UBVal->getType() != KmpTaskTTy->getElementType(6))
    UBVal = Builder.CreateSExtOrTrunc(UBVal, KmpTaskTTy->getElementType(6),
                                      ".ub.cast");
  Builder.CreateStore(UBVal, UBGep);

  Value *STGep = Builder.CreateInBoundsGEP(
      KmpTaskTTy, TaskTTyGep, {Zero, Builder.getInt32(7)}, ".stride.gep");
  Value *STVal =
      Builder.CreateLoad(STPtr->getAllocatedType(), STPtr, ".stride");
  if (STVal->getType() != KmpTaskTTy->getElementType(7))
    STVal = Builder.CreateSExtOrTrunc(STVal, KmpTaskTTy->getElementType(7),
                                      ".stride.cast");
  Builder.CreateStore(STVal, STGep);

  Value *GrainSizeV = nullptr;
  switch (W->getSchedCode()) {
  case 0:
    GrainSizeV = Builder.getInt64(0);
    break;
  case 1:
    GrainSizeV = Builder.CreateSExtOrTrunc(W->getGrainsize(), Int64Ty);
    break;
  case 2:
    GrainSizeV = Builder.CreateSExtOrTrunc(W->getNumTasks(), Int64Ty);
    break;
  default:
    llvm_unreachable("genKmpcTaskLoop: unexpected SchedCode");
  }

  Value *TaskLoopArgs[] = {
      Loc,
      Builder.CreateLoad(Builder.getInt32Ty(), TidPtr),
      TaskAlloc,
      Cmp == nullptr ? Builder.getInt32(1)
                     : Builder.CreateSExtOrTrunc(Cmp, Int32Ty),
      LBGep,
      UBGep,
      STVal,
      Zero,
      Builder.getInt32(W->getSchedCode()),
      GrainSizeV,
      (FnTaskDup == nullptr) ? ConstantPointerNull::get(Int8PtrTy)
                             : Builder.CreateBitCast(FnTaskDup, Int8PtrTy)};
  Type *TypeParams[] = {Loc->getType(), Int32Ty,    Int8PtrTy, Int32Ty,
                        Int64PtrTy,     Int64PtrTy, Int64Ty,   Int32Ty,
                        Int32Ty,        Int64Ty,    Int8PtrTy};
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), TypeParams, false);

  StringRef FnName = UseTbb ? "__tbb_omp_taskloop" : "__kmpc_taskloop";
  Function *FnTaskLoop = M->getFunction(FnName);

  if (!FnTaskLoop)
    FnTaskLoop =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *TaskLoopCall =
      CallInst::Create(FnTy, FnTaskLoop, TaskLoopArgs, "", InsertPt);
  setFuncCallingConv(TaskLoopCall, M);
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
      Builder.CreateLoad(Builder.getInt32Ty(), TidPtr),
      Builder.getInt32(ParmNum),
      Builder.CreatePointerCast(RedRecord, Builder.getInt8PtrTy())};
  Type *TypeParams[] = {Type::getInt32Ty(C), Type::getInt32Ty(C),
                        Type::getInt8PtrTy(C)};
  FunctionType *FnTy =
      FunctionType::get(Type::getInt8PtrTy(C), TypeParams, false);

  StringRef FnName = UseTbb ? "__tbb_omp_task_reduction_init" :
                              "__kmpc_task_reduction_init";

  Function *FnTaskRedInit = M->getFunction(FnName);

  if (!FnTaskRedInit)
    FnTaskRedInit =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *TaskRedInitCall = CallInst::Create(
      FnTy, FnTaskRedInit, TaskRedInitArgs, "task.reduction.init", InsertPt);
  setFuncCallingConv(TaskRedInitCall, M);
  TaskRedInitCall->setTailCall(false);

  return TaskRedInitCall;
}

// Auxiliary routine to generate call to @__kmpc_omp_task_alloc
CallInst *genKmpcTaskAllocImpl(WRegionNode *W, StructType *IdentTy, Value *Tid,
                               Value *TaskFlags,
                               Value *KmpTaskTTWithPrivatesTySz,
                               int KmpSharedTySz, Value *TaskEntryPtr,
                               Instruction *InsertPt, bool UseTbb) {

  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  int Flags = KMP_IDENT_KMPC;
  Constant *Loc = VPOParoptUtils::genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  IRBuilder<> Builder(InsertPt);
  Type *SizeTTy = GeneralUtils::getSizeTTy(F);
  Type *Int32Ty = Builder.getInt32Ty();

  auto *KmpTaskTWithPrivatesTySize =
      Builder.CreateZExtOrTrunc(KmpTaskTTWithPrivatesTySz, SizeTTy);
  auto *SharedsSize = ConstantInt::get(SizeTTy, KmpSharedTySz);

  Value *AllocArgs[] = {Loc,            Tid,
                        TaskFlags,      KmpTaskTWithPrivatesTySize,
                        SharedsSize,    TaskEntryPtr};

  Type *TypeParams[] = {Loc->getType(), Int32Ty,
                        Int32Ty,        SizeTTy,
                        SizeTTy,        TaskEntryPtr->getType()};

  FunctionType *FnTy =
      FunctionType::get(Type::getInt8PtrTy(C), TypeParams, false);

  StringRef FnName = UseTbb? "__tbb_omp_task_alloc" : "__kmpc_omp_task_alloc";
  Function *FnTaskAlloc = M->getFunction(FnName);

  if (!FnTaskAlloc)
    FnTaskAlloc =
        Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *TaskAllocCall =
      CallInst::Create(FnTy, FnTaskAlloc, AllocArgs, "", InsertPt);
  VPOParoptUtils::setFuncCallingConv(TaskAllocCall, M);
  TaskAllocCall->setTailCall(false);

  return TaskAllocCall;
}

// build the CFG for if clause.
void VPOParoptUtils::buildCFGForIfClause(Value *Cmp, Instruction *&ThenTerm,
                                         Instruction *&ElseTerm,
                                         Instruction *InsertPt,
                                         DominatorTree *DT) {
  BasicBlock *SplitBeforeBB = InsertPt->getParent();
  SplitBlockAndInsertIfThenElse(Cmp, InsertPt, &ThenTerm, &ElseTerm);
  ThenTerm->getParent()->setName("if.then");
  ElseTerm->getParent()->setName("if.else");
  InsertPt->getParent()->setName("if.end");

  DT->addNewBlock(ThenTerm->getParent(), SplitBeforeBB);
  DT->addNewBlock(ElseTerm->getParent(), SplitBeforeBB);
  DT->addNewBlock(InsertPt->getParent(), SplitBeforeBB);

  DT->changeImmediateDominator(ThenTerm->getParent(), SplitBeforeBB);
  DT->changeImmediateDominator(ElseTerm->getParent(), SplitBeforeBB);
  BasicBlock *NextBB = InsertPt->getParent()->getSingleSuccessor();
  assert(NextBB && "Null Next BB.");
  if (NextBB->getUniquePredecessor())
    DT->changeImmediateDominator(NextBB, InsertPt->getParent());
}

// This function generates a call as follows.
//    i8* @__kmpc_omp_task_alloc({ i32, i32, i32, i32, i8* }*, i32, i32,
//    size_t, size_t, i32 (i32, i8*)*)
CallInst *VPOParoptUtils::genKmpcTaskAlloc(WRegionNode *W, StructType *IdentTy,
                                           Value *TidPtr, DominatorTree *DT,
                                           Value *KmpTaskTTWithPrivatesTySz,
                                           int KmpSharedTySz,
                                           PointerType *KmpRoutineEntryPtrTy,
                                           Function *MicroTaskFn,
                                           Instruction *InsertPt, bool UseTbb) {
  IRBuilder<> Builder(InsertPt);
  Type *Int32Ty = Builder.getInt32Ty();

  Value *Tid = Builder.CreateLoad(Int32Ty, TidPtr);
  Value *TaskFlags = ConstantInt::get(Int32Ty, W->getTaskFlag());

  Value *VFinal = W->getFinal();
  if (VFinal) {
    // If task construct has final clause, the compiler will set related flag
    // bit in 3rd argument of __kmpc_omp_task_alloc (for final clause with
    // non-zero constant), or generate a if-then-else statement (for final
    // clause with conditional expression), to set the flag bit if the
    // expression is evaluated to true.
    //
    // Example:
    //   #pragma omp task final(n <= 10)
    // IR:
    //   ...
    //   %n.addr = alloca i32, align 4
    //   store i32 %n, i32* %n.addr, align 4
    //   %1 = load i32, i32* %n.addr, align 4
    //   %cmp = icmp sle i32 %1, 10
    //   %conv = zext i1 %cmp to i32
    //   br lablel %codeRepl
    //
    // codeRepl:
    //   %3 = alloca i32, align 4                                     ; (1)
    //   store i32 1, i32* %3, align 4                                ; (2)
    //   %4 = icmp ne i32 %conv, 0                                    ; (3)
    //   br i1 %4, label %if.then, label %if.else                     ; (4)
    //
    // if.then:                                          ; preds = %codeRepl
    //   store i32 3, i32* %3, align 4                                ; (5)
    //   br label %if.end
    //
    // if.else:                                          ; preds = %codeRepl
    //   br label %if.end
    //
    // if.end:                                           ; preds = %if.else,
    // %if.then
    //   %5 = load i32, i32* %3, align 4                              ; (6)
    //   %.task.alloc = call i8* @__kmpc_omp_task_alloc(%struct.ident_t*
    //     @.kmpc_loc.0.0.7, i32 %2, i32 %5, i64 80, i64 0, i32 (i32, i8*)*
    //     bitcast (void (i32, i32, %__struct.kmp_task_t_with_privates*)*
    //     @foo.DIR.OMP.TASK.2.split to i32 (i32, i8*)*))
    //   ...
    //
    const int FinalFlagBit = 0x2;

    if (Constant *CFinal = dyn_cast<Constant>(VFinal)) {
      if (!CFinal->isZeroValue()) {
        W->setTaskFlag(W->getTaskFlag() | FinalFlagBit);
        TaskFlags = ConstantInt::get(Int32Ty, W->getTaskFlag());
      }
    } else {
      auto TaskFlagsAlloca = Builder.CreateAlloca(Int32Ty); // (1)
      Builder.CreateStore(TaskFlags, TaskFlagsAlloca);      // (2)
      Value *Cmp = Builder.CreateICmpNE(
          VFinal, ConstantInt::get(VFinal->getType(), 0)); // (3)

      Instruction *ThenTerm, *ElseTerm;
      buildCFGForIfClause(Cmp, ThenTerm, ElseTerm, InsertPt, DT); // (4)
      Builder.SetInsertPoint(ThenTerm);
      W->setTaskFlag(W->getTaskFlag() | FinalFlagBit);
      Builder.CreateStore(Builder.getInt32(W->getTaskFlag()),
                          TaskFlagsAlloca); // (5)

      Builder.SetInsertPoint(InsertPt);
      TaskFlags = Builder.CreateLoad(Int32Ty, TaskFlagsAlloca); // (6)
    }
  }

  Value *TaskEntry = Builder.CreateBitCast(MicroTaskFn, KmpRoutineEntryPtrTy);

  CallInst *TaskAllocCall = genKmpcTaskAllocImpl(
      W, IdentTy, Tid, TaskFlags, KmpTaskTTWithPrivatesTySz, KmpSharedTySz,
      TaskEntry, InsertPt, UseTbb);

  return TaskAllocCall;
}

// Generate a call to create an AsyncObj for TARGET VARIANT DISPATCH NOWAIT
// The object is actually a repurposed task thunk (kmp_task_t) described
// elsewhere. The call created looks like this:
//    i8* @__kmpc_omp_task_alloc(loc,                // (IdentTy*)
//                               0,                  // (i32) unused
//                               0x10,               // (i32) Proxy flag
//                               sizeof(AsyncObjTy), // (size_t)
//                               0,               // (size_t) sizeof(shareds_t)
//                               null);           // (i8*) unused
CallInst *VPOParoptUtils::genKmpcTaskAllocForAsyncObj(WRegionNode *W,
                                                      StructType *IdentTy,
                                                      int AsyncObjTySize,
                                                      Instruction *InsertPt) {

  IRBuilder<> Builder(InsertPt);
  Type *Int32Ty = Builder.getInt32Ty();
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  Type *SizeTTy = GeneralUtils::getSizeTTy(InsertPt->getFunction());

  Value *ValueZero = ConstantInt::get(Int32Ty, 0);
  Value *ProxyFlag = ConstantInt::get(Int32Ty, WRNTaskFlag::Proxy); // 0x10
  Value *ValueAsyncObjTySize = ConstantInt::get(SizeTTy, AsyncObjTySize);

  ConstantPointerNull *NullPtr = ConstantPointerNull::get(Int8PtrTy);

  CallInst *TaskAllocCall = genKmpcTaskAllocImpl(
      W, IdentTy, ValueZero, ProxyFlag, ValueAsyncObjTySize,
      /*KmpSharedTySz=*/0, NullPtr, InsertPt, false);

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
//     long long iii;
//     #pragma omp distribute parallel for dist_schedule(static, chksize)
//       for (iii=1; iii<100; iii++) ...
//
// the LCV iii is i64, so chksize is cast from i32 to i64 in the call:
//
//     %chunk.cast = sext i32 %chksize to i64
//     call void @__kmpc_team_static_init_8( ... , i64 %chunk.cast)
//
// The cast instruction is not needed if the type is already matching. For
// example, if "long long iii" above is changed to "int iii", then we get
//
//     ; no casting of "i32 %chksize" as it is the correct type
//     call void @__kmpc_team_static_init_4( ... , i32 %chksize)
//
// The cast instruction is not emitted if chunk is a constant and the compiler
// can convert it directly. For example, given:
//
//     long long iii;
//     #pragma omp distribute parallel for schedule(static, 17)
//       for (iii=1; iii<100; iii++) ...
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
  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

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

  if (!FnTeamStaticInit)
    FnTeamStaticInit = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                    FnName, M);

  std::vector<Value *> FnTeamStaticInitArgs;

  FnTeamStaticInitArgs.push_back(Loc);
  FnTeamStaticInitArgs.push_back(Tid);
  FnTeamStaticInitArgs.push_back(IsLastVal);
  FnTeamStaticInitArgs.push_back(LB);
  FnTeamStaticInitArgs.push_back(UB);
  FnTeamStaticInitArgs.push_back(ST);
  FnTeamStaticInitArgs.push_back(Inc);
  FnTeamStaticInitArgs.push_back(Chunk);

  CallInst *TeamStaticInitCall = CallInst::Create(
      FnTy, FnTeamStaticInit, FnTeamStaticInitArgs, "", InsertPt);
  setFuncCallingConv(TeamStaticInitCall, M);
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
//     long long iii;
//     #pragma omp for schedule(static, chksize)
//       for (iii=1; iii<100; iii++) ...
//
// the LCV iii is i64, so chksize is cast from i32 to i64 in the call:
//
//     %chunk.cast = sext i32 %chksize to i64
//     call void @__kmpc_for_static_init_8( ... , i64 %chunk.cast)
//
// The cast instruction is not needed if the type is already matching. For
// example, if "long long iii" above is changed to "int iii", then we get
//
//     ; no casting of "i32 %chksize" as it is the correct type
//     call void @__kmpc_for_static_init_4( ... , i32 %chksize)
//
// The cast instruction is not emitted if chunk is a constant and the compiler
// can convert it directly. For example, given:
//
//     long long iii;
//     #pragma omp for schedule(static, 17)
//       for (iii=1; iii<100; iii++) ...
//
// The original "i32 17" is directly converted to "i64 17" by the compiler
// without needing a sext instruction:
//
//     call void @__kmpc_for_static_init_8( ... , i64 17)
//
CallInst *VPOParoptUtils::genKmpcStaticInit(
    WRegionNode *W, StructType *IdentTy, Value *Tid,
    Value *IsLastVal, Value *LB, Value *UB, Value *DistUB, Value *ST,
    Value *Inc, Value *Chunk, bool IsUnsigned, IntegerType *LoopIVTy,
    Instruction *InsertPt) {

  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();

  int Flags = KMP_IDENT_KMPC;
  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  auto Size = LoopIVTy->getIntegerBitWidth();
  assert((Size == 32 || Size == 64) &&
         "Invalid loop IV type in genKmpcStaticInit().");

  // Verify type size of the increment parameter.
  assert(Inc->getType()->isIntegerTy(Size) &&
         "Invalid parameter types in genKmpcStaticInit().");

  LLVMContext &C = F->getContext();
  auto *Int32Ty = Type::getInt32Ty(C);
  auto *IntArgTy = (Size == 32) ? Int32Ty : Type::getInt64Ty(C);

  // If Chunk's type != IntArgTy, cast it to IntArgTy
  IRBuilder<> Builder(InsertPt);
  Chunk = Builder.CreateSExtOrTrunc(Chunk, IntArgTy, "chunk.cast");

  // Call __kmpc_dist_for_static_init only for "distribute parallel for"
  // without dist_schedule() clause.
  bool IsDistEvenParLoop =
      isa<WRNDistributeParLoopNode>(W) &&
      (VPOParoptUtils::getDistLoopScheduleKind(W) ==
       WRNScheduleDistributeStaticEven);

  // dist_schedule() is only used for distribute-for loops.
  // Side note: we will call __kmpc_for_static_init in this case.
  auto SchedKind =
      isa<WRNDistributeNode>(W) ?
          getDistLoopScheduleKind(W) : getLoopScheduleKind(W);

  auto *SchedID = ConstantInt::getSigned(Int32Ty, SchedKind);

  // The name is: __kmpc_[dist_]for_static_init_4/8[u]
  std::string FnName =
      (Twine("__kmpc_") +
       (IsDistEvenParLoop ? Twine("dist_") : Twine("")) +
       Twine("for_static_init_") + Twine(Size / 8) +
       (IsUnsigned ? Twine("u") : Twine(""))).str();

  SmallVector<Type *, 10> FnArgTypes;

  if (IsDistEvenParLoop)
    // void __kmpc_dist_for_static_init_4/8[u](
    //          ident_t *loc, kmp_int32 gtid,
    //          kmp_int32 schedule, kmp_int32 *plastiter,
    //          kmp_[u]int32/64 *plower, kmp_[u]int32/64 *pupper,
    //          kmp_[u]int32/64 *pupperD, // not used for __kmpc_for_static_init
    //          kmp_int32/64 *pstride, kmp_int32/64 incr, kmp_int32/64 chunk)
    FnArgTypes = { PointerType::getUnqual(IdentTy),
                   Int32Ty, Int32Ty, PointerType::getUnqual(Int32Ty),
                   PointerType::getUnqual(IntArgTy),
                   PointerType::getUnqual(IntArgTy),
                   PointerType::getUnqual(IntArgTy),
                   PointerType::getUnqual(IntArgTy),
                   IntArgTy, IntArgTy };
  else
    // void __kmpc_for_static_init_4/8[u](
    //          ident_t *loc, kmp_int32 gtid,
    //          kmp_int32 schedule, kmp_int32 *plastiter,
    //          kmp_[u]int32/64 *plower, kmp_[u]int32/64 *pupper,
    //          kmp_int32/64 *pstride, kmp_int32/64 incr, kmp_int32/64 chunk)
    FnArgTypes = { PointerType::getUnqual(IdentTy),
                   Int32Ty, Int32Ty, PointerType::getUnqual(Int32Ty),
                   PointerType::getUnqual(IntArgTy),
                   PointerType::getUnqual(IntArgTy),
                   PointerType::getUnqual(IntArgTy),
                   IntArgTy, IntArgTy };

  SmallVector<Value *, 10> FnArgs { Loc, Tid, SchedID, IsLastVal, LB, UB };

  if (IsDistEvenParLoop) {
    FnArgs.push_back(DistUB);
  }

  FnArgs.push_back(ST);
  FnArgs.push_back(Inc);
  FnArgs.push_back(Chunk);

  return genCall(FnName, Type::getVoidTy(C), FnArgs, FnArgTypes, InsertPt);
}

/// This function generates a call to notify the runtime system that the static
/// loop scheduling is done
///
/// \code
/// call void @__kmpc_for_static_fini(ident_t *loc,
///                                   kmp_int32 global_tid)
/// \endcode
CallInst *VPOParoptUtils::genKmpcStaticFini(WRegionNode *W,
                                            StructType *IdentTy,
                                            Value *Tid,
                                            Instruction *InsertPt) {
  BasicBlock *B = W->getEntryBBlock();
  BasicBlock *E = W->getExitBBlock();

  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  int Flags = KMP_IDENT_KMPC;

  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);
  LLVM_DEBUG(dbgs() << "\n---- Loop Source Location Info: " << *Loc << "\n\n");

  // Assert that we used the right type for internally created
  // thread ID.
  assert(Tid->getType()->isIntegerTy(32) &&
         "Thread ID must be 4-byte integer.");

  SmallVector<Type *, 2> FnArgTypes { PointerType::getUnqual(IdentTy),
                                      Type::getInt32Ty(C) };
  SmallVector<Value *, 2> FnArgs { Loc, Tid };

  CallInst *CI = genCall("__kmpc_for_static_fini", Type::getVoidTy(C), FnArgs,
                         FnArgTypes, InsertPt);
  return CI;
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

  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

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
                        Int32Ty, Int32Ty, PointerType::getUnqual(Int32Ty),
                        IntArgTy, IntArgTy, IntArgTy, IntArgTy};
    FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);
  }
  else {
    Type *ParamsTy[] = {PointerType::getUnqual(IdentTy),
                        Int32Ty, Int32Ty,
                        IntArgTy, IntArgTy, IntArgTy, IntArgTy};
    FnTy = FunctionType::get(Type::getVoidTy(C), ParamsTy, false);
  }

  Function *FnDispatchInit = M->getFunction(FnName);

  if (!FnDispatchInit)
    FnDispatchInit = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                      FnName, M);

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

  CallInst *DispatchInitCall =
      CallInst::Create(FnTy, FnDispatchInit, FnDispatchInitArgs, "", InsertPt);
  setFuncCallingConv(DispatchInitCall, M);
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

  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

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

  if (!FnDispatchNext)
    FnDispatchNext = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                      FnName, M);

  std::vector<Value *> FnDispatchNextArgs;

  FnDispatchNextArgs.push_back(Loc);
  FnDispatchNextArgs.push_back(Tid);
  FnDispatchNextArgs.push_back(IsLastVal);
  FnDispatchNextArgs.push_back(LB);
  FnDispatchNextArgs.push_back(UB);
  FnDispatchNextArgs.push_back(ST);

  CallInst *DispatchNextCall =
      CallInst::Create(FnTy, FnDispatchNext, FnDispatchNextArgs, "", InsertPt);
  setFuncCallingConv(DispatchNextCall, M);
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

  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);

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

  Constant *KmpcLoc = genKmpcLocfromDebugLoc(IdentTy, Flags, &B, &E);

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
    IdentTy = VPOParoptUtils::getIdentStructType(F);

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  Constant *KmpcLoc = genKmpcLocfromDebugLoc(IdentTy, Flags, &B, &E);

  FunctionType *FnGetTidTy = FunctionType::get(
      Type::getInt32Ty(C), KmpcLoc->getType(), false);

  Function *FnGetTid = M->getFunction("__kmpc_global_thread_num");

  if (!FnGetTid)
    FnGetTid = Function::Create(FnGetTidTy, GlobalValue::ExternalLinkage,
                                "__kmpc_global_thread_num", M);

  std::vector<Value *> FnGetTidArgs;
  FnGetTidArgs.push_back(KmpcLoc);

  CallInst *GetTidCall =
      CallInst::Create(FnGetTidTy, FnGetTid, FnGetTidArgs, "tid.val");
  setFuncCallingConv(GetTidCall, M);
  GetTidCall->setTailCall(true);

  return GetTidCall;
}

// This function generates a GlobalVariable for the source location string
// consisting of 4 fields. if Loc2==null the last field of the LocString is the
// Column Number otherwise, the last field of the LocString is the end Line.
// This utility is called either from genKmpcLocfromDebugLoc to generate kmpc
// location (in this case loc2 will be non null) or from
// genTgtPushCodeLocation (in which case loc2 will be null).
// Based on the value of Mode, the function will generate the following
// LocStrings
// if Mode = SRC_LOC_NONE;
//    LocString =";unknown;unknown;0;0;;\00";
// if Mode = SRC_LOC_FUNC;
//    LocString
//    =“;unknown;<function name>;<start line>;<end line OR column num>;;”
// if Mode = SRC_LOC_FILE;
//    LocString
//   =“;<file name>;<function name>;<start line>;<end line OR column num>;;”
// if Mode = SRC_LOC_PATH;
//    LocString =
//  “;<Path/file name>;<function name>;<start line>;<end line OR column num>;;”

GlobalVariable *VPOParoptUtils::genLocStrfromDebugLoc(Function *F,
                                                      DILocation *Loc1,
                                                      DILocation *Loc2,
                                                      SrcLocMode Mode) {
  std::string LocString;
  std::string Path("");
  std::string File("unknown");
  std::string FnName("unknown");
  unsigned SLine = 0;
  unsigned ELine = 0;

  if (Loc1 != nullptr) {
    switch (Mode) {
    case SRC_LOC_PATH:
      Path = (Loc1->getDirectory() + "/").str();
      LLVM_FALLTHROUGH;
    case SRC_LOC_FILE:
      File = (Loc1->getFilename()).str();
      LLVM_FALLTHROUGH;
    case SRC_LOC_FUNC:
      FnName = (Loc1->getScope()->getSubprogram()->getName()).str();
      SLine = Loc1->getLine();
      ELine = Loc2 ? Loc2->getLine() : Loc1->getColumn();
      break;
    case SRC_LOC_NONE:
      break;
    }
  }
  LocString = (";" + Path + File + ";" + FnName + ";" + Twine(SLine) + ";" +
               Twine(ELine) + ";;\00")
                  .str();

  LLVM_DEBUG(dbgs() << "\nSource Location Info: " << LocString << "\n");

  Module *M = F->getParent();
  unsigned GlobalAS = VPOAnalysisUtils::isTargetSPIRV(M) ?
      vpo::ADDRESS_SPACE_GLOBAL : 0;
  LLVMContext &C = F->getContext();
  // Define the type of loc string. It is an array of i8/char type.
  ArrayType *LocStringTy = ArrayType::get(Type::getInt8Ty(C), LocString.size());

  // Create a global variable containing the loc string. Example:
  // @.source.0.0.9 = private unnamed_addr constant [22 x i8]
  // c";unknown;unknown;0;0;;"
  Constant *LocStringInit = ConstantDataArray::getString(C, LocString, false);
  GlobalVariable *LocStringVar = new GlobalVariable(
      *M, LocStringTy, /*isConstant=*/true, GlobalValue::PrivateLinkage,
      LocStringInit, ".source." + Twine(SLine) + "." + Twine(ELine),
      /*InsertBefore=*/nullptr, GlobalValue::ThreadLocalMode::NotThreadLocal,
      GlobalAS);
  // Allows merging of variables with same content.
  LocStringVar->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  return LocStringVar;
}

// This function collects path, file name, line, column information for
// generating kmpc_location struct needed for OpenMP runtime library
Constant *VPOParoptUtils::genKmpcLocfromDebugLoc(StructType *IdentTy,
                                                 int Flags,
                                                 BasicBlock *BS,
                                                 BasicBlock *BE) {
  Function *F = BS->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  unsigned GlobalAS = 0;
  unsigned GenericAS = 0;
  if (VPOAnalysisUtils::isTargetSPIRV(M)) {
    GlobalAS = vpo::ADDRESS_SPACE_GLOBAL;
    GenericAS = vpo::ADDRESS_SPACE_GENERIC;
  }

  Instruction *SInst = dyn_cast<Instruction>(BS->begin());
  DILocation *Loc1 = SInst ? SInst->getDebugLoc() : nullptr;
  Instruction *EInst = dyn_cast<Instruction>(BE->begin());
  DILocation *Loc2 = EInst ? EInst->getDebugLoc() : nullptr;

  SrcLocMode Mode = getSrcLocMode();

  GlobalVariable *LocStringVar = genLocStrfromDebugLoc(F, Loc1, Loc2, Mode);

  Flags |= KMP_IDENT_OPENMP_SPEC_VERSION_5_0; // Enable nonmonotonic scheduling

  // Constant Definitions
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
  ConstantInt *ValueFlags = ConstantInt::get(Type::getInt32Ty(C), Flags);
  // TODO: OPAQUEPOINTER: PointerBitcast will not be needed.
  Constant *LocStringPtr = ConstantExpr::getPointerBitCastOrAddrSpaceCast(
      LocStringVar, IdentTy->getTypeAtIndex(4));

  unsigned SLine =
      (Loc1 != nullptr && Mode != SRC_LOC_NONE) ? Loc1->getLine() : 0;
  unsigned ELine =
      (Loc2 != nullptr && Mode != SRC_LOC_NONE) ? Loc2->getLine() : 0;
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
      StructInit, ".kmpc_loc." + Twine(SLine) + "." + Twine(ELine),
      /*InsertBefore=*/nullptr, GlobalValue::ThreadLocalMode::NotThreadLocal,
      GlobalAS);
  // Allows merging of variables with same content.
  KmpcLoc->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  // The returned value will be used as an argument for a function call.
  // We have to make sure that it is a generic address space pointer,
  // which is the address space for pointer arguments of __kmpc functions.
  return ConstantExpr::getPointerBitCastOrAddrSpaceCast(
      KmpcLoc, PointerType::get(IdentTy, GenericAS));
}

// Generate source location information for Explicit barrier
Constant *VPOParoptUtils::genKmpcLocforExplicitBarrier(StructType *IdentTy,
                                                       BasicBlock *BB) {
  int Flags = KMP_IDENT_KMPC | KMP_IDENT_BARRIER_EXPL; // bits 0x2 | 0x20

#if 0
  if (VPOParopt_openmp_dvsm)
    flags |= KMP_IDENT_CLOMP;  // bit 0x4
#endif

  return genKmpcLocfromDebugLoc(IdentTy, Flags, BB, BB);
}

// Generate source location information for Implicit barrier
Constant *VPOParoptUtils::genKmpcLocforImplicitBarrier(
    WRegionNode *W, StructType *IdentTy, BasicBlock *BB) {
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

  return genKmpcLocfromDebugLoc(IdentTy, Flags, BB, BB);
}

// Insert kmpc_[cancel_]barrier(...) call before InsertPt. If the call emitted
// is a __kmpc_cancel_barrier(...), add it to the parent WRegionNode's
// CancellationPoints.
CallInst *VPOParoptUtils::genKmpcBarrier(WRegionNode *W, Value *Tid,
                                         Instruction *InsertPt,
                                         StructType *IdentTy, bool IsExplicit,
                                         bool IsTargetSPIRV) {

  WRegionNode *WParent = W->getParent();
  bool IsCancelBarrier = WParent && WParent->canHaveCancellationPoints() &&
                         !IsTargetSPIRV &&
                         WRegionUtils::hasCancelConstruct(WParent);

  auto *BarrierCall = VPOParoptUtils::genKmpcBarrierImpl(
      W, Tid, InsertPt, IdentTy, IsExplicit, IsCancelBarrier, IsTargetSPIRV);

  if (IsCancelBarrier)
    WParent->addCancellationPoint(BarrierCall);

  return BarrierCall;
}

// Check if the current work item belongs to the master sub group
// EXTERN int __kmpc_master_sub_group();
//
// Check if the current work item is the leader of the master sub group
// EXTERN int __kmpc_master_sub_group_leader();
CallInst *VPOParoptUtils::genMasterSubGroup(WRegionNode *W,
                                            Instruction *InsertPt,
                                            bool LeaderFlag) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  Type *RetTy = Type::getInt32Ty(C);
  StringRef FnName =
      LeaderFlag ? "__kmpc_master_sub_group_leader" : "__kmpc_master_sub_group";
  CallInst *Call = genEmptyCall(M, FnName, RetTy, InsertPt);
  return Call;
}

// Initialize a SPMD kernel execution
// EXTERN void __kmpc_spmd_kernel_init(int thread_limit, short needs_rtl,
//                                     short needs_data_sharing);
CallInst *VPOParoptUtils::genSpmdKernelInit(WRegionNode *W,
                                            Instruction *InsertPt,
                                            Value *ThreadLimit,
                                            Value *NeedsRtl,
                                            Value *NeedsDataSharing) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  Type *RetTy = Type::getVoidTy(C);
  Type *TypeParams[] = {Type::getInt32Ty(C), Type::getInt16Ty(C),
                        Type::getInt16Ty(C)};
  IRBuilder<> Builder(InsertPt);
  Value *Args[] = {ThreadLimit, NeedsRtl, NeedsDataSharing};
  StringRef FnName = "__kmpc_spmd_kernel_init";
  Function *Fn = M->getFunction(FnName);
  FunctionType *FnTy = FunctionType::get(RetTy, TypeParams, false);

  if (!Fn)
    Fn = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *Call = CallInst::Create(FnTy, Fn, Args, "", InsertPt);
  setFuncCallingConv(Call, M);
  return Call;
}

/// Finalize a SPMD kernel execution
/// EXTERN void __kmpc_spmd_kernel_fini(short needs_rtl);
CallInst *VPOParoptUtils::genSpmdKernelFini(WRegionNode *W,
                                            Instruction *InsertPt,
                                            Value *NeedsRtl) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  Type *RetTy = Type::getVoidTy(C);
  Type *TypeParams[] = {Type::getInt16Ty(C)};
  IRBuilder<> Builder(InsertPt);
  Value *Args[] = {NeedsRtl};
  StringRef FnName = "__kmpc_spmd_kernel_fini";
  Function *Fn = M->getFunction(FnName);
  FunctionType *FnTy = FunctionType::get(RetTy, TypeParams, false);

  if (!Fn)
    Fn = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *Call = CallInst::Create(FnTy, Fn, Args, "", InsertPt);
  setFuncCallingConv(Call, M);
  return Call;
}

// Initialize a kernel execution
// EXTERN void __kmpc_kernel_init(int thread_limit, short needs_rtl);
CallInst *VPOParoptUtils::genKernelInit(WRegionNode *W, Instruction *InsertPt,
                                        Value *ThreadLimit, Value *NeedsRtl) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  Type *RetTy = Type::getVoidTy(C);
  Type *TypeParams[] = {Type::getInt32Ty(C), Type::getInt16Ty(C)};
  IRBuilder<> Builder(InsertPt);
  Value *Args[] = {ThreadLimit, NeedsRtl};
  StringRef FnName = "__kmpc_kernel_init";
  Function *Fn = M->getFunction(FnName);
  FunctionType *FnTy = FunctionType::get(RetTy, TypeParams, false);

  if (!Fn)
    Fn = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *Call = CallInst::Create(FnTy, Fn, Args, "", InsertPt);
  setFuncCallingConv(Call, M);
  return Call;
}

// Finalize a kernel execution
// EXTERN void __kmpc_kernel_fini(short needs_rtl);
CallInst *VPOParoptUtils::genKernelFini(WRegionNode *W, Instruction *InsertPt,
                                        Value *NeedsRtl) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  Type *RetTy = Type::getVoidTy(C);
  Type *TypeParams[] = {Type::getInt16Ty(C)};
  IRBuilder<> Builder(InsertPt);
  Value *Args[] = {NeedsRtl};
  StringRef FnName = "__kmpc_kernel_fini";
  Function *Fn = M->getFunction(FnName);
  FunctionType *FnTy = FunctionType::get(RetTy, TypeParams, false);

  if (!Fn)
    Fn = Function::Create(FnTy, GlobalValue::ExternalLinkage, FnName, M);

  CallInst *Call = CallInst::Create(FnTy, Fn, Args, "", InsertPt);
  setFuncCallingConv(Call, M);
  return Call;
}

// Finalize a parallel region -- called by workers
// EXTERN void __kmpc_kernel_end_parallel(void);
CallInst *VPOParoptUtils::genKernelEndParallel(Instruction *InsertPt) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = Type::getVoidTy(C);
  StringRef FnName = "__kmpc_kernel_end_parallel";

  CallInst *Call = genEmptyCall(M, FnName, RetTy, InsertPt);
  return Call;
}

// Init sharing variables
// EXTERN void __kmpc_init_sharing_variables(void);
// End sharing variables
// EXTERN void __kmpc_end_sharing_variables(void);
CallInst *VPOParoptUtils::genInitEndSharingVariables(Instruction *InsertPt,
                                                     bool End) {
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = Type::getVoidTy(C);
  StringRef FnName =
      End ? "__kmpc_end_sharing_variables" : "__kmpc_init_sharing_variables";

  CallInst *Call = genEmptyCall(M, FnName, RetTy, InsertPt);
  return Call;
}

// Insert kmpc_[cancel_]barrier(...) call before InsertPt.
CallInst *VPOParoptUtils::genKmpcBarrierImpl(
    WRegionNode *W, Value *Tid, Instruction *InsertPt, StructType *IdentTy,
    bool IsExplicit, bool IsCancelBarrier, bool IsTargetSPIRV) {
  BasicBlock  *B = InsertPt->getParent();
  Function    *F = B->getParent();
  Module      *M = F->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy;
  StringRef FnName;


  if (IsCancelBarrier) {
    assert(!IsTargetSPIRV && "OMP Cancel not supported for GPU offloading");
    RetTy = Type::getInt32Ty(C);
    FnName = "__kmpc_cancel_barrier";
  } else {
    RetTy = Type::getVoidTy(C);
    FnName = "__kmpc_barrier";
  }

  if (IsTargetSPIRV) {
    // Emit an empty __kmpc_barrier without parameters,
    // and insert it above InsertPt
    CallInst *BarrierCall = genEmptyCall(M, FnName, RetTy, InsertPt);
    BarrierCall->getCalledFunction()->setConvergent();
    setFuncCallingConv(BarrierCall, M);
    return BarrierCall;
  }

  // Create the arg for Loc
  Constant *Loc;
  if (IsExplicit)
    Loc = genKmpcLocforExplicitBarrier(IdentTy, B);
  else // Implicit
    Loc = genKmpcLocforImplicitBarrier(W, IdentTy, B);

  // Create the arg for Tid
  Type *I32Ty = Type::getInt32Ty(C);
  LoadInst *LoadTid = new LoadInst(I32Ty, Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(Align(4));

  // Create the argument list
  SmallVector<Value *, 3> FnArgs = {Loc, LoadTid};

  CallInst *BarrierCall = genCall(M, FnName, RetTy, FnArgs);
  BarrierCall->insertBefore(InsertPt);

  return BarrierCall;
}

// Generates a critical section around the given region
// by emitting calls to `__kmpc_critical` after the region's entry
// directive, and `__kmpc_end_critical` before the region's exit
// directive.
bool VPOParoptUtils::genKmpcCriticalSection(WRegionNode *W, StructType *IdentTy,
                                            Constant *TidPtr,
                                            DominatorTree *DT,
                                            LoopInfo *LI,
                                            bool IsTargetSPIRV,
                                            const Twine &LockNameSuffix) {
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
  //      directive "DIR.OMP.CRITICAL"
  // +------< begin critical >
  // |    br label %BB1
  // |
  // |  BB1:
  // |    ...
  // |  ...
  // |    br label %ExitBB
  // |
  // |  ExitBB:
  // +------< end critical >
  //      directive "DIR.OMP.END.CRITICAL"
  //      br label %..

  assert(W->getEntryBBlock()->size() <= 2 && "Entry BBlock has invalid size.");
  assert(W->getExitBBlock()->size() <= 2 && "Exit BBlock has invalid size.");

  // BeginInst: `br label %BB1` (EntryBB) in the above example.
  Instruction *BeginInst = W->getEntryDirective()->getNextNonDebugInstruction();
  // EndInst: directive "DIR.OMP.END.CRITICAL" in the above example.
  Instruction *EndInst = W->getExitDirective();

  assert(BeginInst != nullptr && "BeginInst is null.");
  assert(EndInst != nullptr && "EndInst is null.");

  return genKmpcCriticalSection(W, IdentTy, TidPtr, BeginInst, EndInst,
                                DT, LI, IsTargetSPIRV, LockNameSuffix);
}

// Generates tree reduce block around Instructions `BeginInst` and `EndInst` and
// atomic reduce block around Instructions `AtomicBeginInst` and
// `AtomicEndInst`.
// Here is an example of fast reduction for the scalar type.
// Source code:
//   int main(void)
//   {
//     int i, sum = 0;
//     #pragma omp parallel for reduction(+:sum)
//     for (i=0; i<10; i++) {
//       sum+=i;
//     }
//     return 0;
//   }
//
// Generated IR:
// omp.loop.exit.split:                              ; preds = %omp.loop.exit
//   br label %DIR.OMP.END.PARALLEL.LOOP.48
// ...
// loop.region.exit.split:                           ; preds =
// %loop.region.exit.split14
//   %7 = bitcast %struct.fast_red_t* %fast_red_struct to i8*            ; (1)
//   %my.tid16 = load i32, i32* %tid, align 4
//   %8 = call i32 @__kmpc_reduce(%struct.ident_t* @.kmpc_loc.0.0.6,
//   i32 %my.tid16, i32 1, i32 4, i8* %7, void (i8*, i8*)* @main_tree_reduce_2,
//   [8 x i32]* @.gomp_critical_user_.fast_reduction.AS0.var)            ; (2)
//   %to.tree.reduce = icmp eq i32 %8, 1                                 ; (3)
//   br i1 %to.tree.reduce, label %tree.reduce, label %tree.reduce.exit  ; (4)
// ...
//
// tree.reduce:                                      ; preds =
// %loop.region.exit.split                                               ; (5)
//   br label %loop.region.exit.split.split
//
// tree.reduce.exit:                                 ; preds =
// %loop.region.exit.split, %loop.region.exit.split.split.split          ; (6)
//   %9 = phi i1 [ false, %loop.region.exit.split ], [ true,
//   %loop.region.exit.split.split.split ]                               ; (7)
//   %10 = icmp eq i1 %9, false                                          ; (8)
//   br i1 %10, label %loop.region.exit.split.split.split.split, label
//   %atomic.reduce.exit                                                 ; (9)
//
// loop.region.exit.split.split:                     ; preds = %tree.reduce
//   %11 = load i32, i32* %sum.fast_red, align 4
//   %12 = load i32, i32* %sum, align 4
//   %13 = add i32 %12, %11
//   store i32 %13, i32* %sum, align 4
//   br label %loop.region.exit.split.split.split
//
// loop.region.exit.split.split.split.split:         ; preds = %tree.reduce.exit
//   %to.atomic.reduce = icmp eq i32 %8, 2                               ; (10)
//   br i1 %to.atomic.reduce, label %atomic.reduce, label
//   %atomic.reduce.exit                                                 ; (11)
//
// atomic.reduce.exit:                               ; preds =
// %tree.reduce.exit, %loop.region.exit.split.split.split.split,
// %loop.region.exit.split.split.split.split.split.split                 ; (12)
//   br label %omp.inner.for.end
//
// loop.region.exit.split.split.split:               ; preds =
// %loop.region.exit.split.split
//   %my.tid17 = load i32, i32* %tid, align 4
//   call void @__kmpc_end_reduce(%struct.ident_t* @.kmpc_loc.0.0.8, i32
//   %my.tid17, [8 x i32]* @.gomp_critical_user_.fast_reduction.AS0.var) ; (13)
//   br label %tree.reduce.exit
//
// atomic.reduce:                                    ; preds =
// %loop.region.exit.split.split.split.split                             ; (14)
//   br label %loop.region.exit.split.split.split.split.split
//
// loop.region.exit.split.split.split.split.split:   ; preds =
// %atomic.reduce
//   %14 = load i32, i32* %sum.fast_red, align 4
//   %my.tid15 = load i32, i32* %tid, align 4
//   call void @__kmpc_atomic_fixed4_add(%struct.ident_t* @.kmpc_loc.0.0.4, i32
//   %my.tid15, i32* %sum, i32 %14)
//   br label %loop.region.exit.split.split.split.split.split.split
//
// loop.region.exit.split.split.split.split.split.split: ; preds =
// %loop.region.exit.split.split.split.split.split
//   %my.tid20 = load i32, i32* %tid, align 4
//   call void @__kmpc_end_reduce(%struct.ident_t* @.kmpc_loc.0.0.10, i32
//   %my.tid20, [8 x i32]* @.gomp_critical_user_.fast_reduction.AS0.var) ; (15)
//   br label %atomic.reduce.exit
//
// DIR.OMP.END.PARALLEL.LOOP.48:                     ; preds =
// %omp.loop.exit.split
//  br label %DIR.OMP.END.PARALLEL.LOOP.4.exitStub
//
bool VPOParoptUtils::genKmpcReduceImpl(
    WRegionNode *W, StructType *IdentTy, Constant *TidPtr, Value *RedVar,
    RDECL RedCallback, Instruction *BeginInst, Instruction *EndInst,
    Instruction *AtomicBeginInst, Instruction *AtomicEndInst,
    GlobalVariable *LockVar, DominatorTree *DT, LoopInfo *LI,
    bool IsTargetSPIRV) {

  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(BeginInst != nullptr && "BeginInst is null.");
  assert(EndInst != nullptr && "EndInst is null.");
  assert(LockVar != nullptr && "LockVar is null.");

  assert(!IsTargetSPIRV && "Support in target region is not implemented!");

  IRBuilder<> Builder(BeginInst);
  auto *RetTy = Builder.getInt32Ty();

  bool Nowait = false;
  if (W->canHaveNowait())
    Nowait = W->getNowait();

  StringRef BeginName = "__kmpc_reduce";
  if (Nowait)
    BeginName = "__kmpc_reduce_nowait";

  CallInst *BeginReduce = nullptr;
  SmallVector<Value *, 5> BeginArgs;

  Value *RedVarI8 =
      Builder.CreateBitCast(RedVar, Builder.getInt8PtrTy()); // (1)

  // add num of reductions, reduction variable, and size of reduction variable
  ReductionClause &RedClause = W->getRed();
  Value *NumOfRed = Builder.getInt32(RedClause.size());
  BeginArgs.push_back(NumOfRed);
  const auto DL = BeginInst->getModule()->getDataLayout();
  Value *SizeOfRedVar = Builder.getInt32(
      DL.getTypeAllocSize(cast<AllocaInst>(RedVar)->getAllocatedType()));
  BeginArgs.push_back(SizeOfRedVar);
  BeginArgs.push_back(RedVarI8);
  BeginArgs.push_back(RedCallback);
  BeginArgs.push_back(LockVar);

  bool EnableAtomicReduce = false;
  if (AtomicBeginInst != nullptr && AtomicEndInst != nullptr)
    EnableAtomicReduce = true;
  BeginReduce = genKmpcCallWithTid(W, IdentTy, TidPtr, BeginInst, BeginName,
                                   RetTy, BeginArgs, EnableAtomicReduce); // (2)
  assert(BeginReduce != nullptr && "Could not call __kmp_reduce");

  if (BeginReduce == nullptr)
    return false;

  StringRef EndName = "__kmpc_end_reduce";
  if (Nowait)
    EndName = "__kmpc_end_reduce_nowait";
  CallInst *EndReduce = nullptr;

  auto *EndRetTy = Builder.getVoidTy();
  Value *EndArg = LockVar;
  EndReduce = genKmpcCallWithTid(W, IdentTy, TidPtr, EndInst, EndName, EndRetTy,
                                 {EndArg}); // (13)

  assert(EndReduce != nullptr && "Could not call __kmpc_end_reduce");

  if (EndReduce == nullptr)
    return false;

  Builder.Insert(BeginReduce);
  if (EndInst->isTerminator())
    EndReduce->insertBefore(EndInst);
  else
    EndReduce->insertAfter(EndInst);

  ConstantInt *ValueOne = Builder.getInt32(1);
  auto IsTrue =
      Builder.CreateICmpEQ(BeginReduce, ValueOne, "to.tree.reduce"); // (3)
  auto EntryBB = Builder.GetInsertBlock();
  auto ContBB = SplitBlock(EntryBB, BeginInst, DT, LI); // (5)
  ContBB->setName("tree.reduce");

  Instruction *SplitPt = GeneralUtils::nextUniqueInstruction(EndReduce);
  auto ElseBB = SplitBlock(SplitPt->getParent(), SplitPt, DT, LI); // (6)
  ElseBB->setName("tree.reduce.exit");

  EntryBB->getTerminator()->eraseFromParent();
  Builder.SetInsertPoint(EntryBB);
  Builder.CreateCondBr(IsTrue, ContBB, ElseBB); // (4)

  if (AtomicBeginInst != nullptr) {
    assert(AtomicEndInst != nullptr &&
           "begin and end instructions for atomic "
           "reduce block must be not null at same time");
    Builder.SetInsertPoint(ElseBB->getTerminator());
    PHINode *PN = Builder.CreatePHI(Builder.getInt1Ty(), 2, ""); // (7)
    auto PhiEntryBBVal = Builder.getFalse();
    PN->addIncoming(PhiEntryBBVal, EntryBB);
    auto PhiElseBBVal = Builder.getTrue();
    PN->addIncoming(PhiElseBBVal, EndReduce->getParent());
    auto IsFalse = Builder.CreateICmpEQ(PN, Builder.getFalse(), ""); // (8)

    Instruction *AtomicEndReduce = AtomicEndInst;
    if (!Nowait) {
      AtomicEndReduce = genKmpcCallWithTid(W, IdentTy, TidPtr, AtomicEndInst,
                                           EndName, EndRetTy, {EndArg}); // (15)
      assert(AtomicEndReduce != nullptr &&
             "Could not call __kmpc_end_reduce after atomic statements.");
      if (AtomicEndReduce == nullptr)
        return false;
      if (AtomicEndInst->isTerminator())
        AtomicEndReduce->insertBefore(AtomicEndInst);
      else
        AtomicEndReduce->insertAfter(AtomicEndInst);
    }

    Builder.SetInsertPoint(AtomicBeginInst);
    ConstantInt *ValueTwo = Builder.getInt32(2);
    auto IsAtomicTrue =
        Builder.CreateICmpEQ(BeginReduce, ValueTwo, "to.atomic.reduce"); // (10)
    auto AtomicEntryBB = Builder.GetInsertBlock();
    auto AtomicContBB =
        SplitBlock(AtomicEntryBB, AtomicBeginInst, DT, LI); // (14)
    AtomicContBB->setName("atomic.reduce");

    BasicBlock *AtomicElseBB = nullptr;
    if (AtomicEndReduce->isTerminator()) {
      assert((AtomicEndReduce == AtomicEndInst) && "Something wrong.");
      assert(Nowait && "Nowait should be true");

      // no __kmpc_end_reduce function call
      AtomicElseBB = AtomicEndReduce->getParent();
    } else {
      SplitPt = GeneralUtils::nextUniqueInstruction(AtomicEndReduce);
      AtomicElseBB =
          SplitBlock(AtomicEndInst->getParent(), SplitPt, DT, LI); // (12)
      AtomicElseBB->setName("atomic.reduce.exit");
    }

    AtomicEntryBB->getTerminator()->eraseFromParent();
    Builder.SetInsertPoint(AtomicEntryBB);
    Builder.CreateCondBr(IsAtomicTrue, AtomicContBB, AtomicElseBB); // (11)

    Builder.SetInsertPoint(ElseBB->getTerminator());
    Builder.CreateCondBr(IsFalse, AtomicEntryBB, AtomicElseBB); // (9)
    ElseBB->getTerminator()->eraseFromParent();
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Fast Reduce Block generated.\n");
  return true;
}

// Generates a reduce block around Instructions `BeginInst` and `Endinst`,
// by emitting calls to `__kmpc_reduce` before `BeginInst`, and
// `__kmpc_end_reduce` after `EndInst`; and generates atomic reduce block around
// Instructions `AtomicBeginInst` and `AtomicEndinst`, by emitting calls to
// atomic update routine and use it to replace reduction update instruction, and
// `__kmpc_end_reduce` after `AtomicEndInst`.
bool VPOParoptUtils::genKmpcReduce(
    WRegionNode *W, StructType *IdentTy, Constant *TidPtr, Value *RedVar,
    RDECL RedCallback, Instruction *BeginInst, Instruction *EndInst,
    Instruction *AtomicBeginInst, Instruction *AtomicEndInst, DominatorTree *DT,
    LoopInfo *LI, bool IsTargetSPIRV, const StringRef LockNameSuffix) {
  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(BeginInst != nullptr && "BeginInst is null.");
  assert(EndInst != nullptr && "EndInst is null.");

  // Generate the Lock object for reduce block.
  GlobalVariable *Lock =
      genKmpcCriticalLockVar(W, LockNameSuffix, IsTargetSPIRV);
  assert(Lock != nullptr && "Could not create reduce block lock variable.");

  return genKmpcReduceImpl(W, IdentTy, TidPtr, RedVar, RedCallback, BeginInst,
                           EndInst, AtomicBeginInst, AtomicEndInst, Lock, DT,
                           LI, IsTargetSPIRV);
}

// Generates a KMPC call to IntrinsicName with Tid obtained using TidPtr.
CallInst *VPOParoptUtils::genKmpcCallWithTid(
    WRegionNode *W, StructType *IdentTy, Value *TidPtr, Instruction *InsertPt,
    StringRef IntrinsicName, Type *ReturnTy, ArrayRef<Value *> Args,
    bool EnableAtomicReduce) {
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
  Type *I32Ty = Type::getInt32Ty(InsertPt->getModule()->getContext());
  LoadInst *LoadTid = new LoadInst(I32Ty, TidPtr, "my.tid", InsertPt);
  LoadTid->setAlignment(Align(4));

  // Now bundle all the function arguments together.
  SmallVector<Value*, 3> FnArgs = {LoadTid};

  if (!Args.empty())
    FnArgs.append(Args.begin(), Args.end());

  // And then try to generate the KMPC call.
  return VPOParoptUtils::genKmpcCall(W, IdentTy, InsertPt, IntrinsicName,
                                     ReturnTy, FnArgs, false,
                                     EnableAtomicReduce);
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
  Type *I32Ty = Type::getInt32Ty(C);

  LoadInst *LoadTid = new LoadInst(I32Ty, Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(Align(4));

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
CallInst *VPOParoptUtils::genKmpcMasterOrEndMasterCall(
    WRegionNode *W, StructType *IdentTy, Value *Tid, Instruction *InsertPt,
    bool IsMasterStart, bool IsTargetSPIRV) {

  BasicBlock  *B = W->getEntryBBlock();
  Function    *F = B->getParent();
  LLVMContext &C = F->getContext();

  Type *RetTy = nullptr;
  Type *I32Ty = Type::getInt32Ty(C);
  StringRef FnName;

  if (IsMasterStart) {
    FnName = "__kmpc_master";
    RetTy = I32Ty;
  }
  else {
    FnName = "__kmpc_end_master";
    RetTy = Type::getVoidTy(C);
  }

  if (IsTargetSPIRV) {
    // Create an empty begin/end master call without parameters.
    // Don't insert it into the IR yet.
    Module *M = F->getParent();
    CallInst *MasterOrEndCall = genEmptyCall(M, FnName, RetTy, nullptr);
    return MasterOrEndCall;
  }

  LoadInst *LoadTid = new LoadInst(I32Ty, Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(Align(4));

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
  Type *I32Ty = Type::getInt32Ty(C);

  if (IsSingleStart) {
    FnName = "__kmpc_single";
    RetTy = Type::getInt32Ty(C);
  }
  else {
    FnName = "__kmpc_end_single";
    RetTy = Type::getVoidTy(C);
  }

  LoadInst *LoadTid = new LoadInst(I32Ty, Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(Align(4));

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
  Type *I32Ty = Type::getInt32Ty(C);

  StringRef FnName;

  if (IsOrderedStart)
    FnName = "__kmpc_ordered";
  else
    FnName = "__kmpc_end_ordered";

  LoadInst *LoadTid = new LoadInst(I32Ty, Tid, "my.tid", InsertPt);
  LoadTid->setAlignment(Align(4));

  // Now bundle all the function arguments together.
  SmallVector<Value *, 3> FnArgs = {LoadTid};

  CallInst *OrderedOrEndCall = VPOParoptUtils::genKmpcCall(W,
                                 IdentTy, InsertPt, FnName, RetTy, FnArgs);
  return OrderedOrEndCall;
}

// This function generates and inserts calls to kmpc_doacross_wait/post for
// '#pragma omp ordered depend(source/sink)'.
//
// Incoming Directive:
//   %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(),
//        "QUAL.OMP.DEPEND.SINK"(i32 %v1, i32 %v2) ]
//
// Generated IR:
//   %dep.vec = alloca i64, i32 2                             ; (1)
//   %conv1 = sext i32 %v1 to i64                             ; (2)
//   %2 = getelementptr inbounds i64, i64* %dep.vec, i64 0    ; (3)
//   store i64 %conv1, i64* %2                                ; (4)
//   %conv2 = sext i32 %v2 to i64                             ; (5)
//   %3 = getelementptr inbounds i64, i64* %dep.vec, i64 1    ; (6)
//   store i64 %conv2, i64* %3                                ; (7)
//   %4 = bitcast i64* %dep.vec to i8*                        ; (8)
//   %tid = load i32, i32* %<tidptr>, align 4                 ; (9)
//   call void @__kmpc_doacross_wait({ i32, i32, i32, i32, i8* }* @loc,
//                                   i32 %tid, i8* %4)        ; (10)
//
// The call is inserted before InsertPt.
//
CallInst *VPOParoptUtils::genDoacrossWaitOrPostCall(
    WRNOrderedNode *W, StructType *IdentTy, Value *TidPtr,
    Instruction *InsertPt, const ArrayRef<Value *> &DepVecValues,
    bool IsDoacrossPost) {

  assert(!DepVecValues.empty() &&
         "Empty Dependence Vector for doacross wait/post.");

  IRBuilder<> Builder(InsertPt);

  Type *Int64Ty = Builder.getInt64Ty();
  unsigned NumLoops =
      DepVecValues.size(); // Number of loops in the doacross nest.
  Value *NumLoopsVal = Builder.getInt32(NumLoops);

  // (1) Allocate space for the Dependence Vector.
  // It should contain one I64 for each loop in the doacross loop-nest.
  AllocaInst *DepVec = Builder.CreateAlloca(Int64Ty, NumLoopsVal, "dep.vec");
  // Populate dependence vector for each loop in the nest.
  for (unsigned I = 0; I < NumLoops; ++I) {
    Value *DepVecValue = DepVecValues[I];

    // Cast to I64.
    Value *DepVecValueCast =
        Builder.CreateSExtOrBitCast(DepVecValue, Int64Ty, "conv"); // (2) (5)

    // Get a pointer to where DepVecValue should go.
    Value *PtrForLoopI = Builder.CreateInBoundsGEP(
        DepVec->getAllocatedType(), DepVec,
        Builder.getInt64(I));                                      // (3) (6)
    Builder.CreateStore(DepVecValueCast, PtrForLoopI);             // (4) (7)
  }

  // (8) Create a bitcast to I8* before sending DepVec to the runtime.
  Value *DepVecI8 = Builder.CreateBitCast(DepVec, Builder.getInt8PtrTy());

  CallInst *Call = genKmpcCallWithTid(W, IdentTy, TidPtr, InsertPt,
                                      IsDoacrossPost ? "__kmpc_doacross_post"
                                                     : "__kmpc_doacross_wait",
                                      nullptr, {DepVecI8}); //        (9) (10)

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Doacross wait/post call emitted.\n");

  Call->insertBefore(InsertPt);
  return Call;
}

// This function generates and inserts a call to kmpc_doacross_init,
// for '#pragma omp [parallel] for ordered(n)'
//
// Incoming Directive:
//   %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
//        "QUAL.OMP.ORDERED"(i32 2, i32 4, i32 2), ...]
//
// Generated IR:
//   %dims.vec = alloca { i64, i64, i64 }, i32 2                         ; (1)
//
//   %3 = getelementptr inbounds { i64, i64, i64 }, %dims.vec, i32 0     ; (2)
//
//   %4 = getelementptr inbounds { i64, i64, i64 }, %3, i32 0, i32 0     ; (3)
//   store i64 0, i64* %4                                                ; (4)
//   %5 = getelementptr inbounds { i64, i64, i64 }, %3, i32 0, i32 1     ; (5)
//   store i64 4, i64* %5                                                ; (6)
//   %6 = getelementptr inbounds { i64, i64, i64 }, %3, i32 0, i32 2     ; (7)
//   store i64 1, i64* %6                                                ; (8)
//
//   %7 = getelementptr inbounds { i64, i64, i64 }, %dims.vec, i32 1     ; (9)
//
//   %8 = getelementptr inbounds { i64, i64, i64 }, %7, i32 0, i32 0     ; (10)
//   store i64 0, i64* %8                                                ; (11)
//   %9 = getelementptr inbounds { i64, i64, i64 }, %7, i32 0, i32 1     ; (12)
//   store i64 2, i64* %9                                                ; (13)
//   %10 = getelementptr inbounds { i64, i64, i64 }, %7, i32 0, i32 2    ; (14)
//   store i64 1, i64* %10                                               ; (15)
//
//   %11 = bitcast { i64, i64, i64 }* %dims.vec to i8*                   ; (16)
//   %tid = load i32, i32* %<tidptr>, align 4                            ; (17)
//   call void @__kmpc_doacross_init({ i32, i32, i32, i32, i8* }* @loc,
//                                   i32 %tid, i32 2, i8* %11)           ; (18)
//
//   The call along with the above initializations of dims.vec, are inserted
//   before InsertPt.
CallInst *
VPOParoptUtils::genKmpcDoacrossInit(WRegionNode *W, StructType *IdentTy,
                                    Value *Tid, Instruction *InsertPt,
                                    const ArrayRef<Value *> &TripCounts) {

  assert(!TripCounts.empty() && "No loop trip counts for doacross loop-nest.");

  IRBuilder<> Builder(InsertPt);

  Type *Int64Ty = Builder.getInt64Ty();
  Value *Zero = Builder.getInt32(0);
  Value *One = Builder.getInt32(1);
  Value *Two = Builder.getInt32(2);

  // The frontend computes doacross source/sink values and trip counts as if
  // the loops in the loop-nest were normalized. So lbound is always 0, and
  // stride 1.
  Value *LBound = Zero;
  Value *Stride = One;
  unsigned NumLoops = TripCounts.size();
  Value *NumLoopsVal = Builder.getInt32(NumLoops);

  // The dims vector needs a struct for each loop in the nest. The struct has 3
  // i64s, which should contain lb, ub and stride respectively.
  StructType *DimsVecTy = VPOParoptUtils::getOrCreateStructType(
      InsertPt->getParent()->getParent(), "__struct.kmp_dim",
      {Int64Ty, Int64Ty, Int64Ty});

  // (1) Create alloca for an array of size NumLoops of the struct.
  AllocaInst *DimsVec =
      Builder.CreateAlloca(DimsVecTy, NumLoopsVal, "dims.vec");

  auto populateDimsVecAtIndex = [&](Value *Base, Value *Index, Value *Val) {
    Value *ValCast = Builder.CreateSExtOrBitCast(Val, Int64Ty);
    Value *DstPtr = Builder.CreateInBoundsGEP(
        Base, {Zero, Index});                     // (3) (5) (7) (10) (12) (14)
    Builder.CreateStore(ValCast, DstPtr);         // (4) (6) (8) (11) (13) (15)
  };

  for (unsigned I = 0; I < NumLoops; ++I) {

    // Since the values for doacross runtime calls are based on normalized loop
    // bounds with lbound 0 and stride 1, trip count of a loop is its ubound.
    Value *UBound = TripCounts[I];

    // First get a handle on the DimsVecTy struct for this loop from the array.
    Value *DimsVecForLoopI =
        Builder.CreateInBoundsGEP(DimsVec, {Builder.getInt32(I)}); //   (2) (9)

    // Store lbound to index 0 of the struct.
    populateDimsVecAtIndex(DimsVecForLoopI, Zero, LBound); // (3) (4) (10) (11)

    // Store ubound to index 1 of the struct.
    populateDimsVecAtIndex(DimsVecForLoopI, One, UBound); //  (5) (6) (12) (13)

    // Store stride to index 2 of the struct.
    populateDimsVecAtIndex(DimsVecForLoopI, Two, Stride); //  (7) (8) (14) (15)
  }

  // Next, we cast our array of structs to I8* before sending it to the runtime.
  Value *DimsVecI8 =
      Builder.CreateBitCast(DimsVec, Builder.getInt8PtrTy()); //           (16)

  // Emit the call to __kmpc_doacross_init.
  CallInst *Call =
      genKmpcCall(W, IdentTy, InsertPt, "__kmpc_doacross_init", nullptr,
                  {Tid, NumLoopsVal, DimsVecI8}); //                  (17) (18)

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
CallInst *VPOParoptUtils::genKmpcDoacrossFini(WRegionNode *W,
                                              StructType *IdentTy, Value *Tid,
                                              Instruction *InsertPt) {

  CallInst *Fini = VPOParoptUtils::genKmpcCall(
      W, IdentTy, InsertPt, "__kmpc_doacross_fini", nullptr, {Tid}, true);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Doacross fini call emitted.\n");

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
                                      ArrayRef<Value *> Args, bool Insert,
                                      bool EnableAtomicReduce) {
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
  if (EnableAtomicReduce)
    Flags |= KMP_IDENT_ATOMIC_REDUCE;

  // Before emitting the KMPC call, we need the Loc information.
  Constant *Loc = genKmpcLocfromDebugLoc(IdentTy, Flags, B, E);
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

  CallInst *CI = genCall(M, IntrinsicName, ReturnTy, FnArgs, InsertPt);
  setFuncCallingConv(CI, M);
  return CI;
}

// Genetates a CallInst for the given FunctionCallee FnC and its argument list.
// FnC is assumed to have a non-null callee.
// If InsertPt!=null, the Call is emitted before InsertPt.
CallInst *VPOParoptUtils::genCall(Module *M,
                                  FunctionCallee FnC,
                                  ArrayRef<Value *> FnArgs,
                                  ArrayRef<Type *> FnArgTypes,
                                  Instruction *InsertPt,
                                  bool IsTail) {
  assert(FnC.getCallee() && "Function callee is null.");
  CallInst *Call = CallInst::Create(FnC, FnArgs, "", InsertPt);
  // Note: if InsertPt!=nullptr, Call is emitted into the IR as well.
  assert(Call && "Failed to generate Function Call");

  if (InsertPt)
    Call->setDebugLoc(InsertPt->getDebugLoc());
  setFuncCallingConv(Call, M);
  Call->setTailCall(IsTail);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Function call: " << *Call << "\n");

  return Call;
}

// Genetates a CallInst for a function with name `FnName`.
// If InsertPt!=null, the Call is emitted before InsertPt.
CallInst *VPOParoptUtils::genCall(Module *M, StringRef FnName, Type *ReturnTy,
                                  ArrayRef<Value *> FnArgs,
                                  ArrayRef<Type *> FnArgTypes,
                                  Instruction *InsertPt, bool IsTail,
                                  bool IsVarArg,
                                  bool AllowMismatchingPointerArgs,
                                  bool EmitErrorOnFnTypeMismatch) {
  assert(M != nullptr && "Module is null.");
  assert(!FnName.empty() && "Function name is empty.");
  assert(FunctionType::isValidReturnType(ReturnTy) && "Invalid Return Type");

  // Create the function type from the return type and argument types.
  FunctionType *NewFnTy = FunctionType::get(ReturnTy, FnArgTypes, IsVarArg);

  // Get the function prototype from the module symbol table. If absent,
  // create and insert it into the symbol table first.
  FunctionCallee FnC = M->getOrInsertFunction(FnName, NewFnTy);
  Value *FnCallee = FnC.getCallee();

  auto NewAndExistingFunctionsDifferOnlyByPointerTypeOfArgs = [&NewFnTy,
                                                               &FnCallee]() {
    Function *ExistingFn =
        dyn_cast_or_null<Function>(FnCallee->stripPointerCasts());
    if (!ExistingFn)
      return false;

    FunctionType *ExistingFnTy = ExistingFn->getFunctionType();
    if (!ExistingFnTy || ExistingFnTy->isVarArg() || NewFnTy->isVarArg())
      return false;

    if (NewFnTy->getNumParams() != ExistingFnTy->getNumParams())
      return false;

    return std::equal(NewFnTy->param_begin(), NewFnTy->param_end(),
                      ExistingFnTy->param_begin(), [](Type *T1, Type *T2) {
                        return (T1 == T2) ||
                               (isa<PointerType>(T1) && isa<PointerType>(T2));
                      });
  };

  if (isa<Function>(FnCallee) ||
      (AllowMismatchingPointerArgs &&
       NewAndExistingFunctionsDifferOnlyByPointerTypeOfArgs()))
    return genCall(M, FnC, FnArgs, FnArgTypes, InsertPt, IsTail);

  std::string Msg =
      ("Function '" + FnName + "' exists, but has an unexpected type.").str();

  if (EmitErrorOnFnTypeMismatch) {
    if (InsertPt) {
      Function *F = InsertPt->getFunction();
      F->getContext().diagnose(DiagnosticInfoUnsupported(*F, Msg));
    } else
      report_fatal_error(Msg);
  }

  llvm_unreachable(Msg.c_str());
}

// A genCall() interface where FunArgTypes is omitted; it will be computed from
// FnArgs.
// If InsertPt!=null, the Call is emitted before InsertPt.
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
// the insertion point. The created call is inserted before InsertPt.
CallInst *VPOParoptUtils::genCall(StringRef FnName, Type *ReturnTy,
                                  ArrayRef<Value*> FnArgs,
                                  ArrayRef<Type*> FnArgTypes,
                                  Instruction *InsertPt, bool IsTail,
                                  bool IsVarArg) {
  assert (InsertPt && "InsertPt is required to find the Module");
  BasicBlock *B = InsertPt->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();

  CallInst *Call = genCall(M, FnName, ReturnTy, FnArgs, FnArgTypes, InsertPt,
                           IsTail, IsVarArg);
  return Call;
}

// Create a variant version of BaseCall using  the same arguments.
// The base and variant functions must have identical signatures.
CallInst *VPOParoptUtils::genVariantCall(CallInst *BaseCall,
                                         StringRef VariantName,
                                         Value *InteropObj,
                                         Instruction *InsertPt, WRegionNode *W,
                                         bool IsTail) {
  assert(BaseCall && "BaseCall is null");

  Module *M = BaseCall->getModule();
  Function *F = BaseCall->getFunction();
  LLVMContext &C = F->getContext();
  Type *Int8PtrTy = Type::getInt8PtrTy(C); // void*

  Type *ReturnTy = BaseCall->getType();
  FunctionType *BaseFnTy = BaseCall->getFunctionType();
  bool IsVarArg = BaseFnTy->isVarArg();

  SmallVector<Value *, 4> FnArgs(BaseCall->arg_operands());

  // When IsVarArg==true, we cannot recreate the FnArgTypes from the FnArgs
  // because there may be more arguments in the call than the formal parameters
  // in the function declaration. Therefore, we have to use BaseFnTy->params()
  // to populate FnArgTypes and call the genCall() version that takes an
  // explicit FnArgTypes.
  SmallVector<Type *, 4> FnArgTypes;
  for (Type *ArgType : BaseFnTy->params()) {
    FnArgTypes.push_back(ArgType);
  }

  if (InteropObj != nullptr) {
    FnArgs.push_back(InteropObj);
    if (!IsVarArg)
      // If not VarArg, then the signature of the variant function has one
      // more parameter (for void *interopObj) than the base function, so
      // we need to update FnArgTypes accordingly.
      FnArgTypes.push_back(Int8PtrTy);
  }

  return genCall(M, VariantName, ReturnTy, FnArgs, FnArgTypes, InsertPt, IsTail,
                 IsVarArg,
                 /*AllowMismatchingPointerArgs=*/true,
                 /*EmitErrorOnFnTypeMismatch=*/true);
}

// Creates a call with no parameters
CallInst *VPOParoptUtils::genEmptyCall(Module *M, StringRef FnName,
                                       Type *ReturnTy, Instruction *InsertPt,
                                       bool IsVarArg) {
  // Create the function type from the return type. The Fn takes no parameters.
  FunctionType *FnTy = FunctionType::get(ReturnTy, IsVarArg);

  // Get the function prototype from the module symbol table. If absent,
  // create and insert it into the symbol table first.
  FunctionCallee FnC = M->getOrInsertFunction(FnName, FnTy);

  CallInst *Call = CallInst::Create(FnC, "", InsertPt);
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

  // TODO: Check if we need to check for Architecture/ OS types here.
  return SmallString<64>(".gomp_critical_user_");
}


// Returns the lock variable to be used in KMPC critical calls.
GlobalVariable *VPOParoptUtils::genKmpcCriticalLockVar(
    WRegionNode *W, const Twine &LockNameSuffix, bool IsTargetSPIRV) {

  assert(W && "WRegionNode is null.");
  unsigned AddressSpace = vpo::ADDRESS_SPACE_PRIVATE;

  if (IsTargetSPIRV) {
    AddressSpace = vpo::ADDRESS_SPACE_GLOBAL;
#if INTEL_CUSTOMIZATION
#if 0
    // FIXME: re-enable this.
    //        WG-local global variables require initialization
    //        for each WG. We have to check if we can find the parent
    //        target (or teams) region and insert the initialization there.
    //        Experimental enabling of this code did not seem to affect
    //        specACCEL performance at all.
    if (isa<WRNDistributeParLoopNode>(W) ||
        isa<WRNParallelLoopNode>(W))
      AddressSpace = vpo::ADDRESS_SPACE_LOCAL;
#endif
#endif  // INTEL_CUSTOMIZATION
  }

  // We first get the lock name prefix for the lock var based on the target.
  SmallString<64> LockName = getKmpcCriticalLockNamePrefix(W);
  LockName += LockNameSuffix.str();

  LockName += ".AS" + std::to_string(AddressSpace) + ".var";

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Lock name:" << LockName << "\n");

  // Now, the type for lock variable is an array of eight 32-bit integers.
  BasicBlock *BB = W->getEntryBBlock();
  assert(BB && "WRegionNode has no Entry BB.");
  Function *F = BB->getParent();
  Module *M = F->getParent();
  LLVMContext &C = M->getContext();

  ArrayType *LockVarTy = ArrayType::get(Type::getInt32Ty(C), 8);

  // See if a lock object already exists, if so, reuse it.
  if (GlobalVariable *GV = M->getGlobalVariable(LockName)) {
    LLVM_DEBUG(dbgs() <<
               __FUNCTION__ << ": Reusing existig lock var: " << *GV << "\n");

    assert(GV->getValueType() == LockVarTy &&
           "Lock variable name conflicts with an existing variable.");
    return GV;
  }

  // Otherwise, Create a new lock object. CommonLinkage is used so that multiple
  // lock variables with the same name (across modules) get merged into a single
  // one at link time.
  //
  // Note that the lock variable must be declared as __global for OpenCL.
  GlobalVariable *GV =
      new GlobalVariable(*M, LockVarTy, false, GlobalValue::CommonLinkage,
                         ConstantAggregateZero::get(LockVarTy), LockName,
                         nullptr, GlobalValue::ThreadLocalMode::NotThreadLocal,
                         AddressSpace);

  assert(GV && "Unable to generate Kmpc critical lock var.");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Lock var generated: " << *GV << "\n");

  return GV;
}

// Generates a critical section around the instructions specified
// by BeginInst and EndInst like this:
//   __kmpc_critical()
//   BeginInst
//   ...
//   __kmpc_end_critical()
//   EndInst
bool VPOParoptUtils::genKmpcCriticalSectionImpl(WRegionNode *W,
                                                StructType *IdentTy,
                                                Constant *TidPtr,
                                                Instruction *BeginInst,
                                                Instruction *EndInst,
                                                GlobalVariable *LockVar,
                                                DominatorTree *DT,
                                                LoopInfo *LI,
                                                bool IsTargetSPIRV) {

  assert(W && "WRegionNode is null.");
  assert(IdentTy && "IdentTy is null.");
  assert(TidPtr && "TidPtr is null.");
  assert(BeginInst && "BeginInst is null.");
  assert(EndInst && "EndInst is null.");
  assert(LockVar && "LockVar is null.");

  auto *RetTy = Type::getVoidTy(BeginInst->getContext());

  StringRef BeginName = "__kmpc_critical";
  CallInst *BeginCritical = nullptr;
  Value *Arg = LockVar;
  Module *M = BeginInst->getModule();

  if (IsTargetSPIRV)
    // OpenCL version of __kmpc_critical takes a generic address space
    // pointer argument.
    Arg = VPOParoptUtils::genAddrSpaceCast(Arg, BeginInst,
                                           vpo::ADDRESS_SPACE_GENERIC);

  if (IsTargetSPIRV)
    BeginCritical = genCall(M, BeginName, RetTy, { Arg });
  else
    BeginCritical =
        genKmpcCallWithTid(W, IdentTy, TidPtr, BeginInst, BeginName,
                           RetTy, { Arg });

  assert(BeginCritical && "Could not call __kmpc_critical");

  StringRef EndName = "__kmpc_end_critical";
  CallInst *EndCritical = nullptr;

  if (IsTargetSPIRV)
    EndCritical = genCall(M, EndName, RetTy, { Arg });
  else
    EndCritical =
        genKmpcCallWithTid(W, IdentTy, TidPtr, EndInst, "__kmpc_end_critical",
                           RetTy, { Arg });

  assert(EndCritical && "Could not call __kmpc_end_critical");

  // Now insert the calls in the IR.
  if (IsTargetSPIRV) {
    // __kmpc_[end_]critical calls must be convergent for SPIR-V targets.
    BeginCritical->getCalledFunction()->setConvergent();
    BeginCritical->setConvergent();
    // Disallow duplicating and merging calls to __kmpc_[end_]critical
    // as well. Basically, we want to disallow any optimizations
    // of these calls.
    BeginCritical->getCalledFunction()->setCannotDuplicate();
    BeginCritical->setCannotDuplicate();
    BeginCritical->setCannotMerge();
    setFuncCallingConv(BeginCritical, M);

    EndCritical->getCalledFunction()->setConvergent();
    EndCritical->setConvergent();
    EndCritical->getCalledFunction()->setCannotDuplicate();
    EndCritical->setCannotDuplicate();
    EndCritical->setCannotMerge();
    setFuncCallingConv(EndCritical, M);
  }

  BeginCritical->insertBefore(BeginInst);
  EndCritical->insertBefore(EndInst);

  if (IsTargetSPIRV) {
    if (!isa<WRNTeamsNode>(W))
      // Critical loop must not be generated for teams region,
      // since it will result in redundant execution and incorrect
      // result. Only the master thread must execute the critical
      // section (the master thread guards will be generated later).
      genCriticalLoopForSPIR(W, BeginCritical, EndCritical, DT, LI);
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Critical Section generated.\n");
  return true;
}

// Generate lane-by-lane loop around BeginInst and EndInst.
bool VPOParoptUtils::genCriticalLoopForSPIRHelper(Instruction *BeginInst,
                                                  Instruction *EndInst,
                                                  DominatorTree *DT,
                                                  LoopInfo *LI) {
  Module *M = BeginInst->getModule();
  // It does not matter, but usually BeginInst is the next node
  // after @__kmpc_critical call, and EndInst is @__kmpc_end_critical
  // call.
  BasicBlock *PreheaderBB = BeginInst->getParent();
  BasicBlock *LoopIncBB = EndInst->getParent();
  assert(PreheaderBB != LoopIncBB &&
         "Begin and end instructions must be in different blocks.");
  // Assertion below will trigger on critical regions with
  // early exits, e.g.:
  //   #pragma omp critical
  //     exit(1);
  // These are not supported for SPIR targets.
  assert((!DT ||
          (DT->getNode(PreheaderBB) && DT->getNode(LoopIncBB) &&
           DT->dominates(BeginInst->getParent(), EndInst->getParent()))) &&
         "Malformed critical region.");
  BasicBlock *HeaderBB = SplitBlock(PreheaderBB, BeginInst, DT, LI);
  BasicBlock *LoopExitBB = SplitBlock(LoopIncBB, EndInst, DT, LI);

  // The current CFG:
  // PreheaderBB:
  //   call spir_func void @__kmpc_critical
  //   br label %HeaderBB
  //
  // HeaderBB:
  //   ...
  //
  // LoopIncBB:
  //   ...
  //   br label %LoopExitBB
  //
  // LoopExitBB:
  //   call spir_func void @__kmpc_end_critical


  // PreheaderBB:
  //   call spir_func void @__kmpc_critical
  //   %0 = call spir_func i32 @_Z18get_sub_group_size
  //   br label %HeaderBB
  IRBuilder<> PreheaderBuilder(PreheaderBB->getTerminator());
  Type *IVTy = PreheaderBuilder.getInt32Ty();
  ConstantInt *ZeroVal = PreheaderBuilder.getInt32(0);
  ConstantInt *OneVal = PreheaderBuilder.getInt32(1);
  CallInst *SGSize = genCall(M, "_Z18get_sub_group_sizev", IVTy, {},
                             &*PreheaderBuilder.GetInsertPoint());
  setFuncCallingConv(SGSize, M);

  // HeaderBB:
  //   %simdlane.id = phi i32 [ 0, %PreheaderBB ]
  //   %exit.pred = icmp uge i32 %simdlane.id, %0
  //   ...
  IRBuilder<> HeaderBuilder(&HeaderBB->front());
  PHINode *IVPHI = HeaderBuilder.CreatePHI(IVTy, 2, "simdlane.id");
  IVPHI->addIncoming(ZeroVal, PreheaderBB);
  Value *ExitPred = HeaderBuilder.CreateICmpUGE(IVPHI, SGSize, "exit.pred");

  // PreheaderBB:
  //   call spir_func void @__kmpc_critical
  //   %0 = call spir_func i32 @_Z18get_sub_group_size
  //   br label %HeaderBB
  //
  // HeaderBB:
  //   %simdlane.id = phi i32 [ 0, %PreheaderBB ]
  //   %exit.pred = icmp uge i32 %simdlane.id, %0
  //   br i1 %exit.pred, label %JumpToExitBB, label %LoopBody
  //
  // JumpToExitBB:
  //   br label %LoopExitBB
  //
  // LoopBody:
  //   ...
  //
  // LoopIncBB:
  //   ...
  //   br label %LoopExitBB
  //
  // LoopExitBB:
  //   call spir_func void @__kmpc_end_critical
  Instruction *SplitPt = &*HeaderBuilder.GetInsertPoint();
  Instruction *ThenTerm =
      SplitBlockAndInsertIfThen(ExitPred, SplitPt,
                                /*Unreachable=*/false,
                                /*BranchWeights=*/nullptr,
                                DT, LI);
  BasicBlock *JumpToExitBB = ThenTerm->getParent();
  ThenTerm->setSuccessor(0, LoopExitBB);
  if (DT)
    DT->changeImmediateDominator(LoopExitBB, HeaderBB);

  BasicBlock *LoopBodyBB = SplitPt->getParent();
  Instruction *LoopBodyInsertPt = &LoopBodyBB->front();
  // LoopBody:
  //   %1 = call spir_func i32 @_Z22get_sub_group_local_id
  //   %skip.pred = icmp ne i32 %simdlane.id, %1
  //   ...
  CallInst *SGLocalId =
      genCall(M, "_Z22get_sub_group_local_idv", IVTy, {}, LoopBodyInsertPt);
  setFuncCallingConv(SGLocalId, M);
  Value *SkipPred = new ICmpInst(LoopBodyInsertPt, ICmpInst::ICMP_NE,
                                 IVPHI, SGLocalId, "skip.pred");

  // PreheaderBB:
  //   call spir_func void @__kmpc_critical
  //   %0 = call spir_func i32 @_Z18get_sub_group_size
  //   br label %HeaderBB
  //
  // HeaderBB:
  //   %simdlane.id = phi i32 [ 0, %PreheaderBB ]
  //   %exit.pred = icmp uge i32 %simdlane.id, %0
  //   br i1 %exit.pred, label %JumpToExitBB, label %LoopBody
  //
  // JumpToExitBB:
  //   br label %LoopExitBB
  //
  // LoopBody:
  //   %1 = call spir_func i32 @_Z22get_sub_group_local_id
  //   %skip.pred = icmp ne i32 %simdlane.id, %1
  //   br i1 %skip.pred, label %JumpToIncBB, label %CritBody
  //
  // JumpToIncBB:
  //   br label %LoopIncBB
  //
  // CritBody:
  //   ...
  //
  // LoopIncBB:
  //   ...
  //   br label %LoopExitBB
  //
  // LoopExitBB:
  //   call spir_func void @__kmpc_end_critical
  ThenTerm = SplitBlockAndInsertIfThen(SkipPred, LoopBodyInsertPt,
                                       /*Unreachable=*/false,
                                       /*BranchWeights=*/nullptr,
                                       DT, LI);
  ThenTerm->setSuccessor(0, LoopIncBB);
  if (DT)
    DT->changeImmediateDominator(LoopIncBB, LoopBodyBB);

  // PreheaderBB:
  //   call spir_func void @__kmpc_critical
  //   %0 = call spir_func i32 @_Z18get_sub_group_size
  //   br label %HeaderBB
  //
  // HeaderBB:
  //   %simdlane.id = phi i32 [ 0, PreheaderBB ], [ %2, %LoopIncBB ]
  //   %exit.pred = icmp uge i32 %simdlane.id, %0
  //   br i1 %exit.pred, label %JumpToExitBB, label %LoopBody
  //
  // JumpToExitBB:
  //   br label %LoopExitBB
  //
  // LoopBody:
  //   %1 = call spir_func i32 @_Z22get_sub_group_local_id
  //   %skip.pred = icmp ne i32 %simdlane.id, %1
  //   br i1 %skip.pred, label %JumpToIncBB, label %CritBody
  //
  // JumpToIncBB:
  //   br label %LoopIncBB
  //
  // CritBody:
  //   ...
  //
  // LoopIncBB:
  //   ...
  //   %2 = add nuw nsw i32 %simdlane.id, 1
  //   br label %HeaderBB
  //
  // LoopExitBB:
  //   call spir_func void @__kmpc_end_critical
  IRBuilder<> IncBuilder(LoopIncBB->getTerminator());
  Value *Increment =
      IncBuilder.CreateAdd(IVPHI, OneVal, "simdlane.id.inc", true, true);
  IVPHI->addIncoming(Increment, LoopIncBB);
  assert(LoopIncBB->getTerminator()->getNumSuccessors() == 1 &&
         "Unexpected number of successors for LoopIncBB.");
  LoopIncBB->getTerminator()->setSuccessor(0, HeaderBB);
  // JumpToExitBB is now the only predecessor of LoopExitBB,
  // and also its immediate dominator.
  if (DT)
    DT->changeImmediateDominator(LoopExitBB, JumpToExitBB);

#ifndef NDEBUG
  // Verify DominatorTree before proceeding further.
  assert((!DT || DT->verify()) && "DominatorTree is invalid.");
#endif  // NDEBUG

  if (!LI)
    return true;

  // Keep loops structure for the current region valid,
  // otherwise, the loop rotation for the loop associated
  // with this region may fail later.

  // Collect blocks for the newly created loop.
  SmallVector<BasicBlock *, 32> WorkList{HeaderBB};
  SmallPtrSet<BasicBlock *, 32> Visited;
  Loop *ParentLoop = LI->getLoopFor(LoopExitBB);
  SmallPtrSet<Loop *, 4> NewChildren;
  while (!WorkList.empty()) {
    BasicBlock *BB = WorkList.pop_back_val();
    Visited.insert(BB);

    // Collect the "top-most" loops that are fully contained
    // within the blocks wrapped into the critical loop.
    // These loops must become children of the critical loop.
    Loop *BBLoop = LI->getLoopFor(BB);
    if (BBLoop && BBLoop->getParentLoop() == ParentLoop)
      NewChildren.insert(BBLoop);

    // If the assertion triggers, this probably means we
    // escaped the critical section somehow.
    assert(BB->getTerminator()->getNumSuccessors() > 0 &&
           "Block inside critical section has zero successors.");
    for (auto *SBB : successors(BB))
      if (SBB != JumpToExitBB && Visited.count(SBB) == 0)
        WorkList.push_back(SBB);
  }

  Loop *NewLoop = LI->AllocateLoop();
  if (ParentLoop) {
    // The new loop must become a parent for all NewChildren loops,
    // and the new loop's parent is the ParentLoop.
    ParentLoop->addChildLoop(NewLoop);
    for (auto *L : NewChildren)
      NewLoop->addChildLoop(ParentLoop->removeChildLoop(L));
  } else {
    // The new loop must become a top level loop, and all NewChildren
    // loops must be its children (i.e. they must be removed from
    // the set of top level loops).
    LI->addTopLevelLoop(NewLoop);

    for (auto *L : NewChildren) {
      auto *NewChild = LI->removeLoop(llvm::find(*LI, L));
      NewLoop->addChildLoop(NewChild);
    }
  }

  for (auto *BB : Visited) {
    NewLoop->addBlockEntry(BB);
    if (LI->getLoopFor(BB) == ParentLoop)
      LI->changeLoopFor(BB, NewLoop);
  }
  NewLoop->moveToHeader(HeaderBB);

  return true;
}

// Generate the serialization loop for code inside a critical section.
bool VPOParoptUtils::genCriticalLoopForSPIR(WRegionNode *W,
                                            CallInst *BeginCritical,
                                            CallInst *EndCritical,
                                            DominatorTree *DT,
                                            LoopInfo *LI) {
  assert(BeginCritical && EndCritical && "Invalid begin and end instructions.");

  Module *M = BeginCritical->getModule();
  std::pair<CallInst *, CallInst *> Regions[2] = {{BeginCritical, EndCritical}};

  // We will need to split the block *before* EndCritical, because it will be
  // an exit point for the get_sub_group_size() loop. We may also need to
  // split the block *after* EndCritical to use singleRegionMultiVersioning().
  // In both cases, a region exit directive may be located in the same block,
  // so we will have to update it. Let's just forcefully split the block
  // after EndCritical now, update the region's exit block and do not care
  // about it for further splits.
  BasicBlock *EndCriticalBB = EndCritical->getParent();
  BasicBlock *NewExitBB = SplitBlock(
      EndCriticalBB, EndCritical->getNextNonDebugInstruction(), DT, LI);
  if (EndCriticalBB == W->getExitBBlock())
    W->setExitBBlock(NewExitBB);

  if (SPIRVTargetHasEUFusion) {
    BasicBlock *CondBB = BeginCritical->getParent();
    BasicBlock *BeginCriticalBB = SplitBlock(CondBB, BeginCritical, DT, LI);

    ValueToValueMapTy VMap;
    SmallVector<BasicBlock *, 32> BBSet;
    IRBuilder<> CondBuilder(CondBB->getTerminator());
    CallInst *SGId = genCall(M, "_Z16get_sub_group_idv",
                             CondBuilder.getInt32Ty(), {},
                             &*CondBuilder.GetInsertPoint());
    setFuncCallingConv(SGId, M);
    Value *Cond = CondBuilder.CreateTrunc(
        SGId, CondBuilder.getInt1Ty(), "sub_group_id_parity");
    // TODO: since the odd and even branches are identical, we need
    //       to do something to avoid merging them into one sequence
    //       by further optimizations.
    VPOUtils::singleRegionMultiVersioning(
        BeginCriticalBB, EndCriticalBB, BBSet, VMap, Cond, DT, LI);

    CallInst *NewBeginCritical = cast<CallInst>(VMap[BeginCritical]);
    CallInst *NewEndCritical = cast<CallInst>(VMap[EndCritical]);
    Regions[1] = {NewBeginCritical, NewEndCritical};
  }

  for (int I = 0; I < 2; ++I) {
    CallInst *CurBeginCritical = Regions[I].first;
    if (!CurBeginCritical)
      continue;

    Instruction *BeginInst = CurBeginCritical->getNextNonDebugInstruction();
    assert(BeginInst && "No instructions after __kmpc_critical.");
    Instruction *EndInst = Regions[I].second;

    VPOParoptUtils::genCriticalLoopForSPIRHelper(BeginInst, EndInst, DT, LI);
  }

#ifndef NDEBUG
#if 0
  // CodeExtractor does not preserve LoopInfo, so verification may fail.
  // For example:
  //   #pragma omp target parallel for
  //     for (...) {}
  //   #pragma omp target parallel for
  //     for (...) {}
  // The LoopInfo will invalid after the first target region is outlined.
  //
  // FIXME: pass LoopInfo to CodeExtractor and update it there.
  if (LI && DT)
    LI->verify(*DT);
#endif
#endif  // NDEBUG

  return true;
}

// Generates a critical section around Instructions `BeginInst` and `Endinst`,
// by emitting calls to `__kmpc_critical` before `BeginInst`, and
// `__kmpc_end_critical` after `EndInst`.
bool VPOParoptUtils::genKmpcCriticalSection(WRegionNode *W, StructType *IdentTy,
                                            Constant *TidPtr,
                                            Instruction *BeginInst,
                                            Instruction *EndInst,
                                            DominatorTree *DT,
                                            LoopInfo *LI,
                                            bool IsTargetSPIRV,
                                            const Twine &LockNameSuffix) {
  assert(W != nullptr && "WRegionNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(BeginInst != nullptr && "BeginInst is null.");
  assert(EndInst != nullptr && "EndInst is null.");

  // Generate the Lock object for critical section.
  GlobalVariable *Lock =
      genKmpcCriticalLockVar(W, LockNameSuffix, IsTargetSPIRV);
  assert(Lock != nullptr && "Could not create critical section lock variable.");

  return genKmpcCriticalSectionImpl(W, IdentTy, TidPtr, BeginInst, EndInst,
                                    Lock, DT, LI, IsTargetSPIRV);
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

void VPOParoptUtils::insertCallsAtRegionBoundary(WRegionNode *W,
                                                 CallInst *Call1,
                                                 CallInst *Call2,
                                                 bool InsideRegion) {
  assert(W && "WRegionNode is null.");
  assert(Call1 && "Call1 is null.");
  assert(Call2 && "Call2 is null.");

  Instruction *EntryDir = W->getEntryDirective();
  Instruction *ExitDir = W->getExitDirective();
  assert(EntryDir && ExitDir && "Null Entry/Exit directive.");

  if (InsideRegion) {
    Call1->insertAfter(EntryDir);
    Call2->insertBefore(ExitDir);
  } else {
    Call1->insertBefore(EntryDir);
    Call2->insertAfter(ExitDir);
  }
}

std::pair<CallInst *, CallInst *>
VPOParoptUtils::genKmpcSpmdPushPopNumThreadsCalls(Module *M,
                                                  Value *NumThreads) {
  assert(M && "Module is null.");

  LLVMContext &C = M->getContext();
  auto *VoidTy = Type::getVoidTy(C);
  auto *RetTy = VoidTy;

  if (!NumThreads)
    NumThreads = ConstantInt::get(Type::getInt32Ty(C), 1);

  CallInst *PushCall =
      genCall(M, "__kmpc_spmd_push_num_threads", RetTy, {NumThreads});
  assert(PushCall && "Could not emit __kmpc_spmd_push_num_threads");

  CallInst *PopCall = genCall(M, "__kmpc_spmd_pop_num_threads", RetTy, {});
  assert(PopCall && "Could not emit __kmpc_spmd_pop_num_threads");

  PushCall->getCalledFunction()->setConvergent();
  setFuncCallingConv(PushCall, M);
  PopCall->getCalledFunction()->setConvergent();
  setFuncCallingConv(PopCall, M);

  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Push/Pop num_threads calls generated.\n");
  return {PushCall, PopCall};
}

// Emit Constructor Call and insert it via created IRBuilder
void VPOParoptUtils::genConstructorCall(Function *Ctor, Value *V,
                                        IRBuilder<> &Builder) {
  if (Ctor == nullptr)
    return;

  FunctionType *FnTy = Ctor->getFunctionType();
  Type *ArgTy = FnTy->getParamType(0);
  Type *ValType = V->getType();
  if (ArgTy != ValType)
    V = Builder.CreateBitCast(V, ArgTy);
  CallInst *Call = genCall(Ctor->getParent(), Ctor, {V}, {ArgTy}, nullptr);
  Builder.Insert(Call);

  BasicBlock::iterator InsertPt = Builder.GetInsertPoint();
  if (Builder.GetInsertBlock()->end() != InsertPt)
    Call->setDebugLoc((&*InsertPt)->getDebugLoc());
  LLVM_DEBUG(dbgs() << "CONSTRUCTOR: " << *Call << "\n");
}

// Emit Constructor call and insert it after PrivAlloca
CallInst *VPOParoptUtils::genConstructorCall(Function *Ctor, Value *V,
                                             Value* PrivAlloca) {
  if (Ctor == nullptr)
    return nullptr;

  Type *ValType = V->getType();
  CallInst *Call = genCall(Ctor->getParent(), Ctor, {V}, {ValType}, nullptr);
  Instruction *InsertAfterPt = cast<Instruction>(PrivAlloca);
  Call->insertAfter(InsertAfterPt);
  Call->setDebugLoc(InsertAfterPt->getDebugLoc());
  LLVM_DEBUG(dbgs() << "CONSTRUCTOR: " << *Call << "\n");
  return Call;
}

// Emit Destructor call and insert it before InsertBeforePt
CallInst *VPOParoptUtils::genDestructorCall(Function *Dtor, Value *V,
                                            Instruction *InsertBeforePt) {
  if (Dtor == nullptr)
    return nullptr;

  Type *ArgTy = Dtor->getFunctionType()->getParamType(0);
  Type *ValType = V->getType();
  if (ArgTy != ValType) {
    IRBuilder<> Builder(InsertBeforePt);
    V = Builder.CreateBitCast(V, ArgTy);
    ValType = ArgTy;
  }
  CallInst *Call = genCall(Dtor->getParent(), Dtor, {V}, {ValType}, nullptr);
  Call->insertBefore(InsertBeforePt);
  Call->setDebugLoc(InsertBeforePt->getDebugLoc());
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

  CallInst *Call =
      genCall(Cctor->getParent(), Cctor, {D,S}, {DTy, STy}, nullptr);
  Call->insertBefore(InsertBeforePt);
  Call->setDebugLoc(InsertBeforePt->getDebugLoc());
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

  CallInst *Call = genCall(Cp->getParent(), Cp, {D,S}, {DTy, STy}, nullptr);
  Call->insertBefore(InsertBeforePt);
  Call->setDebugLoc(InsertBeforePt->getDebugLoc());
  LLVM_DEBUG(dbgs() << "COPY ASSIGN: " << *Call << "\n");
  return Call;
}

// Generate a private variable version for an array of Type ElementType,
// size NumElements, and name VarName.
Value *VPOParoptUtils::genPrivatizationAlloca(
    Type *ElementType, Value *NumElements, MaybeAlign OrigAlignment,
    Instruction *InsertPt, bool IsTargetSPIRV, const Twine &VarName,
    llvm::Optional<unsigned> AllocaAddrSpace,
    llvm::Optional<unsigned> ValueAddrSpace, AllocateItem *AllocItem) {
  assert(ElementType && "Null element type.");
  assert(InsertPt && "Null insertion anchor.");

  Module *M = InsertPt->getModule();
  const DataLayout &DL = M->getDataLayout();
  IRBuilder<> Builder(InsertPt);

  auto AddrSpaceCastValue = [IsTargetSPIRV](
      IRBuilder<> &Builder, Value *V, PointerType *T) {
    unsigned SrcAS = cast<PointerType>(V->getType())->getAddressSpace();
    unsigned DstAS = T->getAddressSpace();

    if (VPOParoptUtils::areCompatibleAddrSpaces(SrcAS, DstAS, IsTargetSPIRV))
      return Builder.CreateAddrSpaceCast(V, T, V->getName() + ".ascast");

    // By returning a value of incompatible addrspace here we let
    // the uses relinking code know that more fixups have to be done
    // during the relinking.
    return V;
  };

  assert((!AllocaAddrSpace ||
          AllocaAddrSpace.getValue() == vpo::ADDRESS_SPACE_PRIVATE ||
          AllocaAddrSpace.getValue() == vpo::ADDRESS_SPACE_GLOBAL ||
          AllocaAddrSpace.getValue() == vpo::ADDRESS_SPACE_LOCAL) &&
         "Address space of an alloca may be either global, local or private.");
  assert(DL.getAllocaAddrSpace() == vpo::ADDRESS_SPACE_PRIVATE &&
         "Default alloca address space does not match "
         "vpo::ADDRESS_SPACE_PRIVATE.");

  if (AllocaAddrSpace &&
      (AllocaAddrSpace.getValue() == vpo::ADDRESS_SPACE_LOCAL ||
       AllocaAddrSpace.getValue() == vpo::ADDRESS_SPACE_GLOBAL)) {
    // OpenCL __local/__global variables are globalized even when declared
    // inside a kernel.
    SmallString<64> GlobalName;
    if (AllocaAddrSpace.getValue() == vpo::ADDRESS_SPACE_LOCAL)
      (VarName + Twine(".__local")).toStringRef(GlobalName);
    else
      (VarName + Twine(".__global")).toStringRef(GlobalName);

    GlobalVariable *GV =
       new GlobalVariable(*M, ElementType, false, GlobalValue::InternalLinkage,
                          Constant::getNullValue(ElementType), GlobalName,
                          nullptr,
                          GlobalValue::ThreadLocalMode::NotThreadLocal,
                          AllocaAddrSpace.getValue());
     GV->setAlignment(OrigAlignment);

    if (!ValueAddrSpace)
      return GV;

    return AddrSpaceCastValue(
        Builder, GV, ElementType->getPointerTo(ValueAddrSpace.getValue()));
  }


  uint64_t ConstNumElements = 0;
  if (auto *CI = dyn_cast_or_null<ConstantInt>(NumElements)) {
    // TODO: use ConstantFoldInstruction to discover more constant values.
    ConstNumElements = CI->getZExtValue();
    assert(ConstNumElements > 0 && "Invalid size for new alloca.");
  }

  // If AllocItem is not null, then call omp_alloc(Size, AllocHandle) to
  // allocate memory according to the allocate clause.
  // This clause is currently not supported for spir64 target compilation.
  if (!IsTargetSPIRV && AllocItem) {
    Value *AllocHandle = AllocItem->getAllocator();
    Value *ElementSize = Builder.getIntN(DL.getPointerSizeInBits(),
                                         DL.getTypeSizeInBits(ElementType) / 8);
    Value *TotalSize = ElementSize;
    if (NumElements != nullptr && ConstNumElements != 1)
      // If the above condition is true then the number of elements may be >1
      // so we compute TotalSize = NumElements * ElementSize.
      TotalSize = Builder.CreateMul(NumElements, ElementSize);

    CallInst *AllocCall = genOmpAlloc(TotalSize, AllocHandle, InsertPt);
    Value *AllocCast = Builder.CreateBitCast(AllocCall, ElementType->getPointerTo());
    AllocCast->setName(VarName);
    return AllocCast;
  }

  // Some targets do not support array allocas, so we have to
  // generate new alloca instruction of an array type to overcome that.
  // For example, if the requested privatization allocation is
  // 10 elements of type i32, then we will generate:
  //   %0 = alloca [10 x i32], instead of
  //   %0 = alloca i32, i64 10
  //
  // To make the resulting privatization value's type consistent
  // with the expectations, we have to create an additional GEP
  // with result type i32*:
  //   %1 = getelementptr inbounds ([10 x i32], [10 x i32]* %0, i32 0, i32 0)
  bool MimicArrayAllocation = false;

  if (ConstNumElements > 0) {
    // NumElements is a constant
    ElementType = ArrayType::get(ElementType, ConstNumElements);
    NumElements = nullptr;
    MimicArrayAllocation = true;
  }

  auto *AI = Builder.CreateAlloca(
      ElementType,
      AllocaAddrSpace ?
          AllocaAddrSpace.getValue() : DL.getAllocaAddrSpace(),
      NumElements, VarName);
  AI->setAlignment(OrigAlignment.getValueOr(DL.getPrefTypeAlign(ElementType)));

  if (IsTargetSPIRV && AI->isArrayAllocation()) {
    LLVM_DEBUG(dbgs() <<
               "Requested privatization alloca with non-constant size:\n" <<
               "\tElementType:\n" << *AI->getAllocatedType() << "\n" <<
               "\tSize:\n" << *AI->getArraySize() << "\n");
#if INTEL_CUSTOMIZATION
#if 0
    // FIXME: either re-enable this or come up with a solution.
    report_fatal_error("VLA alloca is not supported for this target.");
#endif
#endif  // INTEL_CUSTOMIZATION
  }

  Value *V = AI;
  if (MimicArrayAllocation) {
    assert(AI->getAllocatedType()->isArrayTy() &&
           "Expected ArrayType alloca.");
    assert(!AI->isArrayAllocation() &&
           "Unexpected array alloca with array type.");
    V = Builder.CreateInBoundsGEP(
        AI, {Builder.getInt32(0), Builder.getInt32(0)},
        AI->getName() + Twine(".gep"));
  }

  if (!ValueAddrSpace)
    return V;

  // TODO: OPAQUEPOINTER: Use the appropriate API for getting PointerType to a
  // specific AddressSpace. The API currently needs the Element Type as well.
  auto *CastTy = V->getType()->getPointerElementType()->
      getPointerTo(ValueAddrSpace.getValue());
  auto *ASCI = dyn_cast<Instruction>(
      AddrSpaceCastValue(Builder, V, CastTy));

  assert(ASCI && "genPrivatizationAlloca: AddrSpaceCast for an AllocaInst "
         "must be an Instruction.");

  return ASCI;
}

bool VPOParoptUtils::areCompatibleAddrSpaces(
    unsigned AS1, unsigned AS2, bool IsTargetSPIRV) {
  assert((IsTargetSPIRV || (AS1 == AS2 && AS1 == 0)) &&
         "Non-default address space used for non-SPIR target.");

  if (AS1 == AS2)
    return true;

  // AS1 is not equal to AS2 here.

  // All address spaces, except ADDRESS_SPACE_CONSTANT,
  // may only be casted to ADDRESS_SPACE_GENERIC and vice versa.
  if (AS1 == vpo::ADDRESS_SPACE_CONSTANT ||
      AS2 == vpo::ADDRESS_SPACE_CONSTANT)
    return false;

  if (AS1 == vpo::ADDRESS_SPACE_GENERIC ||
      AS2 == vpo::ADDRESS_SPACE_GENERIC)
    return true;

  return false;
}

// Computes the OpenMP loop upper bound so that the iteration space can be
// closed interval.
Value *VPOParoptUtils::computeOmpUpperBound(
    WRegionNode *W, unsigned Idx, Instruction* InsertPt, const Twine &Name) {
  assert(W->getIsOmpLoop() && "computeOmpUpperBound: not a loop-type WRN");

  IRBuilder<> Builder(InsertPt);
  auto &RegionInfo = W->getWRNLoopInfo();
  auto *NormUB = RegionInfo.getNormUB(Idx);

  assert(NormUB && GeneralUtils::isOMPItemLocalVAR(NormUB) &&
         "computeOmpUpperBound: Expect isOMPItemLocalVAR().");

  auto *NormUBAlloca = cast<Instruction>(NormUB);
  assert(isa<PointerType>(NormUBAlloca->getType()) &&
         NormUBAlloca->getType()->getPointerElementType()->isIntegerTy() &&
         "Normalized upper bound must have an integer type.");

  return Builder.CreateLoad(
      NormUBAlloca->getType()->getPointerElementType(), NormUBAlloca,
      ".norm.ub" + Name);
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
                                                     unsigned Idx,
                                                     Value *UB,
                                                     Instruction* InsertPt) {

  assert(W->getIsOmpLoop() && "computeOmpUpperBound: not a loop-type WRN");

  Loop *L = W->getWRNLoopInfo().getLoop(Idx);
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
  }

  if (SExtInst *SI = dyn_cast<SExtInst>(V)) {
    ChainToBase.push_back(SI);
    return findChainToLoad(SI->getOperand(0), ChainToBase);
  }

  if (ZExtInst *ZI = dyn_cast<ZExtInst>(V)) {
    ChainToBase.push_back(ZI);
    return findChainToLoad(ZI->getOperand(0), ChainToBase);
  }

  if (LoadInst *LI = dyn_cast<LoadInst>(V)) {
    ChainToBase.push_back(LI);
    return LI;
  }

  // When the original loop UB is unsigned in the source, Clang may emit IR
  // where the UB is an add (eg: %add4 = add i32 %11, 1). We need to handle
  // such cases.
  if (auto *AddInst = dyn_cast<Instruction>(V))
    if (AddInst->getOpcode() == Instruction::Add) {
      auto *Opnd0 = AddInst->getOperand(0);
      auto *Opnd1 = AddInst->getOperand(1);
      if (!isa<Constant>(Opnd0) && isa<Constant>(Opnd1)) {
        ChainToBase.push_back(AddInst);
        return findChainToLoad(Opnd0, ChainToBase);
      }
    }

  llvm_unreachable("findChainToLoad: unhandled instruction");
}

// Check if the given value may be cloned before the given region.
// This method does not do all the necessary checks to guarantee
// cloneability in general. It works correctly only for a special
// case of values of loops' upper bounds originating from normalized
// upper bounds values produced by FE, for example:
//   [ "DIR.OMP.TARGET"(), "QUAL.OMP.FIRSTPRIVATE"(i32 *%omp.ub) ]
//   [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32 *%omp.ub) ]
//   [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.UB"(i32 *%omp.ub) ]
//   %1 = load i32, i32* %omp.ub
//   ...
//   <use of %1 as a loop upper bound>
//
// We start from the use of %1, and trace it back to the load instruction
// (we do allow some intermediate instructions). We check if the load's
// address is firstprivate() to "omp target" region, and return true
// in this case, otherwise we return false.
//
// Note that this is not enough in general, e.g.:
//   [ "DIR.OMP.TARGET"(), "QUAL.OMP.FIRSTPRIVATE"(i32 *%x) ]
//   [ "DIR.OMP.TEAMS"(), "QUAL.OMP.FIRSTPRIVATE"(i32 *%x) ]
//   [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 *%x) ]
//   store i32 %val, i32* %x
//   %1 = load i32, i32* %x
//   ...
//   <use of %1>
//
// We cannot legally rematerialize value of %1 before any of the regions,
// since its value is changed by the intervening store instruction.
bool VPOParoptUtils::mayCloneUBValueBeforeRegion(
    Value *V, const WRegionNode *W) {
  if (isa<Constant>(V))
    return true;

  if (!W->canHaveFirstprivate())
    return false;

  SmallVector<Instruction *, 3> ChainToBase;
  auto *LI = cast<LoadInst>(findChainToLoad(V, ChainToBase));
  auto *ChainBase = LI->getPointerOperand();

  if (!std::any_of(W->getFpriv().items().begin(),
                   W->getFpriv().items().end(),
                   [ChainBase] (FirstprivateItem *FprivI) {
                     if (ChainBase == FprivI->getOrig())
                       return true;
                     return false;
                   }))
    return false;

  return true;
}

Instruction *VPOParoptUtils::getInsertionPtForAllocas(
    WRegionNode *W, Function *F, bool OutsideRegion) {
  assert(W && "Null WRegionNode.");
  assert(F && "Null Function.");

  auto *WTemp = W;

  if (OutsideRegion)
    WTemp = WTemp->getParent();

  while (WTemp && !WTemp->needsOutlining())
    WTemp = WTemp->getParent();

  if (!WTemp)
    return F->getEntryBlock().getFirstNonPHI();

  if (WTemp == W) {
    assert(!OutsideRegion && "Allocas requested inside region.");
    // Allocas will be inserted inside the region.
    // If they are used to relink some uses inside the region,
    // we have to make sure that the allocas dominate all
    // current instructions in the region. So we want to insert
    // them right before the region's entry directive.
    return WTemp->getEntryDirective();
  }

  BasicBlock *BB = WTemp->getEntryBBlock()->getSingleSuccessor();
  assert(BB && "Couldn't find single successor of the region entry block.");

  return BB->getFirstNonPHI();
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
Value *VPOParoptUtils::genArrayLength(Value *AI, Value *BaseAddr,
                                      Instruction *InsertPt,
                                      IRBuilder<> &Builder, Type *&ElementTy,
                                      Value *&ArrayBegin) {
  // FIXME: we can probably gather the type information from
  //        BaseAddr, and do not pass AI at all.
  assert(GeneralUtils::isOMPItemLocalVAR(AI) &&
         "genArrayLength: Expect isOMPItemLocalVAR().");

  Type *AllocaTy = nullptr;
  Value *NumElements = nullptr;
  Type *AIElemType = AI->getType()->getPointerElementType();
  std::tie(AllocaTy, NumElements) =
      GeneralUtils::getOMPItemLocalVARPointerTypeAndNumElem(AI, AIElemType);
  assert(AllocaTy && "genArrayLength: item type cannot be deduced.");

  // TODO: NumElements??
  Type *ScalarTy = AllocaTy->getScalarType();
  SmallVector<llvm::Value *, 8> GepIndices;
  ArrayType *ArrTy = dyn_cast<ArrayType>(ScalarTy);
  uint64_t CountFromCLAs = 1;
  ConstantInt *Zero = Builder.getInt32(0);

  if (ArrTy != nullptr) {
    GepIndices.push_back(Zero);

    ArrayType *ArrayT = ArrTy;
    while (ArrayT) {
      GepIndices.push_back(Zero);
      CountFromCLAs *= ArrayT->getNumElements();
      ElementTy = ArrayT->getElementType();
      ArrayT = dyn_cast<ArrayType>(ElementTy);
    }
    NumElements = Builder.getInt32(CountFromCLAs);
  } else {
    // For VLA, NumElements is computation result of array length, so we don't
    // need to compute it here.
    assert(!(NumElements == nullptr || isa<ConstantInt>(NumElements)) &&
           "Expect variable length array.");
    ElementTy = ScalarTy;
    GepIndices.push_back(Zero);
  }

  ArrayBegin = Builder.CreateInBoundsGEP(BaseAddr, GepIndices, "array.begin");

  return NumElements;
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
    return ConstantVector::getSplat(ElementCount::getFixed(VTy->getNumElements()),
                                    MinMaxVal);
  return MinMaxVal;
}

Value *VPOParoptUtils::genAddrSpaceCast(Value *Ptr, Instruction *InsertPt,
                                        unsigned AddrSpace) {
  PointerType *PtType = cast<PointerType>(Ptr->getType());
  IRBuilder<> Builder(InsertPt);
  // TODO: OPAQUEPOINTER: Use the appropriate API for getting PointerType to a
  // specific AddressSpace. The API currently needs the Element Type as well.
  Value *RetVal = Builder.CreatePointerBitCastOrAddrSpaceCast(
      Ptr, PtType->getPointerElementType()->getPointerTo(AddrSpace));

  return RetVal;
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

// Break edges from "Src" BasicBlock to "Dst" BasicBlock. e.g.
//---------------------------------------+-------------------------------------
//        Before                         |  After
//---------------------------------------+-------------------------------------
// src:     ; no predecessors            | src:     ; no predecessors
//   ...                                 |   ...
//   br label %dst                       | unreachable             ; (1)
//                                       |
//                                       |
// dst:     ; preds: %src, %other, ...   | dst:     ; preds: %other, ...
//                                       |
//   %p1 = phi i8 [0, %src ],            |  %p1 = phi i8           ; (2)
//                [1, %other], ...       |                [1, %other], ...
//                                       |
//---------------------------------------+-------------------------------------
//        Before                         |  After
//---------------------------------------+-------------------------------------
// src:     ; no predecessors            | src:     ; no predecessors
//   ...                                 |   ...
//   br i1 %x, label %dst, label %y      |   br label %y           ; (3)
//                                       |
// dst:     ; preds: %src, %other, ...   | dst:     ; preds: %other, ...
//   ...                                 |   ...
//                                       |
static void BreakEdge(BasicBlock *Src, BasicBlock *Dst, DominatorTree *DT) {
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Breaking edge %" << Src->getName()
                    << " -> %" << Dst->getName() << "\n");
  IRBuilder<> Builder(Src->getContext());
  auto RemoveEdgeFromConditionalBranch = [&](Instruction *I) {
    auto *Branch = dyn_cast<BranchInst>(I);
    if (!Branch || Branch->isUnconditional())
      return false;
    Builder.SetInsertPoint(Src);

    BasicBlock *OtherSuccessor = Branch->getSuccessor(0) == Dst
                                     ? Branch->getSuccessor(1)
                                     : Branch->getSuccessor(0);
    Instruction *NewBranch = Builder.CreateBr(OtherSuccessor); //    (3)
    NewBranch->setDebugLoc(Branch->getDebugLoc());
    Branch->eraseFromParent();
    return true;
  };

  Instruction *Terminator = Src->getTerminator();
  if (!RemoveEdgeFromConditionalBranch(Terminator)) {
    Terminator->eraseFromParent();
    Builder.SetInsertPoint(Src);
    Builder.CreateUnreachable(); //                                  (1)
  }

  // Fix any phis at the inside/outside join.
  for (PHINode &PN : Dst->phis())
    if (PN.getBasicBlockIndex(Src) >= 0) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Updating PHI: " << PN << "\n");
      PN.removeIncomingValue(Src); //                                  (2)
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Updated PHI: " << PN << "\n");
    }

  // Fix the dom tree after breaking the edge.
  DT->deleteEdge(Src, Dst);
}

// If the region exits to BB, which is also reachable from outside the region,
// and the exit is through an EH pad, the exit path is undefined. We can make
// the exit edge unreachable, and exclude BB from the region.
static void BreakEHToBlock(BasicBlock *BB,
                           const SmallSet<BasicBlock *, 8> &BBSet,
                           SmallSet<BasicBlock *, 4> &DeleteSet,
                           DominatorTree *DT) {
  // Copy the preds as we will be breaking edges.
  SmallVector<BasicBlock *, 4> PredCopy(predecessors(BB));
  for (auto *PredBB : PredCopy) {
    if (BBSet.find(PredBB) != BBSet.end()) {
      // PredBB is in the region and precedes the bad exit block.
      // (lpad_loop_exit in the below example)
      // Find out if there is an EH pad that dominates PredBB.
      // If there is, this is an undefined path and we can terminate it
      // before it hits BB.
      LLVM_DEBUG(dbgs() << "In-set pred is " << PredBB->getName() << "\n");
      auto *DomNode = DT->getNode(PredBB);
      while (DomNode) {
        auto *DomBB = DomNode->getBlock();
        if (DomBB->isEHPad() && (BBSet.find(DomBB) != BBSet.end())) {
          LLVM_DEBUG(dbgs() << "Found EH pad at " << DomBB->getName()
                            << " dominates " << PredBB->getName() << "\n");
          BreakEdge(PredBB, BB, DT);
          // Stop searching.
          break;
        }
        DomNode = DomNode->getIDom();
      }
    }
  }

  // If BB no longer has any incoming edges from the region, exclude it.
  // Even if we didn't break any edges, the block may be excluded by an
  // earlier break.
  bool HasPredInSet = llvm::any_of(predecessors(BB), [&](BasicBlock *PredBB) {
    return (BBSet.find(PredBB) != BBSet.end()) &&
           (DeleteSet.find(PredBB) == DeleteSet.end());
  });

  if (!HasPredInSet) {
    DeleteSet.insert(BB);
    // Exclude any blocks that BB dominates also.
    for (auto *SetBB : BBSet) // order doesn't make a difference
      if (DT->dominates(BB, SetBB))
        DeleteSet.insert(SetBB);
  }
}

// If BB has a predecessor that is not in BBSet, but it is unreachable from
// function entry, then remove that predecessor and break the branch from that
// predecessor to BB.
static void RemoveDeadPredecessors(BasicBlock *BB, DominatorTree *DT) {
  // Copy the preds as we will be breaking edges.
  SmallVector<BasicBlock *, 4> PredCopy(predecessors(BB));
  for (auto *PredBB : PredCopy) {
    if (DT->isReachableFromEntry(DT->getNode(PredBB)))
      continue;
    LLVM_DEBUG(dbgs() << PredBB->getName() << " is unreachable from entry.\n");

    BreakEdge(PredBB, BB, DT);
  }
}

// (1) After inlining, there may be an exception-path that leads
// outside the region to a shared handler.
//
// invoke throw, unreachable, lpad
// ...
// OMP_LOOP {
//    ...
//    invoke throw, unreachable, lpad.i.i
//    ...
//    lpad.i.i:
//      cmp eh_type,..
//      ; if exception uncaught, go to the outer handler
//      br inner_handler, lpad_loop_exit
//
//    inner_handler:
//      ...
// }
// lpad_loop_exit: ; from inside the loop
//      br shared_handler
// lpad: ; from outside the loop
//      br shared_handler
//
// shared_handler: ; preds: lpad, lpad_loop_exit
//
// The shared block will be made part of the region, as it is reachable from
// inside the loop.
// It prevents CE from working, as it has an incoming edge from outside
// the region.
// Since the path from the region to the shared block is actually an undefined
// early-exit, we can break the edge to the shared block with an unreachable
// instruction. In the example above, the unreachable is placed in
// lpad_loop_exit.
// The shared block can then be removed from the region set.
// The non-OMP path is unaffected.
//
// (2) Also, if there is any block in BBVec, which has predecessors that are
// unreachable from entry, then remove branches from those predecessors to the
// block.
static void FixEHEscapesAndDeadPredecessors(ArrayRef<BasicBlock *> &BBVec,
                         SmallVectorImpl<BasicBlock *> &OutputVec,
                         DominatorTree *DT) {
  SmallSet<BasicBlock *, 4> DeleteSet;

  // Scan for EH first, as we only break edges from exception paths.
  bool EHFound = false;
  for (auto *BB : BBVec) {
    if (BB->isEHPad()) {
      EHFound = true;
      break;
    }
  }

  // Use a better search structure.
  SmallSet<BasicBlock *, 8> BBSet;
  for (auto *BB : BBVec)
    BBSet.insert(BB);

  // Search BBVec for blocks that have edges from outside the region.
  for (auto *BB : BBVec) {
    // Exclude the head region block.
    if (BB == *(BBVec.begin()))
      continue;

    bool SeenOutOfSetPredecessors = false;
    for (auto *PredBB : predecessors(BB)) {
      if (BBSet.find(PredBB) == BBSet.end()) {
        LLVM_DEBUG(dbgs() << "Found set block " << BB->getName()
                          << " with non-set pred " << PredBB->getName()
                          << "\n");
        SeenOutOfSetPredecessors = true;

        // If the predecessor is not part of the region, it does not
        // necessarily means that BB is a block from "outside" of the region.
        // PredBB may be an artifact of a noreturn call made inside
        // the region, e.g.:
        //   #pragma omp parallel private (p)
        //   try {
        //     foo();
        //   } catch (int t) {
        //     if ( t != p ) {
        //   #pragma omp critical
        //       exit(0);
        //     }
        //   if_end:
        //   }
        //
        // The if_end BB will have two predecessors, one of which is
        // unreachable from the entry due to the exit() call, and
        // the second one which is dominated by an EH pad.
        // Trying to break the EH path to if_end would be incorrect,
        // so we have to avoid this.
        //
        // We'd better use the post-dominator tree to identify
        // whether BB is "inside" or "outside" of the region,
        // but we do not have it available right now.
        //
        // For the time being, just do not try to break the EH path,
        // if PredBB is just an unreachable block.
        if (!DT->isReachableFromEntry(DT->getNode(PredBB)))
          continue;

        // Try to break the edge from the region to the block.
        if (EHFound)
          BreakEHToBlock(BB, BBSet, DeleteSet, DT);
      }
    }

    // If BB isn't already marked for removal, unlink it from dead
    // predecessors.
    if (SeenOutOfSetPredecessors && DeleteSet.find(BB) == DeleteSet.end())
      RemoveDeadPredecessors(BB, DT);
  }

  if (!DeleteSet.empty()) {
    // Copy BBVec to OutputVec, excluding the former shared blocks that are
    // no longer part of the region.
    LLVM_DEBUG(for (auto *BB
                    : DeleteSet) dbgs()
                   << "Removing block from region: " << BB->getName() << "\n";);

    llvm::copy_if(BBVec, std::back_inserter(OutputVec),
                  [&](BasicBlock *R) { return DeleteSet.count(R) == 0; });
  }
}

Function *VPOParoptUtils::genOutlineFunction(
    const WRegionNode &W, DominatorTree *DT, AssumptionCache *AC,
    llvm::Optional<ArrayRef<BasicBlock *>> BBsToExtractIn, std::string Suffix) {
#if 0
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": WRN BBSet {\n";
           formatted_raw_ostream OS(dbgs());
           W.printEntryExitBB(OS, /*indent*/ 2, /*verbosity*/ 4);
           dbgs() << "}\n\n");
#endif

  // If the function we are outlining is associated with a "omp target"
  // construct, then the generated IR may be different for the host and the
  // device. This may cause trouble because the CodeExtractor infers the
  // arguments of the outlined function based on the order in which these
  // variables appear inside the function body (in IR representation). Different
  // codegen schemes on different devices may result in IR code where the order
  // of appearance is different. In order to enforce a consistent interface, we
  // pass to the CodeExtractor a vector of arguments in the order they appear in
  // the target clause. This order will be enforced across all devices.
  // Arguments include mapped, firstprivate and is_device_ptr variables both
  // explicitly appearing in the target clause and implicitly included by the
  // compiler.
  CodeExtractor::OrderedArgs TgtClauseArgs;
  bool IsTarget = isa<WRNTargetNode>(W);

  if (IsTarget) {
    // Get mapped arguments
    for (auto *Item : W.getMap().items()) {
      TgtClauseArgs.insert(
          std::make_pair(Item->getOrig(), Item->getIsMapAlways()));
    }
    // Get firstprivate arguments
    for (auto *Item : W.getFpriv().items()) {
      TgtClauseArgs.insert(std::make_pair(Item->getOrig(), false));
    }
    // Get is_device_ptr arguments
    for (auto *Item : W.getIsDevicePtr().items()) {
      TgtClauseArgs.insert(std::make_pair(Item->getOrig(), false));
    }
  }

  // Fix "escaping" EH edges that go outside the region, and dead predecessors.
  // More details in FixEHEscapesAndDeadPredecessors() above.
  // If a fix was made, the new set of region blocks is copied to FixedBlocks.
  // We avoid mutating the actual region set.
  SmallVector<BasicBlock *, 16> FixedBlocks;
  auto ExtractArray =
      BBsToExtractIn.getValueOr(makeArrayRef(W.bbset_begin(), W.bbset_end()));
  FixEHEscapesAndDeadPredecessors(ExtractArray, FixedBlocks, DT);
  if (!FixedBlocks.empty())
    ExtractArray = makeArrayRef(FixedBlocks);

  CodeExtractor CE(ExtractArray, DT,
                   /* AggregateArgs */ false,
                   /* BlockFrequencyInfo */ nullptr,
                   /* BranchProbabilityInfo */ nullptr,
                   /* AssumptionCache */ AC,
                   /* AllowVarArgs */ false,
                   /* AllowAlloca */ true,
                   /* Suffix */ Suffix,
                   /* AllowEHTypeID */ true,
                   IsTarget ? &TgtClauseArgs : nullptr);
  CE.setDeclLoc(W.getEntryDirective()->getDebugLoc());
  assert(CE.isEligible() && "Region is not eligible for extraction.");

  if (!BBsToExtractIn ||
      (llvm::is_contained(BBsToExtractIn.getValue(), W.getExitBBlock()) !=
       llvm::is_contained(BBsToExtractIn.getValue(), W.getEntryBBlock())))
    // Remove the use of the entry directive in the exit directive, so that it
    // isn't considered a live-out, in case the end directive is unreachable,
    // and won't be in the outlined function.
    // "llvm::ConstantTokenNone::get(C)" isn't supported by -debugify
    W.getEntryDirective()->replaceAllUsesWith(llvm::UndefValue::get(
        Type::getTokenTy(W.getEntryDirective()->getModule()->getContext())));

  CodeExtractorAnalysisCache CEAC(*W.getEntryBBlock()->getParent());
  auto *NewFunction = CE.extractCodeRegion(CEAC, /* hoistAlloca */ true);
  assert(NewFunction && "Code extraction failed for the region.");
  assert(NewFunction->hasOneUse() && "New function should have one use.");

  auto *CallSite = cast<CallInst>(NewFunction->user_back());

  // Remove the extracted blocks from the LoopInfo of enclosing regions. The
  // Loops and Blocks in the extracted function are no longer valid
  // outside of that function.
  // Some regions (WRNDistribute, etc.) carry the LoopInfo of the child region
  // and should be ignored.
  LoopInfo *ThisLI =
      W.getIsOmpLoop() ? W.getWRNLoopInfo().getLoopInfo() : nullptr;
  WRegionNode *ParentW = W.getParent();
  while (ParentW) {
    if (ParentW->getIsOmpLoop() &&
        ParentW->getWRNLoopInfo().getLoopInfo() != ThisLI) {
      ParentW->getWRNLoopInfo().removeBlocksInFn(NewFunction);
    }
    ParentW = ParentW->getParent();
  }
#if 0
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Call to Outlined Function:\n"
                    << *CallSite << "\n\n");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Outlined Function: ============ \n"
                    << *NewFunction << "\n==============================\n\n");
#endif

  if (EnableOutlineVerification &&
      // Data related outlined functions can legally accept
      // non-pointer arguments.
      !isa<WRNTargetDataNode>(W) && !isa<WRNTargetEnterDataNode>(W) &&
      !isa<WRNTargetExitDataNode>(W) && !isa<WRNTargetUpdateNode>(W)) {
    const auto &DL = CallSite->getModule()->getDataLayout();
    auto &C = CallSite->getModule()->getContext();

    // Verify that the outlined function's arguments are pointers.
    auto *FnType = NewFunction->getFunctionType();
    // Get size of any pointer type.
    auto PointerSize =
        DL.getTypeSizeInBits(PointerType::getUnqual(Type::getInt8Ty(C)));

    User::op_iterator ArgIt = CallSite->arg_begin();
    User::op_iterator ArgEnd = CallSite->arg_end();
    for (auto ArgTyI = FnType->param_begin(), ArgTyE = FnType->param_end();
         ArgTyI != ArgTyE; ++ArgTyI, ++ArgIt) {
      assert(ArgIt != ArgEnd && "Too few arguments in a function call.");
      (void)ArgEnd;

      // If it is not a pointer type and the strict verification is enabled,
      // then fail. If strict verification is disabled, then check
      // if the argument's size matches the pointer size,
      if (!(*ArgTyI)->isPointerTy()) {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Outlined function '";
                   NewFunction->printAsOperand(dbgs());
                   dbgs() << "' has a non-pointer argument of type "
                   << **ArgTyI << "\nCall site:\n" << *CallSite << "\n");

        // Non-pointer FIRSTPRIVATE items may appear due to the optimization
        // that transforms pass-by-pointer to pass-by-value.
        if (W.canHaveFirstprivate() &&
            WRegionUtils::wrnSeenAsFirstprivate(&W, ArgIt->get()))
          continue;

        if (StrictOutlineVerification)
          llvm_unreachable("Outlined function has a non-pointer argument.");

        if (!(*ArgTyI)->isSized() ||
            DL.getTypeSizeInBits((*ArgTyI)) != PointerSize)
          llvm_unreachable("Outlined function's argument has size "
                           "different from pointer size.");
      }
    }
  }

  DT->verify(DominatorTree::VerificationLevel::Full);

  // Set up the calling convention used by OpenMP runtime library.
  setFuncCallingConv(CallSite, CallSite->getModule());

  return NewFunction;
}

// Allow using short names for llvm::vpo::intrinsics::IntrinsicOperandTy
// constants (e.g. I8, I16, etc.)
using namespace llvm::vpo::intrinsics;

Value *VPOParoptUtils::genSPIRVHorizontalReduction(
    ReductionItem *RedI, Type *ScalarTy, Instruction *RedDef,
    spirv::Scope Scope) {

  // Reduction operation is defined by the operation kind (e.g. add)
  // and its signedness (true/false for integer types and llvm::None
  // for floating point types).
  typedef Optional<bool> IsSignedTy;
  typedef std::pair<ReductionItem::WRNReductionKind, IsSignedTy>
      ReductionOperationTy;

  // The horizontal reduction SPIRV builting is picked based on
  // the reduction operation and the type of the reduction value.
  typedef std::pair<ReductionOperationTy, IntrinsicOperandTy>
      SPIRVHorizontalReductionTy;

  static const std::map<SPIRVHorizontalReductionTy, const std::string>
      SPIRVHorizontalReductionMap = {
        //    OperationKind                   IsSigned      Type
        //Builtin name
        { { { ReductionItem::WRNReductionAdd, true       }, I16 },
          "_Z20sub_group_reduce_addi" },
        { { { ReductionItem::WRNReductionAdd, true       }, I32 },
          "_Z20sub_group_reduce_addi" },
        { { { ReductionItem::WRNReductionAdd, true       }, I64 },
          "_Z20sub_group_reduce_addl" },
        { { { ReductionItem::WRNReductionAdd, llvm::None }, F16 },
          "_Z20sub_group_reduce_addDh" },
        { { { ReductionItem::WRNReductionAdd, llvm::None }, F32 },
          "_Z20sub_group_reduce_addf" },
        { { { ReductionItem::WRNReductionAdd, llvm::None }, F64 },
          "_Z20sub_group_reduce_addd" },
        { { { ReductionItem::WRNReductionMin, true       }, I16 },
          "_Z20sub_group_reduce_mini" },
        { { { ReductionItem::WRNReductionMin, false      }, I16 },
          "_Z20sub_group_reduce_minj" },
        { { { ReductionItem::WRNReductionMin, true       }, I32 },
          "_Z20sub_group_reduce_mini" },
        { { { ReductionItem::WRNReductionMin, false      }, I32 },
          "_Z20sub_group_reduce_minj" },
        { { { ReductionItem::WRNReductionMin, true       }, I64 },
          "_Z20sub_group_reduce_minl" },
        { { { ReductionItem::WRNReductionMin, false      }, I64 },
          "_Z20sub_group_reduce_minm" },
        { { { ReductionItem::WRNReductionMin, llvm::None }, F16 },
          "_Z20sub_group_reduce_minDh" },
        { { { ReductionItem::WRNReductionMin, llvm::None }, F32 },
          "_Z20sub_group_reduce_minf" },
        { { { ReductionItem::WRNReductionMin, llvm::None }, F64 },
          "_Z20sub_group_reduce_mind" },
        { { { ReductionItem::WRNReductionMax, true       }, I16 },
          "_Z20sub_group_reduce_maxi" },
        { { { ReductionItem::WRNReductionMax, false      }, I16 },
          "_Z20sub_group_reduce_maxj" },
        { { { ReductionItem::WRNReductionMax, true       }, I32 },
          "_Z20sub_group_reduce_maxi" },
        { { { ReductionItem::WRNReductionMax, false      }, I32 },
          "_Z20sub_group_reduce_maxj" },
        { { { ReductionItem::WRNReductionMax, true       }, I64 },
          "_Z20sub_group_reduce_maxl" },
        { { { ReductionItem::WRNReductionMax, false      }, I64 },
          "_Z20sub_group_reduce_maxm" },
        { { { ReductionItem::WRNReductionMax, llvm::None }, F16 },
          "_Z20sub_group_reduce_maxDh" },
        { { { ReductionItem::WRNReductionMax, llvm::None }, F32 },
          "_Z20sub_group_reduce_maxf" },
        { { { ReductionItem::WRNReductionMax, llvm::None }, F64 },
          "_Z20sub_group_reduce_maxd" },
      };

  // The table above only specialized for Subgroup reductions.
  // It may be extended for other scopes, if needed.
  if (Scope != spirv::Scope::Subgroup)
    return nullptr;

  ReductionItem::WRNReductionKind Kind = RedI->getType();
  Optional<bool> IsSigned = llvm::None;
  if (ScalarTy->isIntegerTy())
    IsSigned = !RedI->getIsUnsigned();

  assert((IsSigned != false ||
          Kind == ReductionItem::WRNReductionMin ||
          Kind == ReductionItem::WRNReductionMax) &&
         "The UNSIGNED modifier is for MIN/MAX reduction only");

  auto TyID = ScalarTy->getTypeID();
  auto TySize = ScalarTy->getScalarSizeInBits();

  auto MapEntry = SPIRVHorizontalReductionMap.find(
      { { Kind, IsSigned }, { TyID, TySize } });

  if (MapEntry == SPIRVHorizontalReductionMap.end())
    return nullptr;

  IRBuilder<> Builder(RedDef->getNextNode());
  Value *Result = RedDef;
  auto CallArgRetTy = ScalarTy;

  // Extend 16-bit integer reduction value to 32-bit one.
  if (TySize == 16 && ScalarTy->isIntegerTy()) {
    CallArgRetTy = Builder.getIntNTy(32);
    if (IsSigned)
      Result = Builder.CreateSExt(Result, CallArgRetTy);
    else
      Result = Builder.CreateZExt(Result, CallArgRetTy);
  }

  StringRef Name = MapEntry->second;

  auto *HRCall = genCall(RedDef->getModule(), Name, CallArgRetTy,  { Result },
                         &*Builder.GetInsertPoint());
  setFuncCallingConv(HRCall, HRCall->getModule());

  LLVM_DEBUG(dbgs() << __FUNCTION__ <<
             ": SPIRV horizontal reduction is used "
             "for critical section reduction: " <<
             HRCall->getCalledFunction()->getName() << "\n");

  Result = HRCall;

  if (TySize == 16 && ScalarTy->isIntegerTy())
    Result = Builder.CreateTrunc(Result, Builder.getIntNTy(16));

  return Result;
}

bool VPOParoptUtils::useSPMDMode(WRegionNode *W) {
  if (!W->getIsOmpLoop())
    return false;

  return W->getWRNLoopInfo().isKnownNDRange();
}

spirv::ExecutionSchemeTy VPOParoptUtils::getSPIRExecutionScheme() {
  return SPIRExecutionScheme;
}

bool VPOParoptUtils::getSPIRImplicitMultipleTeams() {
  return SPIRImplicitMultipleTeams;
}

bool VPOParoptUtils::isOMPCritical(
    const Instruction *I, const TargetLibraryInfo &TLI) {
  const auto *CI = dyn_cast<const CallInst>(I);
  if (!CI)
    return false;

  auto *CalledF =
      dyn_cast<Function>(CI->getCalledOperand()->stripPointerCasts());
  if (!CalledF)
    return false;

  LibFunc LF;
  if (TLI.getLibFunc(*CalledF, LF))
    return LF == LibFunc_kmpc_critical;

  return false;
}

#ifndef NDEBUG
void VPOParoptUtils::verifyFunctionForParopt(
    const Function &F, bool IsTargetSPIRV) {
  uint64_t Errors = 0;

  auto InstFailed =
      [&Errors](const Instruction *I) {
        LLVM_DEBUG(dbgs() <<
                   "ERROR: Instruction failed Paropt verification:\n" << *I <<
                   "\n");
        ++Errors;
      };

  for (auto &I : instructions(F))
    if (auto *ASCI = dyn_cast<AddrSpaceCastInst>(&I)) {
      auto *SrcTy = cast<PointerType>(ASCI->getSrcTy());
      unsigned SrcAS = SrcTy->getAddressSpace();
      auto *DstTy = cast<PointerType>(ASCI->getDestTy());
      unsigned DstAS = DstTy->getAddressSpace();
      if (!areCompatibleAddrSpaces(SrcAS, DstAS, IsTargetSPIRV))
        InstFailed(ASCI);
    }

  if (Errors > 0) {
    LLVM_DEBUG(dbgs() << "ERROR: Paropt found " << Errors <<
               " errors in function:\n" << F << "\n");
    report_fatal_error("Function failed Paropt verification.  "
                       "Use -mllvm -debug-only=" DEBUG_TYPE " to get more "
                       "information.");
  }
}
#endif // NDEBUG

// Find users of V in the function F. Populates UserInsts and UserExprs with the
// users if they're not null.
void VPOParoptUtils::findUsesInFunction(
    Function *F, Value *V, SmallVectorImpl<Instruction *> *UserInsts,
    SmallPtrSetImpl<ConstantExpr *> *UserExprs) {
  assert(UserInsts != nullptr && "UserInsts must not be nullptr.");
  assert(UserExprs != nullptr && "UserExprs must not be nullptr.");

  for (User *U : V->users()) {
    if (Instruction *I = dyn_cast<Instruction>(U)) {
      // If the user I in Function F, push it into UserInsts
      if (I->getFunction() == F)
        UserInsts->push_back(I);
    } else if (ConstantExpr *CE = dyn_cast<ConstantExpr>(U)) {
      // The user of global may not be an instruction but a ConstantExpr.
      UserExprs->insert(CE);
      // Recursively call findUsesInFunction to find all Instructions in \p F
      // that use the ConstantExpr and add such Instructions to \p *Users.
      VPOParoptUtils::findUsesInFunction(F, CE, UserInsts, UserExprs);
    }
  }

  return;
}

// Replace users of Old with New value in function F
void VPOParoptUtils::replaceUsesInFunction(Function *F, Value *Old,
                                           Value *New) {
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": replace ";
             Old->printAsOperand(dbgs()); dbgs() << " with ";
             New->printAsOperand(dbgs()); dbgs() << "\n");

  SmallVector<Instruction *, 8> UserInsts;
  SmallPtrSet<ConstantExpr *, 8> UserExprs;

  findUsesInFunction(F, Old, &UserInsts, &UserExprs);
  while (!UserInsts.empty()) {
    Instruction *UI = UserInsts.pop_back_val();
    UI->replaceUsesOfWith(Old, New);

    if (UserExprs.empty())
      continue;

    // if Old is a ConstantExpr, its uses could be in ConstantExpr
    SmallVector<Instruction *, 2> NewInstArr;
    GeneralUtils::breakExpressions(UI, &NewInstArr, &UserExprs);
    for (Instruction *NewInst : NewInstArr)
      UserInsts.push_back(NewInst);
  }
}

// Returns true, if this is a target compilation invocation
// forced by dedicated compiler option.
bool VPOParoptUtils::isForcedTargetCompilation() {
  return SwitchToOffload;
}

// Create a thread local global GV, insert a store of V to it, and return it.
GlobalVariable *
VPOParoptUtils::storeIntToThreadLocalGlobal(Value *V, Instruction *InsertBefore,
                                            StringRef VarName) {
  assert(isa<IntegerType>(V->getType()) && "V is not an integer.");

  IRBuilder<> Builder(InsertBefore);
  Module *M = InsertBefore->getModule();

  GlobalVariable *GV = new GlobalVariable(
      *M, V->getType(), false, GlobalValue::CommonLinkage,
      Builder.getIntN(V->getType()->getIntegerBitWidth(), 0), VarName, nullptr,
      GlobalVariable::ThreadLocalMode::GeneralDynamicTLSModel);
  Builder.CreateStore(V, GV);
  return GV;
}
#endif // INTEL_COLLAB
