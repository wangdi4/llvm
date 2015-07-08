//=-- SIMDFunctionCloning.cpp - Vector function to loop transform -*- C++ -*-=//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// Main author:
// ------------
// Matt Masten (C) 2015 [matt.masten@intel.com]
//
// Major revisions:
// ----------------
// May 2015, initial development -- Matt Masten
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This pass injects a scalar loop in SIMD declared functions that has a vector
/// length trip count. The pass follows the Intel vector ABI requirements for
/// name mangling and generates both masked and non-masked variants of the
/// function. The appropriate directives are added around the loop to transfer
/// parameter information from the function keywords. The only parameters that
/// are expanded are those defined as vector parameters. One key design point to
/// keep in mind is that for vector parameters/return, the old scalar references
/// referring to the parameters/return must be replaced with a "vector"
/// reference. This is done by allocating a new vector and then bitcasting this
/// vector to a pointer of the element type. In turn, this bitcast is used to
/// replace the old scalar references. This allows the generated LLVM to appear
/// as a more scalar representation. This pass was primarily implemented so that
/// AVR construction could be simplified by not having to worry about both
/// functions and loops.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include <map>
#include <set>

#define SV_NAME "simd-function-cloning"
#define DEBUG_TYPE "SIMDFunctionCloning"

using namespace llvm;
using namespace llvm::vpo;

SIMDFunctionCloning::SIMDFunctionCloning() : ModulePass(ID) { }

bool SIMDFunctionCloning::hasComplexType(Function *F)
{
  Function::ArgumentListType &ArgList = F->getArgumentList();
  Function::ArgumentListType::iterator ArgListIt = ArgList.begin();
  Function::ArgumentListType::iterator ArgListEnd = ArgList.end();

  for (; ArgListIt != ArgListEnd; ++ArgListIt) {
    // Complex types for parameters/return come in as vector.
    if (ArgListIt->getType()->isVectorTy()) {
      return true;
    }
  }

  return false;
}

// Note: This function will be replaced when the front-end checks in the changes
// that put the vector function information in function attributes instead of
// metadata. Instead of populating the FuncArgMap, we can use Gil's
// VectorVariant class that will be ported to VPOUtils.
bool SIMDFunctionCloning::getFunctionAttributeMetadata(Module &M)
{

  NamedMDNode *NMDNode = M.getNamedMetadata("cilk.functions");

  if (!NMDNode) {
    return false; // LLVM IR has not been modified
  }

  NamedMDNode::op_iterator NMDIt = NMDNode->op_begin();
  NamedMDNode::op_iterator NMDEnd = NMDNode->op_end();

  // Named Metadata nodes consist of MDNodes.
  for (; NMDIt != NMDEnd; ++NMDIt) {

    MDNode *MDNodeList = *NMDIt;
    MDNode::op_iterator MDIt = MDNodeList->op_begin();
    MDNode::op_iterator MDEnd = MDNodeList->op_end();
    Function *F = NULL;

    // MDNodes consist of operands.
    for (; MDIt != MDEnd; ++MDIt) {

      const MDOperand *MDOp = MDIt;
      // Metadata of the operand
      const Metadata *MD = MDOp->get();

      // The first part of the "cilk.functions" metadata should be the
      // signature of the vector function.
      if (MD->getMetadataID() == Metadata::ConstantAsMetadataKind) {

        // ConstantAsMetadata inherits from Value, so we can try to
        // dynamically cast to Function*. If F is not null, we found
        // a function that should be cloned.
        const ConstantAsMetadata *CMD = dyn_cast<ConstantAsMetadata>(MD);

        F = dyn_cast<Function>(CMD->getValue());
        assert(F && "Expected metadata to indicate a function to clone");

        if (!hasComplexType(F))
          FuncsToClone.push_back(F);
      }

      // The next part of the "cilk.functions" metadata should be a series
      // of tuples that represent the parameter names, parameter strides,
      // parameter alignment, and vector length.
      if (MD->getMetadataID() == Metadata::MDTupleKind) {

        const MDTuple *Tuple = dyn_cast<MDTuple>(MD);
        const MDOperand& Mode = Tuple->getOperand(0);
        const Metadata *ModeMD = Mode.get();

        // A string at position 0 in the operand list should indicate
        // which metadata we are parsing. Assert if this is not the case.
        //
        // As of this point, we should be seeing one of the following:
        //
        // !{!"elemental"} or
        // !{!"arg_name", !"i", !"x"} or
        // !{!"arg_step", i32 1, i32 0} or
        // !{!"arg_alig", i32 undef, i32 undef} or
        // !{!"vec_length", i32 undef, i32 4}

        assert(Mode->getMetadataID() == Metadata::MDStringKind &&
               "Unexpected metadata operand for tuple");

        const MDString *ModeStr = dyn_cast<MDString>(ModeMD);
        StringRef ModeString = ModeStr->getString();

        unsigned int NumOps = Tuple->getNumOperands();
        for (unsigned int I = 1; I < NumOps; ++I) {

          const MDOperand& ArgOp = Tuple->getOperand(I);
          const Metadata *ArgOpMD = ArgOp.get();

          if (ModeString == "arg_name") {
            // Parameter names should come in as Strings. Assert if
            // this is not true.
            assert(ArgOp->getMetadataID() == Metadata::MDStringKind &&
                   "Expected \"arg_name\" string metadata");

            const MDString *ArgName = dyn_cast<MDString>(ArgOpMD);
              FuncArgMap[F].Name.push_back(ArgName->getString());
            }

          if (ModeString == "arg_step") {
            // parameter strides should come in as constant Values. 
            // Assert if this is not true.
            assert(ArgOp->getMetadataID() == Metadata::ConstantAsMetadataKind &&
                   "Expected \"arg_step\" string metadata");

            const ConstantAsMetadata *ConstVal =
                dyn_cast<ConstantAsMetadata>(ArgOpMD);

            Value *Stride = ConstVal->getValue();
            FuncArgMap[F].Stride.push_back(Stride);
          }

          if (ModeString == "arg_alig") {
            // Parameter strides should come in as constant Values. 
            // Assert if this is not true.
            assert(ArgOp->getMetadataID() == Metadata::ConstantAsMetadataKind &&
                   "Expected \"arg_alig\" string metadata");

            const ConstantAsMetadata *ConstVal =
                dyn_cast<ConstantAsMetadata>(ArgOpMD);

            Value *Align = ConstVal->getValue();
            FuncArgMap[F].Align.push_back(Align);
          }

          if (ModeString == "vec_length") {
            // Vector length should come in as a constant Value. Assert if
            // this is not true.
            assert(ArgOp->getMetadataID() == Metadata::ConstantAsMetadataKind &&
                   "Expected \"vec_length\" string metadata");

            const ConstantAsMetadata *ConstVal =
                dyn_cast<ConstantAsMetadata>(ArgOpMD);

            VectorLength = ConstVal->getValue();
            ConstantInt *VLConst = dyn_cast<ConstantInt>(VectorLength);
            if (VLConst) {
              VL = VLConst->getSExtValue();
            }
          }
        }
      }
    }
  }
  return true;
}

