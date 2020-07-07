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

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include <string>

#define SPECIAL_BUFFER_ADDR_SPACE 0
#define CLK_LOCAL_MEM_FENCE 0x01
#define GET_SPECIAL_BUFFER "get_special_buffer."

#define NO_BARRIER_PATH_ATTRNAME "dpcpp-no-barrier-path"

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

using FunctionToUnsigned = std::map<const Function *, unsigned>;

static constexpr const char BarrierName[] = "__builtin_dpcpp_kernel_barrier";
static constexpr const char DummyBarrierName[] =
    "__builtin_dpcpp_kernel_barrier_dummy";

static std::string AppendWithDimension(std::string S, int Dimension) {
  if (Dimension >= 0)
    S += '0' + Dimension;
  else
    S += "var";
  return S;
}

static std::string AppendWithDimension(std::string S,
                                       const llvm::Value *Dimension) {
  int D = -1;
  if (const llvm::ConstantInt *C = llvm::dyn_cast<llvm::ConstantInt>(Dimension))
    D = C->getZExtValue();
  return AppendWithDimension(S, D);
}

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

  /// Find all kernel functions in the module,
  /// Returns FuncVector container with found functions.
  FuncVector &getAllKernelsWithBarrier();

  /// Return an indicator regarding given function calls
  /// a function defined in the module (that was not inlined).
  /// F the given function,
  /// Returns true if and only if the function calls a module function.
  bool doesCallModuleFunction(Function *F);

  /// Create new call instruction to barrier(),
  /// InsertBefore instruction to insert new call instruction before.
  /// Returns new created call instruction.
  Instruction* createBarrier(Instruction *InsertBefore = nullptr);

  /// Create new call instruction to dummyBarrier(),
  /// InsertBefore instruction to insert new call instruction before.
  /// Returns new created call instruction.
  Instruction* createDummyBarrier(Instruction *InsertBefore = nullptr);

  /// Create new call instruction to get_special_buffer().
  /// InsertBefore instruction to insert new call instruction before,
  /// Returns new created call instruction/
  Instruction *createGetSpecialBuffer(Instruction *InsertBefore = nullptr);

  Instruction *createGetLocalSize(unsigned Dim, Instruction *InsertBefore);

  unsigned getKernelVectorizationWidth(const Function *F) const;

  /// Return all get_local_id call instructions in the module.
  /// Returns container with all get_local_id call instructions.
  InstVector &getAllGetLocalId();

  /// @brief return true if there is a barrier in one of the pathes between
  ///  pValBB and pValUsageBB basic blocks.
  /// @param SyncInstructions container of all synchronize instructions
  /// @param ValUsageBB basic block to start searching the path
  ///   according to its predecessors.
  /// @param ValBB basic block to stop searching the path when reach it.
  /// @returns true if and only if find a barrier in one of the searched
  ///   pathes.
  bool isCrossedByBarrier(InstSet &SyncInstructions, BasicBlock *ValUsageBB,
                          BasicBlock *ValBB);

  /// @brief Check whether instruction is implicit GID.
  /// @param AI Alloca instruction.
  /// @return true if the instruction is implicit GID, false otherwise.
  bool isImplicitGID(AllocaInst *AI);

  /// Return BasicBlock of UserInst (if it is not a PHINode),
  ///  Otherwise, return the prevBB of UserInst with respect to V.
  /// V value that pUserInst is using,
  /// UserInst instruction that is using I value,
  /// Returns BasicBlock of usage instruction with respect to value it is using.
  static BasicBlock *findBasicBlockOfUsageInst(Value *V, Instruction *UserInst);

  /// Collect all kernels and vectorized counterparts.
  /// KernelList - list of kernels,
  /// M - Module,
  /// Returns List - out list with collected functions.
  static FuncVector getAllKernelsAndVectorizedCounterparts(
      const SmallVectorImpl<Function *> &KernelList, Module *M);

private:
  /// Clean all collected values and assure
  /// re-collecting them on the next access to them.
  void clean();

  /// Initialize the barrier and dummyBarrier function pointers in given module.
  void initializeSyncData();

  /// Find all instructions in the module, which use function with the given.
  /// name.
  void findAllUsesOfFunc(llvm::StringRef Name, InstSet &UsesSet);

  /// Create function declaration with given name and type specification.
  /// Name the given function name,
  /// Result type of return value of the function,
  /// FuncTyArgs types vector of all arguments values of the function,
  /// Returns Function new declared function.
  Function *createFunctionDeclaration(const llvm::Twine &Name, Type *Result,
                                      std::vector<Type *> &FuncTyArgs);

  /// Add ReadNone attribute to given function.
  /// Func the given function.
  void SetFunctionAttributeReadNone(Function *Func);

private:
  /// Pointer to current processed module.
  Module *M;

  /// This holds size of size_t of processed module.
  unsigned int SizeTSize;

  /// Pointer to value argument of barier function.
  Value     *LocalMemFenceValue;
  /// Pointer to barrier function in module.
  Function  *BarrierFunc;
  /// Pointer to dummyBarrier function in module.
  Function  *DummyBarrierFunc;

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

  /// This holds the all kernel functions of the module.
  FuncVector KernelFunctions;

  /// This holds Kernel to VF mapping.
  FunctionToUnsigned KernelVectorizationWidths;

  /// This indecator for functions with internal calls initialization.
  bool HasNonInlinedCallsInitialized;

  /// This holds the all function of the module with internal calls.
  FuncSet FunctionsWithNonInlinedCalls;

  /// This holds the get_special_buffer() function.
  Function *GetSpecialBufferFunc;

  /// This holds the get_local_size() function.
  Function *GetLocalSizeFunc;

  Type *SizeTTy;
  Type *I32Ty;

  /// This indecator for get_local_id data initialization.
  bool HasLIDInstsInitialized;

  /// This holds the all get_local_id instructions of the module.
  InstVector GetLIDInstructions;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_UTILS_H
