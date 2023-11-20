//===- BarrierUtils.h - Barrier Utils ---------------------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_BARRIER_UTILS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_BARRIER_UTILS_H

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

#include <map>
#include <vector>
namespace llvm {

// Forward declaration
class Module;
class Function;
class BasicBlock;
class Instruction;
class Value;
class Type;
class StringRef;
class Twine;

// Pseudo functions for barrier passes
#define DUMMY_BARRIER_FUNC_NAME "dummy_barrier."

#define CLK_LOCAL_MEM_FENCE 0x01
#define CLK_GLOBAL_MEM_FENCE 0x02
#define CLK_CHANNEL_MEM_FENCE 0x04

#define SPECIAL_BUFFER_ADDR_SPACE 0
#define CURR_WI_ADDR_SPACE 0

typedef enum {
  CALL_BI_TYPE_WG,
  CALL_BI_TYPE_WG_ASYNC_OR_PIPE, // work-group async_copy and pipe built-ins
  CALL_BI_TYPE_WG_SORT,
  CALL_BI_NUM
} CALL_BI_TYPE;

using FunctionToUnsigned = std::map<const Function *, unsigned>;

/// \brief BarrierUtils is utility class that collects several data
/// and processes several functionality on a given module
class BarrierUtils {
public:
  using BBVec = CompilationUtils::BBVec;
  using FuncSet = CompilationUtils::FuncSet;
  using FuncVec = CompilationUtils::FuncVec;
  using InstSet = CompilationUtils::InstSet;
  using InstVec = CompilationUtils::InstVec;

  BarrierUtils();

  ~BarrierUtils() {}

  /// \brief Initialize Barrier Utils with given module to process data on
  /// \param M module to process
  void init(Module *M);

  /// \brief return the prevBB of pPhiNode with respect to pVal
  /// \param pVal value that pPhiNode is using
  /// \param pPhiNode PhiNode that is using pVal
  /// \returns BasicBlock of usage instruction with respect to value it is using
  static SmallVector<BasicBlock *> findBasicBlocksOfPhiNode(Value *pVal,
                                                            PHINode *pPhiNode);

  /// \brief return synchronize type of given instruction
  /// \param Inst instruction to observe its synchronize type
  /// \returns given instruction synchronize type {barrier dummyBarrier or none}
  SyncType getSyncType(Instruction *Inst);

  /// \brief return synchronize type of given basic block
  /// \param BB basic block to observe its synchronize type
  /// \returns given basic block synchronize type {barrier, dummyBarrier or
  /// none}
  SyncType getSyncType(BasicBlock *BB);

  /// \brief return all synchronize instructions in the module
  /// \returns container with all synchronize instructions
  InstVec getAllSynchronizeInstructions();

  /// \brief Find all functions  in the module that contain synchronize
  /// instructions
  /// \returns InstSet container with found functions
  FuncSet getAllFunctionsWithSynchronization();

  /// @bried return all functions which both call recursive functions and
  ///        synchronize functions.
  FuncSet getRecursiveFunctionsWithSync();

  /// \brief return all intel_device_barrier instructions in the module
  InstVec getDeviceBarrierCallInsts();

  /// \brief Find calls to WG functions upon their type
  /// \param type - type of WG functions to find
  /// \returns container with calls to WG functions of requested type
  InstVec getWGCallInstructions(CALL_BI_TYPE type);

  /// \brief Collect all kernels and vectorized counterparts
  /// \param KernelList - list of kernels
  /// \param List - out list with collected functions
  static FuncVec getAllKernelsAndVectorizedCounterparts(
      const SmallVectorImpl<Function *> &KernelList);

  /// \brief Find all kernel functions in the module
  /// \returns FuncVec container with found functions
  FuncVec getAllKernelsWithBarrier();

  unsigned getFunctionVectorizationWidth(const Function *F) const;

