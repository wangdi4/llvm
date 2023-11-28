//===-------- Intel_VecClone.h - Class definition -*- C++ -*---------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the VecClone pass class.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#ifndef LLVM_TRANSFORMS_VPO_VECCLONE_H
#define LLVM_TRANSFORMS_VPO_VECCLONE_H

namespace llvm {

struct VFInfo;
struct VFParameter;
class ModulePass;

class VecCloneImpl {
  class Factory {
  private:
    class VecCloneImpl *Parent;
    /// The clone being produced
    Function *Clone = nullptr;
    /// Pointers to blocks of the clone beeing produced
    BasicBlock *EntryBlock = nullptr;
    BasicBlock *LoopHeader = nullptr;
    BasicBlock *LoopPreHeader = nullptr;
    BasicBlock *LoopLatch = nullptr;
    BasicBlock *ReturnBlock = nullptr;
    /// Type for each logical argument of a clone
    SmallVector<Type *, 4> LogicalArgTypes;
    /// The clone return type
    Type *LogicalRetType = nullptr;
    /// Stores type legalization information for each logical argument of the
    /// Clone and its return type. That is basically the number of parts
    /// required to pass each logical argument and/or return value.
    SmallVector<int, 4> ArgChunks;
    int RetChunks = 1;

    /// Set of memory locations to mark as private for the SIMD loop
    SetVector<Value *> PrivateMemory;
    /// Set of memory locations to mark as uniform for the SIMD loop. The map
    /// is from the arg of the function to a pair of values that represent the
    /// memory location on the stack and the load from that memory.
    MapVector<Argument *, std::pair<Value *, Value *>> UniformMemory;
    /// Set of memory locations to mark as linear for the SIMD loop
    /// The non-key value is the stride
    MapVector<Value *, Value *> LinearMemory;

    /// The vector of linear pointer args to pointee type size. If the arg is
    /// not a pointer, its position in the vector will be nullptr.
    const SmallVectorImpl<Value *> &PointeeTypeSize;

    /// Maps of values between the original function and a copy.
    ValueToValueMapTy VMap;

    Module &M;
    const DataLayout &DL;
    Function &F;
    const VFInfo &V;

  public:
    Factory(VecCloneImpl *Parent, Module &M, const DataLayout &DL, Function &Fn,
            const VFInfo &Variant,
            const SmallVectorImpl<Value *> &PointeeTypeSize)
        : Parent(Parent), PointeeTypeSize(PointeeTypeSize), M(M), DL(DL), F(Fn),
          V(Variant) {}

    Function *run();

  private:
    void cloneFunction();
    /// Take the entry basic block for the function as split off a second
    /// basic block that will form the loop entry.
    BasicBlock *splitEntryIntoLoop();

    /// Take the loop entry basic block and split off a second basic
    /// block into a new return basic block.
    BasicBlock *splitLoopIntoReturn();

    /// Create the backedge from the loop latch basic block to the loop
    /// entry block.
    PHINode *createPhiAndBackedgeForLoop();

    /// Updates users of vector arguments with gep/load of lane element.
    void updateVectorArgumentUses(Argument *Arg, Argument *OrigArg,
                                  Type *ElemType, Instruction *VecArg,
                                  MaybeAlign Align, PHINode *Phi);

    /// Widen the function arguments and non-void return value of the function
    /// to a vector type. We process the function arguments from left to right.
    /// The alloca of the most left argument is placed at the top of the
    /// EntryBlock. This function returns the instruction corresponding to the
    /// widened return and the instruction corresponding to the mask.
    Instruction *widenVectorArgumentsAndReturn(Instruction *&Mask,
                                               PHINode *Phi);

    // Worker for widenVectorArgumentsAndReturn which generates unpacking
    // instructions to convert mask packed as bits of an integer argument into
    // vector of characteristic data type(i.e.logical mask type).
    // Used to unpack mask for a vector variant of AVX512 ISA class.
    // \p VecArgTy is the logical mask type,
    // \p Arg is actual ("packed") mask argument.
    Value *generateUnpackIntMask(FixedVectorType *VecArgTy, Value *Arg,
                                 Instruction *InsertPt);

    /// Mark memory as uniform for SIMD directives.
    void processUniformArgs();

    /// Returns the pointer element type of a pointer argument.
    Type* getArgPointerElementType(Argument *Arg, Value *ScalarArg);

    /// Update the values of linear arguments by adding the stride before the
    /// use and mark memory and linear for SIMD directives.
    void processLinearArgs(PHINode *Phi);

    /// Update the instructions in the return basic block to return a
    /// vector temp.
    void updateReturnBlockInstructions(Instruction *VecReturnAlloca);

    /// Create a separate basic block to mark the begin and end of the
    /// SIMD loop formed from the vector function. Essentially, this function
    /// transfers the information from the SIMD function keywords and creates
    /// new loop pragmas so that argument information can be transferred to
    /// the loop.
    void insertDirectiveIntrinsics();

    /// Create the basic block indicating the begin of the SIMD loop.
    CallInst *insertBeginRegion();

    /// Create the basic block indicating the end of the SIMD loop.
    void insertEndRegion(CallInst *EntryDirCall);

    /// Check to see if the function is simple enough that a loop does
    /// not need to be inserted into the function.
    bool isSimpleFunction();

    /// Inserts the if/else split and mask condition for masked SIMD
    /// functions.
    void insertSplitForMaskedVariant(Instruction *Mask, PHINode *Phi);

    /// Utility function that generates instructions that calculate the
    /// stride for a linear argument.
    Value *generateStrideForArgument(Value *ArgVal, unsigned ArgIdx,
                                     Instruction *ParmUser, Value *Stride,
                                     PHINode *Phi, const VFParameter &Parm);
    /// Adds metadata to the conditional branch of the simd loop latch to
    /// prevent loop unrolling.
    void disableLoopUnrolling();
  };

  /// Return true iff we should bail out due to the presence of
  /// variable-length array allocas.  Correctly handling a VLA alloca
  /// and all of its operand and memory dependences is a complex
  /// undertaking.
  bool vlaAllocasExist(Function &F);

protected:
  /// Languages like OpenCL override this method. It is called after
  /// the for-loop is created.
  virtual void handleLanguageSpecifics(Function &F, PHINode *Phi,
                                       Function *Clone, BasicBlock *EntryBlock,
                                       const VFInfo &Variant,
                                       const ValueToValueMapTy &VMap);
  /// Languages like OpenCL override this method to perform some
  /// pre-processing for enabling VecClone pass.
  virtual void languageSpecificInitializations(Module &M);

public:
  VecCloneImpl() {}
  virtual ~VecCloneImpl() {}
  bool runImpl(Module &M, OptReportBuilder *ORBuilder = nullptr,
               LoopOptLimiter Limiter = LoopOptLimiter::None);
}; // end pass class

class VecClonePass : public PassInfoMixin<VecClonePass> {
  VecCloneImpl Impl;
  OptReportBuilder ORBuilder;

public:
  VecClonePass() {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  static bool isRequired() { return true; }
};

class VecClone : public ModulePass {
  VecCloneImpl Impl;
  OptReportBuilder ORBuilder;

protected:
  bool runOnModule(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

public:
  static char ID;
  VecClone();
};

ModulePass *createVecClonePass();

} // end llvm namespace

#endif // LLVM_TRANSFORMS_VPO_VECCLONE_H
