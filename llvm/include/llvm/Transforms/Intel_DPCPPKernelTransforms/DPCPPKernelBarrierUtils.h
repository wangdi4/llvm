//==--- DPCPPKernelBarrierUtils.h - Barrier helper functions - C++ -*-------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_UTILS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_UTILS_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"

namespace llvm {

typedef enum {
  SyncTypeNone,
  SyncTypeBarrier,
  SyncTypeDummyBarrier,
  SyncTypeFiber,
  SyncTypeNum
} SyncType;

using InstVector = SmallVector<llvm::Instruction *, 8>;
using ValueVector = SmallVector<llvm::Value *, 8>;
using FuncVector = SmallVector<llvm::Function *, 8>;
using BasicBlockVector = SmallVector<llvm::BasicBlock *, 8>;

using InstSet = SetVector<llvm::Instruction *>;
using FuncSet = SetVector<llvm::Function *>;
using BasicBlockSet = SetVector<llvm::BasicBlock *>;

static constexpr const char BarrierName[] = "__builtin_dpcpp_kernel_barrier";
static constexpr const char DummyBarrierName[] =
    "__builtin_dpcpp_kernel_barrier_dummy";

/// DPCPPKernelBarrierUtils is utility class that collects several data
/// and processes several functionality on a given module.
class DPCPPKernelBarrierUtils {
public:
  DPCPPKernelBarrierUtils();

  ~DPCPPKernelBarrierUtils() {}

  /// Initialize Barrier Utils with given module to process data on.
  /// M module to process.
  void init(Module *M);

  /// Return all synchronize instructions in the module.
  /// Returns container with all synchronize instructions.
  InstVector &getAllSyncInstructions();

  /// Find all functions in the module.
  /// that contain synchronize instructions.
  /// Returns FuncSet container with found functions.
  FuncSet &getAllFunctionsWithSynchronization();

  /// Return synchronize type of given instruction.
  /// I instruction to observe its synchronize type.
  /// Returns given instruction synchronize type:
  ///  {barrier, fiber, dummyBarrier or none}.
  SyncType getSynchronizeType(const Instruction *I);

  /// Checks whether call instruction calls dummyBarrier().
  /// CI instruction of interest,
  /// Returns true if this is dummyBarrier() call and false otherwise.
  bool isDummyBarrierCall(const CallInst *CI);

  /// Checks whether call instruction calls barrier().
  /// CI instruction of interest,
  /// Returns true if this is barrier() call and false otherwise.
  bool isBarrierCall(const CallInst *CI);

  /// Return BasicBlock of UserInst (if it is not a PHINode)
  ///  Otherwise, return the prevBB of UserInst with respect to V.
  /// V value that pUserInst is using,
  /// UserInst instruction that is using I value,
  /// Returns BasicBlock of usage instruction with respect to value it is using.
  static BasicBlock *findBasicBlockOfUsageInst(Value *V, Instruction *UserInst);

private:
  /// Clean all collected values and assure
  /// re-collecting them on the next access to them.
  void clean();

  /// Initialize the barrier and dummyBarrier function pointers in given module.
  void initializeSyncData();

  /// Find all instructions in the module, which use function with the given.
  /// name.
  void findAllUsesOfFunc(llvm::StringRef Name, InstSet &UsesSet);

private:
  /// Pointer to current processed module.
  Module *M;

  /// This indicates that synchronize data is initialized.
  bool IsSyncDataInitialized;

  /// This holds the all barrier instructions of the module.
  InstSet Barriers;

  /// This holds the all dummyBarrier instructions of the module.
  InstSet DummyBarriers;

  /// This holds the all sync instructions of the module.
  InstVector SyncInstructions;

  /// This holds the all functions of the module with sync instruction.
  FuncSet SyncFunctions;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_UTILS_H