  Instruction *createGetLocalSize(unsigned Dim, Instruction *InsertBefore);
  Instruction *createGetBaseGlobalId(Value *Dim, Instruction *InsertBefore);
  /// \brief Create new call instruction to barrier()
  /// \param InsertBefore instruction to insert new call instruction before
  /// \returns new created call instruction
  Instruction *createBarrier(Instruction *InsertBefore = nullptr);

  /// \brief Checks whether call instruction calls barrier()
  /// \param pCallInstr instruction of interest
  /// \returns true if this is barrier() call and false otherwise
  bool isBarrierCall(Instruction *pCallInstr);

  /// \brief Create new call instruction to dummyBarrier()
  /// \param InsertBefore instruction to insert new call instruction before
  /// \returns new created call instruction
  Instruction *createDummyBarrier(Instruction *InsertBefore = nullptr);

  /// \brief Checks whether call instruction calls dummyBarrier()
  /// \param pCallInstr instruction of interest
  /// \returns true if this is dummyBarrier() call and false otherwise
  bool isDummyBarrierCall(Instruction *pCallInstr);

  /// \brief Create new call instruction to get_special_buffer()
  /// \param InsertBefore instruction to insert new call instruction before
  /// \returns new created call instruction
  Instruction *createGetSpecialBuffer(Instruction *InsertBefore = nullptr);

  /// \brief Create new call instruction to get_iter_count()
  /// \param InsertBefore instruction to insert new call instruction before
  /// \returns new created call instruction
  Instruction *createGetIterCount(Instruction *InsertBefore = nullptr);

  /// \brief return all get_local_id call instructions in the module
  /// \returns container with all get_local_id call instructions
  InstVec &getAllGetLocalId();

  /// \brief return all get_global_id call instructions in the module
  /// \returns container with all get_global_id call instructions
  InstVec &getAllGetGlobalId();

  /// \brief Create new call instruction to get_global_id
  /// \param Dim argument of get_global_id call
  /// \param Builder IRBuilder used to create new instructions.
  /// \returns new created call instruction
  Instruction *createGetGlobalId(unsigned Dim, IRBuilderBase &Builder);

  /// \brief Create new call instruction to get_local_id
  /// \param Dim argument of get_local_id call
  /// \param Builder IRBuilder used to create new instructions.
  /// \returns new created call instruction
  Instruction *createGetLocalId(unsigned Dim, IRBuilderBase &Builder);

  /// \brief Create call instructions to get local linear id
  /// \param Builder IRBuilder used to create new instructions.
  /// \returns local linear id result value.
  Value *createGetLocalIdLinearResult(IRBuilderBase &Builder);

  /// \brief Create call instructions to get local linear size.
  /// \param Builder IRBuilder used to create new instructions.
  /// \returns local linear size result value.
  Value *createGetLocalSizeLinearResult(IRBuilderBase &Builder);

  /// \brief Create the workgroup sort's copy call instruction.
  /// \param M Module of the workgroup sort call.
  /// \param B IRBuilder used to create new instructions.
  /// \param WGCallInst The workgroup sort call instruction.
  /// \param ToScratch The direction of copy.
  /// \param LLID Local linear id result value.
  /// \param LLSize Local linear size result value.
  /// \returns The workgroup sort's copy call instruction.
  CallInst *createWorkGroupSortCopyBuiltin(Module &M, IRBuilderBase &B,
                                           CallInst *WGCallInst, bool ToScratch,
                                           Value *LLID, Value *LLSize);

  /// \brief Replace workgroup sort_size with Local_linear_size * sort_size
  /// \param B IRBuilder used to create new instructions.
  /// \param WGCallInst The workgroup sort call instruction.
  /// \param ArgIdx The index of arg which needed to be replace.
  /// \param LLSize Local linear size result value.
  void replaceSortSizeWithTotalSize(IRBuilderBase &B, CallInst *WGCallInst,
                                    unsigned ArgIdx, Value *LLSize);

