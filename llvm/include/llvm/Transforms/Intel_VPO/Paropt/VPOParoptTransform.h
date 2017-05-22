//===-- VPO/Paropt/VPOParoptTranform.h - Paropt Transform Class -*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Dec 2015: Initial Implementation of MT-code generation (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the interface to outline a work region formed from
/// parallel loop/regions/tasks into a new function, replacing it with a
/// call to the threading runtime call by passing new function pointer to
/// the runtime for parallel execution.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORMS_H
#define LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORMS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Pass.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"

#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"

#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <queue>

namespace llvm {

namespace vpo {

typedef SmallVector<WRegionNode *, 32> WRegionListTy;

/// \brief Provide all functionalities to perform paropt threadization
/// such as outlining, privatization, loop partitioning, multithreaded
/// code generation.
class VPOParoptTransform {

public:
  /// \brief ParoptTransform object constructor
  VPOParoptTransform(Function *F, WRegionInfo *WI, DominatorTree *DT,
                     LoopInfo *LI, ScalarEvolution *SE, int Mode)
      : F(F), WI(WI), DT(DT), LI(LI), SE(SE), Mode(Mode), IdentTy(nullptr),
        TidPtr(nullptr), BidPtr(nullptr), KmpcMicroTaskTy(nullptr),
        KmpRoutineEntryPtrTy(nullptr), KmpTaskTTy(nullptr),
        KmpTaskTRedTy(nullptr) {}

  /// \brief Top level interface for parallel and prepare transformation
  bool paroptTransforms();

private:
  /// \brief The W-regions in the function F are to be transformed
  Function *F;

  /// \brief W-Region information holder
  WRegionInfo *WI;

  /// \brief Get the Dominator Tree for code extractor
  DominatorTree *DT;

  /// \brief Get the Loop information for loop candidates
  LoopInfo *LI;

  /// \brief Get the Scalar Evolution information for loop candidates
  ScalarEvolution *SE;

  /// \brief Paropt compilation mode
  int Mode;

  /// \brief Contain all parallel/sync/offload constructs to be transformed
  WRegionListTy WRegionList;

  /// \brief Hold the LOC structure type which is need for KMP library
  StructType *IdentTy;

  /// \brief Hold the pointer to Tid (thread id) Value
  AllocaInst *TidPtr;

  /// \brief Hold the pointer to Bid (binding thread id) Value
  AllocaInst *BidPtr;

  /// \brief Hold the function type for the function
  /// void (*kmpc_micro)(kmp_int32 *global_tid, kmp_int32 *bound_tid, ...)
  FunctionType *KmpcMicroTaskTy;

  /// \brief Hold the function type for the taskloop outlined function in the
  /// form of void @RoutineEntry (i32 %tid, %struct.kmp_task_t_with_privates*
  /// %taskt.withprivates)
  PointerType *KmpRoutineEntryPtrTy;

  /// \brief Hold the struct type in the form of %struct.kmp_task_t = type {
  /// i8*, i32 (i32, i8*)*, i32, %union.kmp_cmplrdata_t, %union.kmp_cmplrdata_t,
  /// i64, i64, i64, i32}
  StructType *KmpTaskTTy;

  /// \brief Hold the struct type in the form of %struct.kmp_task_t_red_item =
  /// type { i8*, i64, i8*, i8*, i8*, i32 }
  StructType *KmpTaskTRedTy;

  /// \brief Use the WRNVisitor class (in WRegionUtils.h) to walk the
  /// W-Region Graph in DFS order and perform outlining transformation.
  /// \param[out] NeedTID : 'true' if any W visited has W->needsTID()==true
  /// \param[out] NeedBID : 'true' if any W visited has W->needsBID()==true
  void gatherWRegionNodeList(bool &NeedTID, bool &NeedBID);

  /// \brief Generate code for private variables
  bool genPrivatizationCode(WRegionNode *W);

  /// \brief Generate code for firstprivate variables
  bool genFirstPrivatizationCode(WRegionNode *W);

  /// \brief Generate code for lastprivate variables
  bool genLastPrivatizationCode(WRegionNode *W, AllocaInst *IsLastVal);

  /// \brief A utility to privatize a variable within the region.
  /// It creates and returns an AllocaInst for \p PrivValue.
  Value *genPrivatizationAlloca(WRegionNode *W, Value *PrivValue,
                                Instruction *InsertPt,
                                const StringRef VarNameSuff);

  /// \brief Replace the variable with the privatized variable
  void genPrivatizationReplacement(WRegionNode *W, Value *PrivValue,
                                   Value *NewPrivInst, Item *IT);

  /// \brief Generate the reduction initialization code.
  void genReductionInit(ReductionItem *RedI, Instruction *InsertPt);

  /// \brief Generate the reduction update code.
  void genReductionFini(ReductionItem *RedI, Value *OldV,
                        Instruction *InsertPt);

  /// \brief Generate the reduction initialization code for Min/Max.
  Value *genReductionMinMaxInit(ReductionItem *RedI, Type *Ty, bool IsMax);

