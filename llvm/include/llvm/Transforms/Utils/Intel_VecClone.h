//===-------------- VecClone.h - Class definition -*- C++ -*---------------===//
//
// Copyright (C) 2015-2022 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#ifndef LLVM_TRANSFORMS_VPO_VECCLONE_H
#define LLVM_TRANSFORMS_VPO_VECCLONE_H

enum InstType {
  ALLOCA = 0,
  STORE,
  BITCAST
};

namespace llvm {

struct VFInfo;
class ModulePass;

class VecCloneImpl {

#if INTEL_CUSTOMIZATION
  protected:
#endif // INTEL_CUSTOMIZATION

    /// Set of memory locations to mark as private for the SIMD loop
    SetVector<Value*> PrivateMemory;
    /// Set of memory locations to mark as uniform for the SIMD loop. The map
    /// is from the arg of the function to a pair of values that represent the
    /// memory location on the stack and the load from that memory.
    MapVector<Argument*, std::pair<Value*, Value*>>  UniformMemory;
    /// Set of memory locations to mark as linear for the SIMD loop
    /// The non-key value is the stride
    MapVector<Value*, Value*> LinearMemory;

    /// \brief Make a copy of the function if it is marked as SIMD.
    Function *CloneFunction(Function &F, const VFInfo &V,
                            ValueToValueMapTy &Vmap);

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

    /// \brief Generate vector alloca instructions for vector arguments and
    /// change the arguments types to vector types. Widen the return value of
    /// the function to a vector type. This function returns the instruction
    /// corresponding to the widened return and the instruction corresponding
    /// to the mask.
    Instruction *widenVectorArgumentsAndReturn(
        Function *Clone, Function &F, const VFInfo &V, Instruction *&Mask,
        BasicBlock *EntryBlock, BasicBlock *LoopHeader, BasicBlock *ReturnBlock,
        PHINode *Phi, ValueToValueMapTy &VMap);

    /// Updates users of vector arguments with gep/load of lane element.
    void updateVectorArgumentUses(Function *Clone, Function &OrigFn,
                                  const DataLayout &DL, Argument *Arg,
                                  Type *ElemType, BitCastInst *VecArgCast,
                                  BasicBlock *EntryBlock,
                                  BasicBlock *LoopHeader, PHINode *Phi);

    /// \brief Widen the function arguments to vector types. This function
    /// returns the instruction corresponding to the mask. LastAlloca indicates
    /// where the alloca of the function argument should be placed in
    /// EntryBlock. We process the function arguments from left to right. The
    /// alloca of the most left argument is placed at the top of the EntryBlock.
    Instruction *widenVectorArguments(Function *Clone, Function &OrigFn,
                                      const VFInfo &V, BasicBlock *EntryBlock,
                                      BasicBlock *LoopHeader, PHINode *Phi,
                                      ValueToValueMapTy &VMap,
                                      AllocaInst *&LastAlloca);

    /// \brief Widen the function's return value to a vector type. LastAlloca
    /// indicates where the alloca of the return value should be placed in
    /// EntryBlock.
    Instruction *widenReturn(Function *Clone, Function &F,
                             BasicBlock *EntryBlock, BasicBlock *LoopHeader,
                             BasicBlock *ReturnBlock, PHINode *Phi,
                             AllocaInst *&LastAlloca);

    /// Mark memory as uniform for SIMD directives.
    void processUniformArgs(Function *Clone, const VFInfo &V,
                            BasicBlock *EntryBlock, BasicBlock *LoopPreheader);

    /// Mark memory as aligned for SIMD directives.
    void processAlignedArgs(Function *Clone, const VFInfo &V,
                            BasicBlock *EntryBlock, BasicBlock *LoopPreheader);

    /// Update the values of linear arguments by adding the stride before the
    /// use and mark memory and linear for SIMD directives.
    void processLinearArgs(Function *Clone, const VFInfo &V, PHINode *Phi,
                           BasicBlock *EntryBlock, BasicBlock *LoopPreheader);

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

    /// \brief Create a new vector alloca instruction for the return vector and
    /// bitcast to the appropriate element type. LastAlloca indicates where the
    /// alloca of the return value should be placed.
    Instruction *createWidenedReturn(Function *F, BasicBlock *BB,
                                     Type *OrigFuncReturnType,
                                     AllocaInst *&LastAlloca);

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
                                     PHINode *Phi);

    /// \brief Adds metadata to the conditional branch of the simd loop latch to
    /// prevent loop unrolling.
    void disableLoopUnrolling(BasicBlock *Latch);

#if INTEL_CUSTOMIZATION
    /// Filter out unsupported R/U/L encodings.
    /// Can be removed once these encodings are supported.
    void filterUnsupportedVectorVariants(Module &M,
                                         SmallVector<Function *, 8> &DiagList,
                                         OptReportBuilder *ORBuilder);

    /// Languages like OpenCL override this method to perform some
    /// pre-processing for enabling VecClone pass.
    virtual void languageSpecificInitializations(Module &M);

    /// Languages like OpenCL override this method. It is called after
    /// the for-loop is created.
    virtual void handleLanguageSpecifics(Function &F, PHINode *Phi,
                                         Function *Clone,
                                         BasicBlock *EntryBlock,
                                         const VFInfo &Variant);
#endif // INTEL_CUSTOMIZATION

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
