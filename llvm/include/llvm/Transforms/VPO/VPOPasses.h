//========-- VPOPasses.h - Class definitions for VPO passes -*- C++ -*-=======//
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
/// This file defines all the passes for VPO.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

#ifndef LLVM_TRANSFORMS_VPO_VPOPASSES_H
#define LLVM_TRANSFORMS_VPO_VPOPASSES_H

namespace llvm {

class FunctionPass;

namespace vpo {

class SIMDFunctionCloning : public ModulePass {

  private:

    /// \brief Vector length extracted from module metadata. The metadata is
    /// generated based on the vectorlength keyword. This field is set when
    /// the user specifies the vectorlength clause. This field is useful for
    /// direct use in function calls that require a Value parameter to generate
    /// LLVM instructions.
    Value *VectorLength;

    /// VL is just an integer conversion of VectorLength. This is used for
    /// convienence purposes when creating VectorTypes, which require an
    /// integer parameter. It may also be used when needing to do things like
    /// generating VL instances of an instruction. The conversion is done a
    /// single time, instead of converting on the fly as needed.
    int64_t VL;

    /// \brief Contains pointers to the functions that are marked as SIMD.
    SmallVector<Function*, 4> FuncsToClone;

    /// \brief Keeps parameter stride and alignment information retrieved from
    /// the module metadata. The metadata is generated based on the SIMD
    /// function keywords. The uniform and linear clauses will map to the Stride
    /// field values of 0 and some constant C, respectively. Parameters that are
    /// not marked as either uniform or linear correspond to vector parameters.
    /// The Stride field for vector parameters are set as an undef Value. The
    /// Align field is set to a constant C when the user specifies the aligned
    /// clause. If not specified, it is set to an undef Value.
    struct ArgMap {
      SmallVector<StringRef, 4> Name;
      SmallVector<Value*, 4> Stride;
      SmallVector<Value*, 4> Align;
    };

    std::map<Function*, ArgMap> FuncArgMap;

    /// \brief Return true if the function has a complex type for the return
    /// or parameters.
    bool hasComplexType(Function *F);

    /// \brief Retrieve the SIMD function attributes from the module level
    /// metadata.
    bool getFunctionAttributeMetadata(Module &M);

    /// \brief Make a copy of the function if it is marked as SIMD.
    Function* CloneFunction(Function *OrigFunc);

    /// \brief Take the entry basic block for the function as split off a second
    /// basic block that will form the loop entry.
    BasicBlock* splitEntryIntoLoop(Function *Clone, BasicBlock *EntryBlock);

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
                                         BasicBlock *ReturnBlock);

    /// \brief Generate vector alloca instructions for vector parameters and
    /// change the parameter types to vector types. Expand the return value of
    /// the function to a vector type.
    Instruction* expandVectorParametersAndReturn(
        Function *Clone,
        BasicBlock *EntryBlock,
        BasicBlock *LoopBlock,
        BasicBlock *ReturnBlock,
        std::map<AllocaInst*, Instruction*>& AllocaMap);

    /// \brief Expand the function parameters to vector types.
    void expandVectorParameters(Function *Clone,
                                std::map<AllocaInst*, Instruction*>& AllocaMap);

    /// \brief Expand the function's return value to a vector type.
    Instruction* expandReturn(Function *Clone, BasicBlock *EntryBlock,
                             BasicBlock *LoopBlock, BasicBlock *ReturnBlock,
                             std::map<AllocaInst*, Instruction*>& AllocaMap);

    /// \brief Update the old parameter references to scalar loads/stores with
    /// the new expanded references.
    void updateScalarMemRefsWithVector(
        Function *Clone,
        Function *F,
        BasicBlock *EntryBlock,
        BasicBlock *ReturnBlock,
        PHINode *Phi,
        std::map<AllocaInst*, Instruction*>& AllocaMap);
        
    /// \brief Update the values of linear parameters by adding the stride
    /// before the use.
    void updateLinearReferences(Function *Clone, Function *F, PHINode *Phi);

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
    void insertDirectiveIntrinsics(Module& M, Function *Clone, Function *F,
                                   BasicBlock *EntryBlock, 
                                   BasicBlock *LoopExitBlock,
                                   BasicBlock *ReturnBlock);

    /// \brief Create the basic block indicating the begin of the SIMD loop.
    void insertBeginRegion(Module& M, Function *Clone, Function *F,
                           BasicBlock *EntryBlock);

    /// \brief Create the basic block indicating the end of the SIMD loop.
    void insertEndRegion(Module& M, Function *Clone, BasicBlock *LoopExitBlock,
                         BasicBlock *ReturnBlock);

    /// \brief Create a new vector alloca instruction for the return vector and
    /// bitcast to the appropriate element type.
    Instruction* createExpandedReturn(Function *F, VectorType *ReturnType,
                                      Instruction *InsertPt);

    /// \brief Lookup the parameter information in the map by name. This
    /// function provides access to the the parameter's stride, alignment,
    /// etc.
    int findParmLocationInMap(Function *F, StringRef ArgName);

    /// \brief Returns true if the parameter is vector.
    bool parmIsVector(Function *F, StringRef ParmName);

    /// \brief Returns true if the parameter is linear.
    bool parmIsLinear(Function *F, StringRef ParmName);

    /// \brief Create a constant vector and return it from the function.
    void createBroadcastReturn(ReturnInst *Return);

    /// \brief Check to see if the function is simple enough that a loop does
    /// not need to be inserted into the function.
    bool isSimpleFunction(Function *F, ReturnInst *Return);

    bool runOnModule(Module &M) override;

  public:

    static char ID;
    SIMDFunctionCloning();
}; // end pass class
} // end vpo namespace

// Create VPO Driver pass
FunctionPass *createVPODriverPass();
ModulePass *createSIMDFunctionCloningPass();
}

#endif // LLVM_TRANSFORMS_VPO_VPOPASSES_H