Function* SIMDFunctionCloning::CloneFunction(Function *F)
{

  DEBUG(dbgs() << "Cloning Function: " << F->getName() << "\n");

  SmallVector<Type*, 4> ParmTypes;
  Type *ReturnType = F->getReturnType();
  ConstantInt *VLConst = dyn_cast<ConstantInt>(VectorLength);
  assert(VLConst && "Vector length is not a constant int!");
  if (!F->getReturnType()->isVoidTy())
    ReturnType = VectorType::get(ReturnType, VL);

  Function::ArgumentListType &ArgList = F->getArgumentList();
  Function::ArgumentListType::iterator ArgListIt = ArgList.begin();
  Function::ArgumentListType::iterator ArgListEnd = ArgList.end();

  for (; ArgListIt != ArgListEnd; ++ArgListIt) {

    // Lookup attributes for this parameter and process accordingly.
    signed Idx = -1;
    for (unsigned J = 0; J < F->arg_size(); ++J) {
      if (FuncArgMap[F].Name[J] == ArgListIt->getName()) {
        // The index used to look up stride and alignment info for the
        // parameter.
        Idx = J;
      }
    }

    // Assert if for some reason the parameter list does not match up
    // with the parameters listed in the metadata.
    assert(Idx >= 0 && "Parameter list for function does not match\
                        parameter list for metadata");

    // Expand vector parameters to vector types. Leave uniform/linear
    // as scalar.
    Value *Stride = FuncArgMap[F].Stride[Idx];
    if (!dyn_cast<UndefValue>(Stride)) {
      // Uniform/Linear parameter, so keep as parm as scalar
      ParmTypes.push_back(ArgListIt->getType());
    } else {
      // Vector parameter, so expand.
      VectorType *VecType = VectorType::get(ArgListIt->getType(), VL);
      ParmTypes.push_back(VecType);
    }
  }

  FunctionType* CloneFuncType = FunctionType::get(ReturnType, ParmTypes,
                                                  false);

  Function* Clone = Function::Create(CloneFuncType, F->getLinkage(),
                                     "clone." + F->getName().str(),
                                     F->getParent());

  Clone->copyAttributesFrom(F);
  ValueToValueMapTy Vmap;
  Function::arg_iterator ArgIt = F->arg_begin();
  Function::arg_iterator ArgEnd = F->arg_end();
  Function::arg_iterator NewArgIt = Clone->arg_begin();
  for (; ArgIt != ArgEnd; ++ArgIt, ++NewArgIt) {
    NewArgIt->setName(ArgIt->getName());
    Vmap[ArgIt] = NewArgIt;
  }

  SmallVector<ReturnInst*, 8> Returns;
  CloneFunctionInto(Clone, F, Vmap, true, Returns);

  DEBUG(dbgs() << "After Cloning and Parameter/Return Expansion\n");
  Clone->dump();

  return Clone;
}

BasicBlock* SIMDFunctionCloning::splitEntryIntoLoop(Function *Clone,
                                                    BasicBlock *EntryBlock)
{

  // Find the initial store of the last parameter and use as a split point for
  // the entry block and loop block. If there are no parameters, then the split
  // point is the last alloca for any function private vars.

  // Split the instructions after the final store to the parameters into a new
  // basic block that will serve as the loop. Or, if there are no parameters,
  // then split after the last alloca. All other instructions after the last
  // parameter store (or alloca) are assumed to be operating on an element-wise
  // basis. The only instructions in the entry block should be the vector alloca
  // instructions and the vector store for the parameters.

  Function::ArgumentListType &CloneArgList = Clone->getArgumentList();
  Function::ArgumentListType::iterator CloneArgListIt = CloneArgList.begin();
  Function::ArgumentListType::iterator CloneArgListEnd = CloneArgList.end();
  BasicBlock::iterator SplitPt = NULL;

  if (Clone->arg_size() == 0) {
    // If the function does not have any parameters, then split after the last
    // alloca. Function private vars should have an associated alloca.
    BasicBlock::iterator BBIt = EntryBlock->begin();
    BasicBlock::iterator BBEnd = EntryBlock->end();
    for (; BBIt != BBEnd; ++BBIt) {
      if (dyn_cast<AllocaInst>(BBIt)) {
        SplitPt = BBIt;
      }
    }
  } else {
    for (; CloneArgListIt != CloneArgListEnd; ++CloneArgListIt) {

      unsigned UserCnt = 0;
      unsigned StoreUserCnt = 0;
      User::user_iterator UserIt = CloneArgListIt->user_begin();
      User::user_iterator UserEnd = CloneArgListIt->user_end();
      StoreInst *StoreUser = NULL;

      for (; UserIt != UserEnd; ++UserIt) {
        StoreUser = dyn_cast<StoreInst>(*UserIt);
        if (StoreUser) {
          SplitPt = StoreUser;
          StoreUserCnt++;
        }
        UserCnt++;
      }

      assert(UserCnt == 1 && StoreUserCnt == 1 &&
             "Expected a single store as a user of the parameter!\n");
    }
  }

  assert(SplitPt && "Could not find an appropriate split point for the loop");

  SplitPt++;
  BasicBlock *LoopBlock = EntryBlock->splitBasicBlock(SplitPt, "simd.loop");
  return LoopBlock;
}

