#if INTEL_COLLAB // -*- C++ -*-
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

#include "llvm/ADT/EquivalenceClasses.h"
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

/// \brief opencl address space.
enum AddressSpace {
  ADDRESS_SPACE_PRIVATE = 0,
  ADDRESS_SPACE_GLOBAL = 1,
  ADDRESS_SPACE_CONSTANT = 2,
  ADDRESS_SPACE_LOCAL = 3,
  ADDRESS_SPACE_GENERIC = 4
};

typedef SmallVector<WRegionNode *, 32> WRegionListTy;

/// \brief Provide all functionalities to perform paropt threadization
/// such as outlining, privatization, loop partitioning, multithreaded
/// code generation.
class VPOParoptTransform {

public:
  /// \brief ParoptTransform object constructor
  VPOParoptTransform(Function *F, WRegionInfo *WI, DominatorTree *DT,
                     LoopInfo *LI, ScalarEvolution *SE,
                     const TargetTransformInfo *TTI, AssumptionCache *AC,
                     const TargetLibraryInfo *TLI, AliasAnalysis *AA, int Mode,
                     unsigned OptLevel = 2, bool SwitchToOffload = false)
      : F(F), WI(WI), DT(DT), LI(LI), SE(SE), TTI(TTI), AC(AC), TLI(TLI),
        AA(AA), Mode(Mode), TargetTriple(F->getParent()->getTargetTriple()),
        OptLevel(OptLevel), SwitchToOffload(SwitchToOffload),
        IdentTy(nullptr), TidPtrHolder(nullptr), BidPtrHolder(nullptr),
        KmpcMicroTaskTy(nullptr), KmpRoutineEntryPtrTy(nullptr),
        KmpTaskTTy(nullptr), KmpTaskTRedTy(nullptr),
        KmpTaskDependInfoTy(nullptr), TgOffloadRegionId(nullptr),
        TgOffloadEntryTy(nullptr), TgDeviceImageTy(nullptr),
        TgBinaryDescriptorTy(nullptr), DsoHandle(nullptr) {}

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

  /// \brief Get the Target Tranform information for loop candidates.
  const TargetTransformInfo *TTI;

  /// \brief Get the assumption cache informtion for loop candidates.
  AssumptionCache *AC;

  /// \brief Get the target library information for the loop candidates.
  const TargetLibraryInfo *TLI;

  AliasAnalysis *AA;

  /// \brief Paropt compilation mode
  int Mode;

  /// \brief Target triple that we are compiling for.
  Triple TargetTriple;

  /// \brief Optimization level.
  unsigned OptLevel;

  /// \brief Offload compilation mode.
  bool SwitchToOffload;

  /// \brief Contain all parallel/sync/offload constructs to be transformed
  WRegionListTy WRegionList;

  /// \brief Hold the LOC structure type which is need for KMP library
  StructType *IdentTy;

  /// \brief Hold the pointer to Tid (thread id) Value
  Constant *TidPtrHolder;

  /// \brief Hold the pointer to Bid (binding thread id) Value
  Constant *BidPtrHolder;

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

  /// \brief Hold the struct type as follows.
  ///           struct kmp_depend_info {
  ///              void* arg_addr;
  ///              size_t arg_size;
  ///              char   depend_type;
  ///           };
  StructType *KmpTaskDependInfoTy;

  /// The target region ID is an unique global varialble used by the runtime
  /// libarary.
  GlobalVariable *TgOffloadRegionId;

  /// \brief Hold the struct type as follows.
  ///    struct __tgt_offload_entry {
  ///      void      *addr;       // The address of a global variable
  ///                             // or entry point in the host.
  ///      char      *name;       // Name of the symbol referring to the
  ///                             // global variable or entry point.
  ///      size_t     size;       // Size in bytes of the global variable or
  ///                             // zero if it is entry point.
  ///      int32_t    flags;      // Flags of the entry.
  ///      int32_t    reserved;   // Reserved by the runtime library.
  /// };
  StructType *TgOffloadEntryTy;

  /// \brief Hold the struct type as follows.
  /// struct __tgt_device_image{
  ///   void   *ImageStart;       // The address of the beginning of the
  ///                             // target code.
  ///   void   *ImageEnd;         // The address of the end of the target
  ///                             // code.
  ///   __tgt_offload_entry  *EntriesBegin;  // The first element of an array
  ///                                        // containing the globals and
  ///                                        // target entry points.
  ///   __tgt_offload_entry  *EntriesEnd;    // The last element of an array
  ///                                        // containing the globals and
  ///                                        // target entry points.
  /// };
  StructType *TgDeviceImageTy;

  /// \brief Hold the struct type as follows.
  /// struct __tgt_bin_desc{
  ///   uint32_t              NumDevices;     // Number of device types i
  ///                                         // supported.
  ///   __tgt_device_image   *DeviceImages;   // A pointer to an array of
  ///                                         // NumDevices elements.
  ///   __tgt_offload_entry  *EntriesBegin;   // The first element of an array
  ///                                         // containing the globals and
  ///                                         // target entry points.
  ///   __tgt_offload_entry  *EntriesEnd;     // The last element of an array
  ///                                         // containing the globals and
  ///                                         // target entry points.
  /// };
  StructType *TgBinaryDescriptorTy;

