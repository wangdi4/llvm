//===-------------- VecClone.h - Class definition -*- C++ -*---------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_VectorVariant.h"
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

class ModulePass;

/// Represents the mapping of a vector parameter to its corresponding vector to
/// scalar type cast instruction. This done so that the scalar loop inserted by
/// this pass contains instructions that are in scalar form so that the loop can
/// later be vectorized.
struct ParmRef {
  // Represents the parameter in one of two forms:
  // 1) A vector alloca instruction if the parameter has not been registerized.
  // 2) The parameter as the Value* passed in via the function call.
  Value *VectorParm;

  // Represents the vector parameter cast from a vector type to scalar type.
  Instruction *VectorParmCast;

  // For ordinary vector parameters - just the type of the parameter. For linear
  // byref - type of the element accessed by the pointer passed. Similarly for
  // vector byval/byref parameters - instead of storing the pointer type we'd
  // have the actual elementtype from the byval/byref attribute here.
  Type *OrigElemType;

  ParmRef(Value *VectorParm, Instruction *VectorParmCast, Type *OrigElemType)
      : VectorParm(VectorParm), VectorParmCast(VectorParmCast),
        OrigElemType(OrigElemType) {}
};

class VecCloneImpl {

#if INTEL_CUSTOMIZATION
  protected:
#endif // INTEL_CUSTOMIZATION

    /// Set of allocas to mark private for the SIMD loop
    SetVector<Value*> PrivateAllocas;

    /// \brief Make a copy of the function if it is marked as SIMD.
    Function *CloneFunction(Function &F, VectorVariant &V,
                            ValueToValueMapTy &Vmap);

    /// \brief Take the entry basic block for the function as split off a second
    /// basic block that will form the loop entry.
    BasicBlock* splitEntryIntoLoop(Function *Clone, VectorVariant &V,
                                   BasicBlock *EntryBlock);

    /// \brief Take the loop entry basic block and split off a second basic
    /// block into a new return basic block.
    BasicBlock* splitLoopIntoReturn(Function *Clone, BasicBlock *LoopBlock);

    /// \brief Create the backedge from the loop exit basic block to the loop
    /// entry block.
    PHINode* createPhiAndBackedgeForLoop(Function *Clone,
                                         BasicBlock *EntryBlock,
                                         BasicBlock *LoopBlock,
                                         BasicBlock *LoopExitBlock,
                                         BasicBlock *ReturnBlock,
                                         int VL);

    /// \brief Generate vector alloca instructions for vector parameters and
    /// change the parameter types to vector types. Expand the return value of
    /// the function to a vector type. This function returns the instruction
    /// corresponding to the expanded return and the instruction corresponding
    /// to the mask.
    Instruction *expandVectorParametersAndReturn(
        Function *Clone, Function &F, VectorVariant &V, Instruction *&Mask,
        BasicBlock *EntryBlock, BasicBlock *LoopBlock, BasicBlock *ReturnBlock,
        std::vector<ParmRef> &ParmMap, ValueToValueMapTy &VMap);

    /// \brief Expand the function parameters to vector types. This function
    /// returns the instruction corresponding to the mask. LastAlloca indicates
    /// where the alloca of the function argument should be placed in
    /// EntryBlock. We process the function arguments from left to right. The
    /// alloca of the most left argument is places at the top of the EntryBlock.
    Instruction *expandVectorParameters(Function *Clone, Function &OrigFn,
                                        VectorVariant &V,
                                        BasicBlock *EntryBlock,
                                        std::vector<ParmRef> &ParmMap,
                                        ValueToValueMapTy &VMap,
                                        AllocaInst *&LastAlloca);

    /// \brief Expand the function's return value to a vector type. LastAlloca
    /// indicates where the alloca of the return value should be placed in
    /// EntryBlock.
    Instruction *expandReturn(Function *Clone, Function &F,
                              BasicBlock *EntryBlock, BasicBlock *LoopBlock,
                              BasicBlock *ReturnBlock,
                              std::vector<ParmRef> &ParmMap,
                              AllocaInst *&LastAlloca);

