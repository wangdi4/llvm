//===-------- Intel_VecClone.h - Class definition -*- C++ -*---------------===//
//
// Copyright (C) 2015-2023 Intel Corporation. All rights reserved.
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

  protected:

    /// Set of memory locations to mark as private for the SIMD loop
    SetVector<Value*> PrivateMemory;
    /// Set of memory locations to mark as uniform for the SIMD loop. The map
    /// is from the arg of the function to a pair of values that represent the
    /// memory location on the stack and the load from that memory.
    MapVector<Argument*, std::pair<Value*, Value*>>  UniformMemory;
    /// Set of memory locations to mark as linear for the SIMD loop
    /// The non-key value is the stride
    MapVector<Value*, Value*> LinearMemory;

    /// The map of linear pointer args to pointee type size.
    DenseMap<Value *, Value *> PointeeTypeSize;

    /// \brief Make a copy of the function if it is marked as SIMD.
    Function *CloneFunction(Function &F, const VFInfo &V,
                            ValueToValueMapTy &VMap,
                            ValueToValueMapTy &ReverseVMap);

    /// \brief Return true iff we should bail out due to the presence of
    /// variable-length array allocas.  Correctly handling a VLA alloca
    /// and all of its operand and memory dependences is a complex
    /// undertaking.
    bool vlaAllocasExist(Function &F);

    /// \brief Take the entry basic block for the function as split off a second
    /// basic block that will form the loop entry.
    BasicBlock *splitEntryIntoLoop(Function *Clone, const VFInfo &V,
                                   BasicBlock *EntryBlock);

    /// \brief Take the loop entry basic block and split off a second basic
    /// block into a new return basic block.
    BasicBlock* splitLoopIntoReturn(Function *Clone, BasicBlock *LoopHeader);

    /// \brief Create the backedge from the loop latch basic block to the loop
    /// entry block.
    PHINode* createPhiAndBackedgeForLoop(Function *Clone,
                                         BasicBlock *EntryBlock,
                                         BasicBlock *LoopHeader,
                                         BasicBlock *LoopLatch,
                                         BasicBlock *ReturnBlock,
                                         int VL);

    /// Updates users of vector arguments with gep/load of lane element.
    void updateVectorArgumentUses(Function *Clone, Function &OrigFn,
                                  const DataLayout &DL, Argument *Arg,
                                  Type *ElemType, Instruction *VecArg,
                                  MaybeAlign Align, BasicBlock *EntryBlock,
                                  BasicBlock *LoopHeader, PHINode *Phi);

    /// Widen the function arguments and non-void return value of the function
    /// to a vector type. We process the function arguments from left to right.
    /// The alloca of the most left argument is placed at the top of the
    /// EntryBlock. This function returns the instruction corresponding to the
    /// widened return and the instruction corresponding to the mask.
    Instruction *
    widenVectorArgumentsAndReturn(Function *Clone, Function &F, const VFInfo &V,
                                  Instruction *&Mask, BasicBlock *EntryBlock,
                                  BasicBlock *LoopHeader,
                                  BasicBlock *ReturnBlock, PHINode *Phi);

    /// Mark memory as uniform for SIMD directives.
    void processUniformArgs(Function *Clone, const VFInfo &V,
                            BasicBlock *EntryBlock, BasicBlock *LoopPreheader);

    /// Update the values of linear arguments by adding the stride before the
    /// use and mark memory and linear for SIMD directives.
    void processLinearArgs(Function *Clone, const VFInfo &V, PHINode *Phi,
                           BasicBlock *EntryBlock, BasicBlock *LoopPreheader,
                           ValueToValueMapTy &ReverseVMap);

    /// \brief Update the instructions in the return basic block to return a
    /// vector temp.
    void updateReturnBlockInstructions(Function *Clone, BasicBlock *ReturnBlock,
                                       Instruction *VecReturnAlloca);

    /// \brief Create a separate basic block to mark the begin and end of the
    /// SIMD loop formed from the vector function. Essentially, this function
    /// transfers the information from the SIMD function keywords and creates
    /// new loop pragmas so that argument information can be transferred to
    /// the loop.
    void insertDirectiveIntrinsics(Module &M, Function *Clone, Function &F,
                                   const VFInfo &V, BasicBlock *EntryBlock,
                                   BasicBlock *LoopPreHeader,
                                   BasicBlock *LoopLatch,
                                   BasicBlock *ReturnBlock);

    /// \brief Create the basic block indicating the begin of the SIMD loop.
    CallInst *insertBeginRegion(Module &M, Function *Clone, Function &F,
                                const VFInfo &V, BasicBlock *EntryBlock,
                                BasicBlock *LoopPreHeader);

    /// \brief Create the basic block indicating the end of the SIMD loop.
    void insertEndRegion(Module &M, Function *Clone, BasicBlock *LoopLatch,
                         BasicBlock *ReturnBlock, CallInst *EntryDirCall);

    /// \brief Check to see if the function is simple enough that a loop does
    /// not need to be inserted into the function.
    bool isSimpleFunction(Function *Func);

    /// \brief Inserts the if/else split and mask condition for masked SIMD
    /// functions.
    void insertSplitForMaskedVariant(Function *Clone, BasicBlock *LoopHeader,
                                     BasicBlock *LoopLatch,
                                     Instruction *Mask, PHINode *Phi);

    /// \brief Utility function that generates instructions that calculate the
    /// stride for a linear argument.
    Value *generateStrideForArgument(Function *Clone, Value *ArgVal,
                                     Instruction *ParmUser, Value *Stride,
                                     PHINode *Phi, const VFParameter &Parm,
                                     ValueToValueMapTy &ReverseVMap);

    /// \brief Adds metadata to the conditional branch of the simd loop latch to
    /// prevent loop unrolling.
    void disableLoopUnrolling(BasicBlock *Latch);

    /// Languages like OpenCL override this method to perform some
    /// pre-processing for enabling VecClone pass.
    virtual void languageSpecificInitializations(Module &M);

    /// Languages like OpenCL override this method. It is called after
    /// the for-loop is created.
    virtual void handleLanguageSpecifics(Function &F, PHINode *Phi,
                                         Function *Clone,
                                         BasicBlock *EntryBlock,
                                         const VFInfo &Variant);

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