BasicBlock* SIMDFunctionCloning::splitLoopIntoReturn(Function *Clone,
                                                     BasicBlock *LoopBlock)
{

  // Determine the basic block with the return. For simple cases, the 'ret'
  // instruction will be part of the entry block. In this case, separate the
  // 'ret' into a new basic block because we don't want this as part of the
  // loop. For more complex cases, the 'ret' and corresponding instructions
  // (i.e., load from auto variable) will already be in a separate basic block,
  // so no need to split here.

  Instruction *SplitPt = LoopBlock->getTerminator();

  if (ReturnInst *Return = dyn_cast<ReturnInst>(SplitPt)) {

    // If the return is from a preceeding load, make sure the load is also put
    // in the return block. This will make the code cleaner when we eventually
    // remove these instructions because we blindly remove all instructions
    // from the return block after generating the vector return temp. If the
    // load remains in part of the loop, then it becomes dead code. This is not
    // a problem and could be cleaned up later, but this will prevent DCE from
    // having to run, plus the LLVM is easier to look at immediately.

    // Make sure this is not a void function before getting the return
    // operand.
    if (!Clone->getReturnType()->isVoidTy()) {
      Value *RetOp = Return->getOperand(0);
      Value::use_iterator UseIt = RetOp->use_begin();
      Value::use_iterator UseEnd = RetOp->use_end();

      for (; UseIt != UseEnd; ++UseIt) {
        LoadInst *RetLoad = dyn_cast<LoadInst>(*UseIt);
        if (RetLoad) {
          SplitPt = RetLoad;
        }
      }
    }
  }

  Function::iterator ReturnBlock = Clone->end();
  if (dyn_cast<LoadInst>(SplitPt) || dyn_cast<ReturnInst>(SplitPt)) {
    ReturnBlock = LoopBlock->splitBasicBlock(SplitPt, "return");
  } else {
    ReturnBlock = Clone->end();
    ReturnBlock--;
  }

  return ReturnBlock;
}

void SIMDFunctionCloning::updateReturnPredecessors(Function *Clone,
                                                   BasicBlock *LoopExitBlock,
                                                   BasicBlock *ReturnBlock)
{ 
  // Update the branches of the ReturnBlock predecessors to point back to
  // LoopBlock if the index is less than VL.

  // First, collect the basic blocks to be updated since we don't want to update
  // the CFG while iterating through it.
  SmallVector<BranchInst*, 4> BranchesToUpdate;
  Function::iterator FI = Clone->begin();
  Function::iterator FE = Clone->end();
  for (; FI != FE; ++FI) {

    BasicBlock::iterator BBI = FI->begin();
    BasicBlock::iterator BBE = FI->end();

    for (; BBI != BBE; ++BBI) {

      BranchInst *Branch = dyn_cast<BranchInst>(BBI);

      if (Branch) {
        unsigned NumSucc = Branch->getNumSuccessors();

        for (unsigned I = 0; I < NumSucc; ++I) {
          if (Branch->getSuccessor(I) == ReturnBlock) {
            BranchesToUpdate.push_back(Branch);
          }
        }
      }
    }
  }

  // Now, do the actual update. The code below handles both conditional and
  // unconditional branches because we loop through all successors of the
  // branch to see if any of them point to the ReturnBlock.
  for (unsigned I = 0; I < BranchesToUpdate.size(); ++I) {
    unsigned int NumOps = BranchesToUpdate[I]->getNumSuccessors();
    for (unsigned Idx = 0; Idx < NumOps; ++Idx) {
      BasicBlock *Successor = BranchesToUpdate[I]->getSuccessor(Idx);
      if (Successor == ReturnBlock) {
        BranchesToUpdate[I]->setOperand(Idx, LoopExitBlock);
      }
    }
  }
}

BasicBlock* SIMDFunctionCloning::createLoopExit(Function *Clone,
                                                BasicBlock *ReturnBlock)
{
  BasicBlock *LoopExitBlock = BasicBlock::Create(Clone->getContext(),
                                                 "simd.loop.exit",
                                                 Clone, ReturnBlock);

  updateReturnPredecessors(Clone, LoopExitBlock, ReturnBlock);
  return LoopExitBlock;
}

PHINode* SIMDFunctionCloning::createPhiAndBackedgeForLoop(
    Function *Clone,
    BasicBlock *EntryBlock,
    BasicBlock *LoopBlock,
    BasicBlock *LoopExitBlock,
    BasicBlock *ReturnBlock)
{
                                                        
  // Create the phi node for the top of the loop block and add the back
  // edge to the loop from the loop exit.

  PHINode *Phi = PHINode::Create(Type::getInt32Ty(Clone->getContext()), 2,
                                 "index", LoopBlock->getFirstInsertionPt());

  Constant *Inc = ConstantInt::get(Type::getInt32Ty(Clone->getContext()), 1);
  Constant *IndInit = ConstantInt::get(Type::getInt32Ty(Clone->getContext()),
                                       0);

  Instruction *Induction = BinaryOperator::CreateNUWAdd(Inc, Phi, "indvar",
                                                        LoopExitBlock);

  Instruction *VLCmp = new ICmpInst(*LoopExitBlock, CmpInst::ICMP_ULT,
                                    Induction, VectorLength, "vlcond");

  BranchInst::Create(LoopBlock, ReturnBlock, VLCmp, LoopExitBlock);

  Phi->addIncoming(IndInit, EntryBlock);
  Phi->addIncoming(Induction, LoopExitBlock);

  DEBUG(dbgs() << "After Loop Insertion\n");
  Clone->dump();

  return Phi;
}

