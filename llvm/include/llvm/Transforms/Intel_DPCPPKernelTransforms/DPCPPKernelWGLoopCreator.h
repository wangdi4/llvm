//==DPCPPKernelWGLoopCreator.h - Create WG loops in DPCPP kernels- C++ -*---==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_WGLOOPCREATOR_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_WGLOOPCREATOR_H

#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"

namespace llvm {

using MapFunctionToReturnInst = DenseMap<Function *, ReturnInst *>;

/// Create workgroup loop.
class DPCPPKernelWGLoopCreatorPass
    : public PassInfoMixin<DPCPPKernelWGLoopCreatorPass> {
public:
  DPCPPKernelWGLoopCreatorPass();

  static StringRef name() { return "DPCPPKernelWGLoopCreatorPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  /// Glue for old PM.
  bool runImpl(Module &M);

  /// Glue for old PM.
  void setFuncReturn(MapFunctionToReturnInst &FuncReturn) {
    this->FuncReturn = std::move(FuncReturn);
  }

private:
  /// Struct that contains dimesion 0 loop attributes.
  struct LoopBoundaries {
    Value *VectorLoopSize; // num vector loop iterations.
    Value *ScalarLoopSize; // num scalar loop iterations.
    Value *MaxVector;      // max vector global id

    /// C'tor.
    LoopBoundaries(Value *VectorLoopSize, Value *ScalarLoopSize,
                   Value *MaxVector)
        : VectorLoopSize(VectorLoopSize), ScalarLoopSize(ScalarLoopSize),
          MaxVector(MaxVector) {}
  };

  /// LLVM context of the current function.
  LLVMContext *Context;

  /// Prefix for name of instructions used for loop of the dimension.
  std::string DimStr;

  /// Scalar kernel return.
  ReturnInst *ScalarRet;

  /// vector kernel return.
  ReturnInst *VectorRet;

  // size_t type.
  Type *IndTy;

  /// size_t one constant.
  Constant *ConstZero;

  /// size_t one constant.
  Constant *ConstOne;

  /// size_t packet constant
  Constant *ConstPacket;

  /// Function being processed.
  Function *Func;

  /// Number of WG dimensions.
  unsigned NumDim;

  /// Scalar kernel entry.
  BasicBlock *ScalarEntry;

  /// New entry block.
  BasicBlock *NewEntry;

  /// Vectorized inner loop func.
  Function *VectorFunc;

  /// Vectorized function width.
  unsigned PacketWidth;

  /// global_id lower bounds per dimension.
  ValueVec InitGIDs;

  /// LoopSize per dimension.
  ValueVec LoopSizes;

  /// Index i contains vector with scalar kernel get_global_id(i) calls.
  InstVecVec GidCallsSc;

  /// Index i contains vector with scalar kernel get_local_id(i) calls.
  InstVecVec LidCallsSc;

  /// Index i contains vector with vector kernel get_global_id(i) calls.
  InstVecVec GidCallsVec;

  /// Index i contains vector with vector kernel get_local_id(i) calls.
  InstVecVec LidCallsVec;

  /// The dimension by which we vectorize (usually 0).
  unsigned int VectorizedDim;

  /// Map from function to its return instruction.
  MapFunctionToReturnInst FuncReturn;

  /// Collect the get_global_id(), get_local_id(), and return of F.
  /// F - kernel to collect information for.
  /// Gids - array of get_global_id call to fill.
  /// Lids - array of get_local_id call to fill.
  /// Returns kernel single return instruction.
  ReturnInst *getFunctionData(Function *F, InstVecVec &Gids, InstVecVec &Lids);

  /// Public interface that allows running on pair of scalar - vector
  /// kernels not through pass manager.
  void processFunction(Function *F, Function *VectorFunc, unsigned PacketWidth);

  /// Obtain initial global id, and loop size per dimension.
  void getLoopsBoundaries(Function *F);

  /// Add work group loops on the kernel. converts get_***_id according
  /// to the generated loops. Moves Alloca instruction in kernel entry
  /// block to the new entry block on the way.
  /// KernelEntry - entry block of the kernel.
  /// IsVector - true iff working on vector kernel
  /// Ret - singel return instruction of the kernel.
  /// GIDs - array with get_global_id calls.
  /// LIDs - array with get_local_id calls.
  /// InitGIDs - initial global id per dimension.
  /// LoopSizes - number of loop iteration per dimension.
  /// Returns struct with preheader and exit block of the outmost loop.
  LoopRegion addWGLoops(BasicBlock *KernelEntry, bool IsVector, ReturnInst *Ret,
                        InstVecVec &GIDs, InstVecVec &LIDs, ValueVec &InitGIDs,
                        ValueVec &LoopSizes);

  /// Replace the get***tid calls with incremented phi in loop head.
  /// TIDs - array of get***id to replace.
  /// InitVal - inital value (for the first iteration).
  /// IncBy - amount by which to increase the tid in each iteration.
  /// Head - head block of the loop.
  /// PreHead - pre header of the loop.
  /// Latch - latch block of the loop.
  void replaceTIDsWithPHI(InstVec &TIDs, Value *InitVal, Value *IncBy,
                          BasicBlock *Head, BasicBlock *PreHead,
                          BasicBlock *Latch);

  /// Create WG loops over vector kernel and remainder loop over scalar kernel.
  /// Returns a struct with entry and exit block of the WG loop region.
  LoopRegion createVectorAndRemainderLoops();

  /// Inline the vector kernel into the scalar kernel.
  /// BB - location to move the vector blocks.
  /// Returns the vector kernel entry block.
  BasicBlock *inlineVectorFunction(BasicBlock *BB);

  /// computes the sizes of scalar and vector loops of the zero
  ///       dimension in case vector kernel exists.
  /// InitVal - Initial global id of zero dimension.
  /// DimSize - Number of iteration of zero dimensions.
  /// Returns struct with the sizes of the vector and scalar loop + the initial
  ///         scalar loop global id.
  LoopBoundaries getVectorLoopBoundaries(Value *InitVal, Value *DimSize);

  /// Returns the "true" dimension taking into account that
  /// the vectorized dimension might not be 0. If VectorizedDim
  /// is 0, then the returned value is always the same as Dim.
  /// but suppose m_vectorizedDim is 1, then resolveDimension(1) = 0,
  /// and resolveDimension(0) = 1. Since now Dim 1 is the innermost loop.
  unsigned int resolveDimension(unsigned int Dim);

  /// Compute a string indicating current loop level to later assing it to a
  /// label.
  void computeDimStr(unsigned Dim, bool IsVector);
};

} // namespace llvm

#endif // INTEL_DPCPP_KERNEL_TRANSFORMS_WGLOOPCREATOR_H
