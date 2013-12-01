/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "ShuffleCallToInst.h"
#include "NameMangleAPI.h"
#include "VectorizerUtils.h"
#include "OCLPassSupport.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/IR/Constants.h"

extern "C" {
  /// @brief Creates new ShuffleCallToInst pass
  void* createShuffleCallToInstPass() {
    return new intel::ShuffleCallToInst();
  }
}

namespace intel{
  using namespace llvm;

  /// @brief Pass identification, replacement for typeid
  char ShuffleCallToInst::ID = 0;

  OCL_INITIALIZE_PASS(ShuffleCallToInst, "shuffle-call-to-inst", "Replace calls to shuffle functions that has const mask with LLVM shuffle instruction", false, false)

  /// @brief  LLVM Function pass entry
  /// @param  F  Function to transform
  /// @return true if changed
  bool ShuffleCallToInst::runOnFunction(Function &F) {
    findShuffleCalls(F);
    return handleShuffleCalls();
  }

  /// @brief Find all shuffle calls in current function
  void ShuffleCallToInst::findShuffleCalls(Function &F) {
    m_shuffleCalls.clear();

    // Run over all instructions in current function
    for (inst_iterator it = inst_begin(F), end = inst_end(F); it != end; ++it) {
      // Check if current instruction is a call instruction
      CallInst* inst = dyn_cast<CallInst>(&*it);
      if (!inst) continue;

      // Check if called function is a shuffle function and the mask is constant
      ShuffleType shuffleType = isConstShuffle(inst);
      std::pair<CallInst*, ShuffleType> shuffleCallPair;
      switch (shuffleType) {
      case SHUFFLE1:
      case SHUFFLE2:
        // Add this instruction for handling
        shuffleCallPair = std::pair<CallInst*, ShuffleType>(inst, shuffleType);
        m_shuffleCalls.push_back(shuffleCallPair);
        break;
      default:
        continue;
      }
    }
  }