void SIMDFunctionCloning::expandVectorParameters(
    Function *Clone,
    std::map<AllocaInst*, Instruction*>& AllocaMap)
{
  // For vector parameters, expand the existing alloca to a vector. Then,
  // bitcast the vector and store this instruction in a map. The map is later
  // used to insert the new instructions and to replace the old scalar memory
  // references. If there are no parameters, then the function simply does not
  // perform any expansion since we iterate over the function's arg list.

  Function::ArgumentListType &CloneArgList = Clone->getArgumentList();
  Function::ArgumentListType::iterator ArgIt = CloneArgList.begin();
  Function::ArgumentListType::iterator ArgEnd = CloneArgList.end();

  for (; ArgIt != ArgEnd; ++ArgIt) {

    User::user_iterator UserIt = ArgIt->user_begin();
    User::user_iterator UserEnd = ArgIt->user_end();

    for (; UserIt != UserEnd; ++UserIt) {

      StoreInst *StoreUser = dyn_cast<StoreInst>(*UserIt);

      if (StoreUser) {

        AllocaInst *Alloca = dyn_cast<AllocaInst>(UserIt->getOperand(1));
        VectorType *VecType = dyn_cast<VectorType>(ArgIt->getType());

        if (VecType) {
          // Create a new vector alloca and bitcast to a pointer to the
          // element type. The following is an example of what the cast
          // should look like:
          //
          // %veccast = bitcast <2 x i32>* %vec_a.addr to i32*
          //
          // geps using the bitcast will appear in a scalar form instead
          // of casting to an array or using vector. For example,
          //
          // %vecgep1 = getelementptr i32, i32* %veccast, i32 %index
          //
          // instead of:
          //
          // getelementptr inbounds [4 x i32], [4 x i32]* %a, i32 0, i64 1
          //
          // We do this to put the geps in a more scalar form.
          Twine VarName = "vec_" + Alloca->getName();
          AllocaInst *VecAlloca = new AllocaInst(VecType, VarName, Alloca);
          BitCastInst *VecCast = new BitCastInst(VecAlloca, Alloca->getType(),
                                                 "veccast");
          AllocaMap[Alloca] = VecCast;
        }
      }
    }
  }
}

Instruction* SIMDFunctionCloning::createExpandedReturn(Function *Clone,
                                                       VectorType *ReturnType,
                                                       Instruction *InsertPt)
{
  // Expand the return temp to a vector.

  Twine VarName = "vec_retval";
  VectorType *AllocaType = dyn_cast<VectorType>(Clone->getReturnType());

  AllocaInst *VecAlloca = new AllocaInst(AllocaType, VarName);
  VecAlloca->insertBefore(InsertPt);

  PointerType *ElemTypePtr =
      PointerType::get(ReturnType->getElementType(),
                       VecAlloca->getType()->getAddressSpace());
  BitCastInst *VecCast = new BitCastInst(VecAlloca, ElemTypePtr, "veccast");

  VecCast->insertBefore(InsertPt);

  return VecCast;
}

Instruction* SIMDFunctionCloning::expandReturn(
    Function *Clone,
    BasicBlock *EntryBlock,
    BasicBlock *LoopBlock,
    BasicBlock *ReturnBlock,
    std::map<AllocaInst*, Instruction*>& AllocaMap)
{
  // Determine how the return is currently handled, since this will determine
  // if a new vector alloca is required for it. For simple functions, an alloca
  // may not have been created for the return value. The function may just
  // simply return a value defined by some operation that now exists within the
  // loop. If an alloca was generated already, then the return block will load
  // from it and then return. Thus, we look for a return resulting from a load
  // in the return block. If found, we have already expanded all alloca
  // instructions to vector types and the old scalar references have already
  // been replaced with them. In this case, we only need to pack the results
  // from the vector alloca into a temp and return the temp. If a vector alloca
  // was not generated for the return, we need to add one for it because we have
  // a scalar reference in the loop that needs to be replaced. After creating
  // the new vector alloca, replace the reference to it in the loop and then
  // pack the results into a temp and return it.
  //
  // Example 1: // alloca not generated in entry block
  //
  // loop:
  //   ... // some set of instructions
  //   %add1 = add nsw i32 %1, %2
  //   br label %loop.exit (loop exit contains br to return block)
  //
  // return:
  //   ret i32 %add1
  //
  //
  // Example 2:
  //
  // loop:
  //  ... // some set of instructions
  //   %vecgep1 = getelementptr <2 x i32>* %vec_ret, i32 0, i32 %index
  //   store i32 %add2, i32* %vecgep1
  //   br label %loop.exit (loop exit contains br to return block)
  //
  // return:
  //   %7 = load i32, i32*, %retval // the original scalar alloca 
  //   ret i32 %7
  //

  ReturnInst *FuncReturn = dyn_cast<ReturnInst>(ReturnBlock->getTerminator());
  assert(FuncReturn && "Expected ret instruction to terminate the return\
                        basic block");

  LoadInst *LoadFromAlloca = dyn_cast<LoadInst>(FuncReturn->getOperand(0));

  // We need to generate a vector alloca for the return vector.
  // Two cases exist, here:
  // 
  // 1) For simple functions, the return is a temp defined within the
  //    loop body and the temp is not an alloca.
  // 
  // 2) The return temp traces back to an alloca.
  // 
  // For both cases, generate a vector alloca so that we can later load from it
  // and return the vector temp from the function. The alloca is used to load
  // and store from so that the scalar loop contains load/store/gep
  // instructions. This enables AVR construction to remain straightforward.
  // E.g., we don't need to worry about figuring out how to represent
  // insert/extract when building AVR nodes. This keeps consistent with how ICC
  // is operating.
  // 
  // Additionally, for case 1 we must generate a gep and store after the
  // instruction that defines the original return temp, so that we can store
  // the result into the proper index of the return vector. For case 2, we must
  // go into the loop and replace the old scalar alloca reference with the one
  // just created as vector.

  Instruction *VecReturn = NULL;
  VectorType *ReturnType = dyn_cast<VectorType>(Clone->getReturnType());

  if (!LoadFromAlloca) {

    // Case 1

    VecReturn = createExpandedReturn(Clone, ReturnType,
                                     EntryBlock->getTerminator());
    Instruction *RetFromTemp = dyn_cast<Instruction>(FuncReturn->getOperand(0));

    BasicBlock::iterator InsertPt = RetFromTemp;
    InsertPt++;

    Value *Phi = LoopBlock->begin();

    GetElementPtrInst *VecGep =
        GetElementPtrInst::Create(ReturnType->getElementType(),
                                  VecReturn, Phi, "vec_gep", InsertPt);
    InsertPt = VecGep;
    InsertPt++;

    new StoreInst(RetFromTemp, VecGep, InsertPt);

  } else {

    // Case 2

    AllocaInst *Alloca = dyn_cast<AllocaInst>(LoadFromAlloca->getOperand(0));
    if (AllocaMap.find(Alloca) != AllocaMap.end()) {
      // There's already a vector alloca created for the return, which is the
      // same one used for the parameter. E.g., we're returning the updated
      // parameter.
      VecReturn = AllocaMap[Alloca];
    } else {
      // A new return vector is needed because we do not load the return value
      // from an alloca.
      VecReturn = createExpandedReturn(Clone, ReturnType, Alloca);
      AllocaMap[Alloca] = VecReturn;
    }
  }

  return VecReturn;
}