  /// \brief Create a variable that binds the atexit to this shared object.
  GlobalVariable *DsoHandle;

  /// \brief A string to describe the device information.
  SmallVector<Triple, 16> TgtDeviceTriples;

  /// \brief Struct that keeps all the information needed to pass to
  /// the runtime library.
  class TgDataInfo {
  public:
    /// The array of base pointers passed to the runtime library.
    Value *BaseDataPtrs = nullptr;
    Value *ResBaseDataPtrs;
    /// The array of data pointers passed to the runtime library.
    Value *DataPtrs = nullptr;
    Value *ResDataPtrs;
    /// The array of data sizes passed to the runtime library.
    Value *DataSizes = nullptr;
    Value *ResDataSizes;
    /// The array of data map types passed to the runtime library.
    Value *DataMapTypes = nullptr;
    Value *ResDataMapTypes;
    /// The number of pointers passed to the runtime library.
    unsigned NumberOfPtrs = 0u;
    explicit TgDataInfo() {}
    void clearArrayInfo() {
      BaseDataPtrs = nullptr;
      DataPtrs = nullptr;
      DataSizes = nullptr;
      DataMapTypes = nullptr;
      NumberOfPtrs = 0u;
    }
    bool isValid() {
      return BaseDataPtrs && DataPtrs && DataSizes && DataMapTypes &&
             NumberOfPtrs;
    }
  };

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  /// \brief Returns true if we are compiling for CSA target.
  bool isTargetCSA() const {
     return TargetTriple.getArch() == Triple::csa;
  }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  /// \brief Use the WRNVisitor class (in WRegionUtils.h) to walk the
  /// W-Region Graph in DFS order and perform outlining transformation.
  /// \param[out] NeedTID : 'true' if any W visited has W->needsTID()==true
  /// \param[out] NeedBID : 'true' if any W visited has W->needsBID()==true
  void gatherWRegionNodeList(bool &NeedTID, bool &NeedBID);

  /// \brief Generate code for private variables
  bool genPrivatizationCode(WRegionNode *W);

  /// Generate code for linear variables.
  ///
  /// The following needs to be done for handling a linear var:
  ///
  /// -# Create two local copies of the linear vars. One to capture the
  /// starting value. Another to be the local linear variable which replaces all
  /// uses of the original inside the region.
  /// -# Capture original value of linear vars before entering the loop.
  /// -# Use the captured value along with the specified step to initialize
  /// the local linear var in each iteration of the loop.
  /// -# At the end of the last loop iteration, copy the value of the local
  /// var back to the original linear var.
  bool genLinearCode(WRegionNode *W, BasicBlock *IfLastIterBB);

  /// \brief Generate code for firstprivate variables
  bool genFirstPrivatizationCode(WRegionNode *W);

  /// \brief Generate code for lastprivate variables
  bool genLastPrivatizationCode(WRegionNode *W, BasicBlock *IfLastIterBB);

  /// \brief Generate destructor calls for [first|last]private variables
  bool genDestructorCode(WRegionNode *W);

  /// \brief A utility to privatize a variable within the region.
  /// It creates and returns an AllocaInst for \p PrivValue.
  AllocaInst *genPrivatizationAlloca(WRegionNode *W, Value *PrivValue,
                                     Instruction *InsertPt,
                                     const StringRef VarNameSuff);

  /// Creates and return an AllocaInst for the local copy of \p PrivValue within
  /// \p W. If \p ArrSecSize is provided, the local copy is a VLA of type \p
  /// ArrSecType and size \p ArrSecSize. Otherwise, the type and length are same
  /// as \p PrivValue.
  AllocaInst *genPrivatizationAlloca(WRegionNode *W, Value *PrivValue,
                                     Instruction *InsertPt,
                                     const StringRef VarNameSuff,
                                     Type *ArrSecType, Value *ArrSecSize);

  /// \brief Replace the variable with the privatized variable
  void genPrivatizationReplacement(WRegionNode *W, Value *PrivValue,
                                   Value *NewPrivInst, Item *IT);

  /// \name Reduction Specific Functions
  /// {@

  /// For array [section] reduction init loop, get the base address of
  /// the destination array, number of elements, and destination element type.
  /// \param [in] ReductionItem Reduction Item.
  /// \param [in] AI Local Value for the reduction operand.
  /// \param [in] OldV Original reduction operand Value.
  /// \param [in] InsertPt Insert point for any Instructions to be inserted.
  /// \param [in] Builder IRBuilder using InsertPt for any new Instructions.
  /// \param [out] NumElements Number of elements in the array [section].
  /// \param [out] DestArrayBegin Base address of the local reduction array.
  /// \param [out] DestElementTy Type of each element of the array [section].
  void genAggrReductionInitDstInfo(const ReductionItem &RedI, AllocaInst *AI,
                                   Instruction *InsertPt, IRBuilder<> &Builder,
                                   Value *&NumElements, Value *&DestArrayBegin,
                                   Type *&DestElementTy);