  /// \brief Generate the reduction intialization instructions.
  Value *genReductionScalarInit(ReductionItem *RedI, Type *ScalarTy);

  /// \brief Generate the reduction code for reduction clause.
  bool genReductionCode(WRegionNode *W);

  /// \brief Prepare the empty basic block for the array
  /// reduction or firstprivate initialization.
  void createEmptyPrvInitBB(WRegionNode *W, BasicBlock *&RedBB);

  /// \brief Prepare the empty basic block for the array
  /// reduction or lastprivate update.
  void createEmptyPrivFiniBB(WRegionNode *W, BasicBlock *&RedEntryBB);

  /// \brief Generate the reduction update instructions for min/max.
  Value* genReductionMinMaxFini(ReductionItem *RedI, Value *Rhs1, Value *Rhs2,
                             Type *ScalarTy, IRBuilder<> &Builder, bool IsMax);

  /// \brief Generate the reduction update instructions.
  Value *genReductionScalarFini(ReductionItem *RedI, Value *Rhs1, Value *Rhs2,
                                Value *Lhs, Type *ScalarTy,
                                IRBuilder<> &Builder);

  /// \brief Generate the reduction initialization/update for array.
  void genRedAggregateInitOrFini(ReductionItem *RedI, AllocaInst *AI,
                                 Value *OldV, Instruction *InsertPt,
                                 bool IsInit);

  /// \brief Generate the reduction fini code for bool and/or.
  Value *genReductionFiniForBoolOps(ReductionItem *RedI, Value *Rhs1,
                                    Value *Rhs2, Type *ScalarTy,
                                    IRBuilder<> &Builder, bool IsAnd);

  /// \brief Generate the firstprivate initialization code.
  void genFprivInit(FirstprivateItem *FprivI, Instruction *InsertPt);

  /// \brief Generate the lastprivate update code.
  void genLprivFini(LastprivateItem *LprivI, Instruction *InsertPt);

  /// \brief Generate loop schdudeling code.
  /// \p IsLastVal is an output from this routine and is used to emit
  /// lastprivate code.
  bool genLoopSchedulingCode(WRegionNode *W, AllocaInst *&IsLastVal);

  /// \brief Generate the code to replace the variables in the task loop with
  /// the thunk field dereferences
  bool genTaskLoopInitCode(WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
                           StructType *&KmpSharedTy, Value *&LBPtr,
                           Value *&UBPtr, Value *&STPtr);

  /// \brief Generate the call __kmpc_omp_task_alloc, __kmpc_taskloop and the
  /// corresponding outlined function
  bool genTaskLoopCode(WRegionNode *W, StructType *KmpTaskTTWithPrivatesTy,
                       StructType *KmpSharedTy, Value *LBPtr, Value *UBPtr,
                       Value *STPtr);

  /// \brief Replace the shared variable reference with the thunk field
  /// derefernce
  bool genSharedCodeForTaskLoop(WRegionNode *W);

  /// \brief Replace the reduction variable reference with the dereference of
  /// the return pointer __kmpc_task_reduction_get_th_data
  bool genRedCodeForTaskLoop(WRegionNode *W);

  /// \brief Generate the struct type kmp_task_red_input
  void genTaskTRedType();

  /// \brief Generate the call __kmpc_task_reduction_init and the corresponding
  /// preparation.
  void genRedInitForTaskLoop(WRegionNode *W, Instruction *InsertBefore);

  /// \brief Set up the mapping between the variables (firstprivate,
  /// lastprivate, reduction and shared) and the counterparts in the thunk.
  AllocaInst *genTaskPrivateMapping(WRegionNode *W, Instruction *InsertPt,
                                    StructType *KmpSharedTy);

  /// \brief Initialize the data in the shared data area inside the thunk
  void genSharedInitForTaskLoop(WRegionNode *W, AllocaInst *Src, Value *Dst,
                                StructType *KmpSharedTy,
                                StructType *KmpTaskTTWithPrivatesTy,
                                Instruction *InsertPt);

  /// \brief Save the loop lower upper bound, upper bound and stride for the use
  /// by the call __kmpc_taskloop
  void genLoopInitCodeForTaskLoop(WRegionNode *W, Value *&LBPtr, Value *&UBPtr,
                                  Value *&STPtr);

  /// \brief Prepare the scope alias metadata for the references of the
  /// firstprivate, lastprivate, private shared, reduction variables
  void prepareNoAliasMetadataInTaskLoop(WRegionNode *W);

  /// \brief Generate the outline function of reduction initilaization
  Function *genTaskLoopRedInitFunc(WRegionNode *W, ReductionItem *RedI);

  /// \brief Generate the outline function for the reduction update
  Function *genTaskLoopRedCombFunc(WRegionNode *W, ReductionItem *RedI);

  /// \brief Annotate the alias scope data for the references of the
  /// firstprivate, lastprivate, private shared, reduction variables
  void annotateInstWithNoAlias(Instruction *ItemInst, Item *IT);

  /// \brief Generate the function type void @routine_entry(i32 %tid, i8*)
  void genKmpRoutineEntryT();

