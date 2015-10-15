/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BuiltinCallToInst.h"
#include "NameMangleAPI.h"
#include "VectorizerUtils.h"
#include "OCLPassSupport.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"

extern "C" {
  /// @brief Creates new BuiltinCallToInst pass
  void* createBuiltinCallToInstPass() {
    return new intel::BuiltinCallToInst();
  }
}

namespace intel{
  using namespace llvm;

  char BuiltinCallToInst::ID = 0;

  OCL_INITIALIZE_PASS(BuiltinCallToInst, "builtin-call-to-inst",
    "Resolve supported builtin calls, e.g. relational, shuffle with const mask, etc.", false, false)


  bool BuiltinCallToInst::runOnFunction(Function &F) {
    findBuiltinCallsToHandle(F);
    return handleSupportedBuiltinCalls();
  }

  void BuiltinCallToInst::findBuiltinCallsToHandle(Function &F) {
    m_builtinCalls.clear();

    // Run over all instructions in current function
    for (inst_iterator it = inst_begin(F), end = inst_end(F); it != end; ++it) {
      // Check if current instruction is a call instruction
      CallInst* inst = dyn_cast<CallInst>(&*it);
      if (!inst) continue;

      // Check if called function is a supported built-in
      BuiltinType builtinType = isSupportedBuiltin(inst);
      if (NOT_SUPPORTED != builtinType) {
        // Add this instruction for handling
        std::pair<CallInst*, BuiltinType> builtinCallPair =
          std::pair<CallInst*, BuiltinType>(inst, builtinType);
        m_builtinCalls.push_back(builtinCallPair);
      }
    }
  }

  bool BuiltinCallToInst::handleSupportedBuiltinCalls() {
    // Check if there are shuffle calls
    if (m_builtinCalls.empty()) return false;

    // Run over all supported built-in calls in function
    for (unsigned i = 0; i < m_builtinCalls.size(); ++i) {
      CallInst* builtinCall = m_builtinCalls[i].first;
      BuiltinType builtinType = m_builtinCalls[i].second;
      switch (builtinType) {
      case SHUFFLE1:
      case SHUFFLE2:
        handleShuffleCalls(builtinCall, builtinType);
        break;
      case REL_IS_LESS:
      case REL_IS_LESS_EQUAL:
      case REL_IS_GREATER:
      case REL_IS_GREATER_EQUAL:
      case REL_IS_EQUAL:
      case REL_IS_NOT_EQUAL:
        handleRelationalCalls(builtinCall, builtinType);
        break;
      default:
        assert(false && "Need to handle new supported built-in");
      }
    }
    return true;
  }

  void BuiltinCallToInst::handleShuffleCalls(CallInst* shuffleCall, BuiltinType shuffleType) {
    // If the function returns by pointer, shift all arguments by one
    // Note that in the mangled name, the arguments are not shifted
    const unsigned int argStart = (shuffleCall->getType()->isVoidTy()) ? 1 : 0;
    Value* retVal = shuffleCall;
    if (shuffleCall->getType()->isVoidTy()) {
      Value* retPtr = shuffleCall->getArgOperand(0);
      // If it's not a pointer, don't try to handle it
      if (!isa<PointerType>(retPtr->getType()))
        return;

      Type* desiredType = (cast<PointerType>(retPtr->getType()))->getElementType();

      retVal = VectorizerUtils::RootReturnValue(retPtr, desiredType, shuffleCall);
      if (!retVal)
        return;
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
      assert(false && "Shuffle function of unknown type");
    }

    if (!firstVec || !secondVec || !mask || !isa<Constant>(mask)) {
      // Failed to generate valid params to shuffle, do not optimize this shufle call!
      // Or mask is not of type const.
      return;
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
      return;
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

  void BuiltinCallToInst::handleRelationalCalls(CallInst* relationalCall, BuiltinType relationalType) {
    Value *operand1 = relationalCall->getOperand(0);
    Value *operand2 = relationalCall->getOperand(1);
    assert((operand1->getType() == operand2->getType()) &&
      operand1->getType()->isFPOrFPVectorTy() &&
      "Relational built-ins assumed to take primitive types");
    CmpInst::Predicate cmpOpcode = CmpInst::BAD_ICMP_PREDICATE;
    switch (relationalType) {
    case REL_IS_LESS:
      cmpOpcode = CmpInst::FCMP_OLT;
      break;
    case REL_IS_LESS_EQUAL:
      cmpOpcode =CmpInst::FCMP_OLE;
      break;
    case REL_IS_GREATER:
      cmpOpcode = CmpInst::FCMP_OGT;
      break;
    case REL_IS_GREATER_EQUAL:
      cmpOpcode = CmpInst::FCMP_OGE;
      break;
    case REL_IS_EQUAL:
      cmpOpcode = CmpInst::FCMP_OEQ;
      break;
    case REL_IS_NOT_EQUAL:
      cmpOpcode = CmpInst::FCMP_UNE;
      break;
    default:
      assert(false && "Unhandled relational built-in type");
    }
    Instruction* newRelationalInst = new FCmpInst(relationalCall, cmpOpcode, operand1, operand2);
    Type *retType = relationalCall->getType();
    if (retType->isVectorTy()) {
      newRelationalInst =  new SExtInst(newRelationalInst, retType, "", relationalCall);
    } else {
      newRelationalInst =  new ZExtInst(newRelationalInst, retType, "", relationalCall);
    }

    relationalCall->replaceAllUsesWith(newRelationalInst);
    // Remove origin relational call
    relationalCall->eraseFromParent();
  }

  BuiltinCallToInst::BuiltinType BuiltinCallToInst::isSupportedBuiltin(CallInst* pCall) {
    BuiltinType builtinType = NOT_SUPPORTED;
    // In case of indirect function call
    if (!pCall->getCalledFunction()) return builtinType;

    // Get function name
    std::string calledFuncName = pCall->getCalledFunction()->getName().str();
    std::string strippedName;
    if (!isMangledName(calledFuncName.c_str()))
      return builtinType;
    strippedName = stripName(calledFuncName.c_str());

    // Check if its a supported built-in function
    if (strippedName == "shuffle" || strippedName == "__ocl_helper_shuffle" ) {
      builtinType = SHUFFLE1;
    } else if (strippedName == "shuffle2" || strippedName == "__ocl_helper_shuffle2" ) {
      builtinType = SHUFFLE2;
    } else if (strippedName == "isless") {
      builtinType = REL_IS_LESS;
    } else if (strippedName == "islessequal") {
      builtinType = REL_IS_LESS_EQUAL;
    } else if (strippedName == "isgreater") {
      builtinType = REL_IS_GREATER;
    } else if (strippedName == "isgreaterequal") {
      builtinType = REL_IS_GREATER_EQUAL;
    } else if (strippedName == "isequal") {
      builtinType = REL_IS_EQUAL;
    } else if (strippedName == "isnotequal") {
      builtinType = REL_IS_NOT_EQUAL;
    }

    return builtinType;
  }

} // namespace intel