  /// For array [section] reduction finalization loop, compute the base address
  /// of the source and destination arrays, number of elements, and the type of
  /// destination array elements.
  /// \param [in] ReductionItem Reduction Item.
  /// \param [in] AI Local Value for the reduction operand.
  /// \param [in] OldV Original reduction operand Value.
  /// \param [in] InsertPt Insert point for any Instructions to be inserted.
  /// \param [in] Builder IRBuilder using InsertPt for any new Instructions.
  /// \param [out] NumElements Number of elements in the array [section].
  /// \param [out] SrcArrayBegin Base address of the local reduction array.
  /// \param [out] DestArrayBegin Starting address of the original reduction
  /// array [section].
  /// \param [out] DestElementTy Type of each element of the array [section].
  void genAggrReductionFiniSrcDstInfo(const ReductionItem &RedI, AllocaInst *AI,
                                      Value *OldV, Instruction *InsertPt,
                                      IRBuilder<> &Builder, Value *&NumElements,
                                      Value *&SrcArrayBegin,
                                      Value *&DestArrayBegin,
                                      Type *&DestElementTy);

  /// Initialize `Size`, `ElementType`, `Offset` and `BaseIsPointer` fields for
  /// ArraySectionInfo of the reduction item \p RI. It may need to emit some
  /// Instructions, which is done \b before \p InsertPt.
  void computeArraySecReductionTypeOffsetSize(ReductionItem &RI,
                                              Instruction *InsertPt);

  /// Return the Value to replace the occurrences of the original Reduction
  /// operand inside the body of the associated WRegion. It may need to emit
  /// some Instructions, which is done \b before \p InsertPt.
  Value *getReductionItemReplacementValue(ReductionItem const &RedI,
                                          Instruction *InsertPt);

  /// \brief Generate the reduction initialization code.
  void genReductionInit(ReductionItem *RedI, Instruction *InsertPt,
                        DominatorTree *DT);