Instruction* SIMDFunctionCloning::expandVectorParametersAndReturn(
    Function *Clone,
    BasicBlock *EntryBlock,
    BasicBlock *LoopBlock,
    BasicBlock *ReturnBlock,
    std::map<AllocaInst*, Instruction*>& AllocaMap)
{
  // If there are no parameters, then this function will do nothing and this
  // is the expected behavior.
  expandVectorParameters(Clone, AllocaMap);

  // If the function returns void, then don't attempt to expand to vector.
  Instruction *ExpandedReturn = ReturnBlock->getTerminator();
  if (!Clone->getReturnType()->isVoidTy()) {
    ExpandedReturn = expandReturn(Clone, EntryBlock, LoopBlock, ReturnBlock,
                                  AllocaMap);
  }

  std::map<AllocaInst*, Instruction*>::iterator MapIt = AllocaMap.begin();
  std::map<AllocaInst*, Instruction*>::iterator MapEnd = AllocaMap.end();

  // So, essentially what has been done to this point is the creation and
  // insertion of the vector alloca instructions. Now, we insert the bitcasts of
  // those instructions, which have been store in the map. The insertion of the
  // vector bitcast to element type pointer is done at the end of the EntryBlock
  // to ensure that any initial stores of vector parameters have been done
  // before the cast.

  for (; MapIt != MapEnd; ++MapIt) {
    Instruction *ExpandedCast = MapIt->second;
    if (!ExpandedCast->getParent())
      ExpandedCast->insertBefore(EntryBlock->getTerminator());
  }

  return ExpandedReturn;
}

void SIMDFunctionCloning::updateScalarMemRefsWithVector(
    Function *Clone,
    Function *F,
    BasicBlock *EntryBlock,
    BasicBlock *ReturnBlock,
    PHINode *Phi,
    std::map<AllocaInst*, Instruction*>& AllocaMap)
{
  // Determine which instruction will serve as the address to the users of the
  // old alloca instructions that will be updated. For parameter stores, use
  // the bitcast on the new vector alloca. For instructions in the loop body,
  // use a gep. These instructions are stored in the AddrMap for later use since
  // we don't want to update instructions on the fly as we're iterating through
  // use-def lists.

  std::map<Instruction*, Instruction*> AddrMap;

  std::map<AllocaInst*, Instruction*>::iterator AllocaMapIt;
  AllocaMapIt = AllocaMap.begin();

  std::map<AllocaInst*, Instruction*>::iterator AllocaMapEnd;
  AllocaMapEnd = AllocaMap.end();

  for (; AllocaMapIt != AllocaMapEnd; ++AllocaMapIt) {

    AllocaInst *Alloca = AllocaMapIt->first;
    Instruction *ExpandedRef = AllocaMapIt->second;
    User::user_iterator UserIt = Alloca->user_begin();
    User::user_iterator UserEnd = Alloca->user_end();

    for (; UserIt != UserEnd; ++UserIt) {

      Instruction *User = dyn_cast<Instruction>(*UserIt);

      // This is an initial store to a parameter, but the memory reference
      // (bitcast) is stored in the map. Replacement of the old references
      // below will ensure that the initial store will be made to the alloca'd
      // vector. All other replacements will be made blindly with the gep
      // that is stored in the map.

      if (User->getParent() == EntryBlock && dyn_cast<StoreInst>(User)) {

        Value *Parm = User->getOperand(0);
        AddrMap[User] = ExpandedRef;

        if (!parmIsVector(F, Parm->getName())) {
          // This is a special case where we have something like a uniform
          // parameter coming in, an operation is applied to it, and the return
          // traces back to a scalar alloca. i.e., the parameter is updated and
          // also returned. However, we need to return a vector. Think broadcast
          // as an example:
          //
          // __declspec(vector(uniform(v))) int foo(int v) {
          //  int t = 2;
          //  v = t + 1;
          //  return v;
          // }
          //
          // Initial LLVM:
          //
          // define i32 @vec_foo(i32 %v) #0 {
          // entry:
          //   %v.addr = alloca i32, align 4
          //   %t = alloca i32, align 4
          //   store i32 %v, i32* %v.addr, align 4
          //   store i32 2, i32* %t, align 4
          //   %0 = load i32, i32* %t, align 4
          //   %add = add nsw i32 %0, 1
          //   store i32 %add, i32* %v.addr, align 4
          //   %1 = load i32, i32* %v.addr, align 4
          //   ret i32 %1
          // }
          //
          // After Transformation (partial LLVM):
          //
          // define <4 x i32> @clone.vec_foo(i32 %v) #0 {
          // entry:
          //   %vec_retval = alloca <4 x i32>   <--- return vector
          //   %veccast = bitcast <4 x i32>* %vec_retval to i32*
          //   %t = alloca i32, align 4
          //   br label %simd.loop
          //
          // simd.loop:           ; preds = %simd.loop.exit, %entry
          //   %index = phi i32 [ 0, %entry ], [ %indvar, %simd.loop.exit ]
          //   %vecgep2 = getelementptr i32, i32* %veccast, i32 %index
          //   store i32 %v, i32* %vecgep2, align 4  <--- Store moved to loop
          //   store i32 2, i32* %t, align 4
          //
          // In this case a copy of the value needs to be broadcast to the
          // return vector, but we cannot do a vector copy in the simd loop
          // preheader. Thus, we simply move the store instruction to the top
          // of simd loop (after the phi) and then the initial parameter value
          // can be stored to the appropriate lane/index before the operation
          // is applied.
          User->removeFromParent();
          User->insertAfter(Phi);
        }
      }

      // Create a gep for the old alloca references within the loop and store it
      // in the map. The map is used below to do the actual replacement.

      if (User->getParent() != EntryBlock && User->getParent() != ReturnBlock) {

        // We know we're within the loop body if the user is not in the entry or
        // exit block. All other basic blocks are part of the loop.

        BitCastInst *BitCast = dyn_cast<BitCastInst>(ExpandedRef);
        PointerType *BitCastType = dyn_cast<PointerType>(BitCast->getType());
        Type *PointeeType = BitCastType->getElementType();

        GetElementPtrInst *VecGep =
            GetElementPtrInst::Create(PointeeType, ExpandedRef, Phi, "vecgep",
                                      User);
        AddrMap[User] = VecGep;
      }
    }
  }

  // Replace the references to the old scalar alloca instructions with the gep.
  // Or, if the reference is the initial store of a parameter, make sure to use
  // the actual alloca and not the bitcast. For this case, since the bitcast
  // is what is stored in the map, we must pull out the operand of the
  // bitcast to get the alloca.

  std::map<Instruction*, Instruction*>::iterator AddrMapIt = AddrMap.begin();
  std::map<Instruction*, Instruction*>::iterator AddrMapEnd = AddrMap.end();

  for (; AddrMapIt != AddrMapEnd; ++AddrMapIt) {

    Instruction *User = AddrMapIt->first;
    Instruction *Addr = AddrMapIt->second;

    if (dyn_cast<LoadInst>(User)) {
      User->setOperand(0, Addr);
    }

    if (dyn_cast<StoreInst>(User)) {

      // The initial store of a vector parameter should be to the alloca'd
      // vector memory and not to the bitcast. All other references can be
      // blindly replaced using the gep stored in the map.

      Instruction *MemRef;
      if (dyn_cast<BitCastInst>(Addr)) {
          // This should point to the vector alloca for the parameter.
          MemRef = dyn_cast<Instruction>(Addr->getOperand(0));
      } else {
          MemRef = Addr;
      }

      User->setOperand(1, MemRef);
    }
  }

  DEBUG(dbgs() << "After Alloca Replacement\n");
  Clone->dump();
}