  /// \brief Replace intel_device_barrier's param with number of work groups
  /// \param NumsGroup number of work groups.
  /// \param B IRBuilder used to create new instructions.
  CallInst *createDeviceBarrierWithWGCount(Value *NumsGroup, IRBuilderBase &B);

  /// \brief return an indicator regarding given function calls
  /// a function defined in the module (that was not inlined)
  /// \param Func the given function
  /// \returns true if and only if the function calls a module function
  bool doesCallModuleFunction(Function *Func);

  /// \brief return true if there is a barrier in one of the pathes between
  ///  pValBB and pValUsageBB basic blocks.
  /// \param SyncInstructions container of all synchronize instructions
  /// \param ValUsageBB basic block to start searching the path according to its
  /// predecessors. \param ValBB basic block to stop searching the path when
  /// reach it. \returns true if and only if find a barrier in one of the
  /// searched pathes.
  static bool isCrossedByBarrier(const InstSet &SyncInstructions,
                                 BasicBlock *ValUsageBB, BasicBlock *ValBB);

  /// Return true if \p Val is barrier or dummy barrier call.
  static bool isBarrierOrDummyBarrierCall(Value *Val);

  /// \brief Find dummybarrier - dummybarrier region.
  /// \param F Function.
  inst_range findDummyRegion(Function &F);

  Type *getSizetTy() const { return SizetTy; }

  Type *getSpecialBufferValueTy() const { return SpecialBufferValueTy; }

private:
  /// \brief Clean all collected values and assure re-collecting them on the
  /// next access to them
  void clean();

  /// \brief Initialize the barrier and dummyBarrier function pointers in given
  /// module
  void initializeSyncData();

  /// \brief Find all instructions in the module, which use function with given
  /// name \param name the given function name \param usesSet container to
  /// insert all found usage instructions into
  void findAllUsesOfFunc(StringRef name, InstSet &usesSet);

private:
  /// Pointer to current processed module
  Module *M;

  /// This holds size of size_t of processed module
  unsigned int UISizeT;

  /// Pointer to value argument of barier function
  Value *LocalMemFenceValue;
  /// Pointer to barrier function in module
  Function *BarrierFunc;
  /// Pointer to dummyBarrier function in module
  Function *DummyBarrierFunc;

  /// This holds the get_special_buffer() function
  Function *GetSpecialBufferFunc;
  /// This holds the get_local_size() function
  Function *GetLocalSizeFunc;
  /// This holds the get_global_id() function
  Function *GetGIDFunc;
  /// This holds the intel_device_barrier() function
  Function *DeviceBarrierFunc = nullptr;
  /// This holds the get_local_id() function
  Function *GetLIDFunc;
  /// This holds the get_sub_group_size() function
  Function *GetSGSizeFunc;
  /// This holds the get_base_global_id() function
  Function *GetBaseGIDFunc;

  /// This holds the all sync basic blocks of the module
  BBVec SyncBasicBlocks;
  FunctionToUnsigned KernelVectorizationWidths;

  /// This indecator for synchronize data initialization
  bool SyncDataInitialized;
  /// This holds the all barrier instructions of the module
  InstSet Barriers;
  /// This holds the all dummyBarrier instructions of the module
  InstSet DummyBarriers;

  /// This indecator for get_local_id data initialization
  bool LIDInitialized;
  /// This holds the all get_local_id instructions of the module
  InstVec GetLIDInstructions;

  /// This indecator for get_global_id data initialization
  bool GIDInitialized;
  /// This holds the all get_global_id instructions of the module
  InstVec GetGIDInstructions;

  /// This indecator for functions with internal calls initialization
  bool NonInlinedCallsInitialized;
  /// This holds the all function of the module with internal calls
  FuncSet FunctionsWithNonInlinedCalls;

  Type *SizetTy = nullptr;
  Type *I32Ty = nullptr;
  Type *SpecialBufferValueTy = nullptr;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_BARRIER_UTILS_H
