/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __VECTORIZER_UTILS_H__
#define __VECTORIZER_UTILS_H__

#include "llvm/Instructions.h"
#include "Logger.h"


using namespace llvm;


namespace intel {

class VectorizerUtils {

public:

  /// @brief Follow thru a function input argument, until finding the root
  //   (where its type matches the "expected" type)
  /// @param arg The actual argument of the CALL inst
  /// @param rootType The desired (or "real") type to find
  /// @param CI The CALL instruction
  /// @return The root value if found, or NULL otherwise
  static Value *RootInputArgument(Value *arg, const Type *rootType, CallInst *CI);

  /// @brief Follow thru a function return argument, until its type matches the "expected" type
  /// @param retVal The actual returned value of the CALL inst
  /// @param rootType The desired (or "real") type to find
  /// @param CI The CALL instruction
  /// @return The "proper" retval if found, or NULL otherwise
  static Value *RootReturnValue(Value *retVal, const Type *rootType, CallInst *CI);

  /// @brief casts value into desired type if types differ
  /// @param inputVal Value to be casted
  /// @param targetType "actual" desired type according to calling convention
  /// @param insertPoint instruction to insert before
  /// @return casted value
  static Value *getCastedArgIfNeeded(Value *inputVal, const Type *targetType, Instruction *insertPoint);

  /// @brief casts call instruction value into desired type if types put casting instruction after the call 
  /// @param CI call instruction to cast
  /// @param targetType "actual" desired type according to calling convention
  /// @return casted value
  static Instruction *getCastedRetIfNeeded(CallInst *CI, const Type *targetType);

  ///@brief checks if both types are pointer to opaque types
  ///@param x type 1
  ///@param y type 2
  static bool isOpaquePtrPair(const Type *x, const Type *y);

private:

  /// @brief Generate type-conversion and place in given location
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static Instruction *BitCastValToType(Value *orig, const Type *targetType, Instruction *insertPoint);

  /// @brief Generate type-conversion and place in given location 
  ///  but on debug accpets only cases size(orig) >= size(target)
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static Instruction *ExtendValToType(Value *orig, const Type *targetType, Instruction *insertPoint);

  /// @brief Generate type-conversion and place in given location 
  ///  but on debug accpets only cases size(orig) <= size(target)
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static Instruction *TruncValToType(Value *orig, const Type *targetType, Instruction *insertPoint);

  /// @brief in case target Type is a pointer for orig allocates memory and store orig
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  /// @return  allcoated pointer
  static Instruction *convertValToPointer(Value *orig, const Type *targetType, Instruction *insertPoint);

  /// @brief check if ShuffleVector instruction is used to artificially extend a vector.
  /// @param SI shuffleVector instruction to check
  /// @param realType the "original" type which we try to root
  /// @return The instruction's input value if True, NULL otherwise
  static Value *isExtendedByShuffle(ShuffleVectorInst *SI, const Type *realType);

  /// @brief check if ShuffleVector instruction is used to artificially truncate a vector result
  /// @param SI shuffleVector instruction to check
  /// @return True if truncation was done thru ShuffleVector, False otherwise
  static bool isShuffleVectorTruncate(ShuffleVectorInst *SI);

};// VectorizerUtils

} // namespace intel


#endif //__VECTORIZER_UTILS_H__