    /// \brief Update the old parameter references to with the new vector
    /// references.
    void updateScalarMemRefsWithVector(Function *Clone, Function &F,
                                       BasicBlock *EntryBlock,
                                       BasicBlock *ReturnBlock, PHINode *Phi,
                                       ArrayRef<ParmRef> ParmMap);

    /// \brief Update the values of linear parameters by adding the stride
    /// before the use.
    void updateLinearReferences(Function *Clone, Function &F,
                                VectorVariant &V, PHINode *Phi);

    /// \brief Update the instructions in the return basic block to return a
    /// vector temp.
    void updateReturnBlockInstructions(Function *Clone, BasicBlock *ReturnBlock,
                                       Instruction *VecReturnAlloca);

    /// \brief Create a separate basic block to mark the begin and end of the
    /// SIMD loop formed from the vector function. Essentially, this function
    /// transfers the information from the SIMD function keywords and creates
    /// new loop pragmas so that parameter information can be transferred to
    /// the loop.
    void insertDirectiveIntrinsics(Module &M, Function *Clone, Function &F,
                                   VectorVariant &V, BasicBlock *EntryBlock,
                                   BasicBlock *LoopPreHeader,
                                   BasicBlock *LoopExitBlock,
                                   BasicBlock *ReturnBlock);

    /// \brief Create the basic block indicating the begin of the SIMD loop.
    CallInst *insertBeginRegion(Module &M, Function *Clone, Function &F,
                                VectorVariant &V, BasicBlock *EntryBlock,
                                BasicBlock *LoopPreHeader);

    /// \brief Create the basic block indicating the end of the SIMD loop.
    void insertEndRegion(Module &M, Function *Clone, BasicBlock *LoopExitBlock,
                         BasicBlock *ReturnBlock, CallInst *EntryDirCall);

    /// \brief Create a new vector alloca instruction for the return vector and
    /// bitcast to the appropriate element type. LastAlloca indicates where the
    /// alloca of the return value should be placed.
    Instruction *createExpandedReturn(Function *F, BasicBlock *BB,
                                      Type *OrigFuncReturnType,
                                      AllocaInst *&LastAlloca);

    /// \brief Check to see if the function is simple enough that a loop does
    /// not need to be inserted into the function.
    bool isSimpleFunction(Function *Func);

    /// \brief Inserts the if/else split and mask condition for masked SIMD
    /// functions.
    void insertSplitForMaskedVariant(Function *Clone, BasicBlock *LoopBlock,
                                     BasicBlock *LoopExitBlock,
                                     Instruction *Mask, PHINode *Phi);

    /// \brief Utility function that generates instructions that calculate the
    /// stride for a linear parameter.
    Value *generateStrideForParameter(Function *Clone, Argument *Arg,
                                      Instruction *ParmUser, int Stride,
                                      PHINode *Phi);

    /// \brief Removes the original scalar alloca instructions that correspond
    /// to a vector parameter before widening.
    void removeScalarAllocasForVectorParams(ArrayRef<ParmRef> VectorParmMap);

    /// \brief Adds metadata to the conditional branch of the simd loop latch to
    /// prevent loop unrolling.
    void disableLoopUnrolling(BasicBlock *Latch);

#if INTEL_CUSTOMIZATION
    /// Languages like OpenCL override this method to perform some
    /// pre-processing for enabling VecClone pass.
    virtual void languageSpecificInitializations(Module &M);

    /// Languages like OpenCL override this method. It is called after
    /// the for-loop is created.
    virtual void handleLanguageSpecifics(Function &F, PHINode *Phi,
                                         Function *Clone,
                                         BasicBlock *EntryBlock,
                                         const VectorVariant &Variant);
#endif // INTEL_CUSTOMIZATION

  public:
    VecCloneImpl() {}
    virtual ~VecCloneImpl() {}
    bool runImpl(Module &M);
}; // end pass class

class VecClonePass : public PassInfoMixin<VecClonePass> {
  VecCloneImpl Impl;

  public:
    VecClonePass() {}
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    static bool isRequired() { return true; }
};

class VecClone : public ModulePass {
  VecCloneImpl Impl;

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