  /// \brief Generate the reduction update code.
  void genReductionFini(ReductionItem *RedI, Value *OldV, Instruction *InsertPt,
                        DominatorTree *DT);

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
                                 bool IsInit, DominatorTree *DT);

  /// \brief Generate the reduction fini code for bool and/or.
  Value *genReductionFiniForBoolOps(ReductionItem *RedI, Value *Rhs1,
                                    Value *Rhs2, Type *ScalarTy,
                                    IRBuilder<> &Builder, bool IsAnd);
  /// @}

  /// \brief Generate the firstprivate initialization code.
  void genFprivInit(FirstprivateItem *FprivI, Instruction *InsertPt);

  /// \brief Utility for last private update or copyprivate code generation.
  void genLprivFini(Value *NewV, Value *OldV, Instruction *InsertPt);
  void genLprivFini(LastprivateItem *LprivI, Instruction *InsertPt);

  /// \brief Generate the lastprivate update code for taskloop
  void genLprivFiniForTaskLoop(Value *Dst, Value *Src, Instruction *InsertPt);

  /// \brief Generate loop schdudeling code.
  /// \p IsLastVal is an output from this routine and is used to emit
  /// lastprivate code.
  bool genLoopSchedulingCode(WRegionNode *W, AllocaInst *&IsLastVal);

  /// \brief Generate the code to replace the variables in the task loop with
  /// the thunk field dereferences
  bool genTaskLoopInitCode(WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
                           StructType *&KmpSharedTy, Value *&LBPtr,
                           Value *&UBPtr, Value *&STPtr, Value *&LastIterGep,
                           bool isLoop = true);
  bool genTaskInitCode(WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
                       StructType *&KmpSharedTy, Value *&LastIterGep);

  /// \brief Generate the call __kmpc_omp_task_alloc, __kmpc_taskloop and the
  /// corresponding outlined function
  bool genTaskGenericCode(WRegionNode *W, StructType *KmpTaskTTWithPrivatesTy,
                          StructType *KmpSharedTy, Value *LBPtr, Value *UBPtr,
                          Value *STPtr, bool isLoop = true);

  /// \brief Generate the call __kmpc_omp_task_alloc, __kmpc_omp_task and the
  /// corresponding outlined function.
  bool genTaskCode(WRegionNode *W, StructType *KmpTaskTTWithPrivatesTy,
                   StructType *KmpSharedTy);

  /// \brief Generate the call __kmpc_omp_taskwait.
  bool genTaskWaitCode(WRegionNode *W);

  /// \brief Replace the shared variable reference with the thunk field
  /// derefernce
  bool genSharedCodeForTaskGeneric(WRegionNode *W);

  /// \brief Replace the reduction variable reference with the dereference of
  /// the return pointer __kmpc_task_reduction_get_th_data
  bool genRedCodeForTaskGeneric(WRegionNode *W);

  /// \brief Generate the struct type kmp_task_red_input
  void genTaskTRedType();

  /// breif Generate the struct type kmp_depend_info
  void genKmpTaskDependInfo();

  /// \brief Generate the call __kmpc_task_reduction_init and the corresponding
  /// preparation.
  void genRedInitForTask(WRegionNode *W, Instruction *InsertBefore);

  /// \brief Generate the initialization code for the depend clause
  AllocaInst *genDependInitForTask(WRegionNode *W, Instruction *InsertBefore);

  /// \brief The wrapper routine to generate the call __kmpc_omp_task_with_deps
  void genTaskDeps(WRegionNode *W, StructType *IdentTy, Value *TidPtr,
                   Value *TaskAlloc, AllocaInst *DummyTaskTDependRec,
                   Instruction *InsertPt, bool IsTaskWait);

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

  /// \brief Generate the outline function of reduction initilaization
  Function *genTaskLoopRedInitFunc(WRegionNode *W, ReductionItem *RedI);

  /// \brief Generate the outline function for the reduction update
  Function *genTaskLoopRedCombFunc(WRegionNode *W, ReductionItem *RedI);

  /// \brief Generate the outline function to set the last iteration
  //  flag at runtime.
  Function *genLastPrivateTaskDup(WRegionNode *W,
                                  StructType *KmpTaskTTWithPrivatesTy);

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

  /// \brief Reset the expression value in the bundle to be empty.
  void resetValueInBundle(WRegionNode *W, Value *V);

  /// \brief Reset the expression value of task depend clause to be empty.
  void resetValueInTaskDependClause(WRegionNode *W);

  /// \brief Reset the expression value in private clause to be empty.
  void resetValueInPrivateClause(WRegionNode *W);

  /// \brief Reset the expression value in IsDevicePtr clause to be empty.
  void resetValueInIsDevicePtrClause(WRegionNode *W);

  /// Set the value in num_teams and thread_limit clause to be empty.
  void resetValueInNumTeamsAndThreadsClause(WRegionNode *W);

  /// \brief Reset the value in the Map clause to be empty.
  void resetValueInMapClause(WRegionNode *W);

  /// \brief Reset the expression value of Intel clause to be empty.
  void resetValueInIntelClauseGeneric(WRegionNode *W, Value *V);

  /// \brief Generate the code for the directive omp target
  bool genTargetOffloadingCode(WRegionNode *W);

  /// \brief Generate the initialization code for the directive omp target
  CallInst *genTargetInitCode(WRegionNode *W, CallInst *Call,
                              Instruction *InsertPt);

  /// \brief Generate the pointers pointing to the array of base pointer, the
  /// array of section pointers, the array of sizes, the array of map types.
  void genOffloadArraysArgument(TgDataInfo *Info, Instruction *InsertPt,
                                bool hasRuntimeEvaluationCaptureSize);

  /// \brief Pass the data to the array of base pointer as well as  array of
  /// section pointers. If the flag hasRuntimeEvaluationCaptureSize is true,
  /// the compiler needs to generate the init code for the size array.
  void genOffloadArraysInit(WRegionNode *W, TgDataInfo *Info, CallInst *Call,
                            Instruction *InsertPt,
                            SmallVectorImpl<Constant *> &ConstSizes,
                            bool hasRuntimeEvaluationCaptureSize);

  /// \brief Utilities to construct the assignment to the base pointers, section
  /// pointers and size pointers if the flag hasRuntimeEvaluationCaptureSize is
  /// true.
  void genOffloadArraysInitUtil(IRBuilder<> &Builder, Value *BasePtr,
                                Value *SectionPtr, Value *Size,
                                TgDataInfo *Info,
                                SmallVectorImpl<Constant *> &ConstSizes,
                                unsigned &Cnt,
                                bool hasRuntimeEvaluationCaptureSize);

  /// \brief Register the offloading descriptors as well the offloading binary
  /// descriptors.
  void genRegistrationFunction(WRegionNode *W, Function *Fn);

  /// \brief Register the offloading descriptors.
  void genOffloadEntriesAndInfoMetadata(WRegionNode *W, Function *Fn);

  /// \brief Register the offloading binary descriptors.
  void genOffloadingBinaryDescriptorRegistration(WRegionNode *W);

  /// \brief Create offloading entry for the provided entry ID and address.
  void genOffloadEntry(Constant *ID, Constant *Addr);

  /// \brief Return/Create the target region ID used by the runtime library to
  /// identify the current target region.
  GlobalVariable *getOMPOffloadRegionId();

  /// \brief Return/Create a variable that binds the atexit to this shared
  /// object.
  GlobalVariable *getDsoHandle();

  /// \brief Return/Create the struct type __tgt_offload_entry.
  StructType *getTgOffloadEntryTy();

  /// \brief Return/Create the struct type __tgt_device_image.
  StructType *getTgDeviceImageTy();

  /// \brief Return/Create the struct type __tgt_bin_desc.
  StructType *getTgBinaryDescriptorTy();

  /// \brief Create the function .omp_offloading.descriptor_reg
  Function *createTgDescRegisterLib(WRegionNode *W, Function *TgDescUnregFn,
                                    GlobalVariable *Desc);

  /// \brief Create the function .omp_offloading.descriptor_unreg
  Function *createTgDescUnregisterLib(WRegionNode *W, GlobalVariable *Desc);

  /// \brief If the incoming data is global variable, create the stack variable
  /// and replace the the global variable with the stack variable.
  bool genGlobalPrivatizationCode(WRegionNode *W);

  /// \brief Pass the value of the DevicePtr to the outlined function.
  bool genDevicePtrPrivationCode(WRegionNode *W);

  /// \brief build the CFG for if clause.
  void buildCFGForIfClause(Value *Cmp, TerminatorInst *&ThenTerm,
                           TerminatorInst *&ElseTerm, Instruction *InsertPt);

  /// \brief Generate the sizes and map type flags for the given map type, map
  /// modifier and the expression V.
  void GenTgtInformationForPtrs(WRegionNode *W, Value *V,
                                SmallVectorImpl<Constant *> &ConstSizes,
                                SmallVectorImpl<uint64_t> &MapTypes,
                                bool &hasRuntimeEvaluationCaptureSize);

  /// \brief Generate multithreaded for a given WRegion
  bool genMultiThreadedCode(WRegionNode *W);

  /// Generate code for master/end master construct and update LLVM
  /// control-flow and dominator tree accordingly
  bool genMasterThreadCode(WRegionNode *W);

  /// Generate code for single/end single construct and update LLVM
  /// control-flow and dominator tree accordingly
  bool genSingleThreadCode(WRegionNode *W, AllocaInst *&IsSingleThread);

  /// Generate code for ordered/end ordered construct for preserving ordered
  /// region execution order
  bool genOrderedThreadCode(WRegionNode *W);

  /// Emit __kmpc_doacross_post/wait call for an 'ordered depend(source/sink)'
  /// construct.
  bool genDoacrossWaitOrPost(WRNOrderedNode *W);

  /// \brief Generates code for the OpenMP critical construct:
  /// #pragma omp critical [(name)]
  bool genCriticalCode(WRNCriticalNode *CriticalNode);

  /// \brief Return true if the program is compiled at the offload mode.
  bool hasOffloadCompilation() {
    return ((Mode & OmpOffload) || SwitchToOffload);
  }

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

  /// Emits an implicit barrier at the end of WRegion \p W if W contains
  /// variables that are linear, or both firstprivate-lastprivate. e.g.
  ///
  ///   #pragma omp for firstprivate(x) lastprivate(x) nowait
  ///
  /// Emitted pseudocode:
  ///
  ///   %x.local = @x                         ; (1) firstprivate copyin
  ///   __kmpc_static_init(...)
  ///   ...
  ///   __kmpc_static_fini(...)
  ///
  ///   __kmpc_barrier(...)                   ; (2)
  ///   @x = %x.local                         ; (3) lastprivate copyout
  ///
  ///  The barrier (2) is needed to prevent a race between (1) and (3), which
  ///  read/write to/from @x.
  bool genBarrierForFpLpAndLinears(WRegionNode *W);

  /// Emits an if-then branch using \p IsLastVal and sets \p IfLastIterOut to
  /// the if-then BBlock. This is used for emitting the final copy-out code for
  /// linear and lastprivate clause operands.
  ///
  /// Code generated looks like:
  ///
  /// \code
  ///       Before             |      After
  /// -------------------------+----------------------------------------------
  ///                          |   %15 = load i32, i32* %is.last
  ///                          |   %16 = icmp ne i32 %15, 0
  ///                          |   br i1 %16, label %last.then, label %last.done
  ///                          |
  ///                          |   last.then:        ; IfLastIterOut
  ///                          |   ...
  ///                          |   br last.done
  ///                          |
  ///                          |   last.done:
  ///                          |   br exit.BB.predecessor
  ///                          |
  ///                          |   exit.BB.predecessor:
  ///                          |   br exit.BB
  ///                          |
  /// exit.BB:                 |   exit.BB:
  /// llvm.region.exit(...)    |   llvm.region.exit(...)
  ///
  /// \endcode
  ///
  /// \param [in] IsLastVal A stack variable which is non-zero if the current
  /// iteration is the last one.
  /// \param [out] IfLastIterOut The BasicBlock for when the last iteration
  /// check is true.
  ///
  /// \returns \b true if the branch is emitted, \b false otherwise.
  ///
  /// The branch is not emitted if \p W has no Linear or Lastprivate var.
  bool genLastIterationCheck(WRegionNode *W, Value *IsLastVal,
                             BasicBlock *&IfLastIterOut);

  /// \brief Insert a barrier at the end of the construct
  bool genBarrier(WRegionNode *W, bool IsExplicit);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  /// \brief Forward declarations for helper classes which implement CSA
  ///  specific privatization for OpenMP constructs.
  class CSAPrivatizer;
  class CSALoopPrivatizer;
  class CSAExpLoopPrivatizer;
  class CSASectionsPrivatizer;
  friend CSAPrivatizer;

  /// \brief Map work region to its region ID.
  SmallDenseMap<WRegionNode*, Value*, 8u> CSAParallelRegions;

  /// \brief Insert CSA parallel region entry/exit calls to the work region
  /// and return region id.
  Value* genCSAParallelRegion(WRegionNode *W);

  /// \brief Insert a pair of CSA parallel section entry/exit calls before given
  /// instructions.
  void genCSAParallelSection(Value *RegionID, Instruction *EntryPt,
                             Instruction *ExitPt);

  /// \brief Transform "omp parallel" work region for CSA target.
  bool genCSAParallel(WRegionNode *W);

  /// \brief Transform "omp [parallel] for" work region for CSA target.
  bool genCSALoop(WRegionNode *W);

  /// \brief Transform "omp [parallel] sections" work region for CSA target.
  bool genCSASections(WRegionNode *W);

  /// \brief Transform "omp single" work region for CSA target.
  bool genCSASingle(WRegionNode *W);

  /// \brief Check whether a given construct is supported in CSA.
  bool isSupportedOnCSA(WRegionNode *W);

  /// \brief Print diagnostic message for the work region.
  void reportCSAWarning(WRegionNode *W, const Twine &Msg);
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  /// \brief Insert a flush call
  bool genFlush(WRegionNode *W);

  /// \name Cancellation Specific Functions
  /// {@

  /// \brief Generates code for the OpenMP cancel constructs:
  /// \code
  /// #pragma omp cancel [type]
  /// #pragma omp cancellation point [type]
  /// \endcode
  bool genCancelCode(WRNCancelNode *W);

  /// \brief Add any cancellation points within \p W's body, to its
  /// `region.entry` directive. This is done in the vpo-paropt-prepare pass, and
  /// is later consumed by the vpo-paropt transformation pass.
  ///
  /// A `cancellation point` can be one of these calls:
  /// \code
  ///   %1 = __kmpc_cancel_barrier(...)
  ///   %2 = __kmpc_cancel(...)
  ///   %3 = __kmpc_cancellationpoint(...)
  /// \endcode
  ///
  /// The IR after the transformation looks like:
  /// \code
  ///   %cp1 = alloca i32
  ///   %cp2 = alloca i32
  ///   %cp3 = alloca i32
  ///   ...
  ///   %0 = call token @llvm.directive.region.entry(...) [...,
  ///   "QUAL.OMP.CANCELLATION.POINTS"(%cp1, %cp2, %cp3) ]
  ///   ...
  ///   %1 = __kmpc_cancel_barrier(...)
  ///   store %1, %cp1
  ///   %2 = __kmpc_cancel(...)
  ///   store %1, %cp2
  ///   %3 = __kmpc_cancellationpoint(...)
  ///   store %1, %cp3
  ///
  ///   call void @llvm.directive.region.exit(%0)
  /// \endcode
  bool propagateCancellationPointsToIR(WRegionNode *W);

  /// Removes from IR, the allocas and stores created by
  /// propagateCancellationPointsToIR(). This is done in the vpo-paropt
  /// transformation pass after the information has already been consumed. The
  /// function also removes these allocas from the
  /// "QUAL.OMP.CANCELLATION.POINTS" clause on the region.entry intrinsic.
  ///
  /// \code
  ///       Before                      |     After
  ///  ---------------------------------+------------------------------------
  ///  %cp = alloca i32                 |   <deleted>
  ///  ...                              |   ...
  ///                                   |
  ///  directive.region.entry(...%cp...)|   directive.region.entry(...null...)
  ///                                   |
  ///  %x = kmpc_cancel(...)            |   %x = kmpc_cancel(...)
  ///  store %x, %cp                    |   <deleted>
  ///  ...                              |   ...
  ///                                   |
  /// \endcode
  bool clearCancellationPointAllocasFromIR(WRegionNode *W);

  /// \brief Generate branches to jump to the end of a construct from
  /// every cancellation point within the construct.
  ///
  /// For each cancellation point '%x' within the body of W:
  ///
  /// \code
  ///       Before                      |     After
  ///  ---------------------------------+------------------------------------
  ///  %x = kmpc_cancel(...)            |     %x = kmpc_cancel(...)
  ///                                   |     if (%x != 0) {
  ///                                   |       goto CANCEL.EXIT.BB;
  ///                                   |     }
  ///                                   |     NOT.CANCELLED.BB:
  ///  <code_after_cancellation_point>  |     <code_after_cancellation_point>
  ///  ...                              |     ...
  ///                                   |
  ///                                   |     CANCEL.EXIT.BB:
  ///                                   |
  ///  EXIT.BB:                         |     EXIT.BB:
  ///  directive.region.exit(%x)        |     directive.region.exit(null)
  ///  return;                          |     return;
  ///
  /// \endcode
  bool genCancellationBranchingCode(WRegionNode *W);

  /// @}

  /// \brief Generate the intrinsic @llvm.invariant.group.barrier to inhibit
  /// the cse for the gep instruction related to array/struture which is marked
  /// as private, firstprivate, lastprivate, reduction or shared.
  void genCodemotionFenceforAggrData(WRegionNode *W);

  /// \brief Clean up the intrinsic @llvm.invariant.group.barrier and replace
  /// the use of the intrinsic with the its operand.
  bool clearCodemotionFenceIntrinsic(WRegionNode *W);

  enum TgtOffloadMappingFlags {
    TGT_MAP_TO =
        0x01, // instructs the runtime to copy the host data to the device.
    TGT_MAP_FROM =
        0x02, // instructs the runtime to copy the device data to the host.
    TGT_MAP_ALWAYS = 0x04, // forces the copying regardless of the reference
                           // count associated with the map.
    TGT_MAP_DELETE =
        0x08, // forces the unmapping of the object in a target data.
    TGT_MAP_IS_PTR = 0x10, // forces the runtime to map the pointer variable as
                           // well as the pointee variable.
    TGT_MAP_FIRST_REF = 0x20,  // instructs the runtime that it is the first
                               // occurrence of this mapped variable within this
                               // construct.
    TGT_MAP_RETURN_PTR = 0x40, // instructs the runtime to return the base
                               // device address of the mapped variable.
    TGT_MAP_PRIVATE_PTR =
        0x80, // informs the runtime that the variable is a private variable.
    TGT_MAP_PRIVATE_VAL = 0x100, // instructs the runtime to forward the value
                                 // to target construct.
  };

  /// \brief Returns the corresponding flag for a given map clause modifier.
  unsigned getMapTypeFlag(MapItem *MpI, bool IsFirstExprFlag,
                          bool IsFirstComponentFlag);

  /// \brief Replace the occurrences of I within the region with the return
  /// value of the intrinsic @llvm.invariant.group.barrier
  void replaceValueWithinRegion(WRegionNode *W, Value *Old);

  /// \brief Generate the intrinsic @llvm.invariant.group.barrier for
  /// local/global variable I.
  void genFenceIntrinsic(WRegionNode *W, Value *I);

  /// \brief If \p I is a call to @llvm.invariant.group.barrier, then return
  /// the CallInst*. Otherwise, return nullptr.
  CallInst* isFenceCall(Instruction *I);

  /// \brief Collect the live-in value for the phis at the loop header.
  void wrnUpdateSSAPreprocess(
      Loop *L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveoutVals,
      EquivalenceClasses<Value *> &ECs);
  /// \brief Replace the live-in value of the phis at the loop header with
  /// the loop carried value.
  void wrnUpdateSSAPreprocessForOuterLoop(
      Loop *L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveOutVals,
      EquivalenceClasses<Value *> &ECs);

  /// \brief Update the SSA form in the region using SSA Updater.
  void wrnUpdateSSAForLoopRecursively(
      Loop *L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveOutVals);

  /// \brief Collect the live-in values for the given loop.
  void wrnCollectLiveInVals(
      Loop &L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      EquivalenceClasses<Value *> &ECs);

  /// \brief Build the equivalence class for the value a, b if there exists some
  /// phi node e.g. a = phi(b).
  void buildECs(Loop *L, PHINode *PN, EquivalenceClasses<Value *> &ECs);

  /// \brief The utility to build the equivalence class for the value phi.
  void AnalyzePhisECs(Loop *L, Value *PV, Value *V,
                      EquivalenceClasses<Value *> &ECs,
                      SmallPtrSet<PHINode *, 16> &PhiUsers);

  /// \brief Collect the live-out values for a given loop.
  void wrnCollectLiveOutVals(Loop &L,
                             SmallSetVector<Instruction *, 8> &LiveOutVals,
                             EquivalenceClasses<Value *> &ECs);

  /// \brief The utility to update the liveout set from the given BB.
  void wrnUpdateLiveOutVals(Loop *L, BasicBlock *BB,
                            SmallSetVector<Instruction *, 8> &LiveOutVals,
                            EquivalenceClasses<Value *> &ECs);

  /// \brief The utility to generate the stack variable to pass the value of
  /// global variable.
  Value *genGlobalPrivatizationImpl(WRegionNode *W, GlobalVariable *G,
                                    BasicBlock *EntryBB, BasicBlock *NextExitBB,
                                    Item *IT);

  /// \brief Generate the copyprivate code.
  bool genCopyPrivateCode(WRegionNode *W, AllocaInst *IsSingleThread);

  /// \brief Generate the helper function for copying the copyprivate data.
  Function *genCopyPrivateFunc(WRegionNode *W, StructType *KmpCopyPrivateTy);

  /// \brief Process the device information into the triples.
  void processDeviceTriples();

  /// \brief Update the SSA form after the basic block LoopExitBB's successor
  /// is added one more incoming edge.
  void rewriteUsesOfOutInstructions(
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveOutVals,
      EquivalenceClasses<Value *> &ECs);

  /// \brief Transform the given OMP loop into the loop as follows.
  ///         do {
  ///             %omp.iv = phi(%omp.lb, %omp.inc)
  ///             ...
  ///             %omp.inc = %omp.iv + 1;
  ///          }while (%omp.inc <= %omp.ub)
  ///
  ///  If the flag First is true, it indicates that it is called
  ///  in the pass VPOParoptPrepare. This utility also promotes the
  ///  loop index variable into the register and performs loop rotation.
  bool regularizeOMPLoop(WRegionNode *W, bool First = true);

  /// \brief Transform the given do-while loop into the canonical form
  /// as follows.
  ///         do {
  ///             %omp.iv = phi(%omp.lb, %omp.inc)
  ///             ...
  ///             %omp.inc = %omp.iv + 1;
  ///          }while (%omp.inc <= %omp.ub)

  void fixOMPDoWhileLoop(WRegionNode *W);

  /// \brief Utility to transform the given do-while loop loop into the
  /// canonical do-while loop.
  void fixOmpDoWhileLoopImpl(Loop *L);

  /// \brief Utilty to transform the loop branch predicate from sle/ule to
  /// sgt/ugt in order to faciliate the scev based loop trip count calculation.
  void fixOmpBottomTestExpr(Loop *L);

  /// \brief Replace the use of OldV within region W with the value NewV.
  void replaceUseWithinRegion(WRegionNode *W, Value *OldV, Value *NewV);

  /// \brief Return true if one of the region W's ancestor is OMP target
  /// construct or the function where W lies in has target declare attribute.
  bool hasParentTarget(WRegionNode *W);

  /// \brief Generate the cast i8* for the incoming value BPVal.
  Value *genCastforAddr(Value *BPVal, IRBuilder<> &Builder);

  /// \brief Replace the new generated local variables with global variables
  /// in the target initialization code.
  /// Given a global variable in the offloading region, the compiler will
  /// generate different code for the following two cases.
  /// case 1: global variable is not in the map clause.
  /// The compiler generates %aaa stack variable which is initialized with
  /// the value of @aaa. The base pointer and section pointer arrays are
  /// initialized with %aaa.
  ///
  ///   #pragma omp target
  ///   {  aaa++; }
  ///
  /// ** IR Dump After VPO Paropt Pass ***
  /// entry:
  ///   %.offload_baseptrs = alloca [1 x i8*]
  ///   %.offload_ptrs = alloca [1 x i8*]
  ///   %aaa = alloca i32
  ///   %0 = load i32, i32* @aaa
  ///   store i32 %0, i32* %aaa
  ///   br label %codeRepl
  ///
  /// codeRepl:
  ///   %1 = bitcast i32* %aaa to i8*
  ///   %2 = getelementptr inbounds [1 x i8*],
  ///        [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
  ///   store i8* %1, i8** %2
  ///   %3 = getelementptr inbounds [1 x i8*],
  ///        [1 x i8*]* %.offload_ptrs, i32 0, i32 0
  ///   %4 = bitcast i32* %aaa to i8*
  ///   store i8* %4, i8** %3
  ///
  /// case 2: global variable is in the map clause
  /// The compiler initializes the base pointer and section pointer arrays
  /// with @aaa.
  ///
  ///   #pragma omp target map(aaa)
  ///   {  aaa++; }
  ///
  /// ** IR Dump After VPO Paropt Pass ***
  /// codeRepl:
  ///   %1 = bitcast i32* @aaa to i8*
  ///   %2 = getelementptr inbounds [1 x i8*],
  ///        [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
  ///   store i8* %1, i8** %2
  ///   %3 = getelementptr inbounds [1 x i8*],
  ///        [1 x i8*]* %.offload_ptrs, i32 0, i32 0
  ///   %4 = bitcast i32* @aaa to i8*
  ///   store i8* %4, i8** %3
  bool finalizeGlobalPrivatizationCode(WRegionNode *W);

  /// \brief Generate the target intialization code for the pointers based
  /// on the order of the map clause.
  void genOffloadArraysInitForClause(WRegionNode *W, TgDataInfo *Info,
                                     CallInst *Call, Instruction *InsertPt,
                                     SmallVectorImpl<Constant *> &ConstSizes,
                                     bool hasRuntimeEvaluationCaptureSize,
                                     Value *BPVal, bool &Match,
                                     IRBuilder<> &Builder, unsigned &Cnt);

  /// \brief Generate code for OMP taskgroup construct.
  /// #pragma omp taskgroup
  bool genTaskgroupRegion(WRegionNode *W);

  /// \brief Add alias_scope and no_alias metadata to improve the alias
  /// results in the outlined function.
  void improveAliasForOutlinedFunc(WRegionNode *W);

  /// \brief Set the kernel arguments' address space as ADDRESS_SPACE_GLOBAL.
  /// Propagate the address space from the arguments to the usage of the
  /// arguments.
  Function *finalizeKernelFunction(WRegionNode *W, Function *Fn,
                                   CallInst *&Call);

  ///  Generate the iteration space partitioning code based on OpenCL.
  ///  Given a loop as follows.
  ///  \code
  ///    for (i = 0; i <= ub; i++)
  ///  \endcode
  ///  The output of partitioning as below.
  ///  \code
  ///    chunk_size = (ub + get_local_size()) / get_local_size();
  ///    new_lb = get_local_id * chunk_size;
  ///    new_ub = min(chunk_size - 1, ub);
  ///    for (i = new_lb; i <= new_ub; i++)
  ///  \endcode
  ///  Here we assume the global_size is equal to local_size, which means
  ///  there is only one workgroup.
  bool genOCLParallelLoop(WRegionNode *W);

  /// \brief Generate the placeholders for the loop lower bound and upper bound.
  void genLoopBoundUpdatePrep(WRegionNode *W, AllocaInst *&LowerBnd,
                              AllocaInst *&UpperBnd);

  /// \brief Generate the OCL loop update code.
  void genOCLLoopBoundUpdateCode(WRegionNode *W, AllocaInst *LowerBnd,
                                 AllocaInst *UpperBnd);

  /// \breif Generate the OCL loop scheduling code.
  void genOCLLoopPartitionCode(WRegionNode *W, AllocaInst *LowerBnd,
                               AllocaInst *UpperBnd);
};
} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORM_H
#endif // INTEL_COLLAB
