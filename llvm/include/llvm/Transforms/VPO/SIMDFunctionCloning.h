//===--------- SIMDFunctionCloning.h - Class definition -*- C++ -*---------===//
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
/// This file defines the SIMDFunctionCloning pass class.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "../../../../lib/Transforms/VPO/Vecopt/Utils/VectorVariant.h"
#include "../../../../lib/Transforms/VPO/Vecopt/Services/utils/VectorizerUtils.h"

#ifndef LLVM_TRANSFORMS_VPO_SIMDFUNCTIONCLONING_H
#define LLVM_TRANSFORMS_VPO_SIMDFUNCTIONCLONING_H

enum InstType {
  ALLOCA = 0,
  STORE,
  BITCAST
};

namespace llvm {

class ModulePass;

namespace vpo {

class SIMDFunctionCloning : public ModulePass {

  private:

    /// \brief Return true if the function has a complex type for the return
    /// or parameters.
    bool hasComplexType(Function *F);

    /// \brief Make a copy of the function if it is marked as SIMD.
    Function* CloneFunction(Function &F, intel::VectorVariant &V);

    /// \brief Take the entry basic block for the function as split off a second
    /// basic block that will form the loop entry.
    BasicBlock* splitEntryIntoLoop(Function *Clone, intel::VectorVariant &V,
                                   BasicBlock *EntryBlock);

    /// \brief Take the loop entry basic block and split off a second basic
    /// block into a new return basic block.
    BasicBlock* splitLoopIntoReturn(Function *Clone, BasicBlock *LoopBlock);

    /// \brief Generate a basic block to test the loop exit condition.
    BasicBlock* createLoopExit(Function *Clone, BasicBlock *ReturnBlock);

    /// \brief Update the predecessors of the return basic block.
    void updateReturnPredecessors(Function *Clone, BasicBlock *LoopExitBlock,
                                  BasicBlock *ReturnBlock);

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
    Instruction* expandVectorParametersAndReturn(
        Function *Clone,
        intel::VectorVariant &V,
        Instruction **Mask,
        BasicBlock *EntryBlock,
        BasicBlock *LoopBlock,
        BasicBlock *ReturnBlock,
        std::map<AllocaInst*, Instruction*>& AllocaMap);

    /// \brief Expand the function parameters to vector types. This function
    /// returns the instruction corresponding to the mask.
    Instruction* expandVectorParameters(
        Function *Clone,
        intel::VectorVariant &V,
        BasicBlock *EntryBlock,
        std::map<AllocaInst*, Instruction*>& AllocaMap);

    /// \brief Expand the function's return value to a vector type.
    Instruction* expandReturn(Function *Clone, BasicBlock *EntryBlock,
                             BasicBlock *LoopBlock, BasicBlock *ReturnBlock,
                             std::map<AllocaInst*, Instruction*>& AllocaMap);

    /// \brief Update the old parameter references to scalar loads/stores with
    /// the new expanded references.
    void updateScalarMemRefsWithVector(
        Function *Clone,
        Function &F,
        BasicBlock *EntryBlock,
        BasicBlock *ReturnBlock,
        PHINode *Phi,
        std::map<AllocaInst*, Instruction*>& AllocaMap);
        
    /// \brief Update the values of linear parameters by adding the stride
    /// before the use.
    void updateLinearReferences(Function *Clone, Function &F,
                                intel::VectorVariant &V, PHINode *Phi);

    /// \brief Remove any scalar alloca instructions that have been replaced
    /// with vector alloca instructions during parameter/return expansion.
    void removeScalarMemRefs(std::map<AllocaInst*, Instruction*>& AllocaMap);

    /// \brief Update the instructions in the return basic block to return a
    /// vector temp.
    void updateReturnBlockInstructions(Function *Clone, BasicBlock *ReturnBlock,
                                       Instruction *VecReturnAlloca);

    /// \brief Create a separate basic block to mark the begin and end of the
    /// SIMD loop formed from the vector function. Essentially, this function
    /// transfers the information from the SIMD function keywords and creates
    /// new loop pragmas so that parameter information can be transferred to
    /// the loop.
    void insertDirectiveIntrinsics(Module& M, Function *Clone, Function &F,
                                   intel::VectorVariant &V,
                                   BasicBlock *EntryBlock, 
                                   BasicBlock *LoopExitBlock,
                                   BasicBlock *ReturnBlock);

    /// \brief Create the basic block indicating the begin of the SIMD loop.
    void insertBeginRegion(Module& M, Function *Clone, Function &F,
                           intel::VectorVariant &V, BasicBlock *EntryBlock);

    /// \brief Create the basic block indicating the end of the SIMD loop.
    void insertEndRegion(Module& M, Function *Clone, BasicBlock *LoopExitBlock,
                         BasicBlock *ReturnBlock);

    /// \brief Create a new vector alloca instruction for the return vector and
    /// bitcast to the appropriate element type.
    Instruction* createExpandedReturn(Function *F, BasicBlock *BB,
                                      VectorType *ReturnType);

    /// \brief Return the position of the parameter in the function's parameter
    /// list.
    int getParmIndexInFunction(Function *F, Value *Parm);

    /// \brief Create a constant vector and return it from the function.
    void createBroadcastReturn(intel::VectorVariant &V, ReturnInst *Return);

    /// \brief Check to see if the function is simple enough that a loop does
    /// not need to be inserted into the function.
    bool isSimpleFunction(Function &F, intel::VectorVariant &V,
                          ReturnInst *Return);

    /// \brief Find the initial parameter store to alloca'd memory for uniform
    /// parameters and sink the store instruction into the loop.
    void sinkUniformParmStoresIntoLoop(Function *Clone, Function &F,
                                       intel::VectorVariant &V,
                                       BasicBlock *EntryBlock,
                                       BasicBlock *LoopBlock);

    /// \brief Utility function that returns the last instruction in the basic
    /// block based on the instruction type indicated by IT.
    Instruction* findLastInstInBlock(BasicBlock *BB, InstType IT);

    /// \brief Inserts the if/else split and mask condition for masked SIMD
    /// functions.
    void insertSplitForMaskedVariant(Function *Clone, BasicBlock *LoopBlock,
                                     BasicBlock *LoopExitBlock,
                                     Instruction *Mask, PHINode *Phi);

    bool runOnModule(Module &M) override;

  public:

    static char ID;
    SIMDFunctionCloning();
}; // end pass class

} // end vpo namespace
} // end llvm namespace

#endif // LLVM_TRANSFORMS_VPO_SIMDFUNCTIONCLONING_H