void SIMDFunctionCloning::updateLinearReferences(Function *Clone, Function *F,
                                                 PHINode *Phi)
{
  // Add stride to parameters marked as linear. This is done by finding all
  // users of the scalar alloca associated with the parameter. The user should
  // be a load from this alloca to a temp. The stride is then added to this temp
  // and its uses are replaced with the new temp.

  Function::ArgumentListType &ArgList = Clone->getArgumentList();
  Function::ArgumentListType::iterator ArgListIt = ArgList.begin();
  Function::ArgumentListType::iterator ArgListEnd = ArgList.end();

  for (; ArgListIt != ArgListEnd; ++ArgListIt) {

    User::user_iterator ArgUserIt = ArgListIt->user_begin();
    User::user_iterator ArgUserEnd = ArgListIt->user_end();
    unsigned ParmIdx = ArgListIt->getArgNo();
    Value *Stride = FuncArgMap[F].Stride[ParmIdx];

    if (!dyn_cast<UndefValue>(Stride)) {

      ConstantInt *StrideConst = dyn_cast<ConstantInt>(Stride);
      int64_t StrideVal = StrideConst->getSExtValue();

      if (StrideVal != 0) {

        for (; ArgUserIt != ArgUserEnd; ++ArgUserIt) {

          AllocaInst *Alloca = dyn_cast<AllocaInst>(ArgUserIt->getOperand(1));

          if (Alloca) {

            User::user_iterator AllocaUserIt = Alloca->user_begin();
            User::user_iterator AllocaUserEnd = Alloca->user_end();

            for (; AllocaUserIt != AllocaUserEnd; ++AllocaUserIt) {

              // Look only for load users since these will represent
              // a temp definition for the linear parameter. This is
              // the temp we will use to add stride.

              LoadInst *ParmLoad = dyn_cast<LoadInst>(*AllocaUserIt);

              if (ParmLoad) {

                // Now, find users of the load. The new temp with stride
                // will replace the original temp. We are assuming only
                // a single user of a load here because of SSA.

                User::user_iterator LoadUser = ParmLoad->user_begin();

                Instruction *Instr = dyn_cast<Instruction>(*LoadUser);
                assert(Instr && "Expected user of the load to be an\
                                 instruction.");

                BinaryOperator *Mul =
                    BinaryOperator::CreateMul(Stride, Phi, "mul");
                Mul->insertAfter(ParmLoad);

                Value *LinearVal;

                if (ParmLoad->getType()->isPointerTy()) {
                  // Linear updates to pointer parameters involves an address
                  // calculation, so use gep.
                  PointerType *ParmPtrType =
                    dyn_cast<PointerType>(ParmLoad->getType());

                  GetElementPtrInst *LinearParmGep =
                    GetElementPtrInst::Create(ParmPtrType->getElementType(),
                                              ParmLoad, Mul, "parmgep");
                  LinearParmGep->insertAfter(Mul);
                  LinearVal = LinearParmGep;
                } else {
                  // For parameters that are values, just add the stride to
                  // the value that is loaded.
                  BinaryOperator *Add =
                      BinaryOperator::CreateAdd(ParmLoad, Mul, "add");

                  Add->insertAfter(Mul);
                  LinearVal = Add;
                }

                unsigned NumOps = LoadUser->getNumOperands();
                for (unsigned I = 0; I < NumOps; ++I) {
                  if (LoadUser->getOperand(I) == ParmLoad) {
                    LoadUser->setOperand(I, LinearVal);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  DEBUG(dbgs() << "After Linear Updates\n");
  Clone->dump();
}

void SIMDFunctionCloning::removeScalarMemRefs(std::map<AllocaInst*,
                                                       Instruction*>& AllocaMap)
{
  // Remove any scalar alloca instructions that have been expanded to vector
  // alloca instructions.

  std::map<AllocaInst*, Instruction*>::iterator AllocaMapIt;
  AllocaMapIt = AllocaMap.begin();

  std::map<AllocaInst*, Instruction*>::iterator AllocaMapEnd;
  AllocaMapEnd = AllocaMap.end();

  for (; AllocaMapIt != AllocaMapEnd; ++AllocaMapIt) {
    AllocaInst *OldAlloca = AllocaMapIt->first;
    OldAlloca->eraseFromParent();
  }
}

void SIMDFunctionCloning::updateReturnBlockInstructions(
    Function *Clone,
    BasicBlock *ReturnBlock,
    Instruction *ExpandedReturn)
{
  // If the vector function returns void, then there is no need to do any
  // packing. The only instruction in the ReturnBlock is 'ret void', so
  // we can just leave this instruction and we're done.
  if (Clone->getReturnType()->isVoidTy()) return;

  // Collect all instructions in the return basic block. They will be removed.
  SmallVector<Instruction*, 4> InstToRemove;
  BasicBlock::iterator InstIt = ReturnBlock->begin();
  BasicBlock::iterator InstEnd = ReturnBlock->end();

  for (; InstIt != InstEnd; ++InstIt) {
    InstToRemove.push_back(InstIt);
  }

  // Remove all instructions from the return block. These will be replaced
  // with the instructions necessary to return a vector temp. The verifier
  // will complain if we remove the definitions of users first, so remove
  // instructions from the bottom up.
  for (int I = InstToRemove.size() - 1; I >= 0; I--) {
    InstToRemove[I]->eraseFromParent();
  }

  // Pack up the elements into a vector temp and return it. If the return
  // vector was bitcast to a pointer to the element type, we must bitcast to
  // vector before returning.
  Instruction *Return;
  if (dyn_cast<BitCastInst>(ExpandedReturn)) {
      // Operand 0 is the actual alloc reference in the bitcast.
      AllocaInst *Alloca = dyn_cast<AllocaInst>(ExpandedReturn->getOperand(0));
      PointerType *PtrVecType =
          PointerType::get(Clone->getReturnType(),
                           Alloca->getType()->getAddressSpace());
      BitCastInst *BitCast = new BitCastInst(ExpandedReturn, PtrVecType,
                                             "cast", ReturnBlock);
      Return = BitCast;
  } else {
      Return = ExpandedReturn;
  }

  LoadInst *VecReturn = new LoadInst(Return, "vec_ret", ReturnBlock);
  ReturnInst::Create(Clone->getContext(), VecReturn, ReturnBlock);
}

int SIMDFunctionCloning::findParmLocationInMap(Function *F,
                                               StringRef ArgName)
{
  unsigned NumParms = FuncArgMap[F].Name.size();
  for (unsigned I = 0; I < NumParms; ++I)
    if (FuncArgMap[F].Name[I] == ArgName) return I;

  return -1;
}

bool SIMDFunctionCloning::parmIsVector(Function *F, StringRef ParmName)
{
  int Idx = findParmLocationInMap(F, ParmName);

  if (Idx >= 0) {
    Value *Stride = FuncArgMap[F].Stride[Idx];
    if (dyn_cast<UndefValue>(Stride)) {
      return true;
    }
  }

  return false;
}

bool SIMDFunctionCloning::parmIsLinear(Function *F, StringRef ParmName)
{
  int Idx = findParmLocationInMap(F, ParmName);

  if (Idx >= 0) {

    Value *Stride = FuncArgMap[F].Stride[Idx];
    ConstantInt *StrideConst = dyn_cast<ConstantInt>(Stride);

    if (!dyn_cast<UndefValue>(Stride)) {

      int64_t StrideVal = StrideConst->getSExtValue();

      if (StrideVal != 0) {
        return true;
      }
    }
  }

  return false;
}

void SIMDFunctionCloning::insertBeginRegion(Module& M, Function *Clone,
                                            Function *F,
                                            BasicBlock *EntryBlock)
{
  // Insert directive indicating the beginning of a SIMD loop.
  CallInst *SIMDBeginCall = VPOUtils::createDirectiveCall(M, "dir.simd");
  SIMDBeginCall->insertBefore(EntryBlock->getTerminator());

  // Now, split into its own basic block and insert the remaining intrinsics
  // after this one.
  EntryBlock->splitBasicBlock(SIMDBeginCall, "simd.begin.region");

  // Insert vectorlength directive
  CallInst *VlenCall =
      VPOUtils::createDirectiveQualOpndCall(M, "dir.qual.simd.vlen",
                                            VectorLength);
  VlenCall->insertAfter(SIMDBeginCall);

  // Add directive for linear parameters
  SmallVector<Value*, 4> VarCallArgs;
  Function::ArgumentListType &ArgList = Clone->getArgumentList();
  Function::ArgumentListType::iterator ArgListIt = ArgList.begin();
  Function::ArgumentListType::iterator ArgListEnd = ArgList.end();

  for (; ArgListIt != ArgListEnd; ++ArgListIt) {
    if (parmIsLinear(F, ArgListIt->getName())) {
      VarCallArgs.push_back(ArgListIt); 
    }
  }

  CallInst *LinearCall =
      VPOUtils::createDirectiveQualOpndListCall(M, "dir.qual.simd.linear",
                                                VarCallArgs);
  LinearCall->insertAfter(VlenCall);
}

void SIMDFunctionCloning::insertEndRegion(Module& M, Function *Clone,
                                          BasicBlock *LoopExitBlock,
                                          BasicBlock *ReturnBlock)
{
  BasicBlock *EndDirectiveBlock = BasicBlock::Create(Clone->getContext(),
                                                     "simd.end.region",
                                                     Clone, ReturnBlock);

  BranchInst *LoopExitBranch =
      dyn_cast<BranchInst>(LoopExitBlock->getTerminator());
  assert(LoopExitBranch && "Expecting br instruction for loop exit block");
  LoopExitBranch->setOperand(1, EndDirectiveBlock);

  BranchInst::Create(ReturnBlock, EndDirectiveBlock);

  CallInst *SIMDEndCall = VPOUtils::createDirectiveCall(M, "dir.simd.end");
  SIMDEndCall->insertBefore(EndDirectiveBlock->getTerminator());
}

void SIMDFunctionCloning::insertDirectiveIntrinsics(Module& M,
                                                    Function *Clone,
                                                    Function *F,
                                                    BasicBlock *EntryBlock,
                                                    BasicBlock *LoopExitBlock,
                                                    BasicBlock *ReturnBlock)
{
  insertBeginRegion(M, Clone, F, EntryBlock);
  insertEndRegion(M, Clone, LoopExitBlock, ReturnBlock);
}

void SIMDFunctionCloning::createBroadcastReturn(ReturnInst *Return)
{
  Value* ScalarRetVal = Return->getOperand(0);

  // Generate a new scalar Value that will be expanded to vector. We need to do
  // this because are removing the original scalar return and we don't want any
  // of the original uses of the Value left around or the LLVM verifier will
  // complain.

  Constant *ConstVal;
  SmallVector<Constant*, 4> Splat;
  Function *ReturnBlockParent = Return->getParent()->getParent();

  if (ConstantInt *Const = dyn_cast<ConstantInt>(ScalarRetVal))
    ConstVal = ConstantInt::get(Const->getType(), Const->getValue());
  else if (ConstantFP *Const = dyn_cast<ConstantFP>(ScalarRetVal))
    ConstVal = ConstantFP::get(ReturnBlockParent->getContext(),
                               Const->getValueAPF());
  else
    // So far, a simple return with a constant value is expected. If we
    // broadcast an incoming parameter, there will be an alloca for the
    // parameter and corresponding store/load after, so this function should
    // not be reached. Allow the rest of the framework to kick in for these
    // cases. Assert just in case we get any other cases that don't involve a
    // simple return with constant. Return of constant expressions will be
    // simplified to a single constant value, which will be covered by the two
    // cases above.
    assert("Expected broadcast value to be a constant");

  for (int J = 0; J < VL; ++J) {
    Splat.push_back(ConstVal);
  }

  Value *ConstVec = ConstantVector::get(Splat);
  ReturnInst::Create(ReturnBlockParent->getContext(), ConstVec,
                     Return->getParent());

  Return->eraseFromParent();
}

bool SIMDFunctionCloning::isSimpleFunction(Function *F, ReturnInst *Return)
{
  // For really simple functions, there is no need to go through the process
  // of inserting a loop.

  // Case 1: void foo(void)
  // 
  // No need to insert a loop for this case since it's basically a no-op. Just
  // clone the function and return. It's possible that we could have some code
  // inside of a vector function that modifies global memory. Let that case go
  // through.
  if (Return && F->getReturnType()->isVoidTy() && F->arg_empty()) {
    return true;
  }

  // Case 2: only instruction is 'ret'
  //
  // If this is just a simple return and no other instructions, then just
  // broadcast the scalar return value to a vector alloca and return it.
  if (Return && !F->getReturnType()->isVoidTy()) {
    createBroadcastReturn(Return);
    return true;
  }

  return false;
}

bool SIMDFunctionCloning::runOnModule(Module &M) {

  DEBUG(dbgs() << "\nExecuting SIMD Function Cloning ...\n\n");

  // No vector functions found within this module, so the LLVM IR is not
  // modified.
  if (!getFunctionAttributeMetadata(M)) {
    return false;
  }

  // Iterate over the functions in the module and clone them if they appeared in
  // the module's simd function metadata. The cloned functions are then injected
  // with the scalar loop.

  for (unsigned I = 0; I < FuncsToClone.size(); ++I) {

    // Clone the original function.
    Function *F = FuncsToClone[I];
    DEBUG(dbgs() << "Before SIMD Function Cloning\n");
    F->dump();
    Function *Clone = CloneFunction(F);
    Function::iterator EntryBlock = Clone->begin();
    BasicBlock::iterator FirstInst = EntryBlock->begin();
    ReturnInst *Return = dyn_cast<ReturnInst>(FirstInst);

    if (isSimpleFunction(F, Return)) continue;

    BasicBlock *LoopBlock = splitEntryIntoLoop(Clone, EntryBlock);
    BasicBlock *ReturnBlock = splitLoopIntoReturn(Clone, LoopBlock);
    BasicBlock *LoopExitBlock = createLoopExit(Clone, ReturnBlock);
    PHINode *Phi = createPhiAndBackedgeForLoop(Clone, EntryBlock,
                                               LoopBlock, LoopExitBlock,
                                               ReturnBlock);

    // At this point, we've gathered some parameter information and have
    // restructured the function into an entry block, a set of blocks
    // forming the loop, a loop exit block, and a return block. Now,
    // we can go through and update instructions since we know what
    // is part of the loop.

    // We don't want to add alloca instructions as we're iterating through
    // the list of them in the entry block. So, collect them and then
    // add them to the entry block. This maps the old alloca to the new
    // one.

    std::map<AllocaInst*, Instruction*> AllocaMap;

    // Create a new vector alloca instruction for all vector parameters and
    // return. For parameters, replace the initial store to the old alloca
    // with the vector one. Users of the old alloca within the loop will be
    // replaced with a gep using this address along with the proper loop
    // index.

    Instruction *ExpandedReturn =
        expandVectorParametersAndReturn(Clone, EntryBlock, LoopBlock,
                                        ReturnBlock, AllocaMap);

    updateScalarMemRefsWithVector(Clone, F, EntryBlock, ReturnBlock, Phi,
                                  AllocaMap);

    // Update any linear variables with the appropriate stride. This function
    // will insert a mul/add sequence just after the load of the parameter.
    updateLinearReferences(Clone, F, Phi);

    // Remove the old scalar instructions associated with the return and
    // replace with packing instructions.
    updateReturnBlockInstructions(Clone, ReturnBlock, ExpandedReturn);

    // Wipe out all of the old scalar alloca instructions that have been
    // replaced with vector ones. This must be done after updating the
    // ReturnBlock because if instructions are removed in ReturnBlock, the
    // original scalar alloca references can be users of those instructions,
    // and LLVM will complain if defs are removed before uses.
    removeScalarMemRefs(AllocaMap);
    AllocaMap.clear();

    // Insert the basic blocks that mark the beginning/end of the SIMD loop.
    insertDirectiveIntrinsics(M, Clone, F, EntryBlock, LoopExitBlock,
                              ReturnBlock);

  } // End of cloning and loop creation

  return true; // LLVM IR has been modified
}

ModulePass *llvm::createSIMDFunctionCloningPass() {
  return new llvm::vpo::SIMDFunctionCloning();
}

using namespace llvm::vpo;

char SIMDFunctionCloning::ID = 0;

static const char lv_name[] = "SIMDFunctionCloning";
INITIALIZE_PASS_BEGIN(SIMDFunctionCloning, SV_NAME, lv_name,
                      false /* modifies CFG */, false /* transform pass */)
INITIALIZE_PASS_END(SIMDFunctionCloning, SV_NAME, lv_name,
                    false /* modififies CFG */, false /* transform pass */)