  /// @brief Handle all shuffle calls in current function
  bool ShuffleCallToInst::handleShuffleCalls() {
    // Check if there are shuffle calls
    if (m_shuffleCalls.empty()) return false;

    // Run over all shuffle calls in function
    for (unsigned i = 0; i < m_shuffleCalls.size(); ++i) {
      CallInst* shuffleCall = m_shuffleCalls[i].first;
      ShuffleType shuffleType = m_shuffleCalls[i].second;

      // If the function returns by pointer, shift all arguments by one
      // Note that in the mangled name, the arguments are not shifted
      const unsigned int argStart = (shuffleCall->getType()->isVoidTy()) ? 1 : 0;
      Value* retVal = shuffleCall;
      if (shuffleCall->getType()->isVoidTy()) {
        Value* retPtr = shuffleCall->getArgOperand(0);
        // If it's not a pointer, don't try to handle it
        if (!isa<PointerType>(retPtr->getType()))
          continue;
        
        Type* desiredType = (cast<PointerType>(retPtr->getType()))->getElementType();
        
        retVal = VectorizerUtils::RootReturnValue(retPtr, desiredType, shuffleCall);
        if (!retVal)
          continue;
      }
      
      // Get function operands, depending on shuffle type
      Value *firstVec = NULL;
      Value *secondVec = NULL;
      Value *mask = NULL;
      switch (shuffleType) {
      case SHUFFLE2:
        // First vector operand
        firstVec = VectorizerUtils::RootInputArgumentBySignature(shuffleCall->getArgOperand(argStart + SHUFFLE2_VEC1_POS), (int)SHUFFLE2_VEC1_POS, shuffleCall);
        // Second vector operand
        secondVec = VectorizerUtils::RootInputArgumentBySignature(shuffleCall->getArgOperand(argStart + SHUFFLE2_VEC2_POS), (int)SHUFFLE2_VEC2_POS, shuffleCall);
        // Mask operand
        // Sometimes rooting the mask results in taking an int argument and creating 
        // a bitcast, which is useless since ShuffleVector requires an explicit vector constant.
        // TODO: If the mask is not a vector, do the conversion manually. VERY manually.
        // (get the actual constant as an APInt, and create the appropriate vector by hand)
        mask = VectorizerUtils::RootInputArgumentBySignature(shuffleCall->getArgOperand(argStart + SHUFFLE2_MASK_POS), (int)SHUFFLE2_MASK_POS, shuffleCall);
        break;

      case SHUFFLE1:
        // First vector operand
        firstVec = VectorizerUtils::RootInputArgumentBySignature(shuffleCall->getArgOperand(argStart + SHUFFLE_VEC1_POS), (int)SHUFFLE_VEC1_POS, shuffleCall);
        if (firstVec) {
        // Second vector operand: undef vector with type of firstVec
          secondVec = UndefValue::get(firstVec->getType());
        }
        // Mask operand
        // Don't root, same as above
        mask = VectorizerUtils::RootInputArgumentBySignature(shuffleCall->getArgOperand(argStart + SHUFFLE_MASK_POS), (int)SHUFFLE_MASK_POS, shuffleCall);
        break;

      default:
        assert(0 && "Shuffle function of unknown type.");
      }

      if (!firstVec || !secondVec || !mask || !isa<Constant>(mask)) {
        // Failed to generate valid params to shuffle, do not optimize this shufle call!
        // Or mask is not of type const.
        continue;
      }

      assert(isa<VectorType>(mask->getType()) && "mask is not vector type");
    
      // Convert mask type to vector of i32, shuffflevector mask scalar size is always 32
      Constant* newMask = NULL;
      unsigned int maskVecSize = cast<VectorType>(mask->getType())->getNumElements();
      Type *maskType = VectorType::get(Type::getInt32Ty(shuffleCall->getContext()), maskVecSize);

      // We previously searched for shuffle calls with constant mask only
      // So we can assume mask is constant here
      // If mask scalar size is not 32 then Zext or Trunc the mask to get to 32
      if (mask->getType()->getScalarSizeInBits() < maskType->getScalarSizeInBits()) {
        newMask = ConstantExpr::getZExt(cast<Constant>(mask), maskType);
      }
      else if (mask->getType()->getScalarSizeInBits() > maskType->getScalarSizeInBits()) {
        newMask = ConstantExpr::getTrunc(cast<Constant>(mask), maskType);
      }
      else {
        newMask = cast<Constant>(mask);
      }

      // Create the new shufflevector instruction
      // Unfortunately, due to the rooting issue discussed above, newMask may be a ConstantExpr,
      // which is not supported by shuffles.
      // Make a more generic check here for safety, but assert...
      if (!ShuffleVectorInst::isValidOperands(firstVec, secondVec, newMask)) {
        assert(isa<ConstantExpr>(newMask) && "Got invalid shuffle paramters not due to mask being a constant expressions");
        continue;
      }
        
      Instruction* newShuffleInst = new ShuffleVectorInst(firstVec, secondVec, newMask, "newShuffle", shuffleCall);
      
      // Due to an optimization in clang, the return type of the original call may be a longer vector
      // than what the shuffle produces.
      if (newShuffleInst->getType() != retVal->getType()) {
        newShuffleInst = VectorizerUtils::ExtendValToType(newShuffleInst, retVal->getType(), shuffleCall);
      }
    
      retVal->replaceAllUsesWith(newShuffleInst);
      // Remove origin shuffle call
      shuffleCall->eraseFromParent();
    }
    return true;
  }


  /// @brief Check if a given called function is shuffle
  /// @return SHUFFLE1 or SHUFFLE2 in case of a shuffle function
  ///         else NOT_SHUFFLE
  ShuffleCallToInst::ShuffleType ShuffleCallToInst::isConstShuffle(CallInst* pCall) {
    // In case of indirect function call
    if (!pCall->getCalledFunction()) return NOT_SHUFFLE;
  
    // Get function name
    std::string calledFuncName = pCall->getCalledFunction()->getName().str();
    std::string strippedName;
    if (!isMangledName(calledFuncName.c_str()))
      return NOT_SHUFFLE;
    strippedName = stripName(calledFuncName.c_str());

    // Check if its shuffle function and mask is constant
    if (strippedName == "shuffle" || strippedName == "__ocl_helper_shuffle" ) {
        return SHUFFLE1;
    } else if (strippedName == "shuffle2" || strippedName == "__ocl_helper_shuffle2" ) {
        return SHUFFLE2;
    }
    return NOT_SHUFFLE;
  }

} // namespace intel