  /// \brief Generate the struct type %struct.kmp_task_t = type { i8*, i32 (i32,
  /// i8*)*, i32, %union.kmp_cmplrdata_t, %union.kmp_cmplrdata_t, i64, i64, i64,
  /// i32 }
  void genKmpTaskTRecordDecl();

  /// \brief Generate the struct type kmpc_task_t as well as its private data
  /// area. One example is as follows.
  /// %struct.kmp_task_t_with_privates = type { %struct.kmp_task_t,
  /// %struct..kmp_privates.t }
  /// %struct.kmp_task_t = type { i8*, i32 (i32, i8*)*, i32,
  /// %union.kmp_cmplrdata_t, %union.kmp_cmplrdata_t, i64, i64, i64, i32}
  /// %struct..kmp_privates.t = type { i64, i64, i32 }
  StructType *genKmpTaskTWithPrivatesRecordDecl(WRegionNode *W,
                                                StructType *&KmpSharedTy,
                                                StructType *&KmpPrivatesTy);

  /// \brief Generate the actual parameters in the outlined function
  /// for copyin variables.
  void genThreadedEntryActualParmList(WRegionNode *W,
                                      std::vector<Value *>& MTFnArgs);

  /// \brief Generate the formal parameters in the outlined function
  /// for copyin variables.
  void genThreadedEntryFormalParmList(WRegionNode *W,
                                      std::vector<Type *>& ParamsTy);

  /// \brief Generate the name of formal parameters in the outlined function
  /// for copyin variables.
  void fixThreadedEntryFormalParmName(WRegionNode *W,
                                      Function *NFn);

  /// \brief Generate the copy code for the copyin variables.
  void genTpvCopyIn(WRegionNode *W,
                    Function *NFn);

  /// \brief Finalize extracted MT-function argument list for runtime
  Function *finalizeExtractedMTFunction(WRegionNode *W, Function *Fn,
                                        bool IsTidArg, unsigned int TidArgNo,
                                        bool hasBid = true);

  /// \brief Generate __kmpc_fork_call Instruction after CodeExtractor
  CallInst* genForkCallInst(WRegionNode *W, CallInst *CI);

  /// \brief If the IR in the WRegion has some kmpc_call_* and the tid
  /// parameter's definition is outside the region, the compiler
  /// generates the call __kmpc_global_thread_num() at the entry of
  /// of the region and replaces all tid uses with the new call.
  /// It also generates the bid alloca instruciton in the region 
  /// if the region has outlined function.
  void codeExtractorPrepare(WRegionNode *W);

  /// \brief Cleans up the generated __kmpc_global_thread_num() in the
  /// outlined function. It also cleans the genererated bid alloca 
  /// instruction in the outline function.
  void finiCodeExtractorPrepare(Function *F, bool ForTaskLoop = false);

  /// \brief Collects the bid alloca instructions used by the outline functions.
  void collectTidAndBidInstructionsForBB(BasicBlock *BB);

  /// \brief Collects the instruction uses for the instructions 
  /// in the set TidAndBidInstructions.
  void collectInstructionUsesInRegion(WRegionNode *W);

  /// \brief Generates the new tid/bid alloca instructions at the entry of the
  /// region and replaces the uses of tid/bid with the new value.
  void codeExtractorPrepareTransform(WRegionNode *W, bool IsTid);

  /// \brief Replaces the use of tid/bid with the outlined function arguments.
  void finiCodeExtractorPrepareTransform(Function *F, bool IsTid,
                                         BasicBlock *NextBB,
                                         bool ForTaskLoop = false);

  /// \brief Generate multithreaded for a given WRegion
  bool genMultiThreadedCode(WRegionNode *W);

  /// Generate code for master/end master construct and update LLVM
  /// control-flow and dominator tree accordingly
  bool genMasterThreadCode(WRegionNode *W);

  /// Generate code for single/end single construct and update LLVM
  /// control-flow and dominator tree accordingly
  bool genSingleThreadCode(WRegionNode *W);

  /// Generate code for ordered/end ordered construct for preserving ordered
  /// region execution order
  bool genOrderedThreadCode(WRegionNode *W);

  /// \brief Generates code for the OpenMP critical construct:
  /// #pragma omp critical [(name)]
  bool genCriticalCode(WRNCriticalNode *CriticalNode);

  /// \brief Finds the alloc stack variables where the tid stores.
  void getAllocFromTid(CallInst *Tid);
 
  /// \brief Finds the function pointer type for the function
  /// void (*kmpc_micro)(kmp_int32 *global_tid, kmp_int32 *bound_tid, ...)
  FunctionType* getKmpcMicroTaskPointerTy();

  /// \brief The data structure which builds the map between the
  /// alloc/tid and the uses instruction in the WRegion.
  SmallDenseMap<Instruction *, std::vector<Instruction *> > IdMap;

  /// \brief The data structure that is used to store the alloca or tid call
  ///  instruction that are used in the WRegion.
  SmallPtrSet<Instruction*, 8> TidAndBidInstructions;
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORM_H